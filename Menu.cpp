///
/// メニューの描画クラスの実装
///
/// @file
/// @author Kohe Tokoi
/// @date November 15, 2022
///
#include "Menu.h"

//
// コンストラクタ
//
Menu::Menu(const Config& config) :
  defaults{ config },
  settings{ config },
  light{ std::make_unique<GgSimpleShader::LightBuffer>(config.light) },
  shader{ std::make_unique<GgSimpleShader>(config.shader) },
  model{ std::make_unique<GgSimpleObj>(config.model, true) }
{
#if defined(IMGUI_VERSION)
  //
  // ImGui の初期設定
  //

  // ファイルダイアログ (Native File Dialog Extended) を初期化する
  NFD_Init();

  // Dear ImGui の入力デバイス
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // キーボードコントロールを使う
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // ゲームパッドを使う

  // Dear ImGui のスタイル
  //ImGui::StyleColorsDark();                                 // 暗めのスタイル
  //ImGui::StyleColorsClassic();                              // 以前のスタイル

  // 日本語を表示できるメニューフォントを読み込む
  if (!ImGui::GetIO().Fonts->AddFontFromFileTTF(config.menuFont.c_str(), config.menuFontSize, nullptr,
    ImGui::GetIO().Fonts->GetGlyphRangesJapanese()))
  {
    // メニューフォントが読み込めなかったらエラーにする
    throw std::runtime_error("Cannot find any menu fonts.");
  }
#endif
}

//
// デストラクタ
//
Menu::~Menu()
{
}

//
// ファイルパスを取得する
//
bool Menu::getFilePath(std::string& path, const nfdfilteritem_t* filter)
{
  // ファイルダイアログから得るパス
  nfdchar_t* filepath{ nullptr };

  // ファイルダイアログを開く
  if (NFD_OpenDialog(&filepath, filter, 1, nullptr) == NFD_OKAY)
  {
    path = TCharToUtf8(filepath);
    return true;
  }

  return false;
}

//
// 描画する
//
void Menu::draw()
{
#if defined(IMGUI_VERSION)
  //
  // ImGui によるユーザインタフェース
  //

  // ImGui のフレームを準備する
  ImGui::NewFrame();

  // メニューの表示位置を決定する
  ImGui::SetNextWindowPos(ImVec2(4, 4), ImGuiCond_Once);
  ImGui::SetNextWindowSize(ImVec2(320, 98), ImGuiCond_Once);

  // メニュー表示開始
  ImGui::Begin(u8"コントロールパネル");

  // 光源
  if (ImGui::SliderFloat3(u8"光源位置", settings.light.position.data(), -10.0f, 10.0f, "%.2f"))
    light->loadPosition(settings.light.position);
  if (ImGui::ColorEdit3(u8"光源色", settings.light.diffuse.data(), ImGuiColorEditFlags_Float))
    light->loadDiffuse(settings.light.diffuse);

  // メニュー表示終了
  ImGui::End();

  // ImGui のフレームに描画する
  ImGui::Render();
#endif
}
