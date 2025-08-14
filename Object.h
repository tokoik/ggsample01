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
class Object
{
  // 光源
  const GgSimpleShader::LightBuffer& light;

  // シェーダ
  const GgSimpleShader& shader;

  // モデル
  const GgSimpleObj& model;

public:

  ///
  /// コンストラクタ
  ///
  Object(const Menu& menu);

  ///
  /// デストラクタ
  ///
  virtual ~Object();

  ///
  /// 描画する
  ///
  /// @param mp 投影変換行列
  /// @param mv モデルビュー変換行列
  ///
  void draw(const GgMatrix& mp, const GgMatrix& mv) const;
};
