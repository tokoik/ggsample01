#pragma once
//
// ゲームグラフィックス特論宿題アプリケーションクラス
//

// Oculus Rift を使う
//#define USE_OCULUS_RIFT

// Dear ImGui を使う
#define USE_IMGUI

// Window 関連の処理
#include "Window.h"

class GgApplication
{
public:

  //
  // コンストラクタ
  //
  GgApplication()
  {
  }

  //
  // アプリケーション本体
  //
  void run();
};
