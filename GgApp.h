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
/// ゲームグラフィックス特論宿題アプリケーションクラスの定義
///
/// @file
/// @author Kohe Tokoi
/// @date July 17, 2025
///

// Dear ImGui を使うなら
#define GG_USE_IMGUI

// Oculus Rift を使うなら
//#define GG_USE_OCULUS_RIFT

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
#include <cassert>
#include <stdexcept>
#include <iostream>

// ImGui の組み込み
#if defined(GG_USE_IMGUI)
#  include "imgui.h"
#  include "imgui_impl_glfw.h"
#  include "imgui_impl_opengl3.h"
#endif

// Oculus Rift SDK ライブラリ (LibOVR) の組み込み
#if defined(GG_USE_OCULUS_RIFT)
#  if defined(_MSC_VER)
#    define GLFW_EXPOSE_NATIVE_WIN32
#    define GLFW_EXPOSE_NATIVE_WGL
#    include <GLFW/glfw3native.h>
#    define OVR_OS_WIN32
#    undef APIENTRY
#    pragma comment(lib, "LibOVR.lib")
#  endif
#  include <OVR_CAPI_GL.h>
#  include <Extras/OVR_Math.h>
#  if OVR_PRODUCT_VERSION > 0
#    include <dxgi.h> // GetDefaultAdapterLuid のため
#    pragma comment(lib, "dxgi.lib")
#  endif
#endif

///
/// ゲームグラフィックス特論宿題アプリケーションクラス
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
  /// コピーコンストラクタは使用しない
  ///
  GgApp(const GgApp& w) = delete;

  ///
  /// ムーブコンストラクタはデフォルトのものを使用する
  ///
  GgApp(GgApp&& w) = default;

  ///
  /// デストラクタ.
  ///
  virtual ~GgApp();

  ///
  /// 代入演算子は使用しない
  ///
  GgApp& operator=(const GgApp& w) = delete;

  ///
  /// ムーブ代入演算子はデフォルトのものを使用する
  ///
  GgApp& operator=(GgApp&& w) = default;

  ///
  /// アプリケーション本体.
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
    GLFWwindow* window;

    // ビューポートの横幅と高さ
    std::array<GLsizei, 2> size;

    // フレームバッファの横幅と高さ
    std::array<GLsizei, 2> fboSize;

#if defined(IMGUI_VERSION)
    // メニューバーの高さ
    GLsizei menubarHeight;
#endif

    // ビューポートの縦横比
    GLfloat aspect;

    // マウスの移動速度[X/Y/Z]
    std::array<GLfloat, 3> velocity;

    // マウスボタンの状態
    std::array<bool, GG_BUTTON_COUNT> status;

    // ユーザインタフェースのデータ構造
    struct HumanInterface
    {
      // 最後にタイプしたキー
      int lastKey;

      // 矢印キー
      std::array<std::array<int, 2>, 4> arrow;

      // マウスの現在位置
      std::array<GLfloat, 2> mouse;

      // マウスホイールの回転量
      std::array<GLfloat, 2> wheel;

      // 平行移動量[ボタン][直前/更新][X/Y/Z]
      std::array<std::array<GgVector, 2>, GG_BUTTON_COUNT> translation;

      // トラックボール
      std::array<GgTrackball, GG_BUTTON_COUNT> rotation;

      // コンストラクタ
      HumanInterface() :
        lastKey{ 0 },
        arrow{},
        mouse{},
        wheel{},
        translation{}
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
    int interfaceNo;

    //
    // ユーザー定義のコールバック関数へのポインタ
    //
    void* userPointer;
    void (*resizeFunc)(const Window* window, int width, int height);
    void (*keyboardFunc)(const Window* window, int key, int scancode, int action, int mods);
    void (*mouseFunc)(const Window* window, int button, int action, int mods);
    void (*wheelFunc)(const Window* window, double x, double y);

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
    /// コピーコンストラクタは使用しない
    ///
    Window(const Window& w) = delete;

    ///
    /// ムーブコンストラクタはデフォルトのものを使用する
    ///
    Window(Window&& w) = default;

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
    /// 代入演算子は使用しない
    ///
    Window& operator=(const Window& w) = delete;

    ///
    /// ムーブ代入演算子はデフォルトのものを使用する
    ///
    Window& operator=(Window&& w) = default;

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
    /// @param size FBO の幅と高さを格納した GLsizei 型の 2 要素の配列.
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
    void getAltlArrow(GLfloat* alt_arrow) const
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

      GgMatrix m
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

      GgMatrix m
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
    /// @return 回転を行う GgMarix 型の変換行列.
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

#if defined(GG_USE_OCULUS_RIFT)
  ///
  /// Oculus Rift 関連の処理.
  ///
  /// @note
  /// Oculus Rift を操作するラッパークラス（シングルトン）.
  ///
  class Oculus
  {
    // Oculus Rift のセッション
    ovrSession session;

    // Oculus Rift の状態
    ovrHmdDesc hmdDesc;

    // Oculus Rift へのレンダリングに使う FBO
    GLuint oculusFbo[ovrEye_Count];

    // Oculus Rift のスクリーンのサイズ
    GLfloat screen[ovrEye_Count][4];

    // ミラー表示用の FBO
    GLuint mirrorFbo;

    // Oculus Rift のミラー表示を行うウィンドウ
    const Window* window;

#  if OVR_PRODUCT_VERSION > 0

    // Oculus Rift に送る描画データ
    ovrLayerEyeFov layerData;

    // Oculus Rift にレンダリングするフレームの番号
    long long frameIndex;

    // Oculus Rift へのレンダリングに使う FBO のデプステクスチャ
    GLuint oculusDepth[ovrEye_Count];

    // ミラー表示用の FBO のサイズ
    int mirrorWidth, mirrorHeight;

    // ミラー表示用の FBO のカラーテクスチャ
    ovrMirrorTexture mirrorTexture;

    //
    // グラフィックスカードのデフォルトの LUID を得る
    //
    static ovrGraphicsLuid GetDefaultAdapterLuid();

    //
    // グラフィックスカードの LUID の比較
    //
    static int Compare(const ovrGraphicsLuid& lhs, const ovrGraphicsLuid& rhs);

#  else

    // Oculus Rift に送る描画データ
    ovrLayer_Union layerData;

    // Oculus Rift のレンダリング情報
    ovrEyeRenderDesc eyeRenderDesc[ovrEye_Count];

    // Oculus Rift の視点情報
    ovrPosef eyePose[ovrEye_Count];

    // ミラー表示用の FBO のカラーテクスチャ
    ovrGLTexture* mirrorTexture;

#  endif

    //
    // コンストラクタ
    //
    Oculus();

    //
    // デストラクタ
    //
    virtual ~Oculus() = default;

  public:

    // シングルトンなのでコピー・ムーブ禁止
    Oculus(const Oculus&) = delete;
    Oculus& operator=(const Oculus&) = delete;
    Oculus(Oculus&&) = delete;
    Oculus& operator=(Oculus&&) = delete;

    ///
    /// Oculus Rift のセッションを作成する.
    /// @param window ミラー表示を行うウィンドウ.
    /// @return Oculus Rift の static object の参照.
    ///
    static Oculus& initialize(const Window& window);

    ///
    /// Oculus Rift のセッションを破棄する.
    ///
    void terminate();

    ///
    /// Oculus Rift による描画開始.
    ///
    /// @return 描画可能なら true.
    ///
    bool begin();

    ///
    /// Oculus Rift の描画する目の指定.
    ///
    /// @param eye 表示する目.
    /// @param screen HMD の視野の視錐台.
    /// @param position HMD の位置.
    /// @param orientation HMD の方法の四元数.
    ///
    void select(int eye, GLfloat* screen, GLfloat* position, GLfloat* orientation);

    ///
    /// Time Warp 処理に使う投影変換行列の成分の設定 (DK1, DK2).
    ///
    /// @param projection 投影変換行列.
    ///
    void timewarp(const GgMatrix& projection);

    ///
    /// 図形の描画を完了する (CV1 以降).
    ///
    /// @param eye 表示する目.
    ///
    void commit(int eye);

    ///
    /// フレームを転送する.
    ///
    /// @param mirror true ならミラー表示を行う, デフォルトは true.
    /// @return フレームの転送に成功したら true.
    ///
    bool submit(bool mirror = true);
  };
#endif
};
