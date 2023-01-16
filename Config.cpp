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
