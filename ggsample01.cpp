//
// メインプログラム
//

// アプリケーション本体
#include "Window.h"

//
// メインプログラム
//
int main() try
{
  // ウィンドウを作成する
  Window window("ggsample01");

  // 背景色を指定する
  glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

  // ウィンドウが開いている間繰り返す
  while (window)
  {
    // ウィンドウを消去する
    glClear(GL_COLOR_BUFFER_BIT);

    //
    // ここで OpenGL による描画を行う
    //

    // カラーバッファを入れ替えてイベントを取り出す
    window.swapBuffers();
  }
}
catch (const std::runtime_error &e)
{
  // エラーメッセージを表示する
  Window::message(e.what());

  // ブログラムを終了する
  return EXIT_FAILURE;
}
