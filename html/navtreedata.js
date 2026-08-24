/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "ゲームグラフィックス特論", "index.html", [
    [ "ゲームグラフィックス特論の宿題用補助プログラム GLFW3 版.", "index.html", null ],
    [ "OpenXR ラッパー (<span class=\"tt\">GgApp::OpenXR</span>) マニュアル＆使用方法解説", "md_OPENXR.html", [
      [ "目次", "md_OPENXR.html#autotoc_md2", null ],
      [ "1. 概要と特徴", "md_OPENXR.html#autotoc_md4", [
        [ "主な特徴", "md_OPENXR.html#autotoc_md5", null ]
      ] ],
      [ "2. クイックスタート（最小の VR プログラム）", "md_OPENXR.html#autotoc_md7", null ],
      [ "3. API リファレンス", "md_OPENXR.html#autotoc_md9", [
        [ "型定義・定数", "md_OPENXR.html#autotoc_md10", null ],
        [ "初期化と終了処理", "md_OPENXR.html#autotoc_md12", [
          [ "<span class=\"tt\">static OpenXR&amp; initialize(const Window&amp; window, XrReferenceSpaceType spaceType = XR_REFERENCE_SPACE_TYPE_STAGE)</span>", "md_OPENXR.html#autotoc_md13", null ],
          [ "<span class=\"tt\">void terminate()</span>", "md_OPENXR.html#autotoc_md14", null ]
        ] ],
        [ "フレーム描画ループ", "md_OPENXR.html#autotoc_md16", [
          [ "<span class=\"tt\">bool begin()</span>", "md_OPENXR.html#autotoc_md17", null ],
          [ "<span class=\"tt\">void select(int eye)</span>", "md_OPENXR.html#autotoc_md18", null ],
          [ "<span class=\"tt\">void select(int eye, GLfloat* screen, GLfloat* position, GLfloat* orientation)</span>", "md_OPENXR.html#autotoc_md19", null ],
          [ "<span class=\"tt\">void commit(int eye)</span>", "md_OPENXR.html#autotoc_md20", null ],
          [ "<span class=\"tt\">bool submit(bool mirror = true)</span>", "md_OPENXR.html#autotoc_md21", null ]
        ] ],
        [ "カメラ・投影変換・視点情報", "md_OPENXR.html#autotoc_md23", [
          [ "<span class=\"tt\">GgMatrix getProjectionMatrix(int eye, GLfloat zNear = 0.1f, GLfloat zFar = 100.0f) const</span>", "md_OPENXR.html#autotoc_md24", null ],
          [ "<span class=\"tt\">GgMatrix getViewMatrix(int eye) const</span>", "md_OPENXR.html#autotoc_md25", null ],
          [ "<span class=\"tt\">GgMatrix getPoseMatrix(int eye) const</span>", "md_OPENXR.html#autotoc_md26", null ],
          [ "<span class=\"tt\">GgVector getPosition(int eye) const</span>", "md_OPENXR.html#autotoc_md27", null ],
          [ "<span class=\"tt\">GgQuaternion getOrientation(int eye) const</span>", "md_OPENXR.html#autotoc_md28", null ],
          [ "<span class=\"tt\">const XrFovf&amp; getFov(int eye) const</span>", "md_OPENXR.html#autotoc_md29", null ],
          [ "<span class=\"tt\">const XrPosef&amp; getPose(int eye) const</span>", "md_OPENXR.html#autotoc_md30", null ],
          [ "<span class=\"tt\">GLsizei getWidth(int eye = 0) const</span> / <span class=\"tt\">GLsizei getHeight(int eye = 0) const</span>", "md_OPENXR.html#autotoc_md31", null ],
          [ "<span class=\"tt\">GLfloat getAspect(int eye = 0) const</span>", "md_OPENXR.html#autotoc_md32", null ],
          [ "<span class=\"tt\">uint32_t getViewCount() const</span>", "md_OPENXR.html#autotoc_md33", null ]
        ] ],
        [ "コントローラートラッキング・姿勢", "md_OPENXR.html#autotoc_md35", [
          [ "<span class=\"tt\">bool isTracked(int hand) const</span>", "md_OPENXR.html#autotoc_md36", null ],
          [ "<span class=\"tt\">GgMatrix getGripMatrix(int hand) const</span>", "md_OPENXR.html#autotoc_md37", null ],
          [ "<span class=\"tt\">GgMatrix getAimMatrix(int hand) const</span>", "md_OPENXR.html#autotoc_md38", null ],
          [ "<span class=\"tt\">GgVector getGripPosition(int hand) const</span> / <span class=\"tt\">GgQuaternion getGripOrientation(int hand) const</span>", "md_OPENXR.html#autotoc_md39", null ],
          [ "<span class=\"tt\">GgVector getAimPosition(int hand) const</span> / <span class=\"tt\">GgQuaternion getAimOrientation(int hand) const</span>", "md_OPENXR.html#autotoc_md40", null ]
        ] ],
        [ "コントローラーボタン・スティック入力・振動", "md_OPENXR.html#autotoc_md42", [
          [ "<span class=\"tt\">float getTrigger(int hand) const</span>", "md_OPENXR.html#autotoc_md43", null ],
          [ "<span class=\"tt\">float getGrip(int hand) const</span>", "md_OPENXR.html#autotoc_md44", null ],
          [ "<span class=\"tt\">std::array&lt;float, 2&gt; getThumbstick(int hand) const</span>", "md_OPENXR.html#autotoc_md45", null ],
          [ "<span class=\"tt\">bool getThumbstickClick(int hand) const</span>", "md_OPENXR.html#autotoc_md46", null ],
          [ "<span class=\"tt\">bool getPrimaryButton(int hand) const</span>", "md_OPENXR.html#autotoc_md47", null ],
          [ "<span class=\"tt\">bool getSecondaryButton(int hand) const</span>", "md_OPENXR.html#autotoc_md48", null ],
          [ "<span class=\"tt\">bool getMenuButton(int hand = Hand::Left) const</span>", "md_OPENXR.html#autotoc_md49", null ],
          [ "<span class=\"tt\">void applyHapticVibration(int hand, float durationSeconds = 0.1f, float frequency = XR_FREQUENCY_UNSPECIFIED, float amplitude = 0.5f)</span>", "md_OPENXR.html#autotoc_md50", null ]
        ] ],
        [ "参照空間（空間座標系）", "md_OPENXR.html#autotoc_md52", [
          [ "<span class=\"tt\">XrReferenceSpaceType getReferenceSpaceType() const</span>", "md_OPENXR.html#autotoc_md53", null ]
        ] ]
      ] ],
      [ "4. 実践チュートリアル＆使用例", "md_OPENXR.html#autotoc_md55", [
        [ "4.1 基本的なステレオレンダリング", "md_OPENXR.html#autotoc_md56", null ],
        [ "4.2 コントローラーの姿勢取得とモデル描画", "md_OPENXR.html#autotoc_md58", null ],
        [ "4.3 スティック入力による移動とトリガー入力", "md_OPENXR.html#autotoc_md60", null ],
        [ "4.4 床面基準 (STAGE) と着席基準 (LOCAL) の切り替え", "md_OPENXR.html#autotoc_md62", null ],
        [ "4.5 PC ウィンドウへのミラー表示", "md_OPENXR.html#autotoc_md64", null ]
      ] ],
      [ "5. 旧 Oculus SDK (LibOVR) ラッパーからの移行ガイド", "md_OPENXR.html#autotoc_md66", [
        [ "コードの対比（Before / After）", "md_OPENXR.html#autotoc_md67", [
          [ "[Before] 旧 API", "md_OPENXR.html#autotoc_md68", null ],
          [ "[After] 最適化版 API", "md_OPENXR.html#autotoc_md69", null ]
        ] ]
      ] ],
      [ "6. 動作環境とセットアップ", "md_OPENXR.html#autotoc_md71", [
        [ "必要な環境", "md_OPENXR.html#autotoc_md72", null ],
        [ "プロジェクトへの組み込み", "md_OPENXR.html#autotoc_md73", null ]
      ] ]
    ] ],
    [ "ggsample01", "md_README.html", [
      [ "ゲームグラフィックス特論A　第１回　宿題", "md_README.html#autotoc_md75", null ],
      [ "宿題プログラムの作成に必要な環境", "md_README.html#autotoc_md76", null ],
      [ "宿題の提出", "md_README.html#autotoc_md77", null ],
      [ "補足", "md_README.html#autotoc_md78", null ],
      [ "宿題プログラム用補助プログラムについて", "md_README.html#autotoc_md79", [
        [ "補助プログラムのドキュメント", "md_README.html#autotoc_md80", null ],
        [ "補助プログラムの使い方", "md_README.html#autotoc_md81", null ],
        [ "OpenXR を使う場合", "md_README.html#autotoc_md82", null ],
        [ "Dear ImGui を使う場合", "md_README.html#autotoc_md83", [
          [ "imconfig.h の変更点", "md_README.html#autotoc_md84", null ]
        ] ]
      ] ]
    ] ],
    [ "名前空間", "namespaces.html", [
      [ "名前空間一覧", "namespaces.html", "namespaces_dup" ],
      [ "名前空間メンバ", "namespacemembers.html", [
        [ "全て", "namespacemembers.html", null ],
        [ "関数", "namespacemembers_func.html", null ],
        [ "変数", "namespacemembers_vars.html", null ],
        [ "列挙型", "namespacemembers_enum.html", null ],
        [ "列挙値", "namespacemembers_eval.html", null ]
      ] ]
    ] ],
    [ "クラス", "annotated.html", [
      [ "クラス一覧", "annotated.html", "annotated_dup" ],
      [ "クラス索引", "classes.html", null ],
      [ "クラス階層", "hierarchy.html", "hierarchy" ],
      [ "クラスメンバ", "functions.html", [
        [ "全て", "functions.html", "functions_dup" ],
        [ "関数", "functions_func.html", "functions_func" ],
        [ "変数", "functions_vars.html", null ],
        [ "関連関数", "functions_rela.html", null ]
      ] ]
    ] ],
    [ "ファイル", "files.html", [
      [ "ファイル一覧", "files.html", "files_dup" ],
      [ "ファイルメンバ", "globals.html", [
        [ "全て", "globals.html", null ],
        [ "関数", "globals_func.html", null ],
        [ "変数", "globals_vars.html", null ],
        [ "型定義", "globals_type.html", null ],
        [ "マクロ定義", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"Config_8cpp.html",
"classgg_1_1GgMatrix.html#a474be4675659c07c9b8eebf3b5c523a9",
"classgg_1_1GgQuaternion.html#a308e08f78d2dfeadc2efbcd7a470d6bd",
"classgg_1_1GgSimpleShader.html#a39cbc1d08f7a06f5811eda62f7d907c2",
"classgg_1_1GgUniformBuffer.html#ad097650d4355144a15deb49bf78e6198",
"namespacegg.html#a15b8b49e0ab9fe3bde8eee67bcdb7e81"
];

var SYNCONMSG = 'クリックで同期表示が無効になります';
var SYNCOFFMSG = 'クリックで同期表示が有効になります';
var LISTOFALLMEMBERS = '全メンバ一覧';