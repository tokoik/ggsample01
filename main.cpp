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
#include "GgApplication.h"

// GLFW のエラー表示
static void glfwErrorCallback(int error, const char* description)
{
#ifdef __aarch64__
  if (error == 65544) return;
#endif
  throw std::runtime_error(description);
}

//
// メインプログラム
//
int main() try
{
  // GLFW のエラー表示関数を登録する
  glfwSetErrorCallback(glfwErrorCallback);

  // GLFW を初期化する
  if (glfwInit() == GL_FALSE) throw std::runtime_error("Can't initialize GLFW");

  // OpenGL のバージョンを指定する
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // プログラム終了時に GLFW の後始末を行う
  atexit(glfwTerminate);

#ifdef USE_OCULUS_RIFT
  // LibOVR を初期化してセッションを作成する
  Window::initialize();
#endif

#ifdef USE_IMGUI
  // ImGui のバージョンをチェックする
  IMGUI_CHECKVERSION();

  // ImGui のコンテキストを作成する
  ImGui::CreateContext();
#endif

  // アプリケーションのオブジェクトを生成する
  GgApplication app;

  // アプリケーションを実行する
  app.run();

  // ImGui のコンテキストを破棄する
  ImGui::DestroyContext();
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

#ifdef USE_OCULUS_RIFT
// Oculus Rift のセッション
ovrSession Window::session(nullptr);
#endif
