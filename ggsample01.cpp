// ウィンドウ関連の処理
#include "Window.h"

//
// アプリケーション本体
//
void app()
{
  // ウィンドウを作成する
  Window window("ggsample01");

  // 背景色を指定する
  glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

  // ウィンドウが開いている間繰り返す
  while (window)
  {
    // ウィンドウを消去する
    glClear(GL_COLOR_BUFFER_BIT);

    //
    // ここで OpenGL による描画を行う
    //
    //
    // ユーザインタフェース
    //
    ImGui::SetNextWindowSize(ImVec2(256, 228));
    ImGui::Begin("Control panel");
    ImGui::Text("Frame rate: %6.2f fps", ImGui::GetIO().Framerate);
    if (ImGui::Button("Quit")) window.setClose();
    ImGui::End();
    ImGui::Render();

    // カラーバッファを入れ替えてイベントを取り出す
    window.swapBuffers();
  }
}
