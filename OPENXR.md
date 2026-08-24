# OpenXR ラッパー (`GgApp::OpenXR`) マニュアル＆使用方法解説

本書は、ゲームグラフィックス特論用補助プログラム (`GgApp`) に組み込まれた **OpenXR ラッパークラス (`GgApp::OpenXR`)** の仕様、API リファレンス、および実践的な使用方法についての解説ドキュメントです。

---

## 目次

1. [概要と特徴](#1-概要と特徴)
2. [クイックスタート（最小の VR プログラム）](#2-クイックスタート最小の-vr-プログラム)
3. [API リファレンス](#3-api-リファレンス)
   - [初期化と終了処理](#初期化と終了処理)
   - [フレーム描画ループ](#フレーム描画ループ)
   - [カメラ・投影変換・視点情報](#カメラ投影変換視点情報)
   - [コントローラートラッキング・姿勢](#コントローラートラッキング姿勢)
   - [コントローラーボタン・スティック入力・振動](#コントローラーボタンスティック入力振動)
   - [参照空間（空間座標系）](#参照空間空間座標系)
4. [実践チュートリアル＆使用例](#4-実践チュートリアル使用例)
   - [4.1 基本的なステレオレンダリング](#41-基本的なステレオレンダリング)
   - [4.2 コントローラーの姿勢取得とモデル描画](#42-コントローラーの姿勢取得とモデル描画)
   - [4.3 スティック入力による移動とトリガー入力](#43-スティック入力による移動とトリガー入力)
   - [4.4 床面基準 (STAGE) と着席基準 (LOCAL) の切り替え](#44-床面基準-stage-と着席基準-local-の切り替え)
   - [4.5 PC ウィンドウへのミラー表示](#45-pc-ウィンドウへのミラー表示)
5. [旧 Oculus SDK (LibOVR) ラッパーからの移行ガイド](#5-旧-oculus-sdk-libovr-ラッパーからの移行ガイド)
6. [動作環境とセットアップ](#6-動作環境とセットアップ)

---

## 1. 概要と特徴

`GgApp::OpenXR` は、業界標準の XR 規格である **[OpenXR](https://www.khronos.org/openxr/)** を用いて、OpenGL による VR アプリケーションを極めてシンプルかつ直感的に開発できるように設計された C++ ラッパークラスです。

### 主な特徴
- **`gg.h` 数学ライブラリとのシームレスな統合**:
  - `getProjectionMatrix(eye, zNear, zFar)` を呼ぶだけで、HMD 特有の非対称視野角（Off-Center Frustum）に対応した正確な透視投影変換行列 (`GgMatrix`) を直接取得できます。
  - `getViewMatrix(eye)` により、HMD の頭部トラッキング姿勢からワールド座標系に対するビュー変換行列 (`GgMatrix`) を 1 行で取得できます。
- **標準化されたコントローラー（入力・トラッキング）の統合サポート**:
  - Meta Quest Touch、HTC Vive、Valve Index、Windows Mixed Reality などの主要な VR コントローラーを標準でサポート。
  - 手のグリップ姿勢 (`getGripMatrix`)、エイム（ポインティング）姿勢 (`getAimMatrix`)、各種ボタン（Trigger, Grip, Thumbstick, Primary/Secondary, Menu）やハプティクス振動を即座に利用可能。
- **シンプルなフレームライフサイクル**:
  - `begin()` -> `select(eye)` -> `commit(eye)` -> `submit(mirror)` の分かりやすいループ構成。FBO バインド、スワップチェーン管理、ビューポート設定、sRGB カラースペース処理を内部で自動処理。
- **柔軟な空間座標系**:
  - ルームスケール・床面基準の `XR_REFERENCE_SPACE_TYPE_STAGE` と、着席・頭部原点基準の `XR_REFERENCE_SPACE_TYPE_LOCAL` に両対応。

---

## 2. クイックスタート（最小の VR プログラム）

OpenXR を有効にするには、`#include "GgApp.h"` の前に `#define GG_USE_OPENXR` を定義します。

```cpp
#define GG_USE_OPENXR
#include "GgApp.h"

int GgApp::main(int argc, const char* const* argv)
{
  // ミラー表示用のウィンドウを作成
  Window window("OpenXR Sample", 1280, 720);

  // OpenXR を初期化
  auto& openxr = OpenXR::initialize(window);

  // 物体の回転・配置行列
  const GgMatrix modelMatrix = ggTranslate(0.0f, 1.2f, -2.0f);

  // ウィンドウが開いている間ループ
  while (window)
  {
    // OpenXR のフレーム開始
    if (openxr.begin())
    {
      // 左右の目を順にレンダリング (通常 2 回)
      for (int eye = 0; eye < openxr.getViewCount(); ++eye)
      {
        // 描画対象の目を指定 (FBO のバインドとビューポート設定が自動で行われる)
        openxr.select(eye);

        // バッファの消去
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 投影変換行列とビュー変換行列を直接取得
        const GgMatrix mp = openxr.getProjectionMatrix(eye, 0.1f, 100.0f);
        const GgMatrix mv = openxr.getViewMatrix(eye);

        //
        // ここでシェーダに mp, mv * modelMatrix を渡してオブジェクトを描画
        //

        // 描画の完了 (スワップチェーンイメージの解放)
        openxr.commit(eye);
      }

      // HMD へフレームを転送し、ウィンドウへミラー表示
      openxr.submit(true);
    }
  }

  // 終了処理
  openxr.terminate();
  return 0;
}
```

---

## 3. API リファレンス

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

#### `static OpenXR& initialize(const Window& window, XrReferenceSpaceType spaceType = XR_REFERENCE_SPACE_TYPE_STAGE)`
OpenXR インスタンス、システム、OpenGL バインディング、セッション、参照空間、スワップチェーン、アクションセットを初期化します。
- **引数**:
  - `window`: ミラー表示を行う `GgApp::Window` オブジェクト。
  - `spaceType`: 参照空間タイプ（`XR_REFERENCE_SPACE_TYPE_STAGE`: 床面基準、`XR_REFERENCE_SPACE_TYPE_LOCAL`: 頭部基準）。STAGE がサポートされない環境では自動的に LOCAL にフォールバックします。
- **戻り値**: 初期化された `OpenXR` クラスの静的参照。

#### `void terminate()`
作成したすべての OpenXR リソース（セッション、スワップチェーン、スペース、アクション、FBO、デプステクスチャ）を安全に破棄します。

---

### フレーム描画ループ

#### `bool begin()`
OpenXR イベントをポーリングし、セッション状態の遷移、フレーム同期（`xrWaitFrame` / `xrBeginFrame`）、視点位置・コントローラー入力状態の同期（`xrLocateViews` / `pollActions`）を行います。
- **戻り値**: レンダリングを実行すべきフレームであれば `true`、描画が不要（またはセッション非アクティブ）であれば `false`。

#### `void select(int eye)`
指定した目（`0`: 左目、`1`: 右目）のレンダリング用 FBO にスワップチェーンイメージとデプステクスチャをアタッチ・バインドし、ビューポート（`glViewport`）を推奨解像度に自動設定します。
- **引数**:
  - `eye`: 表示する目のインデックス (`0` または `1`)。

#### `void select(int eye, GLfloat* screen, GLfloat* position, GLfloat* orientation)`
旧 LibOVR 仕様からの移行用オーバーロードです。FBO バインドに加えて、視野のタンジェント境界値、視点位置、回転四元数を生配列に代入します。

#### `void commit(int eye)`
レンダリング完了後、FBO バインドを解除し、現在のスワップチェーンイメージを OpenXR ランタイムへ解放（Release）します。
- **引数**:
  - `eye`: 完了した目のインデックス (`0` または `1`)。

#### `bool submit(bool mirror = true)`
左右両眼のレンダリング結果を合成レイヤー（`XrCompositionLayerProjection`）として OpenXR ランタイムへ転送（`xrEndFrame`）します。
- **引数**:
  - `mirror`: `true` の場合、PC のウィンドウ（`GgApp::Window`）へ左目画像を `glBlitFramebuffer` でミラー転送します。
- **戻り値**: 成功時に `true`。

---

### カメラ・投影変換・視点情報

#### `GgMatrix getProjectionMatrix(int eye, GLfloat zNear = 0.1f, GLfloat zFar = 100.0f) const`
指定した目の視野角情報（`XrFovf`）から、オフセンター視錐台に対応した透視投影変換行列を生成して返します。
- **引数**:
  - `eye`: 目のインデックス (`0`: 左目, `1`: 右目)。
  - `zNear`: 前方面の位置（メートル、デフォルト `0.1f`）。
  - `zFar`: 後方面の位置（メートル、デフォルト `100.0f`）。
- **戻り値**: `GgMatrix` 型の透視投影変換行列。

#### `GgMatrix getViewMatrix(int eye) const`
トラッキングされた視点位置と頭部回転から、ワールド座標系から視点（カメラ）座標系への変換を行うビュー変換行列を計算して返します。
- **計算式**: $V = R^\top \cdot T(-p) = \text{conjugate}(q) \cdot \text{translate}(-p)$
- **戻り値**: `GgMatrix` 型のビュー変換行列。

#### `GgMatrix getPoseMatrix(int eye) const`
ワールド座標系における各眼のローカル座標系変換行列（カメラのモデル変換行列 / 位置と向き）を返します。
- **計算式**: $M = T(p) \cdot R(q)$
- **戻り値**: `GgMatrix` 型の姿勢行列。

#### `GgVector getPosition(int eye) const`
指定した目の視点位置 $(x, y, z, 1.0)$ を `GgVector` で返します。

#### `GgQuaternion getOrientation(int eye) const`
指定した目の回転四元数 $(x, y, z, w)$ を `GgQuaternion` で返します。

#### `const XrFovf& getFov(int eye) const`
指定した目の OpenXR 視野角構造体（`angleLeft`, `angleRight`, `angleUp`, `angleDown` [ラジアン]）の参照を返します。

#### `const XrPosef& getPose(int eye) const`
指定した目の OpenXR 姿勢構造体（`position`, `orientation`）の参照を返します。

#### `GLsizei getWidth(int eye = 0) const` / `GLsizei getHeight(int eye = 0) const`
OpenXR ランタイムが推奨するレンダリング解像度の幅および高さをピクセル単位で返します。

#### `GLfloat getAspect(int eye = 0) const`
推奨解像度のアスペクト比（幅 / 高さ）を返します。

#### `uint32_t getViewCount() const`
ビューの総数（通常ステレオ表示で `2`）を返します。

---

### コントローラートラッキング・姿勢

#### `bool isTracked(int hand) const`
指定した手（`Hand::Left` または `Hand::Right`）のコントローラーが現在有効にトラッキングされているかを判定します。

#### `GgMatrix getGripMatrix(int hand) const`
コントローラーのグリップ位置・姿勢を表すモデル変換行列（位置と回転）を返します。コントローラーの 3D モデルを描画する際の配置行列として使用できます。

#### `GgMatrix getAimMatrix(int hand) const`
コントローラーのポインティング（エイム）方向を表すモデル変換行列を返します。レーザーポインターや銃の照準ベクトルを描画・計算する際に適しています。

#### `GgVector getGripPosition(int hand) const` / `GgQuaternion getGripOrientation(int hand) const`
グリップのワールド位置座標および回転四元数を取得します。

#### `GgVector getAimPosition(int hand) const` / `GgQuaternion getAimOrientation(int hand) const`
エイムのワールド位置座標および回転四元数を取得します。

---

### コントローラーボタン・スティック入力・振動

#### `float getTrigger(int hand) const`
人差し指トリガーのアナログ押し込み量（`0.0f` ～ `1.0f`）を返します。

#### `float getGrip(int hand) const`
中指グリップ（スクイーズ）のアナログ押し込み量（`0.0f` ～ `1.0f`）を返します。

#### `std::array<float, 2> getThumbstick(int hand) const`
アナログスティック（またはトラックパッド）の 2 次元入力値 $(x, y)$（各 `-1.0f` ～ `1.0f`）を返します。

#### `bool getThumbstickClick(int hand) const`
アナログスティックを押し込んだクリック状態（押されていれば `true`）を返します。

#### `bool getPrimaryButton(int hand) const`
プライマリボタン（左手: `X` ボタン、右手: `A` ボタン）の押下状態を返します。

#### `bool getSecondaryButton(int hand) const`
セカンダリボタン（左手: `Y` ボタン、右手: `B` ボタン）の押下状態を返します。

#### `bool getMenuButton(int hand = Hand::Left) const`
メニューボタンの押下状態を返します。

#### `void applyHapticVibration(int hand, float durationSeconds = 0.1f, float frequency = XR_FREQUENCY_UNSPECIFIED, float amplitude = 0.5f)`
コントローラーに触覚フィードバック（振動）を出力します。
- **引数**:
  - `hand`: 対象の手 (`Hand::Left` または `Hand::Right`)。
  - `durationSeconds`: 振動の持続時間（秒）。
  - `frequency`: 振動周波数（Hz、デフォルト `XR_FREQUENCY_UNSPECIFIED`）。
  - `amplitude`: 振動の強さ（`0.0f` ～ `1.0f`、デフォルト `0.5f`）。

---

### 参照空間（空間座標系）

#### `XrReferenceSpaceType getReferenceSpaceType() const`
現在有効な参照空間のタイプ（`XR_REFERENCE_SPACE_TYPE_STAGE` または `XR_REFERENCE_SPACE_TYPE_LOCAL`）を返します。

---

## 4. 実践チュートリアル＆使用例

### 4.1 基本的なステレオレンダリング

```cpp
#define GG_USE_OPENXR
#include "GgApp.h"

int GgApp::main(int argc, const char* const* argv)
{
  Window window("OpenXR Basic", 1280, 720);
  auto& openxr = OpenXR::initialize(window);

  // 描画設定
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  while (window)
  {
    if (openxr.begin())
    {
      for (int eye = 0; eye < openxr.getViewCount(); ++eye)
      {
        openxr.select(eye);

        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 透視投影行列 (Near: 0.05m, Far: 50.0m)
        const GgMatrix mp = openxr.getProjectionMatrix(eye, 0.05f, 50.0f);
        // 頭部姿勢に追従するビュー行列
        const GgMatrix mv = openxr.getViewMatrix(eye);

        // シーンを描画
        // myShader.use(mp, mv * myObjectMatrix);
        // myObject.draw();

        openxr.commit(eye);
      }
      openxr.submit(true);
    }
  }

  openxr.terminate();
  return 0;
}
```

---

### 4.2 コントローラーの姿勢取得とモデル描画

コントローラーの位置・向きに合わせて手のオブジェクトや武器モデルを配置・描画します。

```cpp
for (int hand = 0; hand < OpenXR::Hand::Count; ++hand)
{
  if (openxr.isTracked(hand))
  {
    // コントローラーのグリップ位置・向きのモデル行列を取得
    const GgMatrix mg = openxr.getGripMatrix(hand);

    // ビュー変換と掛け合わせて描画
    handShader.use(mp, mv * mg, light);
    handModel.draw();
  }
}
```

---

### 4.3 スティック入力による移動とトリガー入力

左スティックでプレイヤーを前後左右に移動させ、右トリガーでオブジェクトを掴む・発射する例です。

```cpp
// プレイヤーのワールド座標位置
static GgVector playerPos{ 0.0f, 0.0f, 0.0f, 1.0f };

// 左スティックの入力を取得
const auto stick = openxr.getThumbstick(OpenXR::Hand::Left);
const float moveSpeed = 0.05f;

// スティックの傾きに応じて移動
playerPos[0] += stick[0] * moveSpeed;
playerPos[2] -= stick[1] * moveSpeed; // 前後移動

// プレイヤーの平行移動行列
const GgMatrix playerMatrix = ggTranslate(playerPos[0], playerPos[1], playerPos[2]);

// 右トリガーの押し込みをチェック
if (openxr.getTrigger(OpenXR::Hand::Right) > 0.8f)
{
  // トリガーが深く引かれたときに右手に振動を与える
  openxr.applyHapticVibration(OpenXR::Hand::Right, 0.05f, XR_FREQUENCY_UNSPECIFIED, 0.8f);
}

// レンダリングループ内でのビュー行列適用
const GgMatrix mv = openxr.getViewMatrix(eye) * playerMatrix.inverse();
```

---

### 4.4 床面基準 (STAGE) と着席基準 (LOCAL) の切り替え

```cpp
// ルームスケール（床面が Y=0 となる座標系、立ち位置・歩行用）
auto& openxr = OpenXR::initialize(window, XR_REFERENCE_SPACE_TYPE_STAGE);

// または着席プレイ（HMD の起動位置が原点となる座標系、フライトシムやドライブゲーム用）
auto& openxr = OpenXR::initialize(window, XR_REFERENCE_SPACE_TYPE_LOCAL);
```

---

### 4.5 PC ウィンドウへのミラー表示

`openxr.submit(mirror)` の引数でミラー表示の有無を切り替えられます。

```cpp
// PC 画面に HMD の左目視界をフルスクリーン／ウィンドウサイズに合わせて表示
openxr.submit(true);

// ミラー表示をオフにしてパフォーマンスを最優先する場合
openxr.submit(false);
```

---

## 5. 旧 Oculus SDK (LibOVR) ラッパーからの移行ガイド

旧 `GgApp::Oculus` (LibOVR) および旧 `GgApp::OpenXR` から、最適化版 API への対応表です。

| 処理内容 | 旧 API (LibOVR ラッパー互換) | 最適化版 OpenXR API |
| :--- | :--- | :--- |
| **初期化** | `Oculus::initialize(window)` | `OpenXR::initialize(window, spaceType)` |
| **目の指定とFBOバインド** | `openxr.select(eye, screen, pos, ori)` | `openxr.select(eye)` |
| **透視投影行列の生成** | 手動で `ggFrustum(screen[0]*n, ...)` | `openxr.getProjectionMatrix(eye, zNear, zFar)` |
| **ビュー行列の生成** | 手動で `GgQuaternion(ori).getConjugateMatrix() * ggTranslate(-pos)` | `openxr.getViewMatrix(eye)` |
| **カメラ姿勢行列** | 手動で合成 | `openxr.getPoseMatrix(eye)` |
| **TimeWarp 設定** | `openxr.timewarp(mp)` (空ダミー) | **不要 (削除)** |
| **フレーム完了** | `openxr.commit(eye)` | `openxr.commit(eye)` |
| **フレーム転送** | `openxr.submit(mirror)` | `openxr.submit(mirror)` |
| **コントローラー姿勢** | なし | `openxr.getGripMatrix(hand)`, `getAimMatrix(hand)` |
| **ボタン・スティック入力**| なし | `getTrigger`, `getGrip`, `getThumbstick`, `getPrimaryButton` など |
| **ハプティクス振動** | なし | `openxr.applyHapticVibration(hand, ...)` |

### コードの対比（Before / After）

#### [Before] 旧 API
```cpp
GLfloat screen[4], position[3], orientation[4];
openxr.select(eye, screen, position, orientation);

// 面倒な手動計算が必要だった
const GgMatrix mp = ggFrustum(
  screen[0] * zNear, screen[1] * zNear,
  screen[2] * zNear, screen[3] * zNear,
  zNear, zFar);

const GgMatrix mv = GgQuaternion(orientation).getConjugateMatrix()
                  * ggTranslate(-position[0], -position[1], -position[2]);

shader.use(mp, mv * model, light);
model.draw();
openxr.commit(eye);
```

#### [After] 最適化版 API
```cpp
openxr.select(eye);

// 1行で直感的に取得可能！
const GgMatrix mp = openxr.getProjectionMatrix(eye, zNear, zFar);
const GgMatrix mv = openxr.getViewMatrix(eye);

shader.use(mp, mv * model, light);
model.draw();
openxr.commit(eye);
```

---

## 6. 動作環境とセットアップ

### 必要な環境
- **OS**:
  - Windows 10 / 11 (64bit)
  - Linux (Ubuntu / Linux Mint 等、X11 環境)
- **VR ハードウェア**:
  - Meta Quest 2 / 3 / 3S / Pro (Quest Link / Air Link / Virtual Desktop / Steam Link)
  - HTC Vive / Vive Pro / Focus 3
  - Valve Index
  - Windows Mixed Reality 各種 HMD
  - Bigscreen Beyond, Pico 4 等
- **OpenXR ランタイム**:
  - Meta Quest Link アプリ（Oculus ランタイムをアクティブに設定）
  - SteamVR（SteamVR 設定 -> OpenXR ->「SteamVR をアクティブな OpenXR ランタイムとして設定」）
  - Monado (Linux 用オープンソースランタイム)

### プロジェクトへの組み込み
1. ソースコード冒頭で OpenXR を有効化：
   ```cpp
   #define GG_USE_OPENXR
   #include "GgApp.h"
   ```
2. OpenXR SDK のヘッダー (`<openxr/openxr.h>`, `<openxr/openxr_platform.h>`) およびローダーライブラリ (`openxr_loader.lib` または `-lopenxr_loader`) をプロジェクトにリンクします。
