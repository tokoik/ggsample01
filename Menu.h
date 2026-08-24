#pragma once

///
/// メニューの描画クラスの定義
///
/// @file
/// @author Kohe Tokoi
/// @date August 15, 2025
///

// 宿題用補助プログラムのラッパー
#include "GgApp.h"

// 構成データ
#include "Config.h"

// ファイルダイアログ
#include "nfd.h"

///
/// メニューの描画
///
class Menu
{
  // 図形の描画クラスから参照する
  friend class Scene;

  // オリジナルの構成データ
  const Config& defaults;

  // 構成データのコピー
  Config settings;

  // 光源データ
  std::unique_ptr<const gg::GgSimpleShader::LightBuffer> light;

  // シェーダ
  std::unique_ptr<const gg::GgSimpleShader> shader;

  // CAD データ
  std::unique_ptr<const gg::GgSimpleObj> model;

  ///
  /// ファイルダイアログ (Native File Dialog Extended) の初期化と終了
  ///
  /// @note
  /// NFD_Init() と NFD_Quit() を 1 対 1 で対応させる. これを Menu の
  /// メンバにしておくと, コンストラクタが途中で例外を投げたときにも
  /// 終了処理が行われ, Menu をムーブしたときにも移動元の抜け殻が
  /// NFD_Quit() を呼んでしまうことがない.
  ///
  class FileDialog
  {
    // 初期化に成功していれば true
    bool initialized{ false };

  public:

    ///
    /// コンストラクタ
    ///
    FileDialog();

    ///
    /// コピーコンストラクタは使用しない
    ///
    FileDialog(const FileDialog& dialog) = delete;

    ///
    /// ムーブコンストラクタ
    ///
    /// @param dialog ムーブ元のオブジェクト
    ///
    FileDialog(FileDialog&& dialog) noexcept :
      initialized{ dialog.initialized }
    {
      // 終了処理を行う権利を移動する
      dialog.initialized = false;
    }

    ///
    /// デストラクタ
    ///
    ~FileDialog();

    ///
    /// 代入演算子は使用しない
    ///
    FileDialog& operator=(const FileDialog& dialog) = delete;

    ///
    /// ムーブ代入演算子は使用しない
    ///
    FileDialog& operator=(FileDialog&& dialog) = delete;

    ///
    /// ファイルダイアログが使用できるかどうか調べる
    ///
    /// @return 使用できれば true
    ///
    explicit operator bool() const
    {
      return initialized;
    }
  };

  // ファイルダイアログ
  FileDialog fileDialog;

  // ファイルパスを取得する
  bool getFilePath(std::string& path, const nfdfilteritem_t* filter);

public:

  ///
  /// コンストラクタ
  ///
  Menu(const Config& config);

  ///
  /// コピーコンストラクタは使用しない
  ///
  Menu(const Menu& menu) = delete;

  ///
  /// ムーブコンストラクタはデフォルトのものを使用する
  ///
  Menu(Menu&& menu) = default;

  ///
  /// デストラクタ.
  ///
  virtual ~Menu();

  ///
  /// 代入演算子は使用しない
  ///
  Menu& operator=(const Menu& menu) = delete;

  ///
  /// 光源データを取り出す
  ///
  const auto& getLight() const
  {
    return *light;
  }

  ///
  /// シェーダを取り出す
  ///
  const auto& getShader() const
  {
    return *shader;
  }

  ///
  /// モデルデータを取り出す
  ///
  const auto& getModel() const
  {
    return *model;
  }

  ///
  /// 描画する
  ///
  void draw();
};
