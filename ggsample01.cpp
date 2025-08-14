//
// ゲームグラフィックス特論宿題アプリケーション
//

// 補助プログラムのラッパー
#include "GgApp.h"

// プロジェクト名
#if !defined(PROJECT_NAME)
#  define PROJECT_NAME "ggsample01"
#endif

// 構成ファイル名
#if !defined(CONFIG_FILE)
#  define CONFIG_FILE PROJECT_NAME "_config.json"
#endif

// 構成データ
#include "Config.h"

// メニューの描画
#include "Menu.h"

// 図形の描画
#include "Object.h"

//
// アプリケーション本体
//
int GgApp::main(int argc, const char* const* argv)
{
  // 設定を読み込む
  const Config config{ CONFIG_FILE };

  // ウィンドウを作成する
  Window window{ argc > 1 ? argv[1] : PROJECT_NAME, config.getWidth(), config.getHeight() };

  // メニューを初期化する
  Menu menu{ config };

  // 図形の描画の設定を行う
  Object object{ menu };

  // ビュー変換行列を設定する
  const GgMatrix mv{ ggLookat(0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f) };

  // ウィンドウが開いている間繰り返す
  while (window)
  {
    // ウィンドウを消去する
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // メニューを表示する
    menu.draw();

    // オブジェクトの回転の変換行列を設定する
    const auto& mr{ window.getRotationMatrix(0) };

    // 視点の平行移動の変換行列を設定する
    const auto& mt{ window.getTranslationMatrix(1) };

    // 投影変換行列を設定する
    const GgMatrix&& mp{ ggPerspective(0.5f, window.getAspect(), 1.0f, 15.0f) };

    // 図形を描画する
    object.draw(mp, mt * mv * mr);

    // カラーバッファを入れ替えてイベントを取り出す
    window.swapBuffers();
  }

  return 0;
}
