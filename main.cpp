//
// メインプログラム
//

// メッセージボックスの表示の準備
#if defined(_MSC_VER)
#  include <Windows.h>
#  include <atlstr.h>  
#endif

// アプリケーション本体
#include "GgApplication.h"

//
// メインプログラム
//
int main() try
{
  // アプリケーション本体
  GgApplication app;

  // アプリケーションを実行する
  app.run();
}
catch (const std::runtime_error &e)
{
  // エラーメッセージを表示する
#if defined(_MSC_VER)
  MessageBox(NULL, CString(e.what()), TEXT("ゲームグラフィックス特論"), MB_ICONERROR);
#else
  std::cerr << e.what() << "\n\n[Type enter key] ";
  std::cin.get();
#endif

  // ブログラムを終了する
  return EXIT_FAILURE;
}
