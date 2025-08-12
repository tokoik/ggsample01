#pragma once

///
/// 構成データクラスの定義
///
/// @file
/// @author Kohe Tokoi
/// @date November 15, 2022
///

// 補助プログラム
#include "gg.h"
using namespace gg;

// 構成ファイルの読み取り補助
#include "parseconfig.h"

///
/// 構成データ
///
class Config
{
  // メニュークラスから参照する
  friend class Menu;

  // ウィンドウサイズ
  std::array<GLsizei, 2> winSize;

  // メニューフォント名
  std::string menuFont;

  // メニューフォントサイズ
  float menuFontSize;

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
  /// ウィンドウの横幅を得る
  /// 
  auto getWidth() const
  {
    return winSize[0];
  }

  ///
  /// ウィンドウの横幅を得る
  /// 
  auto getHeight() const
  {
    return winSize[1];
  }

  ///
  /// 設定ファイルを読み込む
  ///
  bool load(const std::string& filename);

  ///
  /// 設定ファイルを書き出す
  ///
  bool save(const std::string& filename) const;
};
