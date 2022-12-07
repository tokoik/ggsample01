///
/// 設定構造体の実装
///
/// @file
/// @author Kohe Tokoi
/// @date November 15, 2022
///
#include "Config.h"

// 標準ライブラリ
#include <fstream>

// デフォルトの光源データ
constexpr GgSimpleShader::Light defaultLight
{
  { 0.2f, 0.2f, 0.2f, 1.0f }, // 環境光成分
  { 1.0f, 1.0f, 1.0f, 0.0f }, // 拡散反射光成分
  { 1.0f, 1.0f, 1.0f, 0.0f }, // 鏡面反射光成分
  { 0.5f, 0.5f, 1.0f, 1.0f }  // 光源位置
};

//
// コンストラクタ
//
Config::Config() :
  winSize{ 960, 540 },
  menuFont{ "Mplus1-Regular.ttf" },
  menuFontSize{ 20.0f },
  light{ defaultLight },
  model{ "logo.obj" },
#if defined(GL_GLES_PROTOTYPES)
  shader{ "simple_es3.vert", "simple_es3.frag" }
#else
  shader{ "simple.vert", "simple.frag" }
#endif
{
}

//
// ファイルから構成データを読み込むコンストラクタ
//
Config::Config(const std::string& filename) :
  Config{}
{
  // 構成ファイルが読み込めなかったらデフォルト値の構成ファイルを作る
  if (!load(filename)) save(filename);
}

//
// デストラクタ
//
Config::~Config()
{
}

//
// 設定ファイルを読み込む
//
bool Config::load(const std::string& filename)
{
  // 構成ファイルを開く
  std::ifstream file{ Utf8ToTChar(filename) };

  // 開けなかったらエラー
  if (!file) return false;

  // JSON の読み込み
  picojson::value value;
  file >> value;
  file.close();

  // 構成データの取り出し
  const auto& object{ value.get<picojson::object>() };

  //
  // 構成データの読み込み
  //

  // ウィンドウサイズ
  getValue(object, "window_size", winSize);

  // メニューフォント
  getString(object, "menu_font", menuFont);

  // メニューフォントサイズ
  getValue(object, "menu_font_size", menuFontSize);

  // 光源
  getVector(object, "ambient", light.ambient);
  getVector(object, "diffuse", light.diffuse);
  getVector(object, "specular", light.specular);
  getVector(object, "position", light.position);

  // モデル
  getString(object, "model", model);

  // シェーダ
  getString(object, "shader", shader);

  // オブジェクトが空だったらエラー
  if (object.empty()) return false;

  return true;
}

//
// 設定ファイルを書き出す
//
bool Config::save(const std::string& filename) const
{
  // 構成ファイルを開く
  std::ofstream file{ Utf8ToTChar(filename) };

  // 開けなかったらエラー
  if (!file) return false;

  // 構成データの書き出しに使うオブジェクト
  picojson::object object;

  //
  // 構成データの書き出し
  //

  // ウィンドウサイズ
  setValue(object, "window_size", winSize);

  // メニューフォント
  setString(object, "menu_font", menuFont);

  // メニューフォントサイズ
  setValue(object, "menu_font_size", menuFontSize);

  // 光源
  setVector(object, "ambient", light.ambient);
  setVector(object, "diffuse", light.diffuse);
  setVector(object, "specular", light.specular);
  setVector(object, "position", light.position);

  // モデル
  setString(object, "model", model);

  // シェーダ
  setString(object, "shader", shader);

  // 構成出データをシリアライズして JSON で保存
  picojson::value v{ object };
  file << v.serialize(true);
  file.close();

  return true;
}

#if defined(_MSC_VER)
//
// For VC++ MFC Convert UTF-8 to TCHAR, or Convert TCHAR to UTF-8. VC++ MFC用 UTF-8⇔TCHARの変換処理
//
//   Original author: mt-u
//   Copyright (c) 2013 mt-u
//   https://gist.github.com/mt-u/6878251
//
//   Modified by: Kohe Tokoi
//

//
// UTF-8 文字列を CString に変換する
//
CString Utf8ToTChar(const std::string& string)
{
  // UTF-8 文字列を UTF-16 に変換した後の文字列の長さを求める
  const INT length{ MultiByteToWideChar(CP_UTF8, 0, string.c_str(), -1, NULL, 0) };

  // 変換結果の格納に必要な長さのメモリを確保する
  std::vector<WCHAR> utf16(length);

  // UTF-8 文字列を UTF-16 に変換する
  MultiByteToWideChar(CP_UTF8, 0, string.c_str(), -1, utf16.data(), length);

  // 変換した文字列を CString にして返す
  return CString{ utf16.data(), static_cast<int>(wcslen(utf16.data())) };
}

//
// Cstring を UTF-8 文字列に変換する
//
std::string TCharToUtf8(const CString& cstring)
{
  // 与えられた文字列を一旦 UTF-16 に変換する
  CStringW cstringw{ cstring };

  // UTF-16 を UTF-8 に変換した後のの文字列の長さを求める
  const INT length{ WideCharToMultiByte(CP_UTF8, 0, cstringw, -1, NULL, 0, NULL, NULL) };

  // 変換結果の格納に必要な長さのメモリを確保する
  std::vector<CHAR> utf8(length);

  // UTF-16 を UTF-8 に変換する
  WideCharToMultiByte(CP_UTF8, 0, cstringw, -1, utf8.data(), length, NULL, NULL);

  // 変換した文字列を std::string にして返す
  return std::string{ utf8.data(), strlen(utf8.data()) };
}
#endif
