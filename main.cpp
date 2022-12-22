//
// ゲームグラフィックス特論宿題アプリケーション
//

// MessageBox の準備
#if defined(_MSC_VER)
#  define NOMINMAX
#  include <Windows.h>
#  include <atlstr.h>
#elif defined(__APPLE__)
#  include <CoreFoundation/CoreFoundation.h>
#else
#  include <iostream>
#endif
#define HEADER_STR "ゲームグラフィックス特論"

// ゲームグラフィックス特論宿題アプリケーションクラス
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
// コンストラクタ
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
// メインプログラム
//
int main(int argc, const char* const* argv) try
{
  // アプリケーションのオブジェクトを生成する
#if defined(GL_GLES_PROTOTYPES)
  GgApp app(3, 1);
#else
  GgApp app(4, 1);
#endif

  // アプリケーションを実行する
  return app.main(argc, argv);
}
catch (const std::runtime_error &e)
{
  // エラーメッセージを表示する
#if defined(_MSC_VER)
  MessageBox(NULL, CString(e.what()), TEXT(HEADER_STR), MB_ICONERROR);
#elif defined(__APPLE__)
  // the following code is copied from http://blog.jorgearimany.com/2010/05/messagebox-from-windows-to-mac.html
  // convert the strings from char* to CFStringRef
  CFStringRef msg_ref = CFStringCreateWithCString(NULL, e.what(), kCFStringEncodingUTF8);

  // result code from the message box
  CFOptionFlags result;

  //launch the message box
  CFUserNotificationDisplayAlert(
    0,                                 // no timeout
    kCFUserNotificationNoteAlertLevel, // change it depending message_type flags ( MB_ICONASTERISK.... etc.)
    NULL,                              // icon url, use default, you can change it depending message_type flags
    NULL,                              // not used
    NULL,                              // localization of strings
    CFSTR(HEADER_STR),                 // header text
    msg_ref,                           // message text
    NULL,                              // default "ok" text in button
    NULL,                              // alternate button title
    NULL,                              // other button title, null--> no other button
    &result                            // response flags
  );

  // Clean up the strings
  CFRelease(msg_ref);
#else
  std::cerr << HEADER_STR << ": " << e.what() << '\n';
#endif

  // ブログラムを終了する
  return EXIT_FAILURE;
}
