# OpenXR ラッパー (`GgApp::OpenXR`) マニュアル

本書は、ゲームグラフィックス特論用補助プログラム (`GgApp`) に組み込まれた **OpenXR ラッパークラス (`GgApp::OpenXR`)** の仕様、API リファレンス、および使用方法の解説です。

---

## 目次

1. [概要と特徴](#1-概要と特徴)
2. [有効化とビルド設定](#2-有効化とビルド設定)
3. [クイックスタート（最小の VR プログラム）](#3-クイックスタート最小の-vr-プログラム)
4. [フレームループの規約](#4-フレームループの規約)
5. [API リファレンス](#5-api-リファレンス)
   - [初期化と終了処理](#初期化と終了処理)
   - [フレーム描画ループ](#フレーム描画ループ)
   - [ミラー表示](#ミラー表示)
   - [セッションの状態](#セッションの状態)
   - [カメラ・投影変換・視点情報](#カメラ投影変換視点情報)
   - [コントローラのトラッキング・姿勢](#コントローラのトラッキング姿勢)
   - [コントローラのボタン・スティック入力・振動](#コントローラのボタンスティック入力振動)
   - [参照空間（空間座標系）](#参照空間空間座標系)
6. [対話プロファイルとバインディング](#6-対話プロファイルとバインディング)
7. [実践例](#7-実践例)
8. [旧 Oculus SDK (LibOVR) ラッパーからの移行ガイド](#8-旧-oculus-sdk-libovr-ラッパーからの移行ガイド)
9. [動作環境](#9-動作環境)
10. [実装上の注意とトラブルシューティング](#10-実装上の注意とトラブルシューティング)

---

## 1. 概要と特徴

`GgApp::OpenXR` は、業界標準の XR 規格である **[OpenXR](https://www.khronos.org/openxr/)** を用いて、OpenGL による VR アプリケーションを簡潔に記述できるようにした C++ ラッパークラスです。

### 主な特徴

- **`gg.h` の数学ライブラリとの統合**
  - `getProjectionMatrix(eye, zNear, zFar)` は、HMD 特有の非対称視野角（オフセンター視錐台）をそのまま反映した透視投影変換行列を `GgMatrix` で返します。`ggPerspective()` で代用してはいけません。
  - `getViewMatrix(eye)` は、頭部トラッキングの姿勢からビュー変換行列を `GgMatrix` で返します。
- **アクションシステムによるコントローラ対応**
  - Khronos Simple Controller、Meta (Oculus) Touch、HTC Vive、Valve Index、Microsoft Mixed Reality の 5 種類の対話プロファイルに対するバインディングを標準で登録します。
  - グリップ姿勢・エイム姿勢・トリガ・スクイーズ・スティック・ボタン・触覚フィードバックを取得できます。
- **フレームライフサイクルの自動管理**
  - `begin()` → `select(eye)` → `commit(eye)` → `submit()` の 4 段構成です。イベント処理、フレーム同期、スワップチェーンの取得と解放、FBO の結合、ビューポートの設定、sRGB の扱い、ミラー表示はすべて内部で処理します。
  - **描画が不要なフレームでは `begin()` が内部で `xrEndFrame()` まで済ませて `false` を返します**。そのため `xrBeginFrame()` と `xrEndFrame()` の対応が崩れません。
- **柔軟な空間座標系**
  - 床面基準の `XR_REFERENCE_SPACE_TYPE_STAGE` と、着席・頭部原点基準の `XR_REFERENCE_SPACE_TYPE_LOCAL` に対応します（STAGE が使えない環境では自動的に LOCAL にフォールバックします）。

---

## 2. 有効化とビルド設定

### 2.1 記号定数 `GG_USE_OPENXR` の定義

OpenXR の処理は `GG_USE_OPENXR` が定義されているときだけコンパイルされます。この記号定数は **`GgApp.cpp` を含むすべての翻訳単位で同じように定義されていなければなりません**。

> **重要**
> ソースファイルの先頭で `#define GG_USE_OPENXR` してから `#include "GgApp.h"` しても、`GgApp.cpp` 側ではそれが定義されないため、`GgApp::OpenXR` のメンバ関数の実体が生成されず **リンクエラー**になります。次のどちらかの方法で定義してください。

**方法 1（推奨）: `GgApp.h` の該当行のコメントを外す**

```cpp
// OpenXR を使うなら
#define GG_USE_OPENXR
```

**方法 2: プロジェクト全体のプリプロセッサ定義に加える**

- Visual Studio: プロジェクトのプロパティ → C/C++ → プリプロセッサ → プリプロセッサの定義に `GG_USE_OPENXR` を追加。
- Makefile: `CXXFLAGS` に `-DGG_USE_OPENXR` を追加。

### 2.2 Windows (Visual Studio)

1. [OpenXR SDK](https://github.com/KhronosGroup/OpenXR-SDK) の `include` と `lib`（あるいは NuGet パッケージ）を用意します。
2. プロジェクトのプロパティで
   - C/C++ → 追加のインクルードディレクトリ に `openxr/openxr.h` の親ディレクトリを追加
   - リンカー → 追加のライブラリディレクトリ に `openxr_loader.lib` のあるディレクトリを追加
3. ライブラリ本体は `GgApp.h` の中の `#pragma comment(lib, "openxr_loader.lib")` でリンクされるので、「追加の依存ファイル」への記述は不要です。
4. 実行時に `openxr_loader.dll` が必要です。実行ファイルと同じディレクトリに置いてください。

### 2.3 Linux

同梱の `Makefile` は既に `pkg-config` 経由で OpenXR を参照しています。

```make
CXXFLAGS = ... `pkg-config glfw3 openxr --cflags` ...
LDLIBS   = ... `pkg-config glfw3 openxr --libs` ...
```

`libopenxr-dev`（Debian / Ubuntu / Linux Mint）などをインストールしてください。ウィンドウシステムは **X11 が必要**です（Wayland セッションでは XWayland 上で動かしてください）。

> **macOS では利用できません。** OpenXR のランタイムが存在しないため、`GG_USE_OPENXR` を定義するとコンパイルエラーになります。

---

## 3. クイックスタート（最小の VR プログラム）

```cpp
#include "GgApp.h"    // GgApp.h の中で GG_USE_OPENXR を定義しておく

using namespace gg;

int GgApp::main(int argc, const char* const* argv)
{
  // ミラー表示用のウィンドウを作成する（OpenGL のコンテキストが必要）
  Window window{ "OpenXR Sample", 1280, 720 };

  // OpenXR を初期化する
  auto& openxr{ OpenXR::initialize(window) };

  // 隠面消去処理を有効にする
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glClearColor(0.1f, 0.2f, 0.3f, 1.0f);

  // 物体の配置
  const GgMatrix ma{ ggTranslate(0.0f, 1.2f, -2.0f) };

  // ウィンドウが開いている間繰り返す
  while (window)
  {
    // フレームの描画を開始する
    if (openxr.begin())
    {
      // 視点の数だけ繰り返す（通常は左目と右目の 2 回）
      for (int eye = 0; eye < static_cast<int>(openxr.getViewCount()); ++eye)
      {
        // 描画対象の目を指定する（FBO の結合とビューポートの設定が行われる）
        openxr.select(eye);

        // バッファを消去する
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 投影変換行列とビュー変換行列を取得する
        const GgMatrix mp{ openxr.getProjectionMatrix(eye, 0.05f, 100.0f) };
        const GgMatrix mv{ openxr.getViewMatrix(eye) };

        // シーンを描画する
        // shader.use(mp, mv * ma, light);
        // model.draw();

        // この目の描画を完了する
        openxr.commit(eye);
      }

      // HMD にフレームを転送し、ウィンドウにミラー表示する
      openxr.submit();
    }

    // ウィンドウのカラーバッファを入れ替えてイベントを取り出す
    window.swapBuffers();
  }

  // OpenGL のコンテキストが有効なうちに終了処理を行う
  openxr.terminate();

  return 0;
}
```

> `window.swapBuffers()` は **`if (openxr.begin())` の外側**に置きます。ミラー表示や Dear ImGui のメニューはウィンドウのバッファを入れ替えないと画面に出ません。

---

## 4. フレームループの規約

OpenXR は `xrBeginFrame()` と `xrEndFrame()` が 1 対 1 で対応していなければなりません。このラッパーでは次のように整理しています。

| `begin()` の戻り値 | 内部の状態 | アプリケーションがすべきこと |
| :--- | :--- | :--- |
| `true` | `xrBeginFrame()` 済み | `select()` / `commit()` で描画し、**必ず `submit()` を呼ぶ** |
| `false` | セッション停止中、または `xrEndFrame()` 済み | 何もしない（`submit()` は不要。呼んでも安全に無視される） |

- HMD を外している、メニューを開いているなど、ランタイムが描画を必要としないフレームでは `begin()` が `false` を返します。そのときも `while (window)` のループ自体は回し続けてください。
- `select()` / `commit()` / `submit()` は、対応するフレームが開始されていなければ何もしません。順序を誤っても未定義動作にはなりません。
- `submit()` を呼ばずに次の `begin()` を呼ぶと、そのフレームは描画されずに破棄されます。

### 各段階で行われること

```
begin()      xrPollEvent → セッション状態の遷移 → xrWaitFrame → xrBeginFrame
             → xrSyncActions（コントローラの状態を更新）→ xrLocateViews
select(eye)  xrAcquireSwapchainImage → xrWaitSwapchainImage
             → FBO の結合 → glViewport → GL_FRAMEBUFFER_SRGB を有効化
commit(eye)  GL_FRAMEBUFFER_SRGB を無効化 → 描画先をウィンドウに戻す
submit()     ミラー表示 → xrReleaseSwapchainImage（全視点）→ xrEndFrame
```

スワップチェーンイメージの解放を `commit()` ではなく `submit()` まで遅らせているのは、**ミラー表示のためにイメージを読み出す必要がある**ためです（解放済みのイメージを読み出すことは OpenXR の仕様で禁止されています）。

---

## 5. API リファレンス

クラス定義: `GgApp::OpenXR`（シングルトン）

### 型定義・定数

```cpp
enum Hand
{
  Left  = 0,  // 左手
  Right = 1,  // 右手
  Count = 2   // 手の総数
};
```

---

### 初期化と終了処理

#### `static OpenXR& initialize(const Window& window, XrReferenceSpaceType spaceType = XR_REFERENCE_SPACE_TYPE_STAGE, const char* appName = "GgApp")`

OpenXR のインスタンス、システム、OpenGL バインディング、セッション、参照空間、アクションセット、スワップチェーンを初期化します。

- **引数**
  - `window`: ミラー表示を行う `GgApp::Window` オブジェクト（作成済みであること）。
  - `spaceType`: 参照空間タイプ。`XR_REFERENCE_SPACE_TYPE_STAGE`（床面基準）が使えない環境では自動的に `XR_REFERENCE_SPACE_TYPE_LOCAL` にフォールバックします。
  - `appName`: OpenXR のランタイムに通知するアプリケーション名（128 文字を超える分は切り捨てられます）。
- **戻り値**: 初期化された `OpenXR` の静的オブジェクトの参照。
- **例外**: ランタイムが見つからない、HMD が接続されていない、`XR_KHR_opengl_enable` に対応していない、OpenGL のバージョンが不足しているなどの場合、**確保した資源を解放したうえで** `std::runtime_error` を投げます。
- 二度目以降の呼び出しは、初期化済みのオブジェクトをそのまま返します（引数は無視されます）。

#### `void terminate()`

作成したすべての資源（セッション、スワップチェーン、参照空間、アクション、FBO、デプスバッファ）を破棄し、`glfwSwapInterval(1)` を復帰します。何度呼んでも安全です。

> **`terminate()` はウィンドウ（OpenGL のコンテキスト）が有効なうちに呼んでください。** 呼び忘れた場合、OpenXR のハンドルはデストラクタで破棄されますが、OpenGL の資源はプロセス終了に任されます。

---

### フレーム描画ループ

#### `bool begin()`

OpenXR のイベントを処理し、フレームを同期（`xrWaitFrame` / `xrBeginFrame`）し、コントローラの状態と視点の姿勢を更新します。

- **戻り値**: このフレームで描画を行うべきなら `true`。
- `false` のときは内部で `xrEndFrame()` まで完了しているので、`submit()` を呼ぶ必要はありません。

#### `void select(int eye)`

指定した目のスワップチェーンイメージを取得して FBO に結合し、`glViewport()` を推奨解像度に設定します。sRGB のスワップチェーンが選ばれている場合は `GL_FRAMEBUFFER_SRGB` を有効にします（シェーダはリニア色空間で出力すれば、ガンマ補正はランタイムに任されます）。

#### `void select(int eye, GLfloat* screen, GLfloat* position, GLfloat* orientation)`

旧 LibOVR 仕様からの移行用オーバーロードです。`select(eye)` の処理に加えて、視野角の正接（`tanLeft`, `tanRight`, `tanDown`, `tanUp`）、視点位置、回転四元数を生の配列に代入します。新規のコードでは `getProjectionMatrix()` / `getViewMatrix()` を使ってください。

#### `void commit(int eye)`

指定した目の描画を完了します。`GL_FRAMEBUFFER_SRGB` を無効にし、描画先をウィンドウに戻します。**スワップチェーンイメージの解放は `submit()` で行います。**

#### `bool submit(bool mirror = true)`

ミラー表示を行ってからすべてのスワップチェーンイメージを解放し、合成レイヤ（`XrCompositionLayerProjection`）を組み立てて `xrEndFrame()` を呼びます。

- **引数**: `mirror` が `false` ならミラー表示を行いません（`setMirror(-1)` と等価）。
- **戻り値**: 転送を行ったら `true`、開始中のフレームがなければ `false`。

---

### ミラー表示

#### `void setMirror(int eye)` / `int getMirror() const`

ミラー表示を行うビューの番号を設定・取得します。`-1` を設定するとミラー表示を行いません。既定値は `0`（左目）です。

ミラー表示は、HMD の映像の縦横比を保ったままウィンドウの中央に転送します（余白は黒で塗りつぶされます）。転送後にウィンドウのビューポートが復帰されるので、Dear ImGui のメニューはミラー画像の上に重ねて描かれます。

---

### セッションの状態

#### `bool isRunning() const`
セッションが実行中（`xrBeginSession()` 済み）なら `true` を返します。

#### `bool isFocused() const`
アプリケーションが入力を受け付けている（`XR_SESSION_STATE_FOCUSED`）なら `true` を返します。**この状態でないとコントローラの入力は更新されません。**

#### `bool isPoseValid() const`
直前の `begin()` で視点の位置と向きの両方が取得できていれば `true` を返します。

#### `const std::string& getSystemName() const`
OpenXR のシステム（HMD）の名前を返します。

---

### カメラ・投影変換・視点情報

#### `GgMatrix getProjectionMatrix(int eye, GLfloat zNear = 0.1f, GLfloat zFar = 100.0f) const`
視野角情報（`XrFovf`）から、オフセンター視錐台に対応した透視投影変換行列を返します。単位はメートルです。

#### `GgMatrix getViewMatrix(int eye) const`
参照空間から視点座標系への変換を行うビュー変換行列を返します。
計算式: $V = R^{\top} \cdot T(-p) = \mathrm{conjugate}(q) \cdot \mathrm{translate}(-p)$

#### `GgMatrix getPoseMatrix(int eye) const`
参照空間における視点の姿勢（カメラのモデル変換行列）を返します。
計算式: $M = T(p) \cdot R(q)$

#### `GgVector getPosition(int eye) const` / `GgQuaternion getOrientation(int eye) const`
視点の位置 $(x, y, z, 1)$ と回転四元数 $(x, y, z, w)$ を返します。

#### `const XrFovf& getFov(int eye) const` / `const XrPosef& getPose(int eye) const`
OpenXR の生の視野角構造体・姿勢構造体の参照を返します。

#### `GLsizei getWidth(int eye = 0) const` / `GLsizei getHeight(int eye = 0) const` / `GLfloat getAspect(int eye = 0) const`
ランタイムが推奨するレンダリング解像度の幅・高さ・縦横比を返します。

#### `uint32_t getViewCount() const`
ビューの総数（通常のステレオ表示では `2`）を返します。

---

### コントローラのトラッキング・姿勢

#### `bool isTracked(int hand) const`
指定した手のコントローラが接続されていて姿勢のアクションが有効なら `true` を返します。

#### `GgMatrix getGripMatrix(int hand) const`
グリップ姿勢のモデル変換行列を返します。コントローラや手の 3D モデルを配置するのに使います。

#### `GgMatrix getAimMatrix(int hand) const`
エイム（ポインティング）姿勢のモデル変換行列を返します。**-Z 方向がコントローラの指す向き**なので、レーザーポインタや照準に使います。

#### `GgVector getGripPosition(int hand) const` / `GgQuaternion getGripOrientation(int hand) const`
#### `GgVector getAimPosition(int hand) const` / `GgQuaternion getAimOrientation(int hand) const`
それぞれの位置と回転四元数を取得します。

> 姿勢は、位置と向きの**両方**が有効なときにだけ更新されます。トラッキングが外れている間は直前の値が保持されます。

---

### コントローラのボタン・スティック入力・振動

| 関数 | 内容 |
| :--- | :--- |
| `float getTrigger(int hand) const` | 人差し指トリガーの押し込み量（`0.0f`〜`1.0f`） |
| `float getGrip(int hand) const` | グリップ（スクイーズ）の押し込み量（`0.0f`〜`1.0f`） |
| `std::array<float, 2> getThumbstick(int hand) const` | スティック／トラックパッドの入力（各 `-1.0f`〜`1.0f`） |
| `bool getThumbstickClick(int hand) const` | スティック／トラックパッドの押し込み |
| `bool getPrimaryButton(int hand) const` | プライマリボタン（左手 `X` / 右手 `A`） |
| `bool getSecondaryButton(int hand) const` | セカンダリボタン（左手 `Y` / 右手 `B`） |
| `bool getMenuButton(int hand = Hand::Left) const` | メニューボタン |

> **`getMenuButton(Hand::Right)` は Meta Touch と Valve Index では常に `false` を返します。** これらの対話プロファイルには右手のメニューボタンが定義されていない（システムに予約されている）ためです。

#### `void applyHapticVibration(int hand, float durationSeconds = 0.1f, float frequency = XR_FREQUENCY_UNSPECIFIED, float amplitude = 0.5f)`
触覚フィードバック（振動）を出力します。`amplitude` は `0.0f`〜`1.0f` に丸められます。`durationSeconds` に `0` 以下を指定すると、ランタイムが決める最小の振動時間（`XR_MIN_HAPTIC_DURATION`）になります。

---

### 参照空間（空間座標系）

#### `XrReferenceSpaceType getReferenceSpaceType() const`
実際に使われている参照空間のタイプを返します。`initialize()` に `XR_REFERENCE_SPACE_TYPE_STAGE` を指定しても、ランタイムが対応していなければ `XR_REFERENCE_SPACE_TYPE_LOCAL` が返ります。

| タイプ | 原点 | 用途 |
| :--- | :--- | :--- |
| `XR_REFERENCE_SPACE_TYPE_STAGE` | プレイエリアの床面の中心（$y = 0$ が床） | 立位・ルームスケール |
| `XR_REFERENCE_SPACE_TYPE_LOCAL` | アプリケーション開始時の頭の位置 | 着座（フライトシム、ドライブなど） |

座標系は OpenGL と同じ**右手系**で、$+Y$ が上、$-Z$ が前方です。単位はメートルです。

---

## 6. 対話プロファイルとバインディング

このラッパーが登録している推奨バインディングの一覧です。ランタイムが対応していないプロファイルは自動的に読み飛ばされます。

| アクション | Simple Controller | Meta Touch | HTC Vive | Valve Index | WMR |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `getGripMatrix` | `grip/pose` | `grip/pose` | `grip/pose` | `grip/pose` | `grip/pose` |
| `getAimMatrix` | `aim/pose` | `aim/pose` | `aim/pose` | `aim/pose` | `aim/pose` |
| `getTrigger` | `select/click` | `trigger/value` | `trigger/value` | `trigger/value` | `trigger/value` |
| `getGrip` | — | `squeeze/value` | `squeeze/click` | `squeeze/value` | `squeeze/click` |
| `getThumbstick` | — | `thumbstick` | `trackpad` | `thumbstick` | `thumbstick` |
| `getThumbstickClick` | — | `thumbstick/click` | `trackpad/click` | `thumbstick/click` | `thumbstick/click` |
| `getPrimaryButton` | — | 左 `x` / 右 `a` | — | `a/click` | — |
| `getSecondaryButton` | — | 左 `y` / 右 `b` | — | `b/click` | — |
| `getMenuButton` | `menu/click` | **左のみ** `menu/click` | `menu/click` | — | `menu/click` |
| `applyHapticVibration` | `output/haptic` | `output/haptic` | `output/haptic` | `output/haptic` | `output/haptic` |

「—」はそのプロファイルに対応する入力がないことを表します。該当する入力がない場合、値は `0.0f` または `false` になります。

> `getTrigger()` を Simple Controller の `select/click`（論理値）に束縛しているように、論理値の入力を連続値のアクションに束縛することは OpenXR で許されています（`0.0f` または `1.0f` になります）。逆も同様です。

---

## 7. 実践例

### 7.1 コントローラの姿勢に合わせてモデルを描く

```cpp
for (int hand = 0; hand < OpenXR::Hand::Count; ++hand)
{
  if (!openxr.isTracked(hand)) continue;

  // グリップ姿勢のモデル変換行列
  const GgMatrix mg{ openxr.getGripMatrix(hand) };

  handShader.use(mp, mv * mg, light);
  handModel.draw();
}
```

### 7.2 スティックによる移動とトリガーによる振動

```cpp
// プレイヤーの位置（フレームをまたいで保持する）
static GgVector playerPos{ 0.0f, 0.0f, 0.0f, 1.0f };

// 左スティックの傾きに応じて移動する
const auto stick{ openxr.getThumbstick(OpenXR::Hand::Left) };
constexpr GLfloat moveSpeed{ 0.05f };
playerPos[0] += stick[0] * moveSpeed;
playerPos[2] -= stick[1] * moveSpeed;

// 右トリガーを深く引いたら右手を振動させる
if (openxr.getTrigger(OpenXR::Hand::Right) > 0.8f)
{
  openxr.applyHapticVibration(OpenXR::Hand::Right, 0.05f, XR_FREQUENCY_UNSPECIFIED, 0.8f);
}

// プレイヤーの移動は「世界を逆に動かす」ことで表現する
const GgMatrix mw{ ggTranslate(-playerPos[0], -playerPos[1], -playerPos[2]) };
const GgMatrix mv{ openxr.getViewMatrix(eye) * mw };
```

> 平行移動の逆変換は `ggTranslate()` の符号を反転するだけで済みます。一般の変換行列を反転するときは `GgMatrix::invert()` を使ってください（`inverse()` という名前の関数はありません）。

### 7.3 参照空間の切り替え

```cpp
// ルームスケール（床面が y = 0 となる座標系）
auto& openxr{ OpenXR::initialize(window, XR_REFERENCE_SPACE_TYPE_STAGE) };

// 着席プレイ（起動時の頭の位置が原点となる座標系）
auto& openxr{ OpenXR::initialize(window, XR_REFERENCE_SPACE_TYPE_LOCAL) };
```

### 7.4 ミラー表示の切り替え

```cpp
openxr.setMirror(0);     // 左目をミラー表示する（既定）
openxr.setMirror(1);     // 右目をミラー表示する
openxr.setMirror(-1);    // ミラー表示を行わない（描画負荷を減らす）

openxr.submit(false);    // このフレーム以降のミラー表示をやめる（setMirror(-1) と等価）
```

### 7.5 Dear ImGui との併用

Dear ImGui のメニューはウィンドウ側（ミラー表示の上）に描かれます。`GgApp::Window::swapBuffers()` の中でレンダリングされるので、次の順序を守ってください。

```cpp
while (window)
{
  // ImGui のウィンドウを組み立てる（window の operator bool() で NewFrame 済み）
  ImGui::Begin("Control panel");
  ImGui::Text("HMD: %s", openxr.getSystemName().c_str());
  ImGui::Text("Frame rate: %6.2f fps", ImGui::GetIO().Framerate);
  if (ImGui::Button("Quit")) window.setClose();
  ImGui::End();

  if (openxr.begin())
  {
    for (int eye = 0; eye < static_cast<int>(openxr.getViewCount()); ++eye)
    {
      openxr.select(eye);
      // ... HMD への描画 ...
      openxr.commit(eye);
    }
    openxr.submit();
  }

  // ミラー画像の上に ImGui を重ねて描き、バッファを入れ替える
  window.swapBuffers();
}
```

---

## 8. 旧 Oculus SDK (LibOVR) ラッパーからの移行ガイド

| 処理内容 | 旧 API（LibOVR ラッパー） | 現在の OpenXR API |
| :--- | :--- | :--- |
| 初期化 | `Oculus::initialize(window)` | `OpenXR::initialize(window, spaceType, appName)` |
| 目の指定と FBO の結合 | `select(eye, screen, pos, ori)` | `select(eye)` |
| 投影変換行列 | 手動で `ggFrustum(screen[0]*n, ...)` | `getProjectionMatrix(eye, zNear, zFar)` |
| ビュー変換行列 | 手動で四元数から合成 | `getViewMatrix(eye)` |
| カメラの姿勢行列 | 手動で合成 | `getPoseMatrix(eye)` |
| TimeWarp | `timewarp(mp)`（空のダミー） | **廃止**（合成器が行う） |
| 目の描画完了 | `commit(eye)` | `commit(eye)`（解放は `submit()` に移動） |
| フレーム転送 | `submit(mirror)` | `submit(mirror)` |
| コントローラの姿勢 | なし | `getGripMatrix()` / `getAimMatrix()` |
| ボタン・スティック | なし | `getTrigger()` / `getGrip()` / `getThumbstick()` ほか |
| 触覚フィードバック | なし | `applyHapticVibration()` |

### コードの対比

**移行前**

```cpp
GLfloat screen[4], position[3], orientation[4];
openxr.select(eye, screen, position, orientation);

const GgMatrix mp{ ggFrustum(
  screen[0] * zNear, screen[1] * zNear,
  screen[2] * zNear, screen[3] * zNear,
  zNear, zFar) };

const GgMatrix mv{ GgQuaternion(orientation).getConjugateMatrix()
  * ggTranslate(-position[0], -position[1], -position[2]) };

shader.use(mp, mv * ma, light);
model.draw();
openxr.commit(eye);
```

**移行後**

```cpp
openxr.select(eye);

const GgMatrix mp{ openxr.getProjectionMatrix(eye, zNear, zFar) };
const GgMatrix mv{ openxr.getViewMatrix(eye) };

shader.use(mp, mv * ma, light);
model.draw();
openxr.commit(eye);
```

`select(eye, screen, position, orientation)` のオーバーロードは移行のために残していますが、新規のコードでは使わないでください。

---

## 9. 動作環境

- **OS**
  - Windows 10 / 11（64bit）
  - Linux（Ubuntu / Linux Mint 等、X11 環境）
  - macOS は非対応
- **VR ハードウェア**
  - Meta Quest 2 / 3 / 3S / Pro（Quest Link / Air Link / Virtual Desktop / Steam Link 経由）
  - HTC Vive / Vive Pro
  - Valve Index
  - Windows Mixed Reality 各種 HMD
  - その他 OpenXR に対応した HMD
- **OpenXR ランタイム**
  - Meta Quest Link アプリ（設定でアクティブなランタイムに指定）
  - SteamVR（設定 → OpenXR →「SteamVR をアクティブな OpenXR ランタイムとして設定」）
  - Monado（Linux 用のオープンソース実装）
- **OpenGL**
  - ランタイムが `xrGetOpenGLGraphicsRequirementsKHR()` で要求するバージョンを満たす必要があります。要件を満たさない場合は `initialize()` が「The OpenXR runtime requires OpenGL x.y or later」という例外を投げます。
  - ひな形の `main.cpp` は `GgApp app(4, 1);` として OpenGL 4.1 のコンテキストを要求しています。ランタイムがそれより新しいバージョンを要求する場合は、この数値を上げてください。

---

## 10. 実装上の注意とトラブルシューティング

### スワップチェーンのカラーフォーマットと sRGB

`xrEnumerateSwapchainFormats()` で利用可能なフォーマットを問い合わせ、`GL_SRGB8_ALPHA8` → `GL_SRGB8` → `GL_RGBA8` → `GL_RGB10_A2` の順に優先して選びます。sRGB のフォーマットが選ばれた場合、`select()` と `commit()` の間だけ `GL_FRAMEBUFFER_SRGB` が有効になります。したがって **フラグメントシェーダはリニア色空間の値を出力してください**。ウィンドウへのミラー表示と Dear ImGui の描画には影響しません。

### 垂直同期

`initialize()` は `glfwSwapInterval(0)` を設定します。フレームの表示速度は `xrWaitFrame()` が制御するため、ウィンドウ側で垂直同期を待つと HMD の表示が乱れます。`terminate()` で `glfwSwapInterval(1)` に戻ります。

### 隠面消去

各視点の FBO には `GL_DEPTH24_STENCIL8` のレンダーバッファが取り付けられています。デプスバッファは視点ごとに 1 つで、スワップチェーンのすべてのイメージで共有されます（1 フレーム内で視点ごとに 1 回しか描画しないので問題ありません）。

### よくある問題

| 症状 | 原因と対処 |
| :--- | :--- |
| `GgApp::OpenXR` のメンバが未解決とリンクエラーになる | `GG_USE_OPENXR` が `GgApp.cpp` で定義されていない。[2.1 節](#21-記号定数-gg_use_openxr-の定義)を参照。 |
| `Can't create the OpenXR instance` | OpenXR ランタイムがインストールされていない、またはアクティブになっていない。 |
| `Can't get the OpenXR system` | HMD が接続・認識されていない。 |
| `The OpenXR runtime does not support XR_KHR_opengl_enable` | ランタイムが OpenGL に対応していない（一部の Android 系ランタイムなど）。 |
| ウィンドウが真っ黒のまま | `window.swapBuffers()` を呼んでいない、または `setMirror(-1)` になっている。 |
| HMD には出るが動きがカクつく | `submit()` の呼び忘れ、または垂直同期が有効なまま。 |
| コントローラの入力が取れない | HMD を装着してアプリケーションがフォーカスを持つ（`isFocused()` が `true` になる）まで、入力は更新されない。 |
| ビルドは通るが実行時に `openxr_loader.dll` が見つからない | DLL を実行ファイルと同じディレクトリに置く。 |

### 未対応の機能

次の機能はこのラッパーには含まれていません。必要な場合は `xrGetInstanceProcAddr()` などで直接 OpenXR を呼び出してください。

- `XR_KHR_composition_layer_depth`（デプスバッファを合成器に渡す再投影）
- クアッドレイヤー・シリンダーレイヤーなどの投影レイヤ以外の合成レイヤ
- ハンドトラッキング（`XR_EXT_hand_tracking`）、アイトラッキング、パススルー
- マルチサンプリング（`sampleCount` は 1 固定）
- 参照空間の再センタリング

---

Copyright (c) 2011-2025 Kohe Tokoi. All Rights Reserved.
