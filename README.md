# ggsample01 - Dear ImGui を統合したモダン OpenGL アプリケーション

## 1. 概要

本プログラムは、GLFW 3 と Dear ImGui、および Native File Dialog Extended (NFD) を統合したモダン OpenGL のベースアプリケーションフレームワークです。

- 移行元ブログ記事:
  - [ゲームグラフィックス特論 (2018) - 床井研究室](https://tokoik.github.io/blog/2018/10/14/)

## 2. 宿題の内容

[ggnote01.pdf](https://tokoik.github.io/gg/ggnote01.pdf) の宿題ひな型プログラムです。
OpenGL の開発環境（GLFW 3, Dear ImGui, NFD, picojson）を整備し、本プログラムをビルドして正常に起動・操作できることを確認してください。

## 3. 対応環境

- **Windows**: Visual Studio 2019 / 2022 / 2026 (CMake 経由で GLFW, ImGui, NFD, picojson を自動構成)
- **macOS**: Xcode (GLFW, ImGui, NFD を自動ダウンロード、OpenGL Framework を使用)
- **Ubuntu Linux**: GCC / Make (システム標準の libglfw3-dev, libgl1-mesa-dev, libgtk-3-dev を使用)

## 4. ビルド手順

### Windows (Visual Studio)

```pwsh
cmake -B build -S .
cmake --build build --config Release
```

### macOS (Xcode)

```bash
cmake -B build -G Xcode
cmake --build build --config Release
```

### Ubuntu Linux (Makefile)

```bash
sudo apt-get update
sudo apt-get install -y libglfw3-dev libgl1-mesa-dev libgtk-3-dev
cmake -B build -S .
cmake --build build
```

## 5. 起動方法

ビルド完了後、生成された実行ファイルを実行します。

- **Windows**: `build/Release/ggsample01.exe`
- **macOS**: `build/Release/ggsample01.app`
- **Linux**: `build/ggsample01`

## 6. 操作方法

- **マウス左ドラッグ**: シーンの視点回転
- **マウス右ドラッグ**: 視点の平行移動
- **マウスホイール**: 視点のズームイン / ズームアウト
- **ImGui メニューパネル**: 光源位置・拡散反射光色のリアルタイム調整
- **[q] / [Q] / [ESC]**: プログラムの終了

## 7. プログラムの解説

`Menu.cpp` で Dear ImGui の GUI ウィジェットを構築し、光源パラメータを直接操作して `simple.vert` および `simple.frag` にユニフォーム変数として転送します。設定情報は `Config.cpp` および `picojson.h` により `ggsample01_config.json` へ保存・復元が可能です。
