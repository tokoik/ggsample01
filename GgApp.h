#pragma once

/*

ゲームグラフィックス特論用補助プログラム GLFW3 版

Copyright (c) 2011-2025 Kohe Tokoi. All Rights Reserved.

Permission is hereby granted, free of charge,  to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction,  including without limitation the rights
to use, copy,  modify, merge,  publish, distribute,  sublicense,  and/or sell
copies or substantial portions of the Software.

The above  copyright notice  and this permission notice  shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING  BUT  NOT LIMITED  TO THE WARRANTIES  OF MERCHANTABILITY,
FITNESS  FOR  A PARTICULAR PURPOSE  AND NONINFRINGEMENT.  IN  NO EVENT  SHALL
KOHE TOKOI  BE LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER LIABILITY,  WHETHER IN
AN ACTION  OF CONTRACT,  TORT  OR  OTHERWISE,  ARISING  FROM,  OUT OF  OR  IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

///
/// ゲームグラフィックス特論宿題アプリケーションクラスの定義.
///
/// @file
/// @author Kohe Tokoi
/// @date July 17, 2025
///

// Dear ImGui を使うなら
#if !defined(GG_USE_IMGUI) && !defined(GG_NO_IMGUI)
#  if defined(__has_include)
#    if __has_include(<imgui.h>) || __has_include("imgui.h")
#      define GG_USE_IMGUI
#    endif
#  endif
#endif

// OpenXR を使うなら
//#define GG_USE_OPENXR

// 使用するマウスのボタン数
#if !defined(GG_BUTTON_COUNT)
#  define GG_BUTTON_COUNT 3
#endif

// 使用するユーザインタフェースの数
#if !defined(GG_INTERFACE_COUNT)
#  define GG_INTERFACE_COUNT 5
#endif

// 補助プログラム
#include "gg.h"
using namespace gg;

// 標準ライブラリ
#include <stdexcept>
#include <iostream>

// ImGui の組み込み
#if defined(GG_USE_IMGUI)
#  include "imgui.h"
#  include "imgui_impl_glfw.h"
#  include "imgui_impl_opengl3.h"
#endif

// OpenXR ライブラリの組み込み
#if defined(GG_USE_OPENXR)
#  if defined(_WIN32)
#    define XR_USE_PLATFORM_WIN32
#    define XR_USE_GRAPHICS_API_OPENGL
#    define GLFW_EXPOSE_NATIVE_WIN32
#    define GLFW_EXPOSE_NATIVE_WGL
#    include <GLFW/glfw3native.h>
#    include <windows.h>
#    include <unknwn.h>
#    if defined(_MSC_VER)
#      pragma comment(lib, "openxr_loader.lib")
#    endif
#  else
#    if !defined(__gl_h_)
#      define __gl_h_
#    endif
#    define XR_USE_PLATFORM_XLIB
#    define XR_USE_GRAPHICS_API_OPENGL
#    define GLFW_EXPOSE_NATIVE_X11
#    define GLFW_EXPOSE_NATIVE_GLX
#    include <GLFW/glfw3native.h>
#  endif
#  include <openxr/openxr.h>
#  include <openxr/openxr_platform.h>
#  include <algorithm>
#  include <cstdio>
#  include <cstring>
#  include <string>
#  include <utility>
#  include <vector>
#endif

///
/// ゲームグラフィックス特論宿題アプリケーションクラス.
///
class GgApp
{
public:

  ///
  /// コンストラクタ.
  ///
  /// @param major 使用する OpenGL の major 番号, 0 なら無指定.
  /// @param minor 使用する OpenGL の minor 番号, major 番号が 0 なら無視.
  ///
  GgApp(int major = 0, int minor = 1);

  ///
  /// コピーコンストラクタは使用しない.
  ///
  /// @param w コピー元のオブジェクト.
  ///
  GgApp(const GgApp& w) = delete;

  ///
  /// ムーブコンストラクタ.
  ///
  /// @param w ムーブ元のオブジェクト.
  ///
  GgApp(GgApp&& w) = default;

  ///
  /// デストラクタ.
  ///
  virtual ~GgApp();

  ///
  /// 代入演算子は使用しない.
  ///
  /// @param w 代入元のオブジェクト.
  /// @return 代入後のこのオブジェクトの参照.
  ///
  GgApp& operator=(const GgApp& w) = delete;

  ///
  /// ムーブ代入演算子.
  ///
  /// @param w ムーブ代入元のオブジェクト.
  /// @return ムーブ代入後のこのオブジェクトの参照.
  ///
  GgApp& operator=(GgApp&& w) = default;

  ///
  /// アプリケーション本体.
  ///
  /// @param argc コマンドライン引数の数.
  /// @param argv コマンドライン引数の配列.
  /// @return アプリケーションの終了ステータス.
  ///
  int main(int argc, const char* const* argv);

  ///
  /// ウィンドウ関連の処理.
  ///
  /// @note
  /// GLFW を使って OpenGL のウィンドウを操作するラッパークラス.
  ///
  class Window
  {
    // ウィンドウの識別子
    GLFWwindow* window{ nullptr };

    // ビューポートの横幅と高さ
    std::array<GLsizei, 2> size;

    // フレームバッファの横幅と高さ
    std::array<GLsizei, 2> fboSize;

#if defined(IMGUI_VERSION)
    // メニューバーの高さ
    GLsizei menubarHeight{ 0 };
#endif

    // ビューポートの縦横比
    GLfloat aspect{ 1.0f };

    // マウスの移動速度[X/Y/Z]
    std::array<GLfloat, 3> velocity{ 1.0f, 1.0f, 0.1f };

    // マウスボタンの状態
    std::array<bool, GG_BUTTON_COUNT> status{};

    // ユーザインタフェースのデータ構造
    struct HumanInterface
    {
      // 最後にタイプしたキー
      int lastKey{ 0 };

      // 矢印キー
      std::array<std::array<int, 2>, 4> arrow{};

      // マウスの現在位置
      std::array<GLfloat, 2> mouse{};

      // マウスホイールの回転量
      std::array<GLfloat, 2> wheel{};

      // 平行移動量[ボタン][直前/更新][X/Y/Z]
      std::array<std::array<gg::GgVector, 2>, GG_BUTTON_COUNT> translation{};

      // トラックボール
      std::array<gg::GgTrackball, GG_BUTTON_COUNT> rotation;

      // コンストラクタ
      HumanInterface()
      {
        resetTranslation();
      }

      //
      // マウスや矢印キーによる平行移動量を初期化する
      //
      void resetTranslation();

      //
      // 平行移動量と回転量を更新する (X, Y のみ, Z は wheel() で計算する)
      //
      void calcTranslation(int button, const std::array<GLfloat, 3>& velocity);
    };

    // ヒューマンインタフェースデバイスのデータ
    std::array<HumanInterface, GG_INTERFACE_COUNT> interfaceData;

    // ヒューマンインタフェースデバイスの番号
    int interfaceNo{ 0 };

    //
    // ユーザー定義のコールバック関数へのポインタ
    //
    void* userPointer{ nullptr };
    void (*resizeFunc)(const Window* window, int width, int height){ nullptr };
    void (*keyboardFunc)(const Window* window, int key, int scancode, int action, int mods){ nullptr };
    void (*mouseFunc)(const Window* window, int button, int action, int mods){ nullptr };
    void (*wheelFunc)(const Window* window, double x, double y){ nullptr };

    //
    // ウィンドウのサイズ変更時の処理
    //
    static void resize(GLFWwindow* window, int width, int height);

    //
    // キーボードをタイプした時の処理
    //
    static void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);

    //
    // マウスボタンを操作したときの処理
    //
    static void mouse(GLFWwindow* window, int button, int action, int mods);

    //
    // マウスホイールを操作した時の処理
    //
    static void wheel(GLFWwindow* window, double x, double y);

  public:

    ///
    /// コンストラクタ.
    ///
    /// @param title ウィンドウタイトルの文字列.
    /// @param width 開くウィンドウの幅, フルスクリーン時は無視され実際のディスプレイの幅が使われる.
    /// @param height 開くウィンドウの高さ, フルスクリーン時は無視され実際のディスプレイの高さが使われる.
    /// @param fullscreen フルスクリーン表示を行うディスプレイ番号, 0 ならフルスクリーン表示を行わない.
    /// @param share 共有するコンテキスト, nullptr ならコンテキストを共有しない.
    ///
    Window(const std::string& title = "GLFW Window", int width = 640, int height = 480,
      int fullscreen = 0, GLFWwindow* share = nullptr);

    ///
    /// コピーコンストラクタは使用しない.
    ///
    /// @param w コピー元のウィンドウ.
    ///
    Window(const Window& w) = delete;

    ///
    /// ムーブコンストラクタ.
    ///
    /// @param w ムーブ代入元のウィンドウ.
    ///
    Window(Window&& w) noexcept;

    ///
    /// デストラクタ.
    ///
    virtual ~Window()
    {
      // ウィンドウが作成されていなければ戻る
      if (!window) return;

      // ウィンドウを破棄する
      glfwDestroyWindow(window);
    }

    ///
    /// 代入演算子は使用しない.
    ///
    /// @param w 代入元のウィンドウ.
    /// @return 代入後のこのオブジェクトの参照.
    ///
    Window& operator=(const Window& w) = delete;

    ///
    /// ムーブ代入演算子.
    ///
    /// @param w ムーブ代入元のウィンドウ.
    /// @return ムーブ代入後のこのオブジェクトの参照.
    ///
    Window& operator=(Window&& w) noexcept;

    ///
    /// ウィンドウの識別子のポインタを取得する.
    ///
    /// @return GLFWwindow 型のウィンドウ識別子のポインタ.
    ///
    auto* get() const
    {
      return window;
    }

    ///
    /// ウィンドウのクローズフラグを設定する.
    ///
    /// @param flag クローズフラグ, 0 (GLFW_FALSE) 以外ならウィンドウを閉じる.
    ///
    void setClose(int flag = GLFW_TRUE) const
    {
      glfwSetWindowShouldClose(window, flag);
    }

    ///
    /// ウィンドウを閉じるべきかどうか調べる.
    ///
    /// @return ウィンドウを閉じるべきなら true.
    ///
    bool shouldClose() const
    {
      // ウィンドウを閉じるべきなら true を返す
      return glfwWindowShouldClose(window) != GLFW_FALSE;
    }

    ///
    /// イベントを取得してループを継続すべきかどうか調べる.
    ///
    /// @return ループを継続すべきなら true.
    ///
    explicit operator bool();

    ///
    /// カラーバッファを入れ替える.
    ///
    void swapBuffers() const;

    ///
    /// ビューポートを元のサイズに復帰する.
    ///
    void restoreViewport() const
    {
      if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) glViewport(0, 0, fboSize[0], fboSize[1]);
    }

    ///
    /// ビューポートのサイズを更新する.
    ///
    void updateViewport();

#if defined(IMGUI_VERSION)
    ///
    /// 表示領域をメニューバーの高さだけ減らす.
    ///
    /// @param メニューバーの高さ.
    ///
    void setMenubarHeight(GLsizei height)
    {
      // メニューバーの高さを保存する
      menubarHeight = height;

      // ビューポートを復帰する
      updateViewport();
    }
#endif

    ///
    /// ウィンドウの横幅を得る.
    ///
    /// @return ウィンドウの横幅.
    ///
    auto getWidth() const
    {
      return size[0];
    }

    ///
    /// ウィンドウの高さを得る.
    ///
    /// @return ウィンドウの高さ.
    ///
    auto getHeight() const
    {
      return size[1];
    }

    ///
    /// FBO の横幅を得る.
    ///
    /// @return FBO の横幅.
    ///
    auto getFboWidth() const
    {
      return fboSize[0];
    }

    ///
    /// FBO の高さを得る.
    ///
    /// @return FBO の高さ.
    ///
    auto getFboHeight() const
    {
      return fboSize[1];
    }

    ///
    /// ウィンドウのサイズを得る.
    ///
    /// @return ウィンドウの幅と高さを格納した GLsizei 型の 2 要素の配列の参照.
    ///
    const auto& getSize() const
    {
      return size;
    }

    ///
    /// ウィンドウのサイズを得る.
    ///
    /// @param size ウィンドウの幅と高さを格納した GLsizei 型の 2 要素の配列.
    ///
    void getSize(GLsizei* size) const
    {
      size[0] = getWidth();
      size[1] = getHeight();
    }

    ///
    /// FBO のサイズを得る.
    ///
    /// @return FBO の幅と高さを格納した GLsizei 型の 2 要素の配列.
    ///
    const auto& getFboSize() const
    {
      return fboSize;
    }

    ///
    /// FBO のサイズを得る.
    ///
    /// @param fboSize FBO の幅と高さを格納する GLsizei 型の 2 要素の配列.
    ///
    void getFboSize(GLsizei* fboSize) const
    {
      fboSize[0] = getFboWidth();
      fboSize[1] = getFboHeight();
    }

    ///
    /// ウィンドウの縦横比を得る.
    ///
    /// @return ウィンドウの縦横比.
    ///
    auto getAspect() const
    {
      return aspect;
    }

    ///
    /// キーが押されているかどうかを判定する.
    ///
    /// @return キーが押されていれば true.
    ///
    bool getKey(int key) const
    {
#if defined(IMGUI_VERSION)
      // ImGui がキーボードを使うときはキーボードの処理を行わない
      if (ImGui::GetIO().WantCaptureKeyboard) return false;
#endif

      return glfwGetKey(window, key) != GLFW_RELEASE;
    }

    ///
    /// インタフェースを選択する.
    ///
    /// @param no インターフェース番号.
    ///
    void selectInterface(int no)
    {
      assert(static_cast<size_t>(no) < interfaceData.size());
      interfaceNo = no;
    }

    ///
    /// マウスの移動速度を設定する.
    ///
    /// @param vx x 方向の移動速度.
    /// @param vy y 方向の移動速度.
    /// @param vz z 方向の移動速度.
    ///
    void setVelocity(GLfloat vx, GLfloat vy, GLfloat vz = 0.1f)
    {
      velocity = std::array<GLfloat, 3>{ vx, vy, vz };
    }

    ///
    /// 最後にタイプしたキーを得る.
    ///
    /// @return 最後にタイプしたキーの文字.
    ///
    int getLastKey()
    {
      auto& current_if{ interfaceData[interfaceNo] };
      const int key{ current_if.lastKey };
      current_if.lastKey = 0;
      return key;
    }

    ///
    /// 矢印キーの現在の値を得る.
    ///
    /// @param direction 方向 (0: X, 1:Y).
    /// @param mods 修飾キーの状態 (0:なし, 1, SHIFT, 2: CTRL, 3: ALT).
    /// @return 矢印キーの値.
    ///
    auto getArrow(int direction = 0, int mods = 0) const
    {
      const auto& current_if{ interfaceData[interfaceNo] };
      return static_cast<GLfloat>(current_if.arrow[mods & 3][direction & 1]);
    }

    ///
    /// 矢印キーの現在の X 値を得る.
    ///
    /// @param mods 修飾キーの状態 (0:なし, 1, SHIFT, 2: CTRL, 3: ALT).
    /// @return 矢印キーの X 値.
    ///
    auto getArrowX(int mods = 0) const
    {
      return getArrow(0, mods);
    }

    ///
    /// 矢印キーの現在の Y 値を得る.
    ///
    /// @param mods 修飾キーの状態 (0:なし, 1, SHIFT, 2: CTRL, 3: ALT).
    /// @return 矢印キーの Y 値.
    ///
    auto getArrowY(int mods = 0) const
    {
      return getArrow(1, mods);
    }

    ///
    /// 矢印キーの現在の値を得る.
    ///
    /// @param arrow 矢印キーの値を格納する GLfloat[2] の配列.
    /// @param mods 修飾キーの状態 (0:なし, 1, SHIFT, 2: CTRL, 3: ALT).
    ///
    void getArrow(GLfloat* arrow, int mods = 0) const
    {
      arrow[0] = getArrowX(mods);
      arrow[1] = getArrowY(mods);
    }

    ///
    /// SHIFT キーを押しながら矢印キーを押したときの現在の X 値を得る.
    ///
    /// @return SHIFT キーを押しながら矢印キーを押したときの現在の X 値.
    ///
    auto getShiftArrowX() const
    {
      return getArrow(0, 1);
    }

    ///
    /// SHIFT キーを押しながら矢印キーを押したときの現在の Y 値を得る.
    ///
    /// @return SHIFT キーを押しながら矢印キーを押したときの現在の Y 値.
    ///
    auto getShiftArrowY() const
    {
      return getArrow(1, 1);
    }

    ///
    /// SHIFT キーを押しながら矢印キーを押したときの現在の値を得る.
    ///
    /// @param shift_arrow SHIFT キーを押しながら矢印キーを押したときの値を格納する GLfloat 型の 2 要素の配列.
    ///
    void getShiftArrow(GLfloat* shift_arrow) const
    {
      shift_arrow[0] = getShiftArrowX();
      shift_arrow[1] = getShiftArrowY();
    }

    ///
    /// CTRL キーを押しながら矢印キーを押したときの現在の X 値を得る.
    ///
    /// @return CTRL キーを押しながら矢印キーを押したときの現在の X 値.
    ///
    auto getControlArrowX() const
    {
      return getArrow(0, 2);
    }

    ///
    /// CTRL キーを押しながら矢印キーを押したときの現在の Y 値を得る.
    ///
    /// @return CTRL キーを押しながら矢印キーを押したときの現在の Y 値.
    ///
    auto getControlArrowY() const
    {
      return getArrow(1, 2);
    }

    ///
    /// CTRL キーを押しながら矢印キーを押したときの現在の値を得る.
    ///
    /// @param control_arrow CTRL キーを押しながら矢印キーを押したときの値を格納する GLfloat 型の 2 要素の配列.
    ///
    void getControlArrow(GLfloat* control_arrow) const
    {
      control_arrow[0] = getControlArrowX();
      control_arrow[1] = getControlArrowY();
    }

    ///
    /// ALT キーを押しながら矢印キーを押したときの現在の X 値を得る.
    ///
    /// @return ALT キーを押しながら矢印キーを押したときの現在の X 値.
    ///
    auto getAltArrowX() const
    {
      return getArrow(0, 3);
    }

    ///
    /// ALT キーを押しながら矢印キーを押したときの現在の Y 値を得る.
    ///
    /// @return ALT キーを押しながら矢印キーを押したときの現在の Y 値.
    ///
    auto getAltArrowY() const
    {
      return getArrow(1, 3);
    }

    ///
    /// ALT キーを押しながら矢印キーを押したときの現在の値を得る.
    ///
    /// @param alt_arrow ALT キーを押しながら矢印キーを押したときの値を格納する GLfloat 型の 2 要素の配列.
    ///
    void getAltArrow(GLfloat* alt_arrow) const
    {
      alt_arrow[0] = getAltArrowX();
      alt_arrow[1] = getAltArrowY();
    }

    ///
    /// マウスカーソルの現在位置を得る.
    ///
    /// @return マウスカーソルの現在位置を格納した GLfloat 型の 2 要素の配列.
    ///
    const auto* getMouse() const
    {
      const auto& current_if{ interfaceData[interfaceNo] };
      return current_if.mouse.data();
    }

    ///
    /// マウスカーソルの現在位置を得る.
    ///
    /// @param position マウスカーソルの現在位置を格納した GLfloat 型の 2 要素の配列.
    ///
    void getMouse(GLfloat* position) const
    {
      const auto& current_if{ interfaceData[interfaceNo] };
      position[0] = current_if.mouse[0];
      position[1] = current_if.mouse[1];
    }

    ///
    /// マウスカーソルの現在位置を得る.
    ///
    /// @param direction 方向 (0:X, 1:Y).
    /// @return direction 方向のマウスカーソルの現在位置.
    ///
    auto getMouse(int direction) const
    {
      const auto& current_if{ interfaceData[interfaceNo] };
      return current_if.mouse[direction & 1];
    }

    ///
    /// マウスカーソルの現在位置の X 座標を得る.
    ///
    /// @return direction 方向のマウスカーソルの X 方向の現在位置.
    ///
    auto getMouseX() const
    {
      const auto& current_if{ interfaceData[interfaceNo] };
      return current_if.mouse[0];
    }

    ///
    /// マウスカーソルの現在位置の Y 座標を得る.
    ///
    /// @return direction 方向のマウスカーソルの Y 方向の現在位置.
    ///
    auto getMouseY() const
    {
      const auto& current_if{ interfaceData[interfaceNo] };
      return current_if.mouse[1];
    }

    ///
    /// マウスホイールの回転量を得る.
    ///
    /// @return マウスホイールの回転量を格納した GLfloat 型の 2 要素の配列.
    ///
    const auto* getWheel() const
    {
      const auto& current_if{ interfaceData[interfaceNo] };
      return current_if.wheel.data();
    }

    ///
    /// マウスホイールの回転量を得る.
    ///
    /// @param rotation マウスホイールの回転量を格納した GLfloat 型の 2 要素の配列.
    ///
    void getWheel(GLfloat* rotation) const
    {
      const auto& current_if{ interfaceData[interfaceNo] };
      rotation[0] = current_if.wheel[0];
      rotation[1] = current_if.wheel[1];
    }

    ///
    /// マウスホイールの回転量を得る.
    ///
    /// @param direction 方向 (0:X, 1:Y).
    /// @return direction 方向のマウスホイールの回転量.
    ///
    auto getWheel(int direction) const
    {
      const auto& current_if{ interfaceData[interfaceNo] };
      return current_if.wheel[direction & 1];
    }

    ///
    /// マウスホイールの X 方向の回転量を得る.
    ///
    /// @return マウスホイールの X 方向の回転量.
    ///
    auto getWheelX() const
    {
      const auto& current_if{ interfaceData[interfaceNo] };
      return current_if.wheel[0];
    }

    ///
    /// マウスホイールの Y 方向の回転量を得る.
    ///
    /// @return マウスホイールの Y 方向の回転量.
    ///
    auto getWheelY() const
    {
      const auto& current_if{ interfaceData[interfaceNo] };
      return current_if.wheel[1];
    }

    ///
    /// トラックボール処理を考慮したマウスによるスクロールの変換行列を得る.
    ///
    /// @param button 平行移動量を取得するマウスボタン (GLFW_MOUSE_BUTTON_[1,2]).
    /// @return 平行移動量を格納した GLfloat[3] の配列のポインタ.
    ///
    const auto& getTranslation(int button = GLFW_MOUSE_BUTTON_1) const
    {
      const auto& current_if{ interfaceData[interfaceNo] };
      assert(button >= GLFW_MOUSE_BUTTON_1 && button < GLFW_MOUSE_BUTTON_1 + GG_BUTTON_COUNT);
      return current_if.translation[button][1];
    }

    ///
    /// マウスによって視点の平行移動の変換行列を得る.
    ///
    /// @param button 平行移動量を取得するマウスボタン (GLFW_MOUSE_BUTTON_[1,2]).
    /// @return 視点座標系で平行移動を行う GgMatrix 型の変換行列.
    ///
    auto getTranslationMatrix(int button = GLFW_MOUSE_BUTTON_1) const
    {
      const auto& current_if{ interfaceData[interfaceNo] };
      assert(button >= GLFW_MOUSE_BUTTON_1 && button < GLFW_MOUSE_BUTTON_1 + GG_BUTTON_COUNT);
      const auto& t{ current_if.translation[button][1] };

      gg::GgMatrix m
      {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        t[0], t[1], t[2], 1.0f
      };

      return m;
    }

    ///
    /// マウスによってオブジェクトの平行移動の変換行列を得る.
    ///
    /// @param button 平行移動量を取得するマウスボタン (GLFW_MOUSE_BUTTON_[1,2]).
    /// @return クリッピング座標系で平行移動を行う GgMatrix 型の変換行列.
    ///
    auto getScrollMatrix(int button = GLFW_MOUSE_BUTTON_1) const
    {
      const auto& current_if{ interfaceData[interfaceNo] };
      assert(button >= GLFW_MOUSE_BUTTON_1 && button < GLFW_MOUSE_BUTTON_1 + GG_BUTTON_COUNT);
      const auto& t{ current_if.translation[button][1] };

      gg::GgMatrix m
      {
        t[2] + 1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, t[2] + 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        t[0], t[1], 0.0f, 1.0f
      };

      return m;
    }

    ///
    /// トラックボールの回転変換行列を得る.
    ///
    /// @param button 回転変換行列を取得するマウスボタン (GLFW_MOUSE_BUTTON_[1,2]).
    /// @return 回転を行う GgQuaternion 型の四元数.
    ///
    auto getRotation(int button = GLFW_MOUSE_BUTTON_1) const
    {
      const auto& current_if{ interfaceData[interfaceNo] };
      assert(button >= GLFW_MOUSE_BUTTON_1 && button < GLFW_MOUSE_BUTTON_1 + GG_BUTTON_COUNT);
      return current_if.rotation[button].getQuaternion();
    }

    ///
    /// トラックボールの回転変換行列を得る.
    ///
    /// @param button 回転変換行列を取得するマウスボタン (GLFW_MOUSE_BUTTON_[1,2]).
    /// @return 回転を行う GgMatrix 型の変換行列.
    ///
    auto getRotationMatrix(int button = GLFW_MOUSE_BUTTON_1) const
    {
      const auto& current_if{ interfaceData[interfaceNo] };
      assert(button >= GLFW_MOUSE_BUTTON_1 && button < GLFW_MOUSE_BUTTON_1 + GG_BUTTON_COUNT);
      return current_if.rotation[button].getMatrix();
    }

    ///
    /// トラックボール処理を初期化する.
    ///
    void resetRotation()
    {
      // トラックボールを初期化する
      for (auto& tb : interfaceData[interfaceNo].rotation) tb.reset();
    }

    ///
    /// 平行移動量を初期化する.
    ///
    void resetTranslation()
    {
      // 現在のインターフェースの平行移動量を初期化する
      interfaceData[interfaceNo].resetTranslation();
    }

    ///
    /// トラックボール・マウスホイール・矢印キーの値を初期化する.
    ///
    void reset()
    {
      // トラックボール処理を初期化する
      resetRotation();

      // 平行移動量を初期化する
      resetTranslation();
    }

    ///
    /// ユーザーポインタを取り出す.
    ///
    /// @return 保存されているユーザポインタ.
    ///
    void* getUserPointer() const
    {
      return userPointer;
    }

    ///
    /// 任意のユーザポインタを保存する.
    ///
    /// @param pointer 保存するユーザポインタ.
    ///
    void setUserPointer(void* pointer)
    {
      userPointer = pointer;
    }

    ///
    /// ユーザ定義の resize 関数を設定する.
    ///
    /// @param func ユーザ定義の resize 関数, ウィンドウのサイズ変更時に呼び出される.
    ///
    void setResizeFunc(void (*func)(const Window* window, int width, int height))
    {
      resizeFunc = func;
    }

    ///
    /// ユーザ定義の keyboard 関数を設定する.
    ///
    /// @param func ユーザ定義の keyboard 関数, キーボードの操作時に呼び出される.
    ///
    void setKeyboardFunc(void (*func)(const Window* window, int key, int scancode, int action, int mods))
    {
      keyboardFunc = func;
    }

    ///
    /// ユーザ定義の mouse 関数を設定する.
    ///
    /// @param func ユーザ定義の mouse 関数, マウスボタンの操作時に呼び出される.
    ///
    void setMouseFunc(void (*func)(const Window* window, int button, int action, int mods))
    {
      mouseFunc = func;
    }

    ///
    /// ユーザ定義の wheel 関数を設定する.
    ///
    /// @param func ユーザ定義の wheel 関数, マウスホイールの操作時に呼び出される.
    ///
    void setWheelFunc(void (*func)(const Window* window, double x, double y))
    {
      wheelFunc = func;
    }
  };

#if defined(GG_USE_OPENXR)
  ///
  /// OpenXR 関連の処理.
  ///
  /// @note
  /// OpenXR を操作するラッパークラス（シングルトン）.
  /// OpenGL のコンテキストを使うので, GgApp::Window を作成した後に
  /// initialize() を呼び, ウィンドウを破棄する前に terminate() を呼ぶこと.
  ///
  class OpenXR
  {
  public:

    ///
    /// コントローラーの手の識別子.
    ///
    enum Hand
    {
      Left = 0,   ///< 左手
      Right = 1,  ///< 右手
      Count = 2   ///< 手の総数
    };

  private:

    // OpenXR のインスタンスとシステム
    XrInstance instance{ XR_NULL_HANDLE };
    XrSystemId systemId{ XR_NULL_SYSTEM_ID };

    // OpenXR のシステムの名前
    std::string systemName;

    // OpenXR のセッション
    XrSession session{ XR_NULL_HANDLE };
    XrSessionState sessionState{ XR_SESSION_STATE_UNKNOWN };

    // OpenXR の参照空間
    XrSpace appSpace{ XR_NULL_HANDLE };
    XrReferenceSpaceType referenceSpaceType{ XR_REFERENCE_SPACE_TYPE_STAGE };

    // ビュー設定とステート
    std::vector<XrViewConfigurationView> views;
    std::vector<XrView> viewStates;

    // OpenXR のスワップチェーン
    std::vector<XrSwapchain> swapchains;
    std::vector<std::vector<XrSwapchainImageOpenGLKHR>> swapchainImages;

    // OpenXR へのレンダリングに使う FBO とデプスバッファ (ビューの数だけ確保する)
    std::vector<GLuint> openxrFbo;
    std::vector<GLuint> openxrDepth;

    // スワップチェーンのカラーフォーマットが sRGB なら true
    bool swapchainIsSrgb{ true };

    // 環境の合成方法
    XrEnvironmentBlendMode blendMode{ XR_ENVIRONMENT_BLEND_MODE_OPAQUE };

    // フレームの同期状態
    XrFrameState frameState{ XR_TYPE_FRAME_STATE };
    bool isSessionRunning{ false };

    // xrBeginFrame() を呼んで xrEndFrame() を呼んでいない状態なら true
    bool frameBegun{ false };

    // 視点の姿勢が取得できていれば true
    bool viewPoseValid{ false };

    // 初期化が完了していれば true
    bool initialized{ false };

    // 各フレームで取得したスワップチェーンイメージのインデックス
    std::vector<uint32_t> currentImageIndex;

    // スワップチェーンイメージを取得中のビューなら true
    std::vector<bool> imageAcquired;

    // ミラー表示を行うビューの番号, ミラー表示を行わないなら -1
    int mirrorView{ 0 };

    // OpenXR のミラー表示を行うウィンドウ
    const Window* window{ nullptr };

    // アクションセット
    XrActionSet actionSet{ XR_NULL_HANDLE };

    // アクション定義
    XrAction aimPoseAction{ XR_NULL_HANDLE };
    XrAction gripPoseAction{ XR_NULL_HANDLE };
    XrAction triggerAction{ XR_NULL_HANDLE };
    XrAction gripAction{ XR_NULL_HANDLE };
    XrAction thumbstickAction{ XR_NULL_HANDLE };
    XrAction thumbstickClickAction{ XR_NULL_HANDLE };
    XrAction primaryButtonAction{ XR_NULL_HANDLE };
    XrAction secondaryButtonAction{ XR_NULL_HANDLE };
    XrAction menuButtonAction{ XR_NULL_HANDLE };
    XrAction hapticAction{ XR_NULL_HANDLE };

    // コントローラーのスペース
    XrSpace aimSpace[Hand::Count]{ XR_NULL_HANDLE, XR_NULL_HANDLE };
    XrSpace gripSpace[Hand::Count]{ XR_NULL_HANDLE, XR_NULL_HANDLE };

    // コントローラーの状態キャッシュ
    struct ControllerState
    {
      bool isTracked{ false };
      XrPosef gripPose{ { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } };
      XrPosef aimPose{ { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } };
      float trigger{ 0.0f };
      float grip{ 0.0f };
      std::array<float, 2> thumbstick{ 0.0f, 0.0f };
      bool thumbstickClick{ false };
      bool primaryButton{ false };
      bool secondaryButton{ false };
      bool menuButton{ false };
    };
    ControllerState controllerStates[Hand::Count];

    // サポートするパスの文字列
    XrPath handSubactionPath[Hand::Count]{ XR_NULL_PATH, XR_NULL_PATH };

    //
    // アクションシステムを初期化する
    //
    void initActions();

    //
    // アクション状態を更新する
    //
    void pollActions();

    //
    // OpenXR のイベントを処理する
    //
    void pollEvents();

    //
    // スワップチェーンを作成する
    //
    void createSwapchains();

    //
    // 描画中のフレームを合成器に転送する
    //
    void endFrame();

    //
    // OpenXR のハンドルを破棄する (OpenGL の資源には触れない)
    //
    void destroyXr();

    //
    // ミラー表示を行う
    //
    void blitMirror() const;

    //
    // コンストラクタ
    //
    OpenXR();

    //
    // デストラクタ
    //
    virtual ~OpenXR();

  public:

    // シングルトンなのでコピー・ムーブ禁止
    OpenXR(const OpenXR&) = delete;
    OpenXR& operator=(const OpenXR&) = delete;
    OpenXR(OpenXR&&) = delete;
    OpenXR& operator=(OpenXR&&) = delete;

    ///
    /// OpenXR のセッションを作成する.
    ///
    /// @param window ミラー表示を行うウィンドウ.
    /// @param spaceType 使用する参照空間のタイプ（デフォルトは XR_REFERENCE_SPACE_TYPE_STAGE）.
    /// @param appName OpenXR のランタイムに通知するアプリケーション名.
    /// @return OpenXR の static object の参照.
    ///
    /// @note
    /// 初期化に失敗したときは確保した資源を解放したうえで
    /// std::runtime_error を投げる. 二度目以降の呼び出しは
    /// 初期化済みのオブジェクトをそのまま返す.
    ///
    static OpenXR& initialize(const Window& window,
      XrReferenceSpaceType spaceType = XR_REFERENCE_SPACE_TYPE_STAGE,
      const char* appName = "GgApp");

    ///
    /// OpenXR のセッションを破棄する.
    ///
    /// @note
    /// 何度呼んでも安全である. OpenGL の資源を解放するので,
    /// ウィンドウ (OpenGL のコンテキスト) が有効なうちに呼ぶこと.
    ///
    void terminate();

    ///
    /// OpenXR による描画開始.
    ///
    /// @return このフレームで描画を行うべきなら true.
    ///
    /// @note
    /// イベントの処理, フレームの同期 (xrWaitFrame / xrBeginFrame),
    /// 視点の姿勢の取得 (xrLocateViews), コントローラの状態の更新を行う.
    /// 描画が不要なフレームでは内部で xrEndFrame() まで済ませて false を
    /// 返すので, false のときは select() / commit() / submit() を
    /// 呼んではならない (呼んでも安全に無視される).
    ///
    bool begin();

    ///
    /// 描画対象の目を指定してフレームバッファとビューポートを設定する.
    ///
    /// @param eye 表示する目 (0: 左目, 1: 右目).
    ///
    void select(int eye);

    ///
    /// 描画対象の目を指定する (旧 LibOVR 仕様からの移行用オーバーロード).
    ///
    /// @param eye 表示する目 (0: 左目, 1: 右目).
    /// @param screen HMD の視野の視錐台のタンジェント (tanLeft, tanRight, tanDown, tanUp).
    /// @param position HMD の位置 (x, y, z).
    /// @param orientation HMD の方向の四元数 (x, y, z, w).
    ///
    void select(int eye, GLfloat* screen, GLfloat* position, GLfloat* orientation);

    ///
    /// 指定した目の描画を完了する.
    ///
    /// @param eye 完了した目のインデックス (0: 左目, 1: 右目).
    ///
    /// @note
    /// 描画先をウィンドウに戻す. スワップチェーンイメージの解放は
    /// ミラー表示を行った後の submit() の中で行う.
    ///
    void commit(int eye);

    ///
    /// フレームを転送して HMD に表示する.
    ///
    /// @param mirror true ならウィンドウへのミラー表示を行う, デフォルトは true.
    /// @return フレームの転送に成功したら true.
    ///
    /// @note
    /// ミラー表示を行ってからスワップチェーンイメージを解放し,
    /// 合成レイヤを組み立てて xrEndFrame() を呼ぶ.
    /// ミラー表示するビューの番号は setMirror() で変更できる.
    ///
    bool submit(bool mirror = true);

    ///
    /// ミラー表示を行うビューの番号を設定する.
    ///
    /// @param eye ミラー表示を行うビューの番号, -1 ならミラー表示を行わない.
    ///
    void setMirror(int eye);

    ///
    /// ミラー表示を行うビューの番号を取得する.
    ///
    /// @return ミラー表示を行うビューの番号, ミラー表示を行わないなら -1.
    ///
    int getMirror() const;

    ///
    /// セッションが実行中かどうか調べる.
    ///
    /// @return セッションが実行中なら true.
    ///
    bool isRunning() const;

    ///
    /// アプリケーションが入力を受け付けているかどうか調べる.
    ///
    /// @return XR_SESSION_STATE_FOCUSED なら true.
    ///
    bool isFocused() const;

    ///
    /// OpenXR のシステム (HMD) の名前を取得する.
    ///
    /// @return システムの名前の文字列.
    ///
    const std::string& getSystemName() const;

    ///
    /// 指定した目の透視投影変換行列を取得する.
    ///
    /// @param eye 表示する目 (0: 左目, 1: 右目).
    /// @param zNear 前方面の位置 (デフォルトは 0.1f).
    /// @param zFar 後方面の位置 (デフォルトは 100.0f).
    /// @return 透視投影変換行列 (GgMatrix).
    ///
    gg::GgMatrix getProjectionMatrix(int eye, GLfloat zNear = 0.1f, GLfloat zFar = 100.0f) const;

    ///
    /// 指定した目のビュー変換行列（ワールド座標系から視点座標系への変換）を取得する.
    ///
    /// @param eye 表示する目 (0: 左目, 1: 右目).
    /// @return ビュー変換行列 (GgMatrix).
    ///
    gg::GgMatrix getViewMatrix(int eye) const;

    ///
    /// 指定した目の姿勢行列（視点のモデル変換行列: 位置と回転）を取得する.
    ///
    /// @param eye 表示する目 (0: 左目, 1: 右目).
    /// @return 姿勢行列 (GgMatrix).
    ///
    gg::GgMatrix getPoseMatrix(int eye) const;

    ///
    /// 指定した目の視点位置を取得する.
    ///
    /// @param eye 表示する目 (0: 左目, 1: 右目).
    /// @return 視点位置 (GgVector).
    ///
    gg::GgVector getPosition(int eye) const;

    ///
    /// 指定した目の視線方向の回転四元数を取得する.
    ///
    /// @param eye 表示する目 (0: 左目, 1: 右目).
    /// @return 回転四元数 (GgQuaternion).
    ///
    gg::GgQuaternion getOrientation(int eye) const;

    ///
    /// 指定した目の視野角情報 (XrFovf) を取得する.
    ///
    /// @param eye 表示する目 (0: 左目, 1: 右目).
    /// @return 視野角構造体の参照.
    ///
    const XrFovf& getFov(int eye) const;

    ///
    /// 指定した目の姿勢情報 (XrPosef) を取得する.
    ///
    /// @param eye 表示する目 (0: 左目, 1: 右目).
    /// @return 姿勢構造体の参照.
    ///
    const XrPosef& getPose(int eye) const;

    ///
    /// 視点の姿勢が有効かどうか調べる.
    ///
    /// @return 直前の begin() で視点の位置と向きが取得できていれば true.
    ///
    bool isPoseValid() const;

    ///
    /// レンダリング推奨解像度の横幅を取得する.
    ///
    /// @param eye 表示する目 (デフォルトは 0: 左目).
    /// @return レンダリング画像の幅 (ピクセル).
    ///
    GLsizei getWidth(int eye = 0) const;

    ///
    /// レンダリング推奨解像度の高さを取得する.
    ///
    /// @param eye 表示する目 (デフォルトは 0: 左目).
    /// @return レンダリング画像の高さ (ピクセル).
    ///
    GLsizei getHeight(int eye = 0) const;

    ///
    /// アスペクト比 (幅 / 高さ) を取得する.
    ///
    /// @param eye 表示する目 (デフォルトは 0: 左目).
    /// @return アスペクト比.
    ///
    GLfloat getAspect(int eye = 0) const;

    ///
    /// ビューの総数を取得する (通常は 2).
    ///
    /// @return ビューの総数.
    ///
    uint32_t getViewCount() const;

    ///
    /// 現在の参照空間タイプを取得する.
    ///
    /// @return 参照空間タイプ (XR_REFERENCE_SPACE_TYPE_STAGE または XR_REFERENCE_SPACE_TYPE_LOCAL).
    ///
    XrReferenceSpaceType getReferenceSpaceType() const;

    ///
    /// コントローラーがトラッキングされているか取得する.
    ///
    /// @param hand 対象の手 (0: 左手 Hand::Left, 1: 右手 Hand::Right).
    /// @return トラッキングされていれば true.
    ///
    bool isTracked(int hand) const;

    ///
    /// コントローラーのグリップ位置・姿勢を表すモデル変換行列を取得する.
    ///
    /// @param hand 対象の手 (0: 左手 Hand::Left, 1: 右手 Hand::Right).
    /// @return グリップの変換行列 (GgMatrix).
    ///
    gg::GgMatrix getGripMatrix(int hand) const;

    ///
    /// コントローラーのポインティング（エイム）方向を表すモデル変換行列を取得する.
    ///
    /// @param hand 対象の手 (0: 左手 Hand::Left, 1: 右手 Hand::Right).
    /// @return エイムの変換行列 (GgMatrix).
    ///
    gg::GgMatrix getAimMatrix(int hand) const;

    ///
    /// コントローラーのグリップ位置を取得する.
    ///
    /// @param hand 対象の手 (0: 左手 Hand::Left, 1: 右手 Hand::Right).
    /// @return グリップ位置 (GgVector).
    ///
    gg::GgVector getGripPosition(int hand) const;

    ///
    /// コントローラーのグリップ回転四元数を取得する.
    ///
    /// @param hand 対象の手 (0: 左手 Hand::Left, 1: 右手 Hand::Right).
    /// @return グリップ回転四元数 (GgQuaternion).
    ///
    gg::GgQuaternion getGripOrientation(int hand) const;

    ///
    /// コントローラーのエイム位置を取得する.
    ///
    /// @param hand 対象の手 (0: 左手 Hand::Left, 1: 右手 Hand::Right).
    /// @return エイム位置 (GgVector).
    ///
    gg::GgVector getAimPosition(int hand) const;

    ///
    /// コントローラーのエイム回転四元数を取得する.
    ///
    /// @param hand 対象の手 (0: 左手 Hand::Left, 1: 右手 Hand::Right).
    /// @return エイム回転四元数 (GgQuaternion).
    ///
    gg::GgQuaternion getAimOrientation(int hand) const;

    ///
    /// トリガーの押し込み量を取得する.
    ///
    /// @param hand 対象の手 (0: 左手 Hand::Left, 1: 右手 Hand::Right).
    /// @return トリガー値 (0.0f ～ 1.0f).
    ///
    float getTrigger(int hand) const;

    ///
    /// グリップ（スクイーズ）の押し込み量を取得する.
    ///
    /// @param hand 対象の手 (0: 左手 Hand::Left, 1: 右手 Hand::Right).
    /// @return グリップ値 (0.0f ～ 1.0f).
    ///
    float getGrip(int hand) const;

    ///
    /// アナログスティック / トラックパッドの入力値を取得する.
    ///
    /// @param hand 対象の手 (0: 左手 Hand::Left, 1: 右手 Hand::Right).
    /// @return 2次元入力値 (x, y 各 -1.0f ～ 1.0f).
    ///
    std::array<float, 2> getThumbstick(int hand) const;

    ///
    /// アナログスティック / トラックパッドのクリック状態を取得する.
    ///
    /// @param hand 対象の手 (0: 左手 Hand::Left, 1: 右手 Hand::Right).
    /// @return 押されていれば true.
    ///
    bool getThumbstickClick(int hand) const;

    ///
    /// プライマリボタン (X / A ボタン) の押下状態を取得する.
    ///
    /// @param hand 対象の手 (0: 左手 Hand::Left, 1: 右手 Hand::Right).
    /// @return 押されていれば true.
    ///
    bool getPrimaryButton(int hand) const;

    ///
    /// セカンダリボタン (Y / B ボタン) の押下状態を取得する.
    ///
    /// @param hand 対象の手 (0: 左手 Hand::Left, 1: 右手 Hand::Right).
    /// @return 押されていれば true.
    ///
    bool getSecondaryButton(int hand) const;

    ///
    /// メニューボタンの押下状態を取得する.
    ///
    /// @param hand 対象の手 (デフォルトは 0: 左手 Hand::Left).
    /// @return 押されていれば true.
    ///
    /// @note
    /// Meta Touch と Valve Index の対話プロファイルには右手のメニューボタンが
    /// 存在しない (システムに予約されている) ため, これらでは右手を指定しても
    /// 常に false になる.
    ///
    bool getMenuButton(int hand = Hand::Left) const;

    ///
    /// コントローラーに振動（ハプティクスフィードバック）を出力する.
    ///
    /// @param hand 対象の手 (0: 左手 Hand::Left, 1: 右手 Hand::Right).
    /// @param durationSeconds 振動の持続時間（秒）.
    /// @param frequency 振動数（Hz、XR_FREQUENCY_UNSPECIFIED でランタイムデフォルト）.
    /// @param amplitude 振幅強度（0.0f ～ 1.0f）.
    ///
    void applyHapticVibration(int hand, float durationSeconds = 0.1f, float frequency = XR_FREQUENCY_UNSPECIFIED, float amplitude = 0.5f);
  };
#endif

  ///
  /// ユーザ名を得る.
  ///
  /// @return ユーザ名の文字列.
  ///
  static std::string getUsername();
};
