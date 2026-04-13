/*

ゲームグラフィックス特論用補助プログラム GLFW3 版

Copyright (c) 2011-2025 Kohe Tokoi. All Rights Reserved.

Permission is hereby granted, free of charge,  to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction,  including without limitation the rights
to use, copy,  modify, merge,  publish, distribute,  sublicense,  and/or sell
copies or substantial portions of the Software.

The above  copyright notice  and this permission notice  shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING  BUT  NOT LIMITED  TO THE WARRANTIES  OF MERCHANTABILITY,
FITNESS  FOR  A PARTICULAR PURPOSE  AND NONINFRINGEMENT.  IN  NO EVENT  SHALL
KOHE TOKOI  BE LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER LIABILITY,  WHETHER IN
AN ACTION  OF CONTRACT,  TORT  OR  OTHERWISE,  ARISING  FROM,  OUT OF  OR  IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

///
/// ゲームグラフィックス特論宿題アプリケーションクラスの実装.
///
/// @file
/// @author Kohe Tokoi
/// @date July 27, 2025
///
#include "GgApp.h"

//
// GLFW のエラー表示
//
static void glfwErrorCallback(int error, const char* description)
{
#if defined(__aarch64__)
  if (error == 65544) return;
#endif
  throw std::runtime_error(description);
}

//
// GgApp クラスのコンストラクタ
//
GgApp::GgApp(int major, int minor)
{
  // GLFW のエラー処理関数を登録する
  glfwSetErrorCallback(glfwErrorCallback);

  // GLFW を初期化する
  if (glfwInit() == GL_FALSE) throw std::runtime_error("Can't initialize GLFW");

  // OpenGL の major 番号が指定されていれば
  if (major > 0)
  {
    // OpenGL のバージョンを指定する
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);

#if defined(GL_GLES_PROTOTYPES)
    // OpenGL ES 3 のコンテキストを指定する
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
#else
    // OpenGL Version 3.2 以降なら
    if (major * 10 + minor >= 32)
    {
      // Core Profile を選択する (macOS の都合)
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }
#endif
  }

#if defined(GG_USE_OPENXR)

//
// コンストラクタ
//
GgApp::OpenXR::OpenXR() :
  instance{ XR_NULL_HANDLE },
  systemId{ XR_NULL_SYSTEM_ID },
  session{ XR_NULL_HANDLE },
  sessionState{ XR_SESSION_STATE_UNKNOWN },
  appSpace{ XR_NULL_HANDLE },
  openxrFbo{ 0, 0 },
  openxrDepth{ 0, 0 },
  isSessionRunning{ false },
  window{ nullptr }
{
  currentImageIndex[0] = 0;
  currentImageIndex[1] = 0;
}

//
// OpenXR のセッションを作成する
//
GgApp::OpenXR& GgApp::OpenXR::initialize(const Window& window)
{
  static OpenXR openxr;

  if (openxr.instance != XR_NULL_HANDLE) return openxr;

  openxr.window = &window;

  // XrInstance の作成
  XrInstanceCreateInfo createInfo{ XR_TYPE_INSTANCE_CREATE_INFO };
  strcpy(createInfo.applicationInfo.applicationName, "ggsample01");
  createInfo.applicationInfo.applicationVersion = 1;
  strcpy(createInfo.applicationInfo.engineName, "GgApp");
  createInfo.applicationInfo.engineVersion = 1;
  createInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
  
  const char* extensions[] = {
    XR_KHR_OPENGL_ENABLE_EXTENSION_NAME
  };
  createInfo.enabledExtensionCount = 1;
  createInfo.enabledExtensionNames = extensions;

  if (XR_FAILED(xrCreateInstance(&createInfo, &openxr.instance)))
    throw std::runtime_error("Can't create OpenXR instance");

  // Get System Id
  XrSystemGetInfo systemInfo{ XR_TYPE_SYSTEM_GET_INFO };
  systemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
  if (XR_FAILED(xrGetSystem(openxr.instance, &systemInfo, &openxr.systemId)))
    throw std::runtime_error("Can't get OpenXR system");

  // Require OpenGL graphics binding
  PFN_xrGetOpenGLGraphicsRequirementsKHR pfnGetOpenGLGraphicsRequirementsKHR = nullptr;
  xrGetInstanceProcAddr(openxr.instance, "xrGetOpenGLGraphicsRequirementsKHR", (PFN_xrVoidFunction*)&pfnGetOpenGLGraphicsRequirementsKHR);
  
  XrGraphicsRequirementsOpenGLKHR graphicsRequirements{ XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR };
  pfnGetOpenGLGraphicsRequirementsKHR(openxr.instance, openxr.systemId, &graphicsRequirements);

#if defined(_MSC_VER)
  XrGraphicsBindingOpenGLWin32KHR graphicsBinding{ XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR };
  graphicsBinding.hDC = wglGetCurrentDC();
  graphicsBinding.hGLRC = wglGetCurrentContext();
#else
  XrGraphicsBindingOpenGLXlibKHR graphicsBinding{ XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR };
  graphicsBinding.xDisplay = glXGetCurrentDisplay();
  graphicsBinding.visualid = 0;
  graphicsBinding.glxFBConfig = nullptr;
  graphicsBinding.glxDrawable = glXGetCurrentDrawable();
  graphicsBinding.glxContext = glXGetCurrentContext();
#endif

  // Create session
  XrSessionCreateInfo sessionCreateInfo{ XR_TYPE_SESSION_CREATE_INFO };
  sessionCreateInfo.next = &graphicsBinding;
  sessionCreateInfo.systemId = openxr.systemId;
  if (XR_FAILED(xrCreateSession(openxr.instance, &sessionCreateInfo, &openxr.session)))
    throw std::runtime_error("Can't create OpenXR session");

  // Create Reference Space
  XrReferenceSpaceCreateInfo spaceCreateInfo{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
  spaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
  spaceCreateInfo.poseInReferenceSpace.orientation.w = 1.0f;
  if (XR_FAILED(xrCreateReferenceSpace(openxr.session, &spaceCreateInfo, &openxr.appSpace)))
    throw std::runtime_error("Can't create OpenXR reference space");

  // Enumerate views and create swapchains
  uint32_t viewCount = 0;
  xrEnumerateViewConfigurationViews(openxr.instance, openxr.systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &viewCount, nullptr);
  openxr.views.resize(viewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
  xrEnumerateViewConfigurationViews(openxr.instance, openxr.systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, viewCount, &viewCount, openxr.views.data());

  openxr.viewStates.resize(viewCount, { XR_TYPE_VIEW });

  glGenFramebuffers(2, openxr.openxrFbo);
  glGenTextures(2, openxr.openxrDepth);

  for (uint32_t i = 0; i < viewCount; ++i) {
    XrSwapchainCreateInfo swapchainCreateInfo{ XR_TYPE_SWAPCHAIN_CREATE_INFO };
    swapchainCreateInfo.arraySize = 1;
    swapchainCreateInfo.format = GL_SRGB8_ALPHA8;
    swapchainCreateInfo.width = openxr.views[i].recommendedImageRectWidth;
    swapchainCreateInfo.height = openxr.views[i].recommendedImageRectHeight;
    swapchainCreateInfo.mipCount = 1;
    swapchainCreateInfo.faceCount = 1;
    swapchainCreateInfo.sampleCount = 1;
    swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;

    XrSwapchain swapchain;
    xrCreateSwapchain(openxr.session, &swapchainCreateInfo, &swapchain);
    openxr.swapchains.push_back(swapchain);

    uint32_t imageCount;
    xrEnumerateSwapchainImages(swapchain, 0, &imageCount, nullptr);
    std::vector<XrSwapchainImageOpenGLKHR> images(imageCount, { XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR });
    xrEnumerateSwapchainImages(swapchain, imageCount, &imageCount, (XrSwapchainImageBaseHeader*)images.data());
    openxr.swapchainImages.push_back(std::move(images));

    glBindTexture(GL_TEXTURE_2D, openxr.openxrDepth[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, swapchainCreateInfo.width, swapchainCreateInfo.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  }

  // OpenXR にレンダリングするときは sRGB カラースペースを使う
  glEnable(GL_FRAMEBUFFER_SRGB);
  glDrawBuffer(GL_FRONT);
  glfwSwapInterval(0);

  return openxr;
}

//
// OpenXR のセッションを破棄する
//
void GgApp::OpenXR::terminate()
{
  if (session != XR_NULL_HANDLE) {
    for (auto swapchain : swapchains) xrDestroySwapchain(swapchain);
    xrDestroySpace(appSpace);
    xrDestroySession(session);
    session = XR_NULL_HANDLE;
  }
  if (instance != XR_NULL_HANDLE) {
    xrDestroyInstance(instance);
    instance = XR_NULL_HANDLE;
  }
  glDeleteFramebuffers(2, openxrFbo);
  glDeleteTextures(2, openxrDepth);

  glDisable(GL_FRAMEBUFFER_SRGB);
  glDrawBuffer(GL_BACK);
  glfwSwapInterval(1);
}

//
// OpenXR による描画開始
//
bool GgApp::OpenXR::begin()
{
  XrEventDataBuffer eventData{ XR_TYPE_EVENT_DATA_BUFFER };
  while (xrPollEvent(instance, &eventData) == XR_SUCCESS) {
    if (eventData.type == XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED) {
      auto* stateChanged = reinterpret_cast<XrEventDataSessionStateChanged*>(&eventData);
      sessionState = stateChanged->state;
      if (sessionState == XR_SESSION_STATE_READY) {
        XrSessionBeginInfo beginInfo{ XR_TYPE_SESSION_BEGIN_INFO };
        beginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
        xrBeginSession(session, &beginInfo);
        isSessionRunning = true;
      } else if (sessionState == XR_SESSION_STATE_STOPPING) {
        xrEndSession(session);
        isSessionRunning = false;
      } else if (sessionState == XR_SESSION_STATE_EXITING || sessionState == XR_SESSION_STATE_LOSS_PENDING) {
        window->setClose(GLFW_TRUE);
      }
    }
    eventData.type = XR_TYPE_EVENT_DATA_BUFFER;
  }

  if (!isSessionRunning || sessionState != XR_SESSION_STATE_FOCUSED) return false;

  XrFrameWaitInfo waitInfo{ XR_TYPE_FRAME_WAIT_INFO };
  frameState = { XR_TYPE_FRAME_STATE };
  xrWaitFrame(session, &waitInfo, &frameState);

  XrFrameBeginInfo beginInfo{ XR_TYPE_FRAME_BEGIN_INFO };
  xrBeginFrame(session, &beginInfo);

  XrViewLocateInfo viewLocateInfo{ XR_TYPE_VIEW_LOCATE_INFO };
  viewLocateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
  viewLocateInfo.displayTime = frameState.predictedDisplayTime;
  viewLocateInfo.space = appSpace;

  XrViewState viewState{ XR_TYPE_VIEW_STATE };
  uint32_t viewCount;
  xrLocateViews(session, &viewLocateInfo, &viewState, (uint32_t)viewStates.size(), &viewCount, viewStates.data());

  return true;
}

//
// OpenXR の描画する目の指定
//
void GgApp::OpenXR::select(int eye, GLfloat* screen, GLfloat* position, GLfloat* orientation)
{
  XrSwapchainImageAcquireInfo acquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
  xrAcquireSwapchainImage(swapchains[eye], &acquireInfo, &currentImageIndex[eye]);

  XrSwapchainImageWaitInfo waitInfo{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
  waitInfo.timeout = XR_INFINITE_DURATION;
  xrWaitSwapchainImage(swapchains[eye], &waitInfo);

  GLuint texId = swapchainImages[eye][currentImageIndex[eye]].image;
  glBindFramebuffer(GL_FRAMEBUFFER, openxrFbo[eye]);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, openxrDepth[eye], 0);

  glViewport(0, 0, views[eye].recommendedImageRectWidth, views[eye].recommendedImageRectHeight);

  const auto& pose = viewStates[eye].pose;
  const auto& fov = viewStates[eye].fov;

  screen[0] = tanf(fov.angleLeft);
  screen[1] = tanf(fov.angleRight);
  screen[2] = tanf(fov.angleDown);
  screen[3] = tanf(fov.angleUp);

  position[0] = pose.position.x;
  position[1] = pose.position.y;
  position[2] = pose.position.z;

  orientation[0] = pose.orientation.x;
  orientation[1] = pose.orientation.y;
  orientation[2] = pose.orientation.z;
  orientation[3] = pose.orientation.w;
}

//
// Time Warp 処理に使う投影変換行列の成分の設定 (ダミー)
//
void GgApp::OpenXR::timewarp(const GgMatrix& projection)
{
}

//
// 図形の描画を完了する
//
void GgApp::OpenXR::commit(int eye)
{
  glBindFramebuffer(GL_FRAMEBUFFER, openxrFbo[eye]);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);

  XrSwapchainImageReleaseInfo releaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
  xrReleaseSwapchainImage(swapchains[eye], &releaseInfo);
}

//
// フレームを転送する
//
bool GgApp::OpenXR::submit(bool mirror)
{
  XrCompositionLayerProjectionView projectionViews[2];
  for (int eye = 0; eye < 2; ++eye) {
    projectionViews[eye].type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
    projectionViews[eye].next = nullptr;
    projectionViews[eye].pose = viewStates[eye].pose;
    projectionViews[eye].fov = viewStates[eye].fov;
    projectionViews[eye].subImage.swapchain = swapchains[eye];
    projectionViews[eye].subImage.imageRect.offset = {0, 0};
    projectionViews[eye].subImage.imageRect.extent = {(int32_t)views[eye].recommendedImageRectWidth, (int32_t)views[eye].recommendedImageRectHeight};
    projectionViews[eye].subImage.imageArrayIndex = 0;
  }

  XrCompositionLayerProjection layer{ XR_TYPE_COMPOSITION_LAYER_PROJECTION };
  layer.space = appSpace;
  layer.viewCount = 2;
  layer.views = projectionViews;

  const XrCompositionLayerBaseHeader* layers = reinterpret_cast<XrCompositionLayerBaseHeader*>(&layer);

  XrFrameEndInfo endInfo{ XR_TYPE_FRAME_END_INFO };
  endInfo.displayTime = frameState.predictedDisplayTime;
  endInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
  endInfo.layerCount = frameState.shouldRender ? 1 : 0;
  endInfo.layers = &layers;

  xrEndFrame(session, &endInfo);

  if (mirror) {
    GLsizei size[2];
    window->getSize(size);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, openxrFbo[0]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, views[0].recommendedImageRectWidth, views[0].recommendedImageRectHeight,
                      0, 0, size[0], size[1], GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glFlush();
  }

  return true;
}
#endif

#if defined(IMGUI_VERSION)
  // ImGui のバージョンをチェックする
  IMGUI_CHECKVERSION();

  // ImGui のコンテキストを作成する
  ImGui::CreateContext();
#endif
}

//
// デストラクタ
//
GgApp::~GgApp()
{
#if defined(IMGUI_VERSION)
  // Shutdown Platform/Renderer bindings
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
#endif

  // プログラム終了時に GLFW を終了する
  glfwTerminate();
}

//
// マウスや矢印キーによる平行移動量を初期化する
//
void GgApp::Window::HumanInterface::resetTranslation()
{
  // 平行移動量を初期化する
  for (auto& t : translation)
  {
    std::fill(t.begin(), t.end(), GgVector{ 0.0f, 0.0f, 0.0f, 1.0f });
  }

  // 矢印キーの設定値を初期化する
  std::fill(arrow.begin(), arrow.end(), std::array<int, 2>{ 0, 0 });

  // マウスホイールの回転量を初期化する
  std::fill(wheel.begin(), wheel.end(), 0.0f);
}

//
// 平行移動量と回転量を更新する (X, Y のみ, Z は wheel() で計算する)
//
void GgApp::Window::HumanInterface::calcTranslation(int button, const std::array<GLfloat, 3>& velocity)
{
  // マウスの相対変位
  assert(button >= GLFW_MOUSE_BUTTON_1 && button < GLFW_MOUSE_BUTTON_1 + GG_BUTTON_COUNT);
  const auto dx{ (mouse[0] - rotation[button].getStart(0)) * rotation[button].getScale(0) };
  const auto dy{ (rotation[button].getStart(1) - mouse[1]) * rotation[button].getScale(1) };

  // 平行移動量
  auto& t{ translation[button] };

  // 平行移動量の更新
  t[1][0] = dx * velocity[0] + t[0][0];
  t[1][1] = dy * velocity[1] + t[0][1];

  // 回転量の更新
  rotation[button].motion(mouse[0], mouse[1]);
}

//
// ウィンドウのサイズ変更時の処理
//
void GgApp::Window::resize(GLFWwindow* window, int width, int height)
{
  // このインスタンスの this ポインタを得る
  auto* const instance{ static_cast<Window*>(glfwGetWindowUserPointer(window)) };

  if (instance)
  {
    // ウィンドウのサイズを保存する
    instance->size[0] = width;
    instance->size[1] = height;

    // トラックボール処理の範囲を設定する
    for (auto& current_if : instance->interfaceData)
    {
      for (auto& t : current_if.rotation)
      {
        t.region(width, height);
      }
    }

    // ビューポートを更新する
    instance->updateViewport();

    // ユーザー定義のコールバック関数の呼び出し
    if (instance->resizeFunc) (*instance->resizeFunc)(instance, width, height);
  }
}

//
// キーボードをタイプした時の処理
//
void GgApp::Window::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
#if defined(IMGUI_VERSION)
  // ImGui がキーボードを使うときはキーボードの処理を行わない
  if (ImGui::GetIO().WantCaptureKeyboard) return;
#endif

  // このインスタンスの this ポインタを得る
  auto* const instance{ static_cast<Window*>(glfwGetWindowUserPointer(window)) };

  if (instance && action)
  {
    // ユーザー定義のコールバック関数の呼び出し
    if (instance->keyboardFunc) (*instance->keyboardFunc)(instance, key, scancode, action, mods);

    // 対象のユーザインタフェース
    auto& current_if{ instance->interfaceData[instance->interfaceNo] };

    switch (key)
    {
    case GLFW_KEY_HOME:

      // トラックボールを初期化する
      instance->resetRotation();
      [[fallthrough]];

    case GLFW_KEY_END:

      // 平行移動量を初期化する
      instance->resetTranslation();
      break;

    case GLFW_KEY_UP:

      if (mods & GLFW_MOD_SHIFT)
        current_if.arrow[1][1]++;
      else if (mods & GLFW_MOD_CONTROL)
        current_if.arrow[2][1]++;
      else if (mods & GLFW_MOD_ALT)
        current_if.arrow[3][1]++;
      else
        current_if.arrow[0][1]++;
      break;

    case GLFW_KEY_DOWN:

      if (mods & GLFW_MOD_SHIFT)
        current_if.arrow[1][1]--;
      else if (mods & GLFW_MOD_CONTROL)
        current_if.arrow[2][1]--;
      else if (mods & GLFW_MOD_ALT)
        current_if.arrow[3][1]--;
      else
        current_if.arrow[0][1]--;
      break;

    case GLFW_KEY_RIGHT:

      if (mods & GLFW_MOD_SHIFT)
        current_if.arrow[1][0]++;
      else if (mods & GLFW_MOD_CONTROL)
        current_if.arrow[2][0]++;
      else if (mods & GLFW_MOD_ALT)
        current_if.arrow[3][0]++;
      else
        current_if.arrow[0][0]++;
      break;

    case GLFW_KEY_LEFT:

      if (mods & GLFW_MOD_SHIFT)
        current_if.arrow[1][0]--;
      else if (mods & GLFW_MOD_CONTROL)
        current_if.arrow[2][0]--;
      else if (mods & GLFW_MOD_ALT)
        current_if.arrow[3][0]--;
      else
        current_if.arrow[0][0]--;
      break;

    default:
      break;
    }

    current_if.lastKey = key;
  }
}

//
// マウスボタンを操作したときの処理
//
void GgApp::Window::mouse(GLFWwindow* window, int button, int action, int mods)
{
#if defined(IMGUI_VERSION)
  // ImGui がマウスを使うときは Window クラスのマウス位置を更新しない
  if (ImGui::GetIO().WantCaptureMouse) return;
#endif

  // このインスタンスの this ポインタを得る
  auto* const instance{ static_cast<Window*>(glfwGetWindowUserPointer(window)) };

  // マウスボタンの状態を記録する
  assert(button >= GLFW_MOUSE_BUTTON_1 && button < GLFW_MOUSE_BUTTON_1 + GG_BUTTON_COUNT);
  instance->status[button] = action != GLFW_RELEASE;

  if (instance)
  {
    // ユーザー定義のコールバック関数の呼び出し
    if (instance->mouseFunc) (*instance->mouseFunc)(instance, button, action, mods);

    // 対象のユーザインタフェース
    auto& current_if{ instance->interfaceData[instance->interfaceNo] };

    // マウスの現在位置を得る
    const auto x{ current_if.mouse[0] };
    const auto y{ current_if.mouse[1] };

    if (x < 0 || x >= instance->size[0] || y < 0 || y >= instance->size[1]) return;

    if (action)
    {
      // ドラッグ開始
      current_if.rotation[button].begin(x, y);
    }
    else
    {
      // ドラッグ終了
      current_if.translation[button][0] = current_if.translation[button][1];
      current_if.rotation[button].end(x, y);
    }
  }
}

//
// マウスホイールを操作した時の処理
//
void GgApp::Window::wheel(GLFWwindow* window, double x, double y)
{
#if defined(IMGUI_VERSION)
  // ImGui がマウスを使うときは Window クラスのマウス位置を更新しない
  if (ImGui::GetIO().WantCaptureMouse) return;
#endif

  // このインスタンスの this ポインタを得る
  auto* const instance{ static_cast<Window*>(glfwGetWindowUserPointer(window)) };

  if (instance)
  {
    // ユーザー定義のコールバック関数の呼び出し
    if (instance->wheelFunc) (*instance->wheelFunc)(instance, x, y);

    // 対象のユーザインタフェース
    auto& current_if{ instance->interfaceData[instance->interfaceNo] };

    // マウスホイールの回転量の保存
    current_if.wheel[0] += static_cast<GLfloat>(x);
    current_if.wheel[1] += static_cast<GLfloat>(y);

    // マウスによる平行移動量の z 値の更新
    const auto z{ current_if.wheel[1] * instance->velocity[2] };
    for (auto& t : current_if.translation) t[1][2] = z;
  }
}

//
// Window クラスのコンストラクタ
//
GgApp::Window::Window(const std::string& title, int width, int height, int fullscreen, GLFWwindow* share) :
  window{ nullptr },
  size{ width, height },
  fboSize{ width, height },
#if defined(IMGUI_VERSION)
  menubarHeight{ 0 },
#endif
  aspect{ 1.0f },
  velocity{ 1.0f, 1.0f, 0.1f },
  status{ false },
  interfaceNo{ 0 },
  userPointer{ nullptr },
  resizeFunc{ nullptr },
  keyboardFunc{ nullptr },
  mouseFunc{ nullptr },
  wheelFunc{ nullptr }
{
  // ディスプレイの情報
  GLFWmonitor* monitor{ nullptr };

  // フルスクリーン表示
  if (fullscreen > 0)
  {
    // 接続されているモニタの数を数える
    int mcount;
    auto** const monitors{ glfwGetMonitors(&mcount) };

    // セカンダリモニタがあればそれを使う
    if (fullscreen > mcount) fullscreen = mcount;
    monitor = monitors[fullscreen - 1];

    // モニタのモードを調べる
    const auto* mode{ glfwGetVideoMode(monitor) };

    // ウィンドウのサイズをディスプレイのサイズにする
    width = mode->width;
    height = mode->height;
  }

  // GLFW のウィンドウを作成する
  window = glfwCreateWindow(width, height, title.c_str(), monitor, share);

  // ウィンドウが作成できなければエラー
  if (!window) throw std::runtime_error("Unable to open the GLFW window.");

  // 現在のウィンドウを処理対象にする
  glfwMakeContextCurrent(window);

  // ゲームグラフィックス特論の都合による初期化を行う
  ggInit();

  // このインスタンスの this ポインタを記録しておく
  glfwSetWindowUserPointer(window, this);

  // キーボードを操作した時の処理を登録する
  glfwSetKeyCallback(window, keyboard);

  // マウスボタンを操作したときの処理を登録する
  glfwSetMouseButtonCallback(window, mouse);

  // マウスホイール操作時に呼び出す処理を登録する
  glfwSetScrollCallback(window, wheel);

  // ウィンドウのサイズ変更時に呼び出す処理を登録する
  glfwSetFramebufferSizeCallback(window, resize);

  // 垂直同期タイミングに合わせる
  glfwSwapInterval(1);

  // 実際のフレームバッファのサイズを取得する
  glfwGetFramebufferSize(window, &width, &height);

  // ビューポートと投影変換行列を初期化する
  resize(window, width, height);

#if defined(IMGUI_VERSION)
  // 最初のウィンドウを開いたとき
  static bool firstTime{ true };
  if (firstTime)
  {
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(nullptr);

    // 実行済みであることを記録する
    firstTime = false;
  }
#endif
}

//
// イベントを取得してループを継続すべきかどうか調べる
//
GgApp::Window::operator bool()
{
  // イベントを取り出す
  glfwPollEvents();

  // ウィンドウを閉じるべきなら false を返す
  if (shouldClose()) return false;

  // 対象のユーザインタフェース
  auto& current_if{ interfaceData[interfaceNo] };

#if defined(IMGUI_VERSION)
  // ImGui の新規フレームを作成する
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // ImGui の状態を取り出す
  const ImGuiIO& io{ ImGui::GetIO() };

  // ImGui がマウスを使うときは Window クラスのマウス位置を更新しない
  if (io.WantCaptureMouse) return true;

  // マウスの位置を更新する
  current_if.mouse = std::array<GLfloat, 2>{ io.MousePos.x, io.MousePos.y };
#else
  // マウスの現在位置を調べる
  double x, y;
  glfwGetCursorPos(window, &x, &y);

  // マウスの位置を更新する
  current_if.mouse = std::array<GLfloat, 2>{ static_cast<GLfloat>(x), static_cast<GLfloat>(y) };
#endif

  // マウスドラッグ
  for (int button = GLFW_MOUSE_BUTTON_1; button < GLFW_MOUSE_BUTTON_1 + GG_BUTTON_COUNT; ++button)
  {
    // マウスボタンを押していたら
    if (status[button])
    {
      // 現在位置と平行移動量を更新する
      current_if.calcTranslation(button, velocity);
    }
  }

  return true;
}

//
// カラーバッファを入れ替える
//
void GgApp::Window::swapBuffers() const
{
#if defined(IMGUI_VERSION)
  // ImGui の描画データがあればフレームをレンダリングする
  ImGui::Render();
  ImDrawData* data{ ImGui::GetDrawData() };
  if (data) ImGui_ImplOpenGL3_RenderDrawData(data);
#endif

  // エラーチェック
  ggError();

  // カラーバッファを入れ替える
  glfwSwapBuffers(window);
}

//
// ビューポートのサイズを更新する
//
void GgApp::Window::updateViewport()
{
  // フレームバッファの大きさを求める
  glfwGetFramebufferSize(window, &fboSize[0], &fboSize[1]);

#if defined(IMGUI_VERSION)
  // フレームバッファの高さからメニューバーの高さを減じる
  fboSize[1] -= menubarHeight;
#endif

  // ウィンドウの縦横比を保存する
  aspect = static_cast<GLfloat>(fboSize[0]) / static_cast<GLfloat>(fboSize[1]);

  // ビューポートを設定する
  restoreViewport();
}

#if defined(GG_USE_OPENXR)

//
// コンストラクタ
//
GgApp::OpenXR::OpenXR() :
  instance{ XR_NULL_HANDLE },
  systemId{ XR_NULL_SYSTEM_ID },
  session{ XR_NULL_HANDLE },
  sessionState{ XR_SESSION_STATE_UNKNOWN },
  appSpace{ XR_NULL_HANDLE },
  openxrFbo{ 0, 0 },
  openxrDepth{ 0, 0 },
  isSessionRunning{ false },
  window{ nullptr }
{
  currentImageIndex[0] = 0;
  currentImageIndex[1] = 0;
}

//
// OpenXR のセッションを作成する
//
GgApp::OpenXR& GgApp::OpenXR::initialize(const Window& window)
{
  static OpenXR openxr;

  if (openxr.instance != XR_NULL_HANDLE) return openxr;

  openxr.window = &window;

  // XrInstance の作成
  XrInstanceCreateInfo createInfo{ XR_TYPE_INSTANCE_CREATE_INFO };
  strcpy(createInfo.applicationInfo.applicationName, "ggsample01");
  createInfo.applicationInfo.applicationVersion = 1;
  strcpy(createInfo.applicationInfo.engineName, "GgApp");
  createInfo.applicationInfo.engineVersion = 1;
  createInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
  
  const char* extensions[] = {
    XR_KHR_OPENGL_ENABLE_EXTENSION_NAME
  };
  createInfo.enabledExtensionCount = 1;
  createInfo.enabledExtensionNames = extensions;

  if (XR_FAILED(xrCreateInstance(&createInfo, &openxr.instance)))
    throw std::runtime_error("Can't create OpenXR instance");

  // Get System Id
  XrSystemGetInfo systemInfo{ XR_TYPE_SYSTEM_GET_INFO };
  systemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
  if (XR_FAILED(xrGetSystem(openxr.instance, &systemInfo, &openxr.systemId)))
    throw std::runtime_error("Can't get OpenXR system");

  // Require OpenGL graphics binding
  PFN_xrGetOpenGLGraphicsRequirementsKHR pfnGetOpenGLGraphicsRequirementsKHR = nullptr;
  xrGetInstanceProcAddr(openxr.instance, "xrGetOpenGLGraphicsRequirementsKHR", (PFN_xrVoidFunction*)&pfnGetOpenGLGraphicsRequirementsKHR);
  
  XrGraphicsRequirementsOpenGLKHR graphicsRequirements{ XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR };
  pfnGetOpenGLGraphicsRequirementsKHR(openxr.instance, openxr.systemId, &graphicsRequirements);

#if defined(_MSC_VER)
  XrGraphicsBindingOpenGLWin32KHR graphicsBinding{ XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR };
  graphicsBinding.hDC = wglGetCurrentDC();
  graphicsBinding.hGLRC = wglGetCurrentContext();
#else
  XrGraphicsBindingOpenGLXlibKHR graphicsBinding{ XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR };
  graphicsBinding.xDisplay = glXGetCurrentDisplay();
  graphicsBinding.visualid = 0;
  graphicsBinding.glxFBConfig = nullptr;
  graphicsBinding.glxDrawable = glXGetCurrentDrawable();
  graphicsBinding.glxContext = glXGetCurrentContext();
#endif

  // Create session
  XrSessionCreateInfo sessionCreateInfo{ XR_TYPE_SESSION_CREATE_INFO };
  sessionCreateInfo.next = &graphicsBinding;
  sessionCreateInfo.systemId = openxr.systemId;
  if (XR_FAILED(xrCreateSession(openxr.instance, &sessionCreateInfo, &openxr.session)))
    throw std::runtime_error("Can't create OpenXR session");

  // Create Reference Space (STAGE for FloorLevel, fallback to LOCAL)
  XrReferenceSpaceCreateInfo spaceCreateInfo{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
  spaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE;
  spaceCreateInfo.poseInReferenceSpace.orientation.w = 1.0f;
  if (XR_FAILED(xrCreateReferenceSpace(openxr.session, &spaceCreateInfo, &openxr.appSpace))) {
    spaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
    if (XR_FAILED(xrCreateReferenceSpace(openxr.session, &spaceCreateInfo, &openxr.appSpace)))
      throw std::runtime_error("Can't create OpenXR reference space");
  }

  // Enumerate views and create swapchains
  uint32_t viewCount = 0;
  xrEnumerateViewConfigurationViews(openxr.instance, openxr.systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &viewCount, nullptr);
  openxr.views.resize(viewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
  xrEnumerateViewConfigurationViews(openxr.instance, openxr.systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, viewCount, &viewCount, openxr.views.data());

  openxr.viewStates.resize(viewCount, { XR_TYPE_VIEW });

  glGenFramebuffers(2, openxr.openxrFbo);
  glGenTextures(2, openxr.openxrDepth);

  for (uint32_t i = 0; i < viewCount; ++i) {
    XrSwapchainCreateInfo swapchainCreateInfo{ XR_TYPE_SWAPCHAIN_CREATE_INFO };
    swapchainCreateInfo.arraySize = 1;
    swapchainCreateInfo.format = GL_SRGB8_ALPHA8;
    swapchainCreateInfo.width = openxr.views[i].recommendedImageRectWidth;
    swapchainCreateInfo.height = openxr.views[i].recommendedImageRectHeight;
    swapchainCreateInfo.mipCount = 1;
    swapchainCreateInfo.faceCount = 1;
    swapchainCreateInfo.sampleCount = 1;
    swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;

    XrSwapchain swapchain;
    xrCreateSwapchain(openxr.session, &swapchainCreateInfo, &swapchain);
    openxr.swapchains.push_back(swapchain);

    uint32_t imageCount;
    xrEnumerateSwapchainImages(swapchain, 0, &imageCount, nullptr);
    std::vector<XrSwapchainImageOpenGLKHR> images(imageCount, { XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR });
    xrEnumerateSwapchainImages(swapchain, imageCount, &imageCount, (XrSwapchainImageBaseHeader*)images.data());
    openxr.swapchainImages.push_back(std::move(images));

    glBindTexture(GL_TEXTURE_2D, openxr.openxrDepth[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, swapchainCreateInfo.width, swapchainCreateInfo.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  }

  // OpenXR にレンダリングするときは sRGB カラースペースを使う
  glEnable(GL_FRAMEBUFFER_SRGB);
  glDrawBuffer(GL_FRONT);
  glfwSwapInterval(0);

  return openxr;
}

//
// OpenXR のセッションを破棄する
//
void GgApp::OpenXR::terminate()
{
  if (session != XR_NULL_HANDLE) {
    for (auto swapchain : swapchains) xrDestroySwapchain(swapchain);
    xrDestroySpace(appSpace);
    xrDestroySession(session);
    session = XR_NULL_HANDLE;
  }
  if (instance != XR_NULL_HANDLE) {
    xrDestroyInstance(instance);
    instance = XR_NULL_HANDLE;
  }
  glDeleteFramebuffers(2, openxrFbo);
  glDeleteTextures(2, openxrDepth);

  glDisable(GL_FRAMEBUFFER_SRGB);
  glDrawBuffer(GL_BACK);
  glfwSwapInterval(1);
}

//
// OpenXR による描画開始
//
bool GgApp::OpenXR::begin()
{
  XrEventDataBuffer eventData{ XR_TYPE_EVENT_DATA_BUFFER };
  while (xrPollEvent(instance, &eventData) == XR_SUCCESS) {
    if (eventData.type == XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED) {
      auto* stateChanged = reinterpret_cast<XrEventDataSessionStateChanged*>(&eventData);
      sessionState = stateChanged->state;
      if (sessionState == XR_SESSION_STATE_READY) {
        XrSessionBeginInfo beginInfo{ XR_TYPE_SESSION_BEGIN_INFO };
        beginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
        xrBeginSession(session, &beginInfo);
        isSessionRunning = true;
      } else if (sessionState == XR_SESSION_STATE_STOPPING) {
        xrEndSession(session);
        isSessionRunning = false;
      } else if (sessionState == XR_SESSION_STATE_EXITING || sessionState == XR_SESSION_STATE_LOSS_PENDING) {
        window->setClose(GLFW_TRUE);
      }
    }
    eventData.type = XR_TYPE_EVENT_DATA_BUFFER;
  }

  if (!isSessionRunning || sessionState != XR_SESSION_STATE_FOCUSED) return false;

  XrFrameWaitInfo waitInfo{ XR_TYPE_FRAME_WAIT_INFO };
  frameState = { XR_TYPE_FRAME_STATE };
  xrWaitFrame(session, &waitInfo, &frameState);

  XrFrameBeginInfo beginInfo{ XR_TYPE_FRAME_BEGIN_INFO };
  xrBeginFrame(session, &beginInfo);

  XrViewLocateInfo viewLocateInfo{ XR_TYPE_VIEW_LOCATE_INFO };
  viewLocateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
  viewLocateInfo.displayTime = frameState.predictedDisplayTime;
  viewLocateInfo.space = appSpace;

  XrViewState viewState{ XR_TYPE_VIEW_STATE };
  uint32_t viewCount;
  xrLocateViews(session, &viewLocateInfo, &viewState, (uint32_t)viewStates.size(), &viewCount, viewStates.data());

  return true;
}

//
// OpenXR の描画する目の指定
//
void GgApp::OpenXR::select(int eye, GLfloat* screen, GLfloat* position, GLfloat* orientation)
{
  XrSwapchainImageAcquireInfo acquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
  xrAcquireSwapchainImage(swapchains[eye], &acquireInfo, &currentImageIndex[eye]);

  XrSwapchainImageWaitInfo waitInfo{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
  waitInfo.timeout = XR_INFINITE_DURATION;
  xrWaitSwapchainImage(swapchains[eye], &waitInfo);

  GLuint texId = swapchainImages[eye][currentImageIndex[eye]].image;
  glBindFramebuffer(GL_FRAMEBUFFER, openxrFbo[eye]);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, openxrDepth[eye], 0);

  glViewport(0, 0, views[eye].recommendedImageRectWidth, views[eye].recommendedImageRectHeight);

  const auto& pose = viewStates[eye].pose;
  const auto& fov = viewStates[eye].fov;

  screen[0] = tanf(fov.angleLeft);
  screen[1] = tanf(fov.angleRight);
  screen[2] = tanf(fov.angleDown);
  screen[3] = tanf(fov.angleUp);

  position[0] = pose.position.x;
  position[1] = pose.position.y;
  position[2] = pose.position.z;

  orientation[0] = pose.orientation.x;
  orientation[1] = pose.orientation.y;
  orientation[2] = pose.orientation.z;
  orientation[3] = pose.orientation.w;
}

//
// Time Warp 処理に使う投影変換行列の成分の設定 (ダミー)
//
void GgApp::OpenXR::timewarp(const GgMatrix& projection)
{
}

//
// 図形の描画を完了する
//
void GgApp::OpenXR::commit(int eye)
{
  glBindFramebuffer(GL_FRAMEBUFFER, openxrFbo[eye]);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);

  XrSwapchainImageReleaseInfo releaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
  xrReleaseSwapchainImage(swapchains[eye], &releaseInfo);
}

//
// フレームを転送する
//
bool GgApp::OpenXR::submit(bool mirror)
{
  XrCompositionLayerProjectionView projectionViews[2];
  for (int eye = 0; eye < 2; ++eye) {
    projectionViews[eye].type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
    projectionViews[eye].next = nullptr;
    projectionViews[eye].pose = viewStates[eye].pose;
    projectionViews[eye].fov = viewStates[eye].fov;
    projectionViews[eye].subImage.swapchain = swapchains[eye];
    projectionViews[eye].subImage.imageRect.offset = {0, 0};
    projectionViews[eye].subImage.imageRect.extent = {(int32_t)views[eye].recommendedImageRectWidth, (int32_t)views[eye].recommendedImageRectHeight};
    projectionViews[eye].subImage.imageArrayIndex = 0;
  }

  XrCompositionLayerProjection layer{ XR_TYPE_COMPOSITION_LAYER_PROJECTION };
  layer.space = appSpace;
  layer.viewCount = 2;
  layer.views = projectionViews;

  const XrCompositionLayerBaseHeader* layers = reinterpret_cast<XrCompositionLayerBaseHeader*>(&layer);

  XrFrameEndInfo endInfo{ XR_TYPE_FRAME_END_INFO };
  endInfo.displayTime = frameState.predictedDisplayTime;
  endInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
  endInfo.layerCount = frameState.shouldRender ? 1 : 0;
  endInfo.layers = &layers;

  xrEndFrame(session, &endInfo);

  if (mirror) {
    GLsizei size[2];
    window->getSize(size);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, openxrFbo[0]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, views[0].recommendedImageRectWidth, views[0].recommendedImageRectHeight,
                      0, 0, size[0], size[1], GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glFlush();
  }

  return true;
}
#endif

#if !defined(_MSC_VER)
#  include <pwd.h>
#  include <unistd.h>
#endif
//
// ユーザ名を得る
//
std::string GgApp::getUsername()
{
  // 環境変数からユーザ名を得る
  const char* user{
#if defined(_MSC_VER)
    std::getenv("USERNAME")
#else
    std::getenv("USER")
#endif
  };

  // 環境変数からユーザ名が得られたらそれを返す
  if (user) return user;

#if defined(_MSC_VER)
  // Win32 API を使ってユーザ名を得る
  char username[256];
  DWORD size{ sizeof(username) };
  if (GetUserNameA(username, &size)) return std::string(username);
#else
  struct passwd* pw{ getpwuid(getuid()) };
  if (pw) return std::string(pw->pw_name);
#endif

  // ユーザ名が得られなかった
  return "unknown";
}
