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

namespace
{
  //
  // OpenXR の関数の戻り値を文字列にする
  //
  std::string xrMessage(XrInstance instance, XrResult result, const std::string& message)
  {
    char buffer[XR_MAX_RESULT_STRING_SIZE]{ '\0' };
    if (instance == XR_NULL_HANDLE
      || XR_FAILED(xrResultToString(instance, result, buffer))
      || buffer[0] == '\0')
    {
      std::snprintf(buffer, sizeof buffer, "XrResult(%d)", static_cast<int>(result));
    }
    return message + ": " + buffer;
  }

  //
  // OpenXR の関数の戻り値を検査して, エラーなら例外を投げる
  //
  void xrCheck(XrInstance instance, XrResult result, const std::string& message)
  {
    if (XR_SUCCEEDED(result)) return;
    throw std::runtime_error(xrMessage(instance, result, message));
  }

  //
  // OpenXR の関数の戻り値を検査して, エラーなら標準エラー出力に報告する
  //
  bool xrWarn(XrInstance instance, XrResult result, const std::string& message)
  {
    if (XR_SUCCEEDED(result)) return true;
    std::cerr << "OpenXR: " << xrMessage(instance, result, message) << '\n';
    return false;
  }

  //
  // 固定長の文字列に安全にコピーする
  //
  void xrCopyString(char* destination, size_t size, const char* source)
  {
    if (size == 0) return;
    const auto length{ std::min(std::strlen(source), size - 1) };
    std::memcpy(destination, source, length);
    destination[length] = '\0';
  }
}

//
// コンストラクタ
//
GgApp::OpenXR::OpenXR()
{
}

//
// デストラクタ
//
GgApp::OpenXR::~OpenXR()
{
  // このオブジェクトは関数内 static なので, 破棄されるのは main() が
  // 終了した後, すなわち OpenGL のコンテキストが失われた後である.
  // したがってここでは OpenGL の資源には触れず, OpenXR のハンドルだけを
  // 解放する (OpenGL の資源は terminate() で解放しておくこと).
  destroyXr();
}

//
// アクションシステムを初期化する
//
void GgApp::OpenXR::initActions()
{
  // アクションセットの作成
  XrActionSetCreateInfo actionSetInfo{ XR_TYPE_ACTION_SET_CREATE_INFO };
  xrCopyString(actionSetInfo.actionSetName, sizeof actionSetInfo.actionSetName, "gameplay");
  xrCopyString(actionSetInfo.localizedActionSetName, sizeof actionSetInfo.localizedActionSetName, "Gameplay");
  actionSetInfo.priority = 0;
  xrCheck(instance, xrCreateActionSet(instance, &actionSetInfo, &actionSet),
    "Can't create the OpenXR action set");

  // サブアクションパスの取得
  xrCheck(instance, xrStringToPath(instance, "/user/hand/left", &handSubactionPath[Hand::Left]),
    "Can't convert the path of the left hand");
  xrCheck(instance, xrStringToPath(instance, "/user/hand/right", &handSubactionPath[Hand::Right]),
    "Can't convert the path of the right hand");

  // アクション作成ヘルパー
  auto createAction = [this](const char* name, const char* localizedName, XrActionType type, XrAction& action)
  {
    XrActionCreateInfo createInfo{ XR_TYPE_ACTION_CREATE_INFO };
    xrCopyString(createInfo.actionName, sizeof createInfo.actionName, name);
    xrCopyString(createInfo.localizedActionName, sizeof createInfo.localizedActionName, localizedName);
    createInfo.actionType = type;
    createInfo.countSubactionPaths = Hand::Count;
    createInfo.subactionPaths = handSubactionPath;
    xrCheck(instance, xrCreateAction(actionSet, &createInfo, &action),
      std::string("Can't create the OpenXR action \"") + name + "\"");
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

  // バインディング設定ヘルパー (対応していない対話プロファイルは読み飛ばす)
  auto suggestBindings = [this](const char* profileStr, const std::vector<std::pair<XrAction, const char*>>& bindings)
  {
    XrPath profilePath{ XR_NULL_PATH };
    if (XR_FAILED(xrStringToPath(instance, profileStr, &profilePath))) return;

    std::vector<XrActionSuggestedBinding> suggestedBindings;
    suggestedBindings.reserve(bindings.size());
    for (const auto& [action, pathStr] : bindings)
    {
      XrPath path{ XR_NULL_PATH };
      if (XR_FAILED(xrStringToPath(instance, pathStr, &path))) continue;
      suggestedBindings.push_back(XrActionSuggestedBinding{ action, path });
    }
    if (suggestedBindings.empty()) return;

    XrInteractionProfileSuggestedBinding profileSuggestedBindings{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
    profileSuggestedBindings.interactionProfile = profilePath;
    profileSuggestedBindings.suggestedBindings = suggestedBindings.data();
    profileSuggestedBindings.countSuggestedBindings = static_cast<uint32_t>(suggestedBindings.size());
    xrWarn(instance, xrSuggestInteractionProfileBindings(instance, &profileSuggestedBindings),
      std::string("Can't suggest the bindings for ") + profileStr);
  };

  // Simple Controller のバインディング (すべてのランタイムが対応する最小限のもの)
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

  // Meta (Oculus) Touch コントローラーのバインディング
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

  // Microsoft Mixed Reality モーションコントローラーのバインディング
  suggestBindings("/interaction_profiles/microsoft/motion_controller", {
    { aimPoseAction, "/user/hand/left/input/aim/pose" },
    { aimPoseAction, "/user/hand/right/input/aim/pose" },
    { gripPoseAction, "/user/hand/left/input/grip/pose" },
    { gripPoseAction, "/user/hand/right/input/grip/pose" },
    { triggerAction, "/user/hand/left/input/trigger/value" },
    { triggerAction, "/user/hand/right/input/trigger/value" },
    { gripAction, "/user/hand/left/input/squeeze/click" },
    { gripAction, "/user/hand/right/input/squeeze/click" },
    { thumbstickAction, "/user/hand/left/input/thumbstick" },
    { thumbstickAction, "/user/hand/right/input/thumbstick" },
    { thumbstickClickAction, "/user/hand/left/input/thumbstick/click" },
    { thumbstickClickAction, "/user/hand/right/input/thumbstick/click" },
    { menuButtonAction, "/user/hand/left/input/menu/click" },
    { menuButtonAction, "/user/hand/right/input/menu/click" },
    { hapticAction, "/user/hand/left/output/haptic" },
    { hapticAction, "/user/hand/right/output/haptic" }
  });

  // セッションにアクションセットをアタッチする (これ以降はバインディングを追加できない)
  XrSessionActionSetsAttachInfo attachInfo{ XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
  attachInfo.countActionSets = 1;
  attachInfo.actionSets = &actionSet;
  xrCheck(instance, xrAttachSessionActionSets(session, &attachInfo),
    "Can't attach the OpenXR action set to the session");

  // アクションスペースの作成
  for (int i = 0; i < Hand::Count; ++i)
  {
    XrActionSpaceCreateInfo spaceInfo{ XR_TYPE_ACTION_SPACE_CREATE_INFO };
    spaceInfo.poseInActionSpace.orientation.w = 1.0f;
    spaceInfo.subactionPath = handSubactionPath[i];

    spaceInfo.action = aimPoseAction;
    xrCheck(instance, xrCreateActionSpace(session, &spaceInfo, &aimSpace[i]),
      "Can't create the OpenXR action space for the aim pose");

    spaceInfo.action = gripPoseAction;
    xrCheck(instance, xrCreateActionSpace(session, &spaceInfo, &gripSpace[i]),
      "Can't create the OpenXR action space for the grip pose");
  }
}

//
// アクション状態を更新する
//
void GgApp::OpenXR::pollActions()
{
  // 入力を受け付けていなければコントローラーの状態を無効にする
  if (!isSessionRunning || actionSet == XR_NULL_HANDLE || sessionState != XR_SESSION_STATE_FOCUSED)
  {
    for (auto& state : controllerStates) state = ControllerState{};
    return;
  }

  XrActiveActionSet activeActionSet{ actionSet, XR_NULL_PATH };
  XrActionsSyncInfo syncInfo{ XR_TYPE_ACTIONS_SYNC_INFO };
  syncInfo.countActiveActionSets = 1;
  syncInfo.activeActionSets = &activeActionSet;
  if (!xrWarn(instance, xrSyncActions(session, &syncInfo),
    "Can't synchronize the OpenXR actions")) return;

  // 空間の姿勢を取り出すヘルパー (位置と向きの両方が有効なときだけ更新する)
  auto locate = [this](XrSpace space, XrPosef& pose)
  {
    if (space == XR_NULL_HANDLE) return false;

    XrSpaceLocation location{ XR_TYPE_SPACE_LOCATION };
    if (XR_FAILED(xrLocateSpace(space, appSpace, frameState.predictedDisplayTime, &location)))
      return false;

    constexpr XrSpaceLocationFlags valid
    {
      XR_SPACE_LOCATION_POSITION_VALID_BIT | XR_SPACE_LOCATION_ORIENTATION_VALID_BIT
    };
    if ((location.locationFlags & valid) != valid) return false;

    pose = location.pose;
    return true;
  };

  for (int i = 0; i < Hand::Count; ++i)
  {
    auto& state = controllerStates[i];
    const XrPath subaction = handSubactionPath[i];

    XrActionStateGetInfo getInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
    getInfo.subactionPath = subaction;

    // グリップポーズの取得
    XrActionStatePose gripPoseState{ XR_TYPE_ACTION_STATE_POSE };
    getInfo.action = gripPoseAction;
    const bool gripActive
    {
      XR_SUCCEEDED(xrGetActionStatePose(session, &getInfo, &gripPoseState))
        && gripPoseState.isActive != XR_FALSE
    };

    // エイムポーズの取得
    XrActionStatePose aimPoseState{ XR_TYPE_ACTION_STATE_POSE };
    getInfo.action = aimPoseAction;
    const bool aimActive
    {
      XR_SUCCEEDED(xrGetActionStatePose(session, &getInfo, &aimPoseState))
        && aimPoseState.isActive != XR_FALSE
    };

    // どちらかの姿勢が有効ならコントローラーが接続されている
    state.isTracked = gripActive || aimActive;
    if (gripActive) locate(gripSpace[i], state.gripPose);
    if (aimActive) locate(aimSpace[i], state.aimPose);

    // 連続値のアクションを取り出すヘルパー
    auto getFloat = [this, &getInfo](XrAction action, float& value)
    {
      getInfo.action = action;
      XrActionStateFloat floatState{ XR_TYPE_ACTION_STATE_FLOAT };
      value = XR_SUCCEEDED(xrGetActionStateFloat(session, &getInfo, &floatState))
        && floatState.isActive != XR_FALSE ? floatState.currentState : 0.0f;
    };

    // 論理値のアクションを取り出すヘルパー
    auto getBoolean = [this, &getInfo](XrAction action, bool& value)
    {
      getInfo.action = action;
      XrActionStateBoolean booleanState{ XR_TYPE_ACTION_STATE_BOOLEAN };
      value = XR_SUCCEEDED(xrGetActionStateBoolean(session, &getInfo, &booleanState))
        && booleanState.isActive != XR_FALSE && booleanState.currentState != XR_FALSE;
    };

    // トリガーとグリップ
    getFloat(triggerAction, state.trigger);
    getFloat(gripAction, state.grip);

    // スティック
    getInfo.action = thumbstickAction;
    XrActionStateVector2f thumbstickState{ XR_TYPE_ACTION_STATE_VECTOR2F };
    if (XR_SUCCEEDED(xrGetActionStateVector2f(session, &getInfo, &thumbstickState))
      && thumbstickState.isActive != XR_FALSE)
      state.thumbstick = { thumbstickState.currentState.x, thumbstickState.currentState.y };
    else
      state.thumbstick = { 0.0f, 0.0f };

    // ボタン
    getBoolean(thumbstickClickAction, state.thumbstickClick);
    getBoolean(primaryButtonAction, state.primaryButton);
    getBoolean(secondaryButtonAction, state.secondaryButton);
    getBoolean(menuButtonAction, state.menuButton);
  }
}

//
// OpenXR のイベントを処理する
//
void GgApp::OpenXR::pollEvents()
{
  XrEventDataBuffer eventData{ XR_TYPE_EVENT_DATA_BUFFER };

  while (xrPollEvent(instance, &eventData) == XR_SUCCESS)
  {
    switch (eventData.type)
    {
    case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:

      // OpenXR のインスタンスが失われるのでアプリケーションを終了する
      isSessionRunning = false;
      if (window) window->setClose(GLFW_TRUE);
      break;

    case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
    {
      const auto* stateChanged{ reinterpret_cast<const XrEventDataSessionStateChanged*>(&eventData) };
      sessionState = stateChanged->state;

      switch (sessionState)
      {
      case XR_SESSION_STATE_READY:
      {
        // セッションを開始する
        XrSessionBeginInfo beginInfo{ XR_TYPE_SESSION_BEGIN_INFO };
        beginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
        if (xrWarn(instance, xrBeginSession(session, &beginInfo),
          "Can't begin the OpenXR session")) isSessionRunning = true;
        break;
      }

      case XR_SESSION_STATE_STOPPING:

        // セッションを終了する
        isSessionRunning = false;
        frameBegun = false;
        xrWarn(instance, xrEndSession(session), "Can't end the OpenXR session");
        break;

      case XR_SESSION_STATE_EXITING:
      case XR_SESSION_STATE_LOSS_PENDING:

        // アプリケーションを終了する
        isSessionRunning = false;
        if (window) window->setClose(GLFW_TRUE);
        break;

      default:
        break;
      }
      break;
    }

    default:
      break;
    }

    // 次のイベントを取り出す準備をする
    eventData = XrEventDataBuffer{ XR_TYPE_EVENT_DATA_BUFFER };
  }
}

//
// スワップチェーンを作成する
//
void GgApp::OpenXR::createSwapchains()
{
  // ビュー構成を取得する
  uint32_t viewCount{ 0 };
  xrCheck(instance, xrEnumerateViewConfigurationViews(instance, systemId,
    XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &viewCount, nullptr),
    "Can't count the OpenXR view configuration views");
  views.assign(viewCount, XrViewConfigurationView{ XR_TYPE_VIEW_CONFIGURATION_VIEW });
  xrCheck(instance, xrEnumerateViewConfigurationViews(instance, systemId,
    XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, viewCount, &viewCount, views.data()),
    "Can't enumerate the OpenXR view configuration views");
  if (viewCount == 0) throw std::runtime_error("The OpenXR system has no view");

  viewStates.assign(viewCount, XrView{ XR_TYPE_VIEW });
  currentImageIndex.assign(viewCount, 0);
  imageAcquired.assign(viewCount, false);

  // 環境の合成方法を取得する (最初のものが最も推奨される)
  uint32_t blendModeCount{ 0 };
  if (XR_SUCCEEDED(xrEnumerateEnvironmentBlendModes(instance, systemId,
    XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &blendModeCount, nullptr))
    && blendModeCount > 0)
  {
    std::vector<XrEnvironmentBlendMode> blendModes(blendModeCount);
    if (XR_SUCCEEDED(xrEnumerateEnvironmentBlendModes(instance, systemId,
      XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, blendModeCount,
      &blendModeCount, blendModes.data())))
    {
      blendMode = blendModes[0];
    }
  }

  // 利用可能なスワップチェーンのカラーフォーマットを取得する
  uint32_t formatCount{ 0 };
  xrCheck(instance, xrEnumerateSwapchainFormats(session, 0, &formatCount, nullptr),
    "Can't count the OpenXR swapchain formats");
  std::vector<int64_t> formats(formatCount);
  xrCheck(instance, xrEnumerateSwapchainFormats(session, formatCount, &formatCount, formats.data()),
    "Can't enumerate the OpenXR swapchain formats");
  if (formats.empty()) throw std::runtime_error("The OpenXR runtime has no swapchain format");

  // 使用したいカラーフォーマットの候補 (前にあるものを優先する)
  static const int64_t preferred[]{ GL_SRGB8_ALPHA8, GL_SRGB8, GL_RGBA8, GL_RGB10_A2 };

  // 利用可能なカラーフォーマットの中から使用するものを選ぶ
  int64_t format{ formats[0] };
  for (const auto candidate : preferred)
  {
    if (std::find(formats.begin(), formats.end(), candidate) != formats.end())
    {
      format = candidate;
      break;
    }
  }

  // sRGB のフォーマットならリニア色空間で描画してガンマ補正をランタイムに任せる
  swapchainIsSrgb = format == GL_SRGB8_ALPHA8 || format == GL_SRGB8;

  // ビューの数だけ FBO とデプスバッファを作成する
  openxrFbo.assign(viewCount, 0);
  openxrDepth.assign(viewCount, 0);
  glGenFramebuffers(static_cast<GLsizei>(viewCount), openxrFbo.data());
  glGenRenderbuffers(static_cast<GLsizei>(viewCount), openxrDepth.data());

  for (uint32_t i = 0; i < viewCount; ++i)
  {
    // スワップチェーンを作成する
    XrSwapchainCreateInfo swapchainCreateInfo{ XR_TYPE_SWAPCHAIN_CREATE_INFO };
    swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT
      | XR_SWAPCHAIN_USAGE_SAMPLED_BIT;
    swapchainCreateInfo.format = format;
    swapchainCreateInfo.sampleCount = 1;
    swapchainCreateInfo.width = views[i].recommendedImageRectWidth;
    swapchainCreateInfo.height = views[i].recommendedImageRectHeight;
    swapchainCreateInfo.faceCount = 1;
    swapchainCreateInfo.arraySize = 1;
    swapchainCreateInfo.mipCount = 1;

    XrSwapchain swapchain{ XR_NULL_HANDLE };
    xrCheck(instance, xrCreateSwapchain(session, &swapchainCreateInfo, &swapchain),
      "Can't create the OpenXR swapchain");
    swapchains.push_back(swapchain);

    // スワップチェーンのイメージを取得する
    uint32_t imageCount{ 0 };
    xrCheck(instance, xrEnumerateSwapchainImages(swapchain, 0, &imageCount, nullptr),
      "Can't count the OpenXR swapchain images");
    std::vector<XrSwapchainImageOpenGLKHR> images(imageCount,
      XrSwapchainImageOpenGLKHR{ XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR });
    xrCheck(instance, xrEnumerateSwapchainImages(swapchain, imageCount, &imageCount,
      reinterpret_cast<XrSwapchainImageBaseHeader*>(images.data())),
      "Can't enumerate the OpenXR swapchain images");
    swapchainImages.push_back(std::move(images));

    // 隠面消去処理に使うデプスバッファを作成する
    glBindRenderbuffer(GL_RENDERBUFFER, openxrDepth[i]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
      static_cast<GLsizei>(swapchainCreateInfo.width),
      static_cast<GLsizei>(swapchainCreateInfo.height));
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // FBO にデプスバッファを取り付けておく (カラーバッファは select() で取り付ける)
    glBindFramebuffer(GL_FRAMEBUFFER, openxrFbo[i]);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
      GL_RENDERBUFFER, openxrDepth[i]);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//
// OpenXR のセッションを作成する
//
GgApp::OpenXR& GgApp::OpenXR::initialize(const Window& window,
  XrReferenceSpaceType spaceType, const char* appName)
{
  static OpenXR openxr;

  // 初期化済みならそのまま返す
  if (openxr.initialized) return openxr;

  // 初期化に失敗していた場合に備えて後始末をしておく
  openxr.terminate();

  openxr.window = &window;
  openxr.referenceSpaceType = spaceType;

  try
  {
    // 利用可能な拡張機能を調べる
    uint32_t extensionCount{ 0 };
    xrCheck(XR_NULL_HANDLE,
      xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr),
      "Can't count the OpenXR instance extensions");
    std::vector<XrExtensionProperties> extensionProperties(extensionCount,
      XrExtensionProperties{ XR_TYPE_EXTENSION_PROPERTIES });
    xrCheck(XR_NULL_HANDLE,
      xrEnumerateInstanceExtensionProperties(nullptr, extensionCount,
        &extensionCount, extensionProperties.data()),
      "Can't enumerate the OpenXR instance extensions");

    // OpenGL との連携に必要な拡張機能が使えなければあきらめる
    const auto found{ std::any_of(extensionProperties.begin(), extensionProperties.end(),
      [](const XrExtensionProperties& p)
      {
        return std::strcmp(p.extensionName, XR_KHR_OPENGL_ENABLE_EXTENSION_NAME) == 0;
      }) };
    if (!found)
    {
      throw std::runtime_error(
        "The OpenXR runtime does not support " XR_KHR_OPENGL_ENABLE_EXTENSION_NAME);
    }

    // XrInstance の作成
    XrInstanceCreateInfo createInfo{ XR_TYPE_INSTANCE_CREATE_INFO };
    xrCopyString(createInfo.applicationInfo.applicationName,
      sizeof createInfo.applicationInfo.applicationName, appName ? appName : "GgApp");
    createInfo.applicationInfo.applicationVersion = 1;
    xrCopyString(createInfo.applicationInfo.engineName,
      sizeof createInfo.applicationInfo.engineName, "GgApp");
    createInfo.applicationInfo.engineVersion = 1;
    createInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;

    const char* const extensions[]{ XR_KHR_OPENGL_ENABLE_EXTENSION_NAME };
    createInfo.enabledExtensionCount = 1;
    createInfo.enabledExtensionNames = extensions;

    xrCheck(XR_NULL_HANDLE, xrCreateInstance(&createInfo, &openxr.instance),
      "Can't create the OpenXR instance (is an OpenXR runtime installed and active?)");

    // システムの取得
    XrSystemGetInfo systemInfo{ XR_TYPE_SYSTEM_GET_INFO };
    systemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    xrCheck(openxr.instance, xrGetSystem(openxr.instance, &systemInfo, &openxr.systemId),
      "Can't get the OpenXR system (is the head mounted display connected?)");

    // システムの名前の取得
    XrSystemProperties systemProperties{ XR_TYPE_SYSTEM_PROPERTIES };
    if (XR_SUCCEEDED(xrGetSystemProperties(openxr.instance, openxr.systemId, &systemProperties)))
    {
      openxr.systemName = systemProperties.systemName;
    }

    // OpenGL との連携に必要な拡張機能の関数の取得
    PFN_xrGetOpenGLGraphicsRequirementsKHR pfnGetOpenGLGraphicsRequirementsKHR{ nullptr };
    xrCheck(openxr.instance, xrGetInstanceProcAddr(openxr.instance,
      "xrGetOpenGLGraphicsRequirementsKHR",
      reinterpret_cast<PFN_xrVoidFunction*>(&pfnGetOpenGLGraphicsRequirementsKHR)),
      "Can't get the address of xrGetOpenGLGraphicsRequirementsKHR");
    if (!pfnGetOpenGLGraphicsRequirementsKHR)
      throw std::runtime_error("Can't get the address of xrGetOpenGLGraphicsRequirementsKHR");

    // OpenGL の要件の取得 (セッションの作成前に必ず呼ばなければならない)
    XrGraphicsRequirementsOpenGLKHR graphicsRequirements{ XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR };
    xrCheck(openxr.instance, pfnGetOpenGLGraphicsRequirementsKHR(openxr.instance,
      openxr.systemId, &graphicsRequirements),
      "Can't get the OpenGL graphics requirements");

    // OpenGL のバージョンが要件を満たしているかどうか調べる
    GLint major{ 0 }, minor{ 0 };
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    if (XR_MAKE_VERSION(major, minor, 0) < graphicsRequirements.minApiVersionSupported)
    {
      char message[128];
      std::snprintf(message, sizeof message,
        "The OpenXR runtime requires OpenGL %d.%d or later, but %d.%d is current",
        static_cast<int>(XR_VERSION_MAJOR(graphicsRequirements.minApiVersionSupported)),
        static_cast<int>(XR_VERSION_MINOR(graphicsRequirements.minApiVersionSupported)),
        major, minor);
      throw std::runtime_error(message);
    }

    // OpenGL のコンテキストをセッションに結びつける
#if defined(XR_USE_PLATFORM_WIN32)
    XrGraphicsBindingOpenGLWin32KHR graphicsBinding{ XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR };
    graphicsBinding.hDC = wglGetCurrentDC();
    graphicsBinding.hGLRC = glfwGetWGLContext(window.get());
#elif defined(XR_USE_PLATFORM_XLIB)
    XrGraphicsBindingOpenGLXlibKHR graphicsBinding{ XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR };
    graphicsBinding.xDisplay = glfwGetX11Display();
    graphicsBinding.visualid = 0;
    graphicsBinding.glxFBConfig = nullptr;
    graphicsBinding.glxDrawable = glfwGetGLXWindow(window.get());
    graphicsBinding.glxContext = glfwGetGLXContext(window.get());
#else
#  error "GG_USE_OPENXR is not supported on this platform"
#endif

    // セッションの作成
    XrSessionCreateInfo sessionCreateInfo{ XR_TYPE_SESSION_CREATE_INFO };
    sessionCreateInfo.next = &graphicsBinding;
    sessionCreateInfo.systemId = openxr.systemId;
    xrCheck(openxr.instance, xrCreateSession(openxr.instance, &sessionCreateInfo, &openxr.session),
      "Can't create the OpenXR session");

    // 参照空間の作成 (要求されたものが使えなければ LOCAL にフォールバックする)
    XrReferenceSpaceCreateInfo spaceCreateInfo{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
    spaceCreateInfo.referenceSpaceType = openxr.referenceSpaceType;
    spaceCreateInfo.poseInReferenceSpace.orientation.w = 1.0f;
    if (XR_FAILED(xrCreateReferenceSpace(openxr.session, &spaceCreateInfo, &openxr.appSpace)))
    {
      openxr.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
      spaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
      xrCheck(openxr.instance,
        xrCreateReferenceSpace(openxr.session, &spaceCreateInfo, &openxr.appSpace),
        "Can't create the OpenXR reference space");
    }

    // アクションシステムの初期化
    openxr.initActions();

    // スワップチェーンの作成
    openxr.createSwapchains();
  }
  catch (...)
  {
    // 途中まで確保した資源を解放してから例外を投げ直す
    openxr.terminate();
    throw;
  }

  // OpenXR は xrWaitFrame() でフレームの表示速度を制御するので
  // ウィンドウ側の垂直同期の待ち合わせは行わない
  glfwSwapInterval(0);

  openxr.initialized = true;

  return openxr;
}

//
// OpenXR のハンドルを破棄する (OpenGL の資源には触れない)
//
void GgApp::OpenXR::destroyXr()
{
  for (int i = 0; i < Hand::Count; ++i)
  {
    if (aimSpace[i] != XR_NULL_HANDLE) { xrDestroySpace(aimSpace[i]); aimSpace[i] = XR_NULL_HANDLE; }
    if (gripSpace[i] != XR_NULL_HANDLE) { xrDestroySpace(gripSpace[i]); gripSpace[i] = XR_NULL_HANDLE; }
  }

  // アクションはアクションセットと一緒に破棄される
  if (actionSet != XR_NULL_HANDLE) xrDestroyActionSet(actionSet);
  actionSet = XR_NULL_HANDLE;
  aimPoseAction = gripPoseAction = XR_NULL_HANDLE;
  triggerAction = gripAction = XR_NULL_HANDLE;
  thumbstickAction = thumbstickClickAction = XR_NULL_HANDLE;
  primaryButtonAction = secondaryButtonAction = menuButtonAction = XR_NULL_HANDLE;
  hapticAction = XR_NULL_HANDLE;

  for (auto swapchain : swapchains) xrDestroySwapchain(swapchain);
  swapchains.clear();
  swapchainImages.clear();

  if (appSpace != XR_NULL_HANDLE) { xrDestroySpace(appSpace); appSpace = XR_NULL_HANDLE; }
  if (session != XR_NULL_HANDLE) { xrDestroySession(session); session = XR_NULL_HANDLE; }
  if (instance != XR_NULL_HANDLE) { xrDestroyInstance(instance); instance = XR_NULL_HANDLE; }

  systemId = XR_NULL_SYSTEM_ID;
  sessionState = XR_SESSION_STATE_UNKNOWN;
  isSessionRunning = false;
  frameBegun = false;
  viewPoseValid = false;
  initialized = false;
}

//
// OpenXR のセッションを破棄する
//
void GgApp::OpenXR::terminate()
{
  // 取得中のスワップチェーンイメージがあれば解放する (xrEndFrame() より前に行う)
  for (size_t i = 0; i < imageAcquired.size(); ++i)
  {
    if (!imageAcquired[i]) continue;
    XrSwapchainImageReleaseInfo releaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
    xrReleaseSwapchainImage(swapchains[i], &releaseInfo);
    imageAcquired[i] = false;
  }

  // 描画中のフレームがあれば完了しておく
  if (frameBegun) endFrame();

  // OpenGL の資源を解放する
  if (!openxrFbo.empty())
  {
    glDeleteFramebuffers(static_cast<GLsizei>(openxrFbo.size()), openxrFbo.data());
    openxrFbo.clear();
  }
  if (!openxrDepth.empty())
  {
    glDeleteRenderbuffers(static_cast<GLsizei>(openxrDepth.size()), openxrDepth.data());
    openxrDepth.clear();
  }

  // OpenXR のハンドルを破棄する
  destroyXr();

  views.clear();
  viewStates.clear();
  currentImageIndex.clear();
  imageAcquired.clear();
  systemName.clear();

  for (auto& state : controllerStates) state = ControllerState{};

  // ウィンドウ側の設定を元に戻す
  if (window)
  {
    glDisable(GL_FRAMEBUFFER_SRGB);
    glfwSwapInterval(1);
    window = nullptr;
  }
}

//
// OpenXR による描画開始
//
bool GgApp::OpenXR::begin()
{
  // 初期化されていなければ何もしない
  if (instance == XR_NULL_HANDLE) return false;

  // OpenXR のイベントを処理する
  pollEvents();

  // セッションが実行中でなければ描画しない
  if (!isSessionRunning) return false;

  // 合成器がこのフレームの描画を始めるべき時刻まで待つ
  XrFrameWaitInfo waitInfo{ XR_TYPE_FRAME_WAIT_INFO };
  frameState = XrFrameState{ XR_TYPE_FRAME_STATE };
  if (!xrWarn(instance, xrWaitFrame(session, &waitInfo, &frameState),
    "Can't wait for the OpenXR frame")) return false;

  // フレームの描画を開始する
  XrFrameBeginInfo beginInfo{ XR_TYPE_FRAME_BEGIN_INFO };
  if (!xrWarn(instance, xrBeginFrame(session, &beginInfo),
    "Can't begin the OpenXR frame")) return false;

  // ここから先は必ず xrEndFrame() を呼ばなければならない
  frameBegun = true;
  viewPoseValid = false;

  // コントローラーの状態を更新する
  pollActions();

  // 描画すべきフレームなら視点の姿勢を取得する
  if (frameState.shouldRender != XR_FALSE)
  {
    XrViewLocateInfo viewLocateInfo{ XR_TYPE_VIEW_LOCATE_INFO };
    viewLocateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    viewLocateInfo.displayTime = frameState.predictedDisplayTime;
    viewLocateInfo.space = appSpace;

    XrViewState viewState{ XR_TYPE_VIEW_STATE };
    uint32_t viewCount{ 0 };
    if (XR_SUCCEEDED(xrLocateViews(session, &viewLocateInfo, &viewState,
      static_cast<uint32_t>(viewStates.size()), &viewCount, viewStates.data())))
    {
      // 位置と向きの両方が有効なときだけ描画する
      constexpr XrViewStateFlags valid
      {
        XR_VIEW_STATE_POSITION_VALID_BIT | XR_VIEW_STATE_ORIENTATION_VALID_BIT
      };
      viewPoseValid = (viewState.viewStateFlags & valid) == valid
        && viewCount == static_cast<uint32_t>(viewStates.size());
    }
  }

  // 描画するなら true を返す
  if (viewPoseValid) return true;

  // 描画しないフレームでもここで xrEndFrame() を呼んで辻褄を合わせる
  endFrame();

  return false;
}

//
// 描画対象の目を指定してフレームバッファとビューポートを設定する
//
void GgApp::OpenXR::select(int eye)
{
  // 描画すべきフレームでなければ何もしない
  if (!frameBegun || !viewPoseValid) return;

  // 視点の番号が範囲を外れていたら何もしない
  assert(eye >= 0 && eye < static_cast<int>(swapchains.size()));
  if (eye < 0 || eye >= static_cast<int>(swapchains.size())) return;

  // 取得済みなら描画先を結合し直すだけにする
  if (imageAcquired[eye])
  {
    glBindFramebuffer(GL_FRAMEBUFFER, openxrFbo[eye]);
    glViewport(0, 0,
      static_cast<GLsizei>(views[eye].recommendedImageRectWidth),
      static_cast<GLsizei>(views[eye].recommendedImageRectHeight));
    if (swapchainIsSrgb) glEnable(GL_FRAMEBUFFER_SRGB);
    return;
  }

  // 描画可能なスワップチェーンイメージを取得する
  XrSwapchainImageAcquireInfo acquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
  if (!xrWarn(instance, xrAcquireSwapchainImage(swapchains[eye], &acquireInfo,
    &currentImageIndex[eye]), "Can't acquire the OpenXR swapchain image")) return;

  // そのスワップチェーンイメージが描画可能になるのを待つ
  XrSwapchainImageWaitInfo waitInfo{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
  waitInfo.timeout = XR_INFINITE_DURATION;
  if (!xrWarn(instance, xrWaitSwapchainImage(swapchains[eye], &waitInfo),
    "Can't wait for the OpenXR swapchain image"))
  {
    XrSwapchainImageReleaseInfo releaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
    xrReleaseSwapchainImage(swapchains[eye], &releaseInfo);
    return;
  }

  imageAcquired[eye] = true;

  // 描画先をこのスワップチェーンイメージに切り替える
  const GLuint texture{ swapchainImages[eye][currentImageIndex[eye]].image };
  glBindFramebuffer(GL_FRAMEBUFFER, openxrFbo[eye]);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

  // フレームバッファオブジェクトが完成しているか確かめる (最初の一度だけ報告する)
  static bool reported{ false };
  if (!reported && glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    reported = true;
    std::cerr << "OpenXR: The framebuffer object for the swapchain image is not complete\n";
  }

  glViewport(0, 0,
    static_cast<GLsizei>(views[eye].recommendedImageRectWidth),
    static_cast<GLsizei>(views[eye].recommendedImageRectHeight));

  // sRGB のスワップチェーンならリニア色空間で描画する
  if (swapchainIsSrgb) glEnable(GL_FRAMEBUFFER_SRGB);
}

//
// 描画対象の目を指定する (旧 LibOVR 仕様互換)
//
void GgApp::OpenXR::select(int eye, GLfloat* screen, GLfloat* position, GLfloat* orientation)
{
  select(eye);

  assert(eye >= 0 && eye < static_cast<int>(viewStates.size()));
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
// 指定した目の描画を完了する
//
void GgApp::OpenXR::commit(int eye)
{
  if (!frameBegun) return;
  assert(eye >= 0 && eye < static_cast<int>(swapchains.size()));
  if (eye < 0 || eye >= static_cast<int>(swapchains.size())) return;
  if (!imageAcquired[eye]) return;

  // ガンマ補正を元に戻して描画先をウィンドウに戻す
  if (swapchainIsSrgb) glDisable(GL_FRAMEBUFFER_SRGB);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // スワップチェーンイメージはミラー表示に使うので, ここでは解放しない
  // (解放は submit() の中でミラー表示を行った後に実施する)
}

//
// ミラー表示を行う
//
void GgApp::OpenXR::blitMirror() const
{
  // ミラー表示を行わないなら何もしない
  if (mirrorView < 0 || !window) return;
  const auto eye{ static_cast<size_t>(mirrorView) };
  if (eye >= swapchains.size() || !imageAcquired[eye]) return;

  // 転送元の大きさ
  const auto srcWidth{ static_cast<GLint>(views[eye].recommendedImageRectWidth) };
  const auto srcHeight{ static_cast<GLint>(views[eye].recommendedImageRectHeight) };
  if (srcWidth <= 0 || srcHeight <= 0) return;

  // 転送先 (ウィンドウ) の大きさ
  const auto& fboSize{ window->getFboSize() };
  if (fboSize[0] <= 0 || fboSize[1] <= 0) return;

  // 縦横比を保ったままウィンドウに収まる転送先の矩形を求める
  const auto scale{ std::min(
    static_cast<float>(fboSize[0]) / static_cast<float>(srcWidth),
    static_cast<float>(fboSize[1]) / static_cast<float>(srcHeight)) };
  const auto dstWidth{ static_cast<GLint>(static_cast<float>(srcWidth) * scale) };
  const auto dstHeight{ static_cast<GLint>(static_cast<float>(srcHeight) * scale) };
  const auto dstLeft{ (static_cast<GLint>(fboSize[0]) - dstWidth) / 2 };
  const auto dstBottom{ (static_cast<GLint>(fboSize[1]) - dstHeight) / 2 };

  // sRGB の再変換を避けるためにガンマ補正を無効にする
  if (swapchainIsSrgb) glDisable(GL_FRAMEBUFFER_SRGB);

  // 上下左右の余白を黒で塗りつぶす (消去色は元に戻す)
  GLfloat clearColor[4];
  glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColor);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glDisable(GL_SCISSOR_TEST);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);

  // スワップチェーンイメージをウィンドウに転送する
  glBindFramebuffer(GL_READ_FRAMEBUFFER, openxrFbo[eye]);
  glBlitFramebuffer(0, 0, srcWidth, srcHeight,
    dstLeft, dstBottom, dstLeft + dstWidth, dstBottom + dstHeight,
    GL_COLOR_BUFFER_BIT, GL_LINEAR);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

//
// 描画中のフレームを合成器に転送する
//
void GgApp::OpenXR::endFrame()
{
  if (!frameBegun) return;

  // 合成する層
  std::vector<XrCompositionLayerProjectionView> projectionViews;
  XrCompositionLayerProjection layer{ XR_TYPE_COMPOSITION_LAYER_PROJECTION };
  const XrCompositionLayerBaseHeader* layers[1]{ nullptr };

  // 描画したのなら層を用意する
  if (viewPoseValid && frameState.shouldRender != XR_FALSE)
  {
    projectionViews.resize(swapchains.size());
    for (size_t i = 0; i < swapchains.size(); ++i)
    {
      auto& projectionView{ projectionViews[i] };
      projectionView = XrCompositionLayerProjectionView{ XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
      projectionView.pose = viewStates[i].pose;
      projectionView.fov = viewStates[i].fov;
      projectionView.subImage.swapchain = swapchains[i];
      projectionView.subImage.imageRect.offset = { 0, 0 };
      projectionView.subImage.imageRect.extent = {
        static_cast<int32_t>(views[i].recommendedImageRectWidth),
        static_cast<int32_t>(views[i].recommendedImageRectHeight)
      };
      projectionView.subImage.imageArrayIndex = 0;
    }

    // 環境の合成方法に応じて層の属性を設定する
    layer.layerFlags = blendMode == XR_ENVIRONMENT_BLEND_MODE_OPAQUE ? 0
      : XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT
      | XR_COMPOSITION_LAYER_UNPREMULTIPLIED_ALPHA_BIT;
    layer.space = appSpace;
    layer.viewCount = static_cast<uint32_t>(projectionViews.size());
    layer.views = projectionViews.data();
    layers[0] = reinterpret_cast<const XrCompositionLayerBaseHeader*>(&layer);
  }

  // フレームを合成器に転送する
  XrFrameEndInfo endInfo{ XR_TYPE_FRAME_END_INFO };
  endInfo.displayTime = frameState.predictedDisplayTime;
  endInfo.environmentBlendMode = blendMode;
  endInfo.layerCount = layers[0] ? 1u : 0u;
  endInfo.layers = layers;
  xrWarn(instance, xrEndFrame(session, &endInfo), "Can't end the OpenXR frame");

  frameBegun = false;
}

//
// フレームを転送して HMD に表示する
//
bool GgApp::OpenXR::submit(bool mirror)
{
  // 描画中のフレームがなければ何もしない
  if (!frameBegun) return false;

  // ミラー表示の有無を設定する
  if (!mirror) mirrorView = -1;
  else if (mirrorView < 0) mirrorView = 0;

  // スワップチェーンイメージを解放する前にミラー表示を行う
  blitMirror();

  // ウィンドウのビューポートを復帰して Dear ImGui などの描画に備える
  if (window) window->restoreViewport();

  // 取得したスワップチェーンイメージを解放する
  for (size_t i = 0; i < imageAcquired.size(); ++i)
  {
    if (!imageAcquired[i]) continue;
    XrSwapchainImageReleaseInfo releaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
    xrWarn(instance, xrReleaseSwapchainImage(swapchains[i], &releaseInfo),
      "Can't release the OpenXR swapchain image");
    imageAcquired[i] = false;
  }

  // 合成器にフレームを転送する
  endFrame();

  return true;
}

//
// ミラー表示を行うビューの番号を設定する
//
void GgApp::OpenXR::setMirror(int eye)
{
  mirrorView = eye;
}

//
// ミラー表示を行うビューの番号を取得する
//
int GgApp::OpenXR::getMirror() const
{
  return mirrorView;
}

//
// セッションが実行中かどうか調べる
//
bool GgApp::OpenXR::isRunning() const
{
  return isSessionRunning;
}

//
// アプリケーションが入力を受け付けているかどうか調べる
//
bool GgApp::OpenXR::isFocused() const
{
  return sessionState == XR_SESSION_STATE_FOCUSED;
}

//
// OpenXR のシステム (HMD) の名前を取得する
//
const std::string& GgApp::OpenXR::getSystemName() const
{
  return systemName;
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
// 視点の姿勢が有効かどうか調べる
//
bool GgApp::OpenXR::isPoseValid() const
{
  return viewPoseValid;
}

//
// レンダリング推奨解像度の横幅を取得する
//
GLsizei GgApp::OpenXR::getWidth(int eye) const
{
  assert(eye >= 0 && eye < static_cast<int>(views.size()));
  return static_cast<GLsizei>(views[eye].recommendedImageRectWidth);
}

//
// レンダリング推奨解像度の高さを取得する
//
GLsizei GgApp::OpenXR::getHeight(int eye) const
{
  assert(eye >= 0 && eye < static_cast<int>(views.size()));
  return static_cast<GLsizei>(views[eye].recommendedImageRectHeight);
}

//
// アスペクト比を取得する
//
GLfloat GgApp::OpenXR::getAspect(int eye) const
{
  assert(eye >= 0 && eye < static_cast<int>(views.size()));
  return static_cast<GLfloat>(views[eye].recommendedImageRectWidth)
    / static_cast<GLfloat>(views[eye].recommendedImageRectHeight);
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
  vibration.duration = durationSeconds > 0.0f
    ? static_cast<XrDuration>(static_cast<double>(durationSeconds) * 1.0e9)
    : XR_MIN_HAPTIC_DURATION;
  vibration.frequency = frequency;
  vibration.amplitude = std::min(std::max(amplitude, 0.0f), 1.0f);

  XrHapticActionInfo actionInfo{ XR_TYPE_HAPTIC_ACTION_INFO };
  actionInfo.action = hapticAction;
  actionInfo.subactionPath = handSubactionPath[hand];

  xrWarn(instance, xrApplyHapticFeedback(session, &actionInfo,
    reinterpret_cast<const XrHapticBaseHeader*>(&vibration)),
    "Can't apply the haptic feedback");
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
