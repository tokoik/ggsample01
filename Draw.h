#pragma once

///
/// 図形の描画クラスの定義
///
/// @file
/// @author Kohe Tokoi
/// @date November 15, 2022
///

// メニュー
#include "Menu.h"

///
/// 図形の描画クラス
///
class Draw
{
  // 光源
  const GgSimpleShader::LightBuffer& light;

  // モデル
  const GgSimpleObj& model;

  // シェーダ
  const GgSimpleShader& shader;

public:

  ///
  /// コンストラクタ
  ///
  Draw(const Menu& menu);

  ///
  /// デストラクタ
  ///
  virtual ~Draw();

  ///
  /// 描画する
  ///
  void draw(const GgMatrix& mp, const GgMatrix& mv) const;
};
