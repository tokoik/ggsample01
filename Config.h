#pragma once

///
/// 構成データクラスの定義
///
/// @file
/// @author Kohe Tokoi
/// @date November 15, 2022
///

// アプリケーションの設定
#include "GgApp.h"

// 構成ファイルの読み取り補助
#include "parseconfig.h"

///
/// 構成データ
///
class Config
{
  // メニュークラスから参照する
  friend class Menu;

  // 光源
  GgSimpleShader::Light light;

  // 形状ファイル名
  std::string model;

  // シェーダのソースファイル名
  std::array<std::string, 3> shader;

public:

  ///
  /// コンストラクタ
  ///
  Config();

  ///
  /// ファイルから構成データを読み込むコンストラクタ
  ///
  Config(const std::string& filename);

  ///
  /// デストラクタ
  ///
  virtual ~Config();

  ///
  /// 設定ファイルを読み込む
  ///
  bool load(const std::string& filename);

  ///
  /// 設定ファイルを書き出す
  ///
  bool save(const std::string& filename) const;
};
