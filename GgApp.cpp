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
using namespace gg;

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
  size{ width, height },
  fboSize{ width, height }
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
// Window クラスのムーブコンストラクタ
//
GgApp::Window::Window(Window&& w) noexcept :
  window{ w.window },
  size{ w.size },
  fboSize{ w.fboSize },
  interfaceData{ std::move(w.interfaceData) },
  interfaceNo{ w.interfaceNo },
  userPointer{ w.userPointer },
  resizeFunc{ w.resizeFunc },
  keyboardFunc{ w.keyboardFunc },
  mouseFunc{ w.mouseFunc },
  wheelFunc{ w.wheelFunc }
{
  w.window = nullptr;
  if (window)
  {
    glfwSetWindowUserPointer(window, this);
  }
}

//
// Window クラスのムーブ代入演算子
//
GgApp::Window& GgApp::Window::operator=(Window&& w) noexcept
{
  if (&w != this)
  {
    if (window)
    {
      glfwDestroyWindow(window);
    }
    window = w.window;
    size = w.size;
    fboSize = w.fboSize;
    interfaceData = std::move(w.interfaceData);
    interfaceNo = w.interfaceNo;
    userPointer = w.userPointer;
    resizeFunc = w.resizeFunc;
    keyboardFunc = w.keyboardFunc;
    mouseFunc = w.mouseFunc;
    wheelFunc = w.wheelFunc;

    w.window = nullptr;
    if (window)
    {
      glfwSetWindowUserPointer(window, this);
    }
  }
  return *this;
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
  referenceSpaceType{ XR_REFERENCE_SPACE_TYPE_STAGE },
  openxrFbo{ 0, 0 },
  openxrDepth{ 0, 0 },
  frameState{ XR_TYPE_FRAME_STATE },
  isSessionRunning{ false },
  window{ nullptr },
  actionSet{ XR_NULL_HANDLE },
  aimPoseAction{ XR_NULL_HANDLE },
  gripPoseAction{ XR_NULL_HANDLE },
  triggerAction{ XR_NULL_HANDLE },
  gripAction{ XR_NULL_HANDLE },
  thumbstickAction{ XR_NULL_HANDLE },
  thumbstickClickAction{ XR_NULL_HANDLE },
  primaryButtonAction{ XR_NULL_HANDLE },
  secondaryButtonAction{ XR_NULL_HANDLE },
  menuButtonAction{ XR_NULL_HANDLE },
  hapticAction{ XR_NULL_HANDLE }
{
  currentImageIndex[0] = 0;
  currentImageIndex[1] = 0;
  for (int i = 0; i < Hand::Count; ++i)
  {
    aimSpace[i] = XR_NULL_HANDLE;
    gripSpace[i] = XR_NULL_HANDLE;
    handSubactionPath[i] = XR_NULL_PATH;
    controllerStates[i] = ControllerState{};
  }
}

//
// アクションシステムを初期化する
//
void GgApp::OpenXR::initActions()
{
  // アクションセットの作成
  XrActionSetCreateInfo actionSetInfo{ XR_TYPE_ACTION_SET_CREATE_INFO };
  strcpy(actionSetInfo.actionSetName, "gameplay");
  strcpy(actionSetInfo.localizedActionSetName, "Gameplay");
  actionSetInfo.priority = 0;
  if (XR_FAILED(xrCreateActionSet(instance, &actionSetInfo, &actionSet))) return;

  // サブアクションパスの取得
  xrStringToPath(instance, "/user/hand/left", &handSubactionPath[Hand::Left]);
  xrStringToPath(instance, "/user/hand/right", &handSubactionPath[Hand::Right]);

  // アクション作成ヘルパー
  auto createAction = [this](const char* name, const char* localizedName, XrActionType type, XrAction& action, bool subactions = true)
  {
    XrActionCreateInfo createInfo{ XR_TYPE_ACTION_CREATE_INFO };
    strcpy(createInfo.actionName, name);
    strcpy(createInfo.localizedActionName, localizedName);
    createInfo.actionType = type;
    if (subactions)
    {
      createInfo.countSubactionPaths = Hand::Count;
      createInfo.subactionPaths = handSubactionPath;
    }
    xrCreateAction(actionSet, &createInfo, &action);
  };

  createAction("aim_pose", "Aim Pose", XR_ACTION_TYPE_POSE_INPUT, aimPoseAction);
  createAction("grip_pose", "Grip Pose", XR_ACTION_TYPE_POSE_INPUT, gripPoseAction);
  createAction("trigger", "Trigger", XR_ACTION_TYPE_FLOAT_INPUT, triggerAction);
  createAction("grip", "Grip", XR_ACTION_TYPE_FLOAT_INPUT, gripAction);
  createAction("thumbstick", "Thumbstick", XR_ACTION_TYPE_VECTOR2F_INPUT, thumbstickAction);
  createAction("thumbstick_click", "Thumbstick Click", XR_ACTION_TYPE_BOOLEAN_INPUT, thumbstickClickAction);
  createAction("primary_button", "Primary Button", XR_ACTION_TYPE_BOOLEAN_INPUT, primaryButtonAction);
  createAction("secondary_button", "Secondary Button", XR_ACTION_TYPE_BOOLEAN_INPUT, secondaryButtonAction);
  createAction("menu_button", "Menu Button", XR_ACTION_TYPE_BOOLEAN_INPUT, menuButtonAction);
  createAction("haptic", "Haptic Vibration", XR_ACTION_TYPE_VIBRATION_OUTPUT, hapticAction);

  // アクションスペースの作成
  for (int i = 0; i < Hand::Count; ++i)
  {
    XrActionSpaceCreateInfo spaceInfo{ XR_TYPE_ACTION_SPACE_CREATE_INFO };
    spaceInfo.poseInActionSpace.orientation.w = 1.0f;
    spaceInfo.subactionPath = handSubactionPath[i];

    spaceInfo.action = aimPoseAction;
    xrCreateActionSpace(session, &spaceInfo, &aimSpace[i]);

    spaceInfo.action = gripPoseAction;
    xrCreateActionSpace(session, &spaceInfo, &gripSpace[i]);
  }

  // パス文字列から XrPath を取得するヘルパー
  auto getPath = [this](const char* pathStr) -> XrPath
  {
    XrPath path{ XR_NULL_PATH };
    xrStringToPath(instance, pathStr, &path);
    return path;
  };

  // バインディング設定ヘルパー
  auto suggestBindings = [this, &getPath](const char* profileStr, const std::vector<std::pair<XrAction, const char*>>& bindings)
  {
    XrPath profilePath = getPath(profileStr);
    std::vector<XrActionSuggestedBinding> suggestedBindings;
    suggestedBindings.reserve(bindings.size());
    for (const auto& [action, pathStr] : bindings)
    {
      suggestedBindings.push_back({ action, getPath(pathStr) });
    }
    XrInteractionProfileSuggestedBinding profileSuggestedBindings{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
    profileSuggestedBindings.interactionProfile = profilePath;
    profileSuggestedBindings.suggestedBindings = suggestedBindings.data();
    profileSuggestedBindings.countSuggestedBindings = static_cast<uint32_t>(suggestedBindings.size());
    xrSuggestInteractionProfileBindings(instance, &profileSuggestedBindings);
  };

  // Oculus Touch コントローラーのバインディング
  suggestBindings("/interaction_profiles/oculus/touch_controller", {
    { aimPoseAction, "/user/hand/left/input/aim/pose" },
    { aimPoseAction, "/user/hand/right/input/aim/pose" },
    { gripPoseAction, "/user/hand/left/input/grip/pose" },
    { gripPoseAction, "/user/hand/right/input/grip/pose" },
    { triggerAction, "/user/hand/left/input/trigger/value" },
    { triggerAction, "/user/hand/right/input/trigger/value" },
    { gripAction, "/user/hand/left/input/squeeze/value" },
    { gripAction, "/user/hand/right/input/squeeze/value" },
    { thumbstickAction, "/user/hand/left/input/thumbstick" },
    { thumbstickAction, "/user/hand/right/input/thumbstick" },
    { thumbstickClickAction, "/user/hand/left/input/thumbstick/click" },
    { thumbstickClickAction, "/user/hand/right/input/thumbstick/click" },
    { primaryButtonAction, "/user/hand/left/input/x/click" },
    { primaryButtonAction, "/user/hand/right/input/a/click" },
    { secondaryButtonAction, "/user/hand/left/input/y/click" },
    { secondaryButtonAction, "/user/hand/right/input/b/click" },
    { menuButtonAction, "/user/hand/left/input/menu/click" },
    { hapticAction, "/user/hand/left/output/haptic" },
    { hapticAction, "/user/hand/right/output/haptic" }
  });

  // Simple Controller のバインディング (フォールバック用)
  suggestBindings("/interaction_profiles/khr/simple_controller", {
    { aimPoseAction, "/user/hand/left/input/aim/pose" },
    { aimPoseAction, "/user/hand/right/input/aim/pose" },
    { gripPoseAction, "/user/hand/left/input/grip/pose" },
    { gripPoseAction, "/user/hand/right/input/grip/pose" },
    { triggerAction, "/user/hand/left/input/select/click" },
    { triggerAction, "/user/hand/right/input/select/click" },
    { menuButtonAction, "/user/hand/left/input/menu/click" },
    { menuButtonAction, "/user/hand/right/input/menu/click" },
    { hapticAction, "/user/hand/left/output/haptic" },
    { hapticAction, "/user/hand/right/output/haptic" }
  });

  // HTC Vive コントローラーのバインディング
  suggestBindings("/interaction_profiles/htc/vive_controller", {
    { aimPoseAction, "/user/hand/left/input/aim/pose" },
    { aimPoseAction, "/user/hand/right/input/aim/pose" },
    { gripPoseAction, "/user/hand/left/input/grip/pose" },
    { gripPoseAction, "/user/hand/right/input/grip/pose" },
    { triggerAction, "/user/hand/left/input/trigger/value" },
    { triggerAction, "/user/hand/right/input/trigger/value" },
    { gripAction, "/user/hand/left/input/squeeze/click" },
    { gripAction, "/user/hand/right/input/squeeze/click" },
    { thumbstickAction, "/user/hand/left/input/trackpad" },
    { thumbstickAction, "/user/hand/right/input/trackpad" },
    { thumbstickClickAction, "/user/hand/left/input/trackpad/click" },
    { thumbstickClickAction, "/user/hand/right/input/trackpad/click" },
    { menuButtonAction, "/user/hand/left/input/menu/click" },
    { menuButtonAction, "/user/hand/right/input/menu/click" },
    { hapticAction, "/user/hand/left/output/haptic" },
    { hapticAction, "/user/hand/right/output/haptic" }
  });

  // Valve Index コントローラーのバインディング
  suggestBindings("/interaction_profiles/valve/index_controller", {
    { aimPoseAction, "/user/hand/left/input/aim/pose" },
    { aimPoseAction, "/user/hand/right/input/aim/pose" },
    { gripPoseAction, "/user/hand/left/input/grip/pose" },
    { gripPoseAction, "/user/hand/right/input/grip/pose" },
    { triggerAction, "/user/hand/left/input/trigger/value" },
    { triggerAction, "/user/hand/right/input/trigger/value" },
    { gripAction, "/user/hand/left/input/squeeze/value" },
    { gripAction, "/user/hand/right/input/squeeze/value" },
    { thumbstickAction, "/user/hand/left/input/thumbstick" },
    { thumbstickAction, "/user/hand/right/input/thumbstick" },
    { thumbstickClickAction, "/user/hand/left/input/thumbstick/click" },
    { thumbstickClickAction, "/user/hand/right/input/thumbstick/click" },
    { primaryButtonAction, "/user/hand/left/input/a/click" },
    { primaryButtonAction, "/user/hand/right/input/a/click" },
    { secondaryButtonAction, "/user/hand/left/input/b/click" },
    { secondaryButtonAction, "/user/hand/right/input/b/click" },
    { hapticAction, "/user/hand/left/output/haptic" },
    { hapticAction, "/user/hand/right/output/haptic" }
  });

  // セッションにアクションセットをアタッチ
  XrSessionActionSetsAttachInfo attachInfo{ XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
  attachInfo.countActionSets = 1;
  attachInfo.actionSets = &actionSet;
  xrAttachSessionActionSets(session, &attachInfo);
}

//
// アクション状態を更新する
//
void GgApp::OpenXR::pollActions()
{
  if (!isSessionRunning || sessionState < XR_SESSION_STATE_VISIBLE || actionSet == XR_NULL_HANDLE) return;

  XrActiveActionSet activeActionSet{ actionSet, XR_NULL_PATH };
  XrActionsSyncInfo syncInfo{ XR_TYPE_ACTIONS_SYNC_INFO };
  syncInfo.countActiveActionSets = 1;
  syncInfo.activeActionSets = &activeActionSet;
  if (XR_FAILED(xrSyncActions(session, &syncInfo))) return;

  for (int i = 0; i < Hand::Count; ++i)
  {
    auto& state = controllerStates[i];
    XrPath subaction = handSubactionPath[i];

    // グリップポーズの取得
    XrActionStateGetInfo getInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
    getInfo.subactionPath = subaction;

    XrActionStatePose gripPoseState{ XR_TYPE_ACTION_STATE_POSE };
    getInfo.action = gripPoseAction;
    xrGetActionStatePose(session, &getInfo, &gripPoseState);
    state.isTracked = gripPoseState.isActive;

    if (state.isTracked && gripSpace[i] != XR_NULL_HANDLE)
    {
      XrSpaceLocation location{ XR_TYPE_SPACE_LOCATION };
      xrLocateSpace(gripSpace[i], appSpace, frameState.predictedDisplayTime, &location);
      if (location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
      {
        state.gripPose = location.pose;
      }
    }

    // エイムポーズの取得
    if (state.isTracked && aimSpace[i] != XR_NULL_HANDLE)
    {
      XrSpaceLocation location{ XR_TYPE_SPACE_LOCATION };
      xrLocateSpace(aimSpace[i], appSpace, frameState.predictedDisplayTime, &location);
      if (location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
      {
        state.aimPose = location.pose;
      }
    }

    // トリガー
    XrActionStateFloat triggerState{ XR_TYPE_ACTION_STATE_FLOAT };
    getInfo.action = triggerAction;
    if (XR_SUCCEEDED(xrGetActionStateFloat(session, &getInfo, &triggerState)) && triggerState.isActive)
      state.trigger = triggerState.currentState;
    else
      state.trigger = 0.0f;

    // グリップ
    XrActionStateFloat gripState{ XR_TYPE_ACTION_STATE_FLOAT };
    getInfo.action = gripAction;
    if (XR_SUCCEEDED(xrGetActionStateFloat(session, &getInfo, &gripState)) && gripState.isActive)
      state.grip = gripState.currentState;
    else
      state.grip = 0.0f;

    // スティック
    XrActionStateVector2f thumbstickState{ XR_TYPE_ACTION_STATE_VECTOR2F };
    getInfo.action = thumbstickAction;
    if (XR_SUCCEEDED(xrGetActionStateVector2f(session, &getInfo, &thumbstickState)) && thumbstickState.isActive)
      state.thumbstick = { thumbstickState.currentState.x, thumbstickState.currentState.y };
    else
      state.thumbstick = { 0.0f, 0.0f };

    // スティッククリック
    XrActionStateBoolean clickState{ XR_TYPE_ACTION_STATE_BOOLEAN };
    getInfo.action = thumbstickClickAction;
    if (XR_SUCCEEDED(xrGetActionStateBoolean(session, &getInfo, &clickState)) && clickState.isActive)
      state.thumbstickClick = clickState.currentState != XR_FALSE;
    else
      state.thumbstickClick = false;

    // プライマリボタン
    XrActionStateBoolean primState{ XR_TYPE_ACTION_STATE_BOOLEAN };
    getInfo.action = primaryButtonAction;
    if (XR_SUCCEEDED(xrGetActionStateBoolean(session, &getInfo, &primState)) && primState.isActive)
      state.primaryButton = primState.currentState != XR_FALSE;
    else
      state.primaryButton = false;

    // セカンダリボタン
    XrActionStateBoolean secState{ XR_TYPE_ACTION_STATE_BOOLEAN };
    getInfo.action = secondaryButtonAction;
    if (XR_SUCCEEDED(xrGetActionStateBoolean(session, &getInfo, &secState)) && secState.isActive)
      state.secondaryButton = secState.currentState != XR_FALSE;
    else
      state.secondaryButton = false;

    // メニューボタン
    XrActionStateBoolean menuState{ XR_TYPE_ACTION_STATE_BOOLEAN };
    getInfo.action = menuButtonAction;
    if (XR_SUCCEEDED(xrGetActionStateBoolean(session, &getInfo, &menuState)) && menuState.isActive)
      state.menuButton = menuState.currentState != XR_FALSE;
    else
      state.menuButton = false;
  }
}

//
// OpenXR のセッションを作成する
//
GgApp::OpenXR& GgApp::OpenXR::initialize(const Window& window, XrReferenceSpaceType spaceType)
{
  static OpenXR openxr;

  if (openxr.instance != XR_NULL_HANDLE) return openxr;

  openxr.window = &window;
  openxr.referenceSpaceType = spaceType;

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
  if (pfnGetOpenGLGraphicsRequirementsKHR)
  {
    pfnGetOpenGLGraphicsRequirementsKHR(openxr.instance, openxr.systemId, &graphicsRequirements);
  }

#if defined(_WIN32)
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

  // Create Reference Space (要求されたスペースタイプを作成、失敗した場合は LOCAL にフォールバック)
  XrReferenceSpaceCreateInfo spaceCreateInfo{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
  spaceCreateInfo.referenceSpaceType = openxr.referenceSpaceType;
  spaceCreateInfo.poseInReferenceSpace.orientation.w = 1.0f;
  if (XR_FAILED(xrCreateReferenceSpace(openxr.session, &spaceCreateInfo, &openxr.appSpace)))
  {
    openxr.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
    spaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
    if (XR_FAILED(xrCreateReferenceSpace(openxr.session, &spaceCreateInfo, &openxr.appSpace)))
      throw std::runtime_error("Can't create OpenXR reference space");
  }

  // アクションシステムの初期化
  openxr.initActions();

  // Enumerate views and create swapchains
  uint32_t viewCount = 0;
  xrEnumerateViewConfigurationViews(openxr.instance, openxr.systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &viewCount, nullptr);
  openxr.views.resize(viewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
  xrEnumerateViewConfigurationViews(openxr.instance, openxr.systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, viewCount, &viewCount, openxr.views.data());

  openxr.viewStates.resize(viewCount, { XR_TYPE_VIEW });

  glGenFramebuffers(2, openxr.openxrFbo);
  glGenTextures(2, openxr.openxrDepth);

  for (uint32_t i = 0; i < viewCount; ++i)
  {
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
  for (int i = 0; i < Hand::Count; ++i)
  {
    if (aimSpace[i] != XR_NULL_HANDLE) { xrDestroySpace(aimSpace[i]); aimSpace[i] = XR_NULL_HANDLE; }
    if (gripSpace[i] != XR_NULL_HANDLE) { xrDestroySpace(gripSpace[i]); gripSpace[i] = XR_NULL_HANDLE; }
  }

  if (actionSet != XR_NULL_HANDLE)
  {
    if (aimPoseAction != XR_NULL_HANDLE) xrDestroyAction(aimPoseAction);
    if (gripPoseAction != XR_NULL_HANDLE) xrDestroyAction(gripPoseAction);
    if (triggerAction != XR_NULL_HANDLE) xrDestroyAction(triggerAction);
    if (gripAction != XR_NULL_HANDLE) xrDestroyAction(gripAction);
    if (thumbstickAction != XR_NULL_HANDLE) xrDestroyAction(thumbstickAction);
    if (thumbstickClickAction != XR_NULL_HANDLE) xrDestroyAction(thumbstickClickAction);
    if (primaryButtonAction != XR_NULL_HANDLE) xrDestroyAction(primaryButtonAction);
    if (secondaryButtonAction != XR_NULL_HANDLE) xrDestroyAction(secondaryButtonAction);
    if (menuButtonAction != XR_NULL_HANDLE) xrDestroyAction(menuButtonAction);
    if (hapticAction != XR_NULL_HANDLE) xrDestroyAction(hapticAction);
    xrDestroyActionSet(actionSet);
    actionSet = XR_NULL_HANDLE;
  }

  if (session != XR_NULL_HANDLE)
  {
    for (auto swapchain : swapchains) xrDestroySwapchain(swapchain);
    swapchains.clear();
    swapchainImages.clear();
    if (appSpace != XR_NULL_HANDLE) { xrDestroySpace(appSpace); appSpace = XR_NULL_HANDLE; }
    xrDestroySession(session);
    session = XR_NULL_HANDLE;
  }
  if (instance != XR_NULL_HANDLE)
  {
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
  while (xrPollEvent(instance, &eventData) == XR_SUCCESS)
  {
    if (eventData.type == XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED)
    {
      auto* stateChanged = reinterpret_cast<XrEventDataSessionStateChanged*>(&eventData);
      sessionState = stateChanged->state;
      if (sessionState == XR_SESSION_STATE_READY)
      {
        XrSessionBeginInfo beginInfo{ XR_TYPE_SESSION_BEGIN_INFO };
        beginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
        xrBeginSession(session, &beginInfo);
        isSessionRunning = true;
      }
      else if (sessionState == XR_SESSION_STATE_STOPPING)
      {
        xrEndSession(session);
        isSessionRunning = false;
      }
      else if (sessionState == XR_SESSION_STATE_EXITING || sessionState == XR_SESSION_STATE_LOSS_PENDING)
      {
        if (window) window->setClose(GLFW_TRUE);
      }
    }
    eventData.type = XR_TYPE_EVENT_DATA_BUFFER;
  }

  if (!isSessionRunning || sessionState < XR_SESSION_STATE_VISIBLE) return false;

  XrFrameWaitInfo waitInfo{ XR_TYPE_FRAME_WAIT_INFO };
  frameState = { XR_TYPE_FRAME_STATE };
  xrWaitFrame(session, &waitInfo, &frameState);

  XrFrameBeginInfo beginInfo{ XR_TYPE_FRAME_BEGIN_INFO };
  xrBeginFrame(session, &beginInfo);

  if (frameState.shouldRender)
  {
    XrViewLocateInfo viewLocateInfo{ XR_TYPE_VIEW_LOCATE_INFO };
    viewLocateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    viewLocateInfo.displayTime = frameState.predictedDisplayTime;
    viewLocateInfo.space = appSpace;

    XrViewState viewState{ XR_TYPE_VIEW_STATE };
    uint32_t viewCount;
    xrLocateViews(session, &viewLocateInfo, &viewState, static_cast<uint32_t>(viewStates.size()), &viewCount, viewStates.data());

    pollActions();
    return true;
  }

  return false;
}

//
// 描画対象の目を指定してフレームバッファとビューポートを設定する
//
void GgApp::OpenXR::select(int eye)
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
}

//
// 描画対象の目を指定する (旧 LibOVR 仕様互換)
//
void GgApp::OpenXR::select(int eye, GLfloat* screen, GLfloat* position, GLfloat* orientation)
{
  select(eye);

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
// 描画した目のスワップチェーンイメージを解放する
//
void GgApp::OpenXR::commit(int eye)
{
  glBindFramebuffer(GL_FRAMEBUFFER, openxrFbo[eye]);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);

  XrSwapchainImageReleaseInfo releaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
  xrReleaseSwapchainImage(swapchains[eye], &releaseInfo);
}

//
// フレームを転送して HMD に表示する
//
bool GgApp::OpenXR::submit(bool mirror)
{
  if (frameState.shouldRender)
  {
    XrCompositionLayerProjectionView projectionViews[2];
    for (int eye = 0; eye < 2; ++eye)
    {
      projectionViews[eye].type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
      projectionViews[eye].next = nullptr;
      projectionViews[eye].pose = viewStates[eye].pose;
      projectionViews[eye].fov = viewStates[eye].fov;
      projectionViews[eye].subImage.swapchain = swapchains[eye];
      projectionViews[eye].subImage.imageRect.offset = { 0, 0 };
      projectionViews[eye].subImage.imageRect.extent = {
        static_cast<int32_t>(views[eye].recommendedImageRectWidth),
        static_cast<int32_t>(views[eye].recommendedImageRectHeight)
      };
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
    endInfo.layerCount = 1;
    endInfo.layers = &layers;

    xrEndFrame(session, &endInfo);

    if (mirror && window)
    {
      GLsizei size[2];
      window->getSize(size);
      glBindFramebuffer(GL_READ_FRAMEBUFFER, openxrFbo[0]);
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
      glBlitFramebuffer(0, 0, views[0].recommendedImageRectWidth, views[0].recommendedImageRectHeight,
                        0, 0, size[0], size[1], GL_COLOR_BUFFER_BIT, GL_NEAREST);
      glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
      glFlush();
    }
  }
  else
  {
    XrFrameEndInfo endInfo{ XR_TYPE_FRAME_END_INFO };
    endInfo.displayTime = frameState.predictedDisplayTime;
    endInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    endInfo.layerCount = 0;
    endInfo.layers = nullptr;

    xrEndFrame(session, &endInfo);
  }

  return true;
}

//
// 指定した目の透視投影変換行列を取得する
//
GgMatrix GgApp::OpenXR::getProjectionMatrix(int eye, GLfloat zNear, GLfloat zFar) const
{
  assert(eye >= 0 && eye < static_cast<int>(viewStates.size()));
  const auto& fov{ viewStates[eye].fov };
  const GLfloat l{ tanf(fov.angleLeft) * zNear };
  const GLfloat r{ tanf(fov.angleRight) * zNear };
  const GLfloat b{ tanf(fov.angleDown) * zNear };
  const GLfloat t{ tanf(fov.angleUp) * zNear };
  return ggFrustum(l, r, b, t, zNear, zFar);
}

//
// 指定した目のビュー変換行列を取得する
//
GgMatrix GgApp::OpenXR::getViewMatrix(int eye) const
{
  assert(eye >= 0 && eye < static_cast<int>(viewStates.size()));
  const auto& pose{ viewStates[eye].pose };
  const GgQuaternion q{ pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w };
  return q.getConjugateMatrix() * ggTranslate(-pose.position.x, -pose.position.y, -pose.position.z);
}

//
// 指定した目の姿勢行列を取得する
//
GgMatrix GgApp::OpenXR::getPoseMatrix(int eye) const
{
  assert(eye >= 0 && eye < static_cast<int>(viewStates.size()));
  const auto& pose{ viewStates[eye].pose };
  const GgQuaternion q{ pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w };
  return ggTranslate(pose.position.x, pose.position.y, pose.position.z) * q.getMatrix();
}

//
// 指定した目の視点位置を取得する
//
GgVector GgApp::OpenXR::getPosition(int eye) const
{
  assert(eye >= 0 && eye < static_cast<int>(viewStates.size()));
  const auto& pos{ viewStates[eye].pose.position };
  return GgVector{ pos.x, pos.y, pos.z, 1.0f };
}

//
// 指定した目の視線方向の回転四元数を取得する
//
GgQuaternion GgApp::OpenXR::getOrientation(int eye) const
{
  assert(eye >= 0 && eye < static_cast<int>(viewStates.size()));
  const auto& ori{ viewStates[eye].pose.orientation };
  return GgQuaternion{ ori.x, ori.y, ori.z, ori.w };
}

//
// 指定した目の視野角情報 (XrFovf) を取得する
//
const XrFovf& GgApp::OpenXR::getFov(int eye) const
{
  assert(eye >= 0 && eye < static_cast<int>(viewStates.size()));
  return viewStates[eye].fov;
}

//
// 指定した目の姿勢情報 (XrPosef) を取得する
//
const XrPosef& GgApp::OpenXR::getPose(int eye) const
{
  assert(eye >= 0 && eye < static_cast<int>(viewStates.size()));
  return viewStates[eye].pose;
}

//
// レンダリング推奨解像度の横幅を取得する
//
GLsizei GgApp::OpenXR::getWidth(int eye) const
{
  assert(eye >= 0 && eye < static_cast<int>(views.size()));
  return views[eye].recommendedImageRectWidth;
}

//
// レンダリング推奨解像度の高さを取得する
//
GLsizei GgApp::OpenXR::getHeight(int eye) const
{
  assert(eye >= 0 && eye < static_cast<int>(views.size()));
  return views[eye].recommendedImageRectHeight;
}

//
// アスペクト比を取得する
//
GLfloat GgApp::OpenXR::getAspect(int eye) const
{
  assert(eye >= 0 && eye < static_cast<int>(views.size()));
  return static_cast<GLfloat>(views[eye].recommendedImageRectWidth) / static_cast<GLfloat>(views[eye].recommendedImageRectHeight);
}

//
// ビューの総数を取得する
//
uint32_t GgApp::OpenXR::getViewCount() const
{
  return static_cast<uint32_t>(views.size());
}

//
// 現在の参照空間タイプを取得する
//
XrReferenceSpaceType GgApp::OpenXR::getReferenceSpaceType() const
{
  return referenceSpaceType;
}

//
// コントローラーがトラッキングされているか取得する
//
bool GgApp::OpenXR::isTracked(int hand) const
{
  assert(hand >= 0 && hand < Hand::Count);
  return controllerStates[hand].isTracked;
}

//
// コントローラーのグリップ変換行列を取得する
//
GgMatrix GgApp::OpenXR::getGripMatrix(int hand) const
{
  assert(hand >= 0 && hand < Hand::Count);
  const auto& pose = controllerStates[hand].gripPose;
  const GgQuaternion q{ pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w };
  return ggTranslate(pose.position.x, pose.position.y, pose.position.z) * q.getMatrix();
}

//
// コントローラーのエイム変換行列を取得する
//
GgMatrix GgApp::OpenXR::getAimMatrix(int hand) const
{
  assert(hand >= 0 && hand < Hand::Count);
  const auto& pose = controllerStates[hand].aimPose;
  const GgQuaternion q{ pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w };
  return ggTranslate(pose.position.x, pose.position.y, pose.position.z) * q.getMatrix();
}

//
// コントローラーのグリップ位置を取得する
//
GgVector GgApp::OpenXR::getGripPosition(int hand) const
{
  assert(hand >= 0 && hand < Hand::Count);
  const auto& pos = controllerStates[hand].gripPose.position;
  return GgVector{ pos.x, pos.y, pos.z, 1.0f };
}

//
// コントローラーのグリップ回転四元数を取得する
//
GgQuaternion GgApp::OpenXR::getGripOrientation(int hand) const
{
  assert(hand >= 0 && hand < Hand::Count);
  const auto& ori = controllerStates[hand].gripPose.orientation;
  return GgQuaternion{ ori.x, ori.y, ori.z, ori.w };
}

//
// コントローラーのエイム位置を取得する
//
GgVector GgApp::OpenXR::getAimPosition(int hand) const
{
  assert(hand >= 0 && hand < Hand::Count);
  const auto& pos = controllerStates[hand].aimPose.position;
  return GgVector{ pos.x, pos.y, pos.z, 1.0f };
}

//
// コントローラーのエイム回転四元数を取得する
//
GgQuaternion GgApp::OpenXR::getAimOrientation(int hand) const
{
  assert(hand >= 0 && hand < Hand::Count);
  const auto& ori = controllerStates[hand].aimPose.orientation;
  return GgQuaternion{ ori.x, ori.y, ori.z, ori.w };
}

//
// トリガーの押し込み量を取得する
//
float GgApp::OpenXR::getTrigger(int hand) const
{
  assert(hand >= 0 && hand < Hand::Count);
  return controllerStates[hand].trigger;
}

//
// グリップの押し込み量を取得する
//
float GgApp::OpenXR::getGrip(int hand) const
{
  assert(hand >= 0 && hand < Hand::Count);
  return controllerStates[hand].grip;
}

//
// アナログスティックの入力値を取得する
//
std::array<float, 2> GgApp::OpenXR::getThumbstick(int hand) const
{
  assert(hand >= 0 && hand < Hand::Count);
  return controllerStates[hand].thumbstick;
}

//
// アナログスティックのクリック状態を取得する
//
bool GgApp::OpenXR::getThumbstickClick(int hand) const
{
  assert(hand >= 0 && hand < Hand::Count);
  return controllerStates[hand].thumbstickClick;
}

//
// プライマリボタンの押下状態を取得する
//
bool GgApp::OpenXR::getPrimaryButton(int hand) const
{
  assert(hand >= 0 && hand < Hand::Count);
  return controllerStates[hand].primaryButton;
}

//
// セカンダリボタンの押下状態を取得する
//
bool GgApp::OpenXR::getSecondaryButton(int hand) const
{
  assert(hand >= 0 && hand < Hand::Count);
  return controllerStates[hand].secondaryButton;
}

//
// メニューボタンの押下状態を取得する
//
bool GgApp::OpenXR::getMenuButton(int hand) const
{
  assert(hand >= 0 && hand < Hand::Count);
  return controllerStates[hand].menuButton;
}

//
// コントローラーに振動を出力する
//
void GgApp::OpenXR::applyHapticVibration(int hand, float durationSeconds, float frequency, float amplitude)
{
  assert(hand >= 0 && hand < Hand::Count);
  if (session == XR_NULL_HANDLE || hapticAction == XR_NULL_HANDLE) return;

  XrHapticVibration vibration{ XR_TYPE_HAPTIC_VIBRATION };
  vibration.duration = static_cast<XrDuration>(durationSeconds * 1e9f);
  vibration.frequency = frequency;
  vibration.amplitude = amplitude;

  XrHapticActionInfo actionInfo{ XR_TYPE_HAPTIC_ACTION_INFO };
  actionInfo.action = hapticAction;
  actionInfo.subactionPath = handSubactionPath[hand];

  xrApplyHapticFeedback(session, &actionInfo, reinterpret_cast<const XrHapticBaseHeader*>(&vibration));
}

#endif

#if defined(_WIN32)
#  if !defined(_INC_WINDOWS) && !defined(_WINDOWS_)
#    include <windows.h>
#  endif
#else
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
#if defined(_WIN32)
    std::getenv("USERNAME")
#else
    std::getenv("USER")
#endif
  };

  // 環境変数からユーザ名が得られたらそれを返す
  if (user) return user;

#if defined(_WIN32)
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
