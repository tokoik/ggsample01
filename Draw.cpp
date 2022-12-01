///
/// 図形の描画クラスの実装
///
/// @file
/// @author Kohe Tokoi
/// @date November 15, 2022
///
#include "Draw.h"

//
// コンストラクタ
//
Draw::Draw(const Menu& menu) :
  light{ *menu.light.get() },
  model{ *menu.model.get() },
  shader{ *menu.shader.get() }
{
  // 背景色を設定する
  glClearColor(0.1f, 0.2f, 0.3f, 0.0f);

  // 隠面消去処理を設定する
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
}

//
// デストラクタ
//
Draw::~Draw()
{
}

//
// 描画する
//
void Draw::draw(const GgMatrix& mp, const GgMatrix& mv) const
{
  // シェーダプログラムを指定する
  shader.use(mp, mv, light);

  // 図形を描画する
  model.draw();
}
