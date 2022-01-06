//
// ゲームグラフィックス特論宿題アプリケーション
//
#include "GgApp.h"

// プロジェクト名
#if !defined(PROJECT_NAME)
#  define PROJECT_NAME "ggsample01"
#endif

// 形状データ
static std::string model{ "logo.obj" };

// 光源データ
static GgSimpleShader::Light light
{
  { 0.2f, 0.2f, 0.2f, 1.0f }, // 環境光成分
  { 1.0f, 1.0f, 1.0f, 0.0f }, // 拡散反射光成分
  { 1.0f, 1.0f, 1.0f, 0.0f }, // 鏡面反射光成分
  { 0.5f, 0.5f, 1.0f, 1.0f }  // 光源位置
};

//
// アプリケーション本体
//
int GgApp::main(int argc, const char* const* argv)
{
  // ウィンドウを作成する (この行は変更しないでください)
  Window window{ argc > 1 ? argv[1] : PROJECT_NAME };

  // 図形データを読み込む (大きさを正規化する)
  GgSimpleObj object{ model, true };

  // シェーダを作成する
#if defined(GL_GLES_PROTOTYPES)
  const GgSimpleShader simple{ "simple_es3.vert", "simple_es3.frag" };
#else
  const GgSimpleShader simple{ "simple.vert", "simple.frag" };
#endif

  // 光源データから光源のバッファオブジェクトを作成する
  GgSimpleShader::LightBuffer lightBuffer{ light };

  // ビュー変換行列を設定する
  const GgMatrix mv{ ggLookat(0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f) };

#if defined(IMGUI_VERSION)
  //
  // ImGui の初期設定
  //

  // ImGui のコンテキストの設定と入出力のデータへの参照を得る
  ImGuiIO& io = ImGui::GetIO();
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

  // Setup Dear ImGui style
  //ImGui::StyleColorsDark();
  //ImGui::StyleColorsClassic();

  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
  // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
  // - Read 'docs/FONTS.txt' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
  //io.Fonts->AddFontDefault();
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
  ImFont* font = io.Fonts->AddFontFromFileTTF("Mplus1-Regular.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
  IM_ASSERT(font != NULL);
#endif

  // 背景色を設定する
  glClearColor(0.1f, 0.2f, 0.3f, 0.0f);

  // 隠面消去処理を設定する
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  // ウィンドウが開いている間繰り返す
  while (window)
  {
    // ウィンドウを消去する
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if defined(IMGUI_VERSION)
    //
    // ImGui によるユーザインタフェース
    //

    // ImGui のフレームを準備する
    ImGui::NewFrame();

    // 光源位置を決定する
    ImGui::SetNextWindowPos(ImVec2(4, 4), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(320, 92), ImGuiCond_Once);
    ImGui::Begin(u8"コントロールパネル");
    if (ImGui::SliderFloat3(u8"光源位置", light.position.data(), -10.0f, 10.0f, "%.2f"))
      lightBuffer.loadPosition(light.position);
    if (ImGui::ColorEdit3(u8"光源色", light.diffuse.data(), ImGuiColorEditFlags_Float))
      lightBuffer.loadDiffuse(light.diffuse);
    ImGui::End();

    // ImGui のフレームに描画する
    ImGui::Render();
#endif

    // オブジェクトの回転の変換行列を設定する
    const GgMatrix&& mr{ window.getRotationMatrix(0) };

    // 視点の平行移動の変換行列を設定する
    const GgMatrix&& mt{ window.getTranslationMatrix(1) };

    // 投影変換行列を設定する
    const GgMatrix&& mp{ ggPerspective(0.5f, window.getAspect(), 1.0f, 15.0f) };

    // シェーダプログラムを指定する
    simple.use(mp, mt * mv * mr, lightBuffer);

    // 図形を描画する
    object.draw();

    // カラーバッファを入れ替えてイベントを取り出す
    window.swapBuffers();
  }

  return 0;
}
