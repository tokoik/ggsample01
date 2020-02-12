// ウィンドウ関連の処理
#define USE_IMGUI
#include "Window.h"

// 光源データ
GgSimpleShader::Light light =
{
  { 0.2f, 0.2f, 0.2f, 1.0f }, // 環境光成分
  { 1.0f, 1.0f, 1.0f, 0.0f }, // 拡散反射光成分
  { 1.0f, 1.0f, 1.0f, 0.0f }, // 鏡面反射光成分
  { 0.0f, 0.0f, 1.0f, 1.0f }  // 光源位置
};

//
// アプリケーション本体
//
void app()
{
  // ウィンドウを作成する
  Window window("ggsample01");

  // シェーダを作成する
  const GgSimpleShader simple("simple.vert", "simple.frag");

  // 図形データを読み込む (大きさを正規化する)
  const GgSimpleObj object("bunny.obj", true);

  // 光源データから光源のバッファオブジェクトを作成する
  const GgSimpleShader::LightBuffer lightBuffer(light);

  // ビュー変換行列を設定する
  const GgMatrix mv(ggLookat(0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f));

#ifdef USE_IMGUI
  //
  // ImGui の初期設定
  //

  //ImGuiIO& io = ImGui::GetIO();
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
  //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
  //IM_ASSERT(font != NULL);
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

    // オブジェクトのモデル変換行列を設定する
    GgMatrix mm(window.getTrackball());

#ifdef USE_IMGUI
    // オブジェクトのオイラー角
    static float roll, pitch, yaw;

    // ImGui のフレームを準備する
    ImGui::NewFrame();

    // ImGui のフレームに一つ目の ImGui のウィンドウを描く
    ImGui::Begin("Control panel");
    ImGui::Text("Frame rate: %6.2f fps", ImGui::GetIO().Framerate);
    ImGui::SliderAngle("Roll", &roll);
    ImGui::SliderAngle("Pitch", &pitch);
    ImGui::SliderAngle("Yaw", &yaw);
    if (ImGui::Button("Quit")) window.setClose();
    ImGui::End();

    // モデル変換行列にオイラー角を乗じる
    mm = mm.rotateY(yaw).rotateX(pitch).rotateZ(roll);
#endif

    // 投影変換行列を設定する
    const GgMatrix mp(ggPerspective(0.5f, window.getAspect(), 1.0f, 15.0f));

    // シェーダプログラムを指定する
    simple.use(mp, mv * mm, lightBuffer);

    // 図形を描画する
    object.draw();

#ifdef USE_IMGUI
    // ImGui のフレームを重ねて表示する
    ImGui::Render();
#endif

    // カラーバッファを入れ替えてイベントを取り出す
    window.swapBuffers();
  }
}
