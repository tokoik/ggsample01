#pragma once

// Dear ImGui を使うなら
#define GG_USE_IMGUI

// Oculus Rift を使うなら
//#define GG_USE_OCULUS_RIFT

// ウィンドウ関連の処理
#include "Window.h"

//
// ゲームグラフィックス特論宿題アプリケーションクラス
//
class GgApp
{
public:

  //
  // アプリケーション本体
  //
  int main(int argc, const char* const* argv);
};
