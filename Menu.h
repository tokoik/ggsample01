#pragma once

///
/// メニューの描画クラスの定義
///
/// @file
/// @author Kohe Tokoi
/// @date August 15, 2025
///

// 宿題用補助プログラムのラッパー
#include "GgApp.h"

// 構成データ
#include "Config.h"

// ファイルダイアログ
#include "nfd.h"

///
/// メニューの描画
///
class Menu
{
  // 図形の描画クラスから参照する
  friend class Scene;

  // オリジナルの構成データ
  const Config& defaults;

  // 構成データのコピー
  Config settings;

  // 光源データ
  std::unique_ptr<const GgSimpleShader::LightBuffer> light;

  // シェーダ
  std::unique_ptr<const GgSimpleShader> shader;

  // CAD データ
  std::unique_ptr<const GgSimpleObj> model;

  // ファイルパスを取得する
  bool getFilePath(std::string& path, const nfdfilteritem_t* filter);

public:

  ///
  /// コンストラクタ
  ///
  Menu(const Config& config);

  ///
  /// コピーコンストラクタは使用しない
  ///
  Menu(const Menu& menu) = delete;

  ///
  /// ムーブコンストラクタはデフォルトのものを使用する
  ///
  Menu(Menu&& menu) = default;

  ///
  /// デストラクタ.
  ///
  virtual ~Menu();

  ///
  /// 代入演算子は使用しない
  ///
  Menu& operator=(const Menu& menu) = delete;

  ///
  /// ムーブ代入演算子はデフォルトのものを使用する
  ///
  Menu& operator=(Menu&& menu) = default;

  ///
  /// 光源データを取り出す
  ///
  const auto& getLight() const
  {
    return *light;
  }

  ///
  /// シェーダを取り出す
  ///
  const auto& getShader() const
  {
    return *shader;
  }

  ///
  /// モデルデータを取り出す
  ///
  const auto& getModel() const
  {
    return *model;
  }

  ///
  /// 描画する
  ///
  void draw();
};
