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

#if defined(_MSC_VER)
//
// Windows / Visual C++ におけるパス名
//

// 文字コード
#include <atlstr.h>
using pathString = CString;
using pathChar = wchar_t;

// 文字コード変換
extern CString Utf8ToTChar(const std::string& string);
extern std::string TCharToUtf8(const CString& cstring);
#else
using pathString = std::string;
using pathChar = char;
#define Utf8ToTChar(string) (string)
#define TCharToUtf8(cstring) (cstring)
#endif

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
