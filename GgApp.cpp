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

#if defined(GG_USE_OCULUS_RIFT)
  // Oculus Rift では SRGB でレンダリングする
  glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);
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

#if defined(GG_USE_OCULUS_RIFT)
#  if OVR_PRODUCT_VERSION > 0
//
// グラフィックスカードのデフォルトの LUID を得る
//
ovrGraphicsLuid GgApp::Oculus::GetDefaultAdapterLuid()
{
  ovrGraphicsLuid luid = ovrGraphicsLuid();

#    if defined(_MSC_VER)
  IDXGIFactory* factory{ nullptr };

  if (SUCCEEDED(CreateDXGIFactory(IID_PPV_ARGS(&factory))))
  {
    IDXGIAdapter* adapter{ nullptr };

    if (SUCCEEDED(factory->EnumAdapters(0, &adapter)))
    {
      DXGI_ADAPTER_DESC desc;

      adapter->GetDesc(&desc);
      memcpy(&luid, &desc.AdapterLuid, sizeof luid);
      adapter->Release();
    }

    factory->Release();
  }
#    endif

  return luid;
}

//
// グラフィックスカードの LUID の比較
//
int GgApp::Oculus::Compare(const ovrGraphicsLuid& lhs, const ovrGraphicsLuid& rhs)
{
  return memcmp(&lhs, &rhs, sizeof(ovrGraphicsLuid));
}
#  endif

//
// コンストラクタ
//
GgApp::Oculus::Oculus() :
  session{ nullptr },
  oculusFbo{ 0 },
  screen{ -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, },
  mirrorFbo{ 0 },
  window{ nullptr },
#  if OVR_PRODUCT_VERSION > 0
  frameIndex{ 0LL },
  oculusDepth{ 0 },
  mirrorWidth{ 1280 },
  mirrorHeight{ 640 },
#  endif
  mirrorTexture{ nullptr }
{
}

//
// Oculus Rift のセッションを作成する
//
GgApp::Oculus& GgApp::Oculus::initialize(const Window& window)
{
  // Oculus Rift のコンテキスト
  static Oculus oculus;

  // 既に Oculus Rift のセッションが作成されていたら参照を返す
  if (oculus.session) return oculus;

  // 最初に呼び出したときだけ実行する
  static bool firstTime{ true };
  if (firstTime)
  {
    // Oculus Rift (LibOVR) を初期化する
    ovrInitParams initParams{ ovrInit_RequestVersion, OVR_MINOR_VERSION, NULL, 0, 0 };
    if (OVR_FAILURE(ovr_Initialize(&initParams)))
      throw std::runtime_error("Can't initialize LibOVR");

    // アプリケーションの終了時に LibOVR を終了する
    atexit(ovr_Shutdown);

    // 実行済みであることを記録する
    firstTime = false;
  }

  // Oculus Rift のセッションを作成する
  ovrGraphicsLuid luid;
  if (OVR_FAILURE(ovr_Create(&oculus.session, &luid)))
    throw std::runtime_error("Can't create Oculus Rift session");

#  if OVR_PRODUCT_VERSION > 0
  // デフォルトのグラフィックスアダプタが使われているか確かめる
  if (Compare(luid, GetDefaultAdapterLuid()))
    throw std::runtime_error("Graphics adapter is not default");
#  endif

  // session が無効ならエラー
  if (!oculus.session) std::runtime_error("Unable to use the Oculus Rift.");

  // ミラー表示を行うウィンドウを設定する
  oculus.window = &window;

  // Oculus Rift の情報を取り出す
  oculus.hmdDesc = ovr_GetHmdDesc(oculus.session);

#  if defined(_DEBUG)
  // Oculus Rift の情報を表示する
  std::cerr
    << "\nProduct name: " << oculus.hmdDesc.ProductName
    << "\nResolution:   " << oculus.hmdDesc.Resolution.w << " x " << oculus.hmdDesc.Resolution.h
    << "\nDefault Fov:  (" << oculus.hmdDesc.DefaultEyeFov[ovrEye_Left].LeftTan
    << "," << oculus.hmdDesc.DefaultEyeFov[ovrEye_Left].DownTan
    << ") - (" << oculus.hmdDesc.DefaultEyeFov[ovrEye_Left].RightTan
    << "," << oculus.hmdDesc.DefaultEyeFov[ovrEye_Left].UpTan
    << ")\n              (" << oculus.hmdDesc.DefaultEyeFov[ovrEye_Right].LeftTan
    << "," << oculus.hmdDesc.DefaultEyeFov[ovrEye_Right].DownTan
    << ") - (" << oculus.hmdDesc.DefaultEyeFov[ovrEye_Right].RightTan
    << "," << oculus.hmdDesc.DefaultEyeFov[ovrEye_Right].UpTan
    << ")\nMaximum Fov:  (" << oculus.hmdDesc.MaxEyeFov[ovrEye_Left].LeftTan
    << "," << oculus.hmdDesc.MaxEyeFov[ovrEye_Left].DownTan
    << ") - (" << oculus.hmdDesc.MaxEyeFov[ovrEye_Left].RightTan
    << "," << oculus.hmdDesc.MaxEyeFov[ovrEye_Left].UpTan
    << ")\n              (" << oculus.hmdDesc.MaxEyeFov[ovrEye_Right].LeftTan
    << "," << oculus.hmdDesc.MaxEyeFov[ovrEye_Right].DownTan
    << ") - (" << oculus.hmdDesc.MaxEyeFov[ovrEye_Right].RightTan
    << "," << oculus.hmdDesc.MaxEyeFov[ovrEye_Right].UpTan
    << ")\n" << std::endl;
#  endif

  // Oculus Rift に転送する描画データを作成する
#  if OVR_PRODUCT_VERSION > 0
  oculus.layerData.Header.Type = ovrLayerType_EyeFov;
#  else
  oculus.layerData.Header.Type = ovrLayerType_EyeFovDepth;
#  endif

  // OpenGL なので左下が原点
  oculus.layerData.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

  // Oculus Rift のレンダリングに使う FBO を作成する
  glGenFramebuffers(ovrEye_Count, oculus.oculusFbo);

  // 全ての目について
  for (int eye = 0; eye < ovrEye_Count; ++eye)
  {
    // Oculus Rift の視野を取得する
    const auto& fov{ oculus.hmdDesc.DefaultEyeFov[ovrEyeType(eye)] };

    // Oculus Rift 用の FBO のサイズを求める
    const auto textureSize{ ovr_GetFovTextureSize(oculus.session, ovrEyeType(eye), fov, 1.0f) };

    // Oculus Rift のスクリーンのサイズを保存する
    oculus.screen[eye][0] = -fov.LeftTan;
    oculus.screen[eye][1] = fov.RightTan;
    oculus.screen[eye][2] = -fov.DownTan;
    oculus.screen[eye][3] = fov.UpTan;

#  if OVR_PRODUCT_VERSION > 0

    // 描画データに視野を設定する
    oculus.layerData.Fov[eye] = fov;

    // 描画データにビューポートを設定する
    oculus.layerData.Viewport[eye].Pos = OVR::Vector2i(0, 0);
    oculus.layerData.Viewport[eye].Size = textureSize;

    // Oculus Rift 用の FBO のカラーバッファとして使うテクスチャセットの特性
    const ovrTextureSwapChainDesc colorDesc
    {
      ovrTexture_2D,                    // Type
      OVR_FORMAT_R8G8B8A8_UNORM_SRGB,   // Format
      1,                                // ArraySize
      textureSize.w,                    // Width
      textureSize.h,                    // Height
      1,                                // MipLevels
      1,                                // SampleCount
      ovrFalse,                         // StaticImage
      0, 0
    };

    // Oculus Rift 用の FBO のレンダーターゲットとして使うテクスチャチェインを作成する
    oculus.layerData.ColorTexture[eye] = nullptr;
    if (OVR_SUCCESS(ovr_CreateTextureSwapChainGL(oculus.session, &colorDesc, &oculus.layerData.ColorTexture[eye])))
    {
      // 作成したテクスチャチェインの長さを取得する
      int length(0);
      if (OVR_SUCCESS(ovr_GetTextureSwapChainLength(oculus.session, oculus.layerData.ColorTexture[eye], &length)))
      {
        // テクスチャチェインの個々の要素について
        for (int i = 0; i < length; ++i)
        {
          // テクスチャのパラメータを設定する
          GLuint texId{ 0 };
          ovr_GetTextureSwapChainBufferGL(oculus.session, oculus.layerData.ColorTexture[eye], i, &texId);
          glBindTexture(GL_TEXTURE_2D, texId);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
      }

      // Oculus Rift 用の FBO のデプスバッファとして使うテクスチャを作成する
      glGenTextures(1, oculus.oculusDepth + eye);
      glBindTexture(GL_TEXTURE_2D, oculus.oculusDepth[eye]);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, textureSize.w, textureSize.h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

#  else

    // 描画データに視野を設定する
    oculus.layerData.EyeFov.Fov[eye] = fov;

    // 描画データにビューポートを設定する
    oculus.layerData.EyeFov.Viewport[eye].Pos = OVR::Vector2i(0, 0);
    oculus.layerData.EyeFov.Viewport[eye].Size = textureSize;

    // Oculus Rift 用の FBO のカラーバッファとして使うテクスチャセットを作成する
    ovrSwapTextureSet* colorTexture{ nullptr };
    ovr_CreateSwapTextureSetGL(oculus.session, GL_SRGB8_ALPHA8, textureSize.w, textureSize.h, &colorTexture);
    oculus.layerData.EyeFov.ColorTexture[eye] = colorTexture;

    // Oculus Rift 用の FBO のデプスバッファとして使うテクスチャセットを作成する
    ovrSwapTextureSet* depthTexture{ nullptr };
    ovr_CreateSwapTextureSetGL(oculus.session, GL_DEPTH_COMPONENT32F, textureSize.w, textureSize.h, &depthTexture);
    oculus.layerData.EyeFovDepth.DepthTexture[eye] = depthTexture;

    // Oculus Rift のレンズ補正等の設定値を取得する
    oculus.eyeRenderDesc[eye] = ovr_GetRenderDesc(oculus.session, ovrEyeType(eye), fov);

#  endif
  }

#  if OVR_PRODUCT_VERSION > 0

  // 姿勢のトラッキングにおける床の高さを 0 に設定する
  ovr_SetTrackingOriginType(oculus.session, ovrTrackingOrigin_FloorLevel);

  // ミラー表示用の FBO を作成する
  const GLsizei* size{ oculus.window->getSize() };
  const ovrMirrorTextureDesc mirrorDesc
  {
    OVR_FORMAT_R8G8B8A8_UNORM_SRGB, // Format
    oculus.mirrorWidth = size[0],   // Width
    oculus.mirrorHeight = size[1],  // Height
    0                               // Flags
  };

  // ミラー表示用の FBO のカラーバッファとして使うテクスチャを作成する
  if (OVR_SUCCESS(ovr_CreateMirrorTextureGL(oculus.session, &mirrorDesc, &oculus.mirrorTexture)))
  {
    // 作成したテクスチャのテクスチャ名を得る
    GLuint texId{ 0 };
    if (OVR_SUCCESS(ovr_GetMirrorTextureBufferGL(oculus.session, oculus.mirrorTexture, &texId)))
    {
      // ミラー表示用の FBO を作成してテクスチャをカラーバッファとして組み込む
      glGenFramebuffers(1, &oculus.mirrorFbo);
      glBindFramebuffer(GL_READ_FRAMEBUFFER, oculus.mirrorFbo);
      glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
      glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
      glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    }
  }

#  else

  // 作成したテクスチャのテクスチャ名を得る
  if (OVR_SUCCESS(ovr_CreateMirrorTextureGL(oculus.session, GL_SRGB8_ALPHA8, width, height, reinterpret_cast<ovrTexture**>(&mirrorTexture))))
  {
    // ミラー表示用の FBO を作成してテクスチャをカラーバッファとして組み込む
    oculus.mirrorFbo = 0;
    glGenFramebuffers(1, &oculus.mirrorFbo);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, oculus.mirrorFbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTexture->OGL.TexId, 0);
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  }

#  endif

  // Oculus Rift にレンダリングするときは sRGB カラースペースを使う
  glEnable(GL_FRAMEBUFFER_SRGB);

  // フロントバッファに描く
  glDrawBuffer(GL_FRONT);

  // Oculus Rift への表示では垂直同期タイミングに合わせない
  glfwSwapInterval(0);

  return oculus;
}

//
// Oculus Rift のセッションを破棄する
//
void GgApp::Oculus::terminate()
{
  // session が無効なら何もしない
  if (!session) return;

  // ミラー表示用の FBO を作っていたら削除する
  if (mirrorFbo)
  {
    glDeleteFramebuffers(1, &mirrorFbo);
    mirrorFbo = 0;
  }

  // ミラー表示用の FBO のカラーバッファ用のテクスチャを作っていたら削除する
  if (mirrorTexture)
  {
#  if OVR_PRODUCT_VERSION > 0
    ovr_DestroyMirrorTexture(session, mirrorTexture);
#  else
    glDeleteTextures(1, &mirrorTexture->OGL.TexId);
    ovr_DestroyMirrorTexture(session, reinterpret_cast<ovrTexture*>(mirrorTexture));
#  endif
    mirrorTexture = nullptr;
  }

  // 全ての目について
  for (int eye = 0; eye < ovrEye_Count; ++eye)
  {
    // Oculus Rift へのレンダリング用の FBO を削除する
    glDeleteFramebuffers(1, oculusFbo + eye);
    oculusFbo[eye] = 0;

#  if OVR_PRODUCT_VERSION > 0

    // レンダリングターゲットに使ったテクスチャを削除する
    if (layerData.ColorTexture[eye])
    {
      ovr_DestroyTextureSwapChain(session, layerData.ColorTexture[eye]);
      layerData.ColorTexture[eye] = nullptr;
    }

    // デプスバッファとして使ったテクスチャを削除する
    glDeleteTextures(1, oculusDepth + eye);
    oculusDepth[eye] = 0;

#  else

    // レンダリングターゲットに使ったテクスチャを削除する
    auto* const colorTexture(layerData.EyeFov.ColorTexture[eye]);
    for (int i = 0; i < colorTexture->TextureCount; ++i)
    {
      const auto* const ctex(reinterpret_cast<ovrGLTexture*>(&colorTexture->Textures[i]));
      glDeleteTextures(1, &ctex->OGL.TexId);
    }
    ovr_DestroySwapTextureSet(session, colorTexture);

    // デプスバッファとして使ったテクスチャを削除する
    auto* const depthTexture(layerData.EyeFovDepth.DepthTexture[eye]);
    for (int i = 0; i < depthTexture->TextureCount; ++i)
    {
      const auto* const dtex(reinterpret_cast<ovrGLTexture*>(&depthTexture->Textures[i]));
      glDeleteTextures(1, &dtex->OGL.TexId);
    }
    ovr_DestroySwapTextureSet(session, depthTexture);

#  endif
  }

  // Oculus Rift のセッションを破棄する
  ovr_Destroy(session);
  session = nullptr;

  // カラースペースを元に戻す
  glDisable(GL_FRAMEBUFFER_SRGB);

  // バックバッファに描く
  glDrawBuffer(GL_BACK);

  // 垂直同期タイミングに合わせる
  glfwSwapInterval(1);
}

//
// Oculus Rift による描画開始
//
bool GgApp::Oculus::begin()
{
#  if OVR_PRODUCT_VERSION > 0

  // セッションの状態を取得する
  ovrSessionStatus sessionStatus;
  ovr_GetSessionStatus(session, &sessionStatus);

  // アプリケーションが終了を要求しているときはウィンドウのクローズフラグを立てる
  if (sessionStatus.ShouldQuit) window->setClose(GLFW_TRUE);

  // Oculus Rift に表示されていないときは戻る
  if (!sessionStatus.IsVisible) return false;

  // 現在の状態をトラッキングの原点にする
  if (sessionStatus.ShouldRecenter) ovr_RecenterTrackingOrigin(session);

  // HmdToEyeOffset などは実行時に変化するので毎フレーム ovr_GetRenderDesc() で ovrEyeRenderDesc を取得する
  const ovrEyeRenderDesc eyeRenderDesc[]
  {
    ovr_GetRenderDesc(session, ovrEyeType(0), hmdDesc.DefaultEyeFov[0]),
    ovr_GetRenderDesc(session, ovrEyeType(1), hmdDesc.DefaultEyeFov[1])
  };

  // Oculus Rift のスクリーンのヘッドトラッキング位置からの変位を取得する
  const ovrPosef hmdToEyePose[]
  {
    eyeRenderDesc[0].HmdToEyePose,
    eyeRenderDesc[1].HmdToEyePose
  };

  // 視点の姿勢情報を取得する
  ovr_GetEyePoses(session, frameIndex, ovrTrue, hmdToEyePose, layerData.RenderPose, &layerData.SensorSampleTime);

#  else

  // フレームのタイミング計測開始
  const auto ftiming(ovr_GetPredictedDisplayTime(session, 0));

  // sensorSampleTime の取得は可能な限り ovr_GetTrackingState() の近くで行う
  layerData.EyeFov.SensorSampleTime = ovr_GetTimeInSeconds();

  // ヘッドトラッキングの状態を取得する
  const auto hmdState(ovr_GetTrackingState(session, ftiming, ovrTrue));

  // Oculus Rift のスクリーンのヘッドトラッキング位置からの変位を取得する
  const ovrVector3f hmdToEyeViewOffset[]
  {
    eyeRenderDesc[0].HmdToEyeViewOffset,
    eyeRenderDesc[1].HmdToEyeViewOffset
  };

  // 視点の姿勢情報を求める
  ovr_CalcEyePoses(hmdState.HeadPose.ThePose, hmdToEyeViewOffset, eyePose);

#  endif

  return true;
}

//
// Oculus Rift の描画する目の指定
//
void GgApp::Oculus::select(int eye, GLfloat* screen, GLfloat* position, GLfloat* orientation)
{
#  if OVR_PRODUCT_VERSION > 0

  // Oculus Rift にレンダリングする FBO に切り替える
  if (layerData.ColorTexture[eye])
  {
    // FBO のカラーバッファに使う現在のテクスチャのインデックスを取得する
    int curIndex;
    ovr_GetTextureSwapChainCurrentIndex(session, layerData.ColorTexture[eye], &curIndex);

    // FBO のカラーバッファに使うテクスチャを取得する
    GLuint curTexId;
    ovr_GetTextureSwapChainBufferGL(session, layerData.ColorTexture[eye], curIndex, &curTexId);

    // FBO を設定する
    glBindFramebuffer(GL_FRAMEBUFFER, oculusFbo[eye]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, oculusDepth[eye], 0);

    // ビューポートを設定する
    const auto& vp{ layerData.Viewport[eye] };
    glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
  }

  // Oculus Rift の片目の位置と回転を取得する
  const auto& p{ layerData.RenderPose[eye].Position };
  const auto& o{ layerData.RenderPose[eye].Orientation };

#  else

  // レンダーターゲットに描画する前にレンダーターゲットのインデックスをインクリメントする
  auto* const colorTexture{ layerData.EyeFov.ColorTexture[eye] };
  colorTexture->CurrentIndex = (colorTexture->CurrentIndex + 1) % colorTexture->TextureCount;
  auto* const depthTexture{ layerData.EyeFovDepth.DepthTexture[eye] };
  depthTexture->CurrentIndex = (depthTexture->CurrentIndex + 1) % depthTexture->TextureCount;

  // レンダーターゲットを切り替える
  glBindFramebuffer(GL_FRAMEBUFFER, oculusFbo[eye]);
  const auto& ctex{ reinterpret_cast<ovrGLTexture*>(&colorTexture->Textures[colorTexture->CurrentIndex]) };
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ctex->OGL.TexId, 0);
  const auto& dtex{ reinterpret_cast<ovrGLTexture*>(&depthTexture->Textures[depthTexture->CurrentIndex]) };
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dtex->OGL.TexId, 0);

  // ビューポートを設定する
  const auto& vp{ layerData.EyeFov.Viewport[eye] };
  glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);

  // Oculus Rift の片目の位置と回転を取得する
  const auto& p{ eyePose[eye].Position };
  const auto& o{ eyePose[eye].Orientation };

#  endif

  // Oculus Rift のスクリーンの大きさを返す
  screen[0] = this->screen[eye][0];
  screen[1] = this->screen[eye][1];
  screen[2] = this->screen[eye][2];
  screen[3] = this->screen[eye][3];

  // Oculus Rift の位置を返す
  position[0] = p.x;
  position[1] = p.y;
  position[2] = p.z;

  // Oculus Rift の方向を返す
  orientation[0] = o.x;
  orientation[1] = o.y;
  orientation[2] = o.z;
  orientation[3] = o.w;
}

//
// Time Warp 処理に使う投影変換行列の成分の設定 (DK1, DK2)
//
void GgApp::Oculus::timewarp(const GgMatrix& projection)
{
#  if OVR_PRODUCT_VERSION < 1
  // TimeWarp に使う変換行列の成分を設定する
  auto& posTimewarpProjectionDesc{ layerData.EyeFovDepth.ProjectionDesc };
  posTimewarpProjectionDesc.Projection22 = (projection.get()[4 * 2 + 2] + projection.get()[4 * 3 + 2]) * 0.5f;
  posTimewarpProjectionDesc.Projection23 = projection.get()[4 * 2 + 3] * 0.5f;
  posTimewarpProjectionDesc.Projection32 = projection.get()[4 * 3 + 2];
#  endif
}

//
// 図形の描画を完了する (CV1 以降)
//
void GgApp::Oculus::commit(int eye)
{
#  if OVR_PRODUCT_VERSION > 0
  // GL_COLOR_ATTACHMENT0 に割り当てられたテクスチャが wglDXUnlockObjectsNV() によって
  // アンロックされるために次のフレームの処理において無効な GL_COLOR_ATTACHMENT0 が
  // FBO に結合されるのを避ける
  glBindFramebuffer(GL_FRAMEBUFFER, oculusFbo[eye]);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);

  // 保留中の変更を layerData.ColorTexture[eye] に反映しインデックスを更新する
  ovr_CommitTextureSwapChain(session, layerData.ColorTexture[eye]);
#  endif
}

//
// フレームを転送する
//
bool GgApp::Oculus::submit(bool mirror)
{
  // エラーチェック
  ggError();

#  if OVR_PRODUCT_VERSION > 0
  // 描画データを Oculus Rift に転送する
  const auto* const layers{ &layerData.Header };
  if (OVR_FAILURE(ovr_SubmitFrame(session, frameIndex++, nullptr, &layers, 1))) return false;
#  else
  // Oculus Rift 上の描画位置と拡大率を求める
  ovrViewScaleDesc viewScaleDesc;
  viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;
  viewScaleDesc.HmdToEyeViewOffset[0] = eyeRenderDesc[0].HmdToEyeViewOffset;
  viewScaleDesc.HmdToEyeViewOffset[1] = eyeRenderDesc[1].HmdToEyeViewOffset;

  // 描画データを更新する
  layerData.EyeFov.RenderPose[0] = eyePose[0];
  layerData.EyeFov.RenderPose[1] = eyePose[1];

  // 描画データを Oculus Rift に転送する
  const auto* const layers{ &layerData.Header };
  if (OVR_FAILURE(ovr_SubmitFrame(session, 0, &viewScaleDesc, &layers, 1))) return false;
#  endif

  // ミラー表示
  if (mirror)
  {
#  if OVR_PRODUCT_VERSION > 0
    const auto& sx1{ mirrorWidth };
    const auto& sy1{ mirrorHeight };
#  else
    const auto& sx1{ mirrorTexture->OGL.Header.TextureSize.w };
    const auto& sy1{ mirrorTexture->OGL.Header.TextureSize.h };
#  endif

    // ミラー表示のウィンドウのサイズ
    GLsizei size[2];
    window->getSize(size);

    // ミラー表示の表示領域
    GLint dx0{ 0 }, dx1{ size[0] }, dy0{ 0 }, dy1{ size[1] };

    // ミラー表示がウィンドウからはみ出ないようにする
    if ((size[0] *= sy1) < (size[1] *= sx1))
    {
      const GLint ty1{ size[0] / sx1 };
      dy0 = (dy1 - ty1) / 2;
      dy1 = dy0 + ty1;
    }
    else
    {
      const GLint tx1{ size[1] / sy1 };
      dx0 = (dx1 - tx1) / 2;
      dx1 = dx0 + tx1;
    }

    // レンダリング結果をミラー表示用のフレームバッファにも転送する
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, sy1, sx1, 0, dx0, dy0, dx1, dy1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    // 残っている OpenGL コマンドを実行する
    glFlush();
  }

  return true;
}
#endif
