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
  std::ifstream json{ Utf8ToTChar(filename) };

  // 開けなかったらエラー
  if (!json) return false;

  // JSON の読み込み
  picojson::value value;
  json >> value;
  json.close();

  // 構成データの取り出し
  const auto& object{ value.get<picojson::object>() };

  //
  // 構成データの読み込み
  //

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
  std::ofstream preference{ Utf8ToTChar(filename) };

  // 開けなかったらエラー
  if (!preference) return false;

  // 構成データの書き出しに使うオブジェクト
  picojson::object object;

  //
  // 構成データの書き出し
  // 

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
  preference << v.serialize(true);
  preference.close();

  return true;
}

// Visual C++ の場合
#if defined(_MSC_VER)

//
// For VC++ MFC Convert UTF-8 to TCHAR, or Convert TCHAR to UTF-8. VC++ MFC用 UTF-8⇔TCHARの変換処理
//
//   Refer to the following web page.
//     https://gist.github.com/mt-u/6878251
//

// ----------------------------------------
// Convert UTF-8 to TCHAR(MBCS or Unicode).
// UTF-8文字列をTCHAR文字列に変換する
// (プロジェクトの設定に応じてMBCSかUnicodeになる)
// Author mt-u
// Copyright (c) 2013 mt-u
// ----------------------------------------
CString Utf8ToTChar(const std::string& utf8)
{
  // ----------------------------------------
  // Convert UTF-8 to Unicode(UTF-16).
  // UTF-8をUnicode(UTF-16)に変換する
  // ----------------------------------------
  // First, get the size of the buffer to store Unicode chars.
  // まず、Unicode文字列を格納するバッファサイズを取得する
  INT bufsize = ::MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
  WCHAR* utf16 = new WCHAR[bufsize];

  // Then, convert UTF-8 to Unicode.
  // UTF-8をUnicodeに変換する
  ::MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, utf16, bufsize);

  // ----------------------------------------
  // Convert Unicode(UTF-16) to TCHAR
  // Unicode(UTF-16)をTCHARに変換する
  // ----------------------------------------
  // Convert Unicode(WCHAR) to TCHAR(MBCS or Unicode).
  // WCHARからCStringWまたはCStringAに変換する(プロジェクトの設定による)
  CString ret(utf16, static_cast<int>(::wcslen(utf16)));
  delete[] utf16;

  return ret;
}

// ----------------------------------------
// Convert TCHAR(MBCS or Unicode) to UTF-8.
// TCHAR文字列をUTF-8文字列に変換する
// Author mt-u
// Copyright (c) 2013 mt-u
// ----------------------------------------
std::string TCharToUtf8(const CString& tchar)
{
  // ----------------------------------------
  // Convert TCHAR to Unicode(UTF-16).
  // TCHARをUnicode(UTF-16)に変換する
  // ----------------------------------------
  // Convert TCHAR to Unicode.
  // TCHARをUniocodeに変換する
  CStringW utf16(tchar);

  // ----------------------------------------
  // Convert Unicode(UTF-16) to UTF-8.
  // Unicode(UTF-16)をUTF-8に変換する
  // ----------------------------------------
  // First, get the size of the buffer to store UTF-8 chars.
  // まず、UTF-8文字列を格納するバッファサイズを取得する
  INT bufsize = ::WideCharToMultiByte(CP_UTF8, 0, utf16, -1, NULL, 0, NULL, NULL);
  CHAR* utf8 = new CHAR[bufsize];

  // Then, convert Unicode to UTF-8.
  // 次にUTF-8をUnicodeに変換する
  ::WideCharToMultiByte(CP_UTF8, 0, utf16, -1, utf8, bufsize, NULL, NULL);

  // At the end, create the std::string that contains the UTF-8 chars.
  // std::stringにいれて返す
  std::string ret(utf8, ::strlen(utf8));
  delete[] utf8;

  return ret;
}
#endif
