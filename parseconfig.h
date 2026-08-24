#pragma once

///
/// 構成ファイルの読み取り補助
///
/// @file
/// @author Kohe Tokoi
/// @date November 15, 2022
///

// 補助プログラム
#include "gg.h"

// JSON
#include "picojson.h"

// 標準ライブラリ
#include <string>
#include <vector>
#include <array>
#include <algorithm>

///
/// 構成ファイルの JSON オブジェクトから数値を取得する
///
/// @tparam T 構成ファイルから取得する数値のデータ型
/// @param object 構成ファイルの JSON オブジェクト
/// @param key 取得する JSON オブジェクトのキー
/// @param scalar 取得した JSON オブジェクトの数値を格納する変数
/// @return 取得に成功すれば true
///
template <typename T>
bool getValue(const picojson::object& object,
  const std::string& key, T& scalar)
{
  // key に一致するオブジェクトを探す
  const auto value{ object.find(key) };

  // オブジェクトが無かったら戻る
  if (value == object.end()) return false;

  // 数値または真偽値として格納する
  if (value->second.is<double>())
  {
    scalar = static_cast<T>(value->second.get<double>());
    return true;
  }
  if (value->second.is<bool>())
  {
    scalar = static_cast<T>(value->second.get<bool>());
    return true;
  }

  return false;
}

///
/// 構成ファイルの JSON オブジェクトから真偽値を取得する
///
/// @param object 構成ファイルの JSON オブジェクト
/// @param key 取得する JSON オブジェクトのキー
/// @param boolean 取得した JSON オブジェクトの真偽値を格納する変数
/// @return 取得に成功すれば true
///
inline
bool getValue(const picojson::object& object,
  const std::string& key, bool& boolean)
{
  // key に一致するオブジェクトを探す
  const auto value{ object.find(key) };

  // オブジェクトが無かったら戻る
  if (value == object.end()) return false;

  // 真偽値または数値として格納する
  if (value->second.is<bool>())
  {
    boolean = value->second.get<bool>();
    return true;
  }
  if (value->second.is<double>())
  {
    boolean = value->second.get<double>() != 0.0;
    return true;
  }

  return false;
}

///
/// 構成ファイルの JSON オブジェクトから数値の固定長配列を取得する
///
/// @tparam T 構成ファイルから取得する配列の要素の数値のデータ型
/// @tparam U 構成ファイルから取得する配列の要素の数値の数
/// @param object 構成ファイルの JSON オブジェクト
/// @param key 取得する JSON オブジェクトのキー
/// @param vector 取得した数値の配列を格納する変数
/// @return 取得に成功すれば true
///
template <typename T, std::size_t U>
bool getValue(const picojson::object& object,
  const std::string& key, std::array<T, U>& vector)
{
  // key に一致するオブジェクトを探す
  const auto value{ object.find(key) };

  // オブジェクトが無いか配列でなかったら戻る
  if (value == object.end() || !value->second.is<picojson::array>()) return false;

  // 配列を取り出す
  const auto& array{ value->second.get<picojson::array>() };

  // 配列の要素数とデータの格納先の要素数の少ない方の数
  const auto n{ std::min(U, array.size()) };

  // 配列の要素について
  for (std::size_t i = 0; i < n; ++i)
  {
    // 要素が数値なら格納する
    if (array[i].is<double>())
    {
      vector[i] = static_cast<T>(array[i].get<double>());
    }
    else if (array[i].is<bool>())
    {
      vector[i] = static_cast<T>(array[i].get<bool>());
    }
  }

  return true;
}

///
/// 構成ファイルの JSON オブジェクトから数値の可変長配列を取得する
///
/// @tparam T 構成ファイルから取得する配列の要素の数値のデータ型
/// @param object 構成ファイルの JSON オブジェクト
/// @param key 取得する JSON オブジェクトのキー
/// @param vector 取得した数値の配列を格納する変数
/// @return 取得に成功すれば true
///
template <typename T>
bool getValue(const picojson::object& object,
  const std::string& key, std::vector<T>& vector)
{
  // key に一致するオブジェクトを探す
  const auto value{ object.find(key) };

  // オブジェクトが無いか配列でなかったら戻る
  if (value == object.end() || !value->second.is<picojson::array>()) return false;

  // 配列を取り出す
  const auto& array{ value->second.get<picojson::array>() };

  // メモリを確保してクリア
  vector.clear();
  vector.reserve(array.size());

  // 配列の要素について
  for (const auto& element : array)
  {
    if (element.is<double>())
    {
      vector.push_back(static_cast<T>(element.get<double>()));
    }
    else if (element.is<bool>())
    {
      vector.push_back(static_cast<T>(element.get<bool>()));
    }
  }

  return true;
}

///
/// 構成ファイルの JSON オブジェクトから４要素の数値のベクトルを取得する
///
/// @param object 構成ファイルの JSON オブジェクト
/// @param key 取得する JSON オブジェクトのキー
/// @param vector 取得した数値のベクトルを格納する変数
/// @return 取得に成功すれば true
///
inline
bool getVector(const picojson::object& object,
  const std::string& key, gg::GgVector& vector)
{
  // key に一致するオブジェクトを探す
  const auto value{ object.find(key) };

  // オブジェクトが無いか配列でなかったら戻る
  if (value == object.end() || !value->second.is<picojson::array>()) return false;

  // 配列を取り出す
  const auto& array{ value->second.get<picojson::array>() };

  // 配列の要素数とデータの格納先の要素数の少ない方の数
  const auto n{ std::min(vector.size(), array.size()) };

  // 配列の要素について
  for (std::size_t i = 0; i < n; ++i)
  {
    // 要素が数値なら格納する
    if (array[i].is<double>()) vector[i] = static_cast<GLfloat>(array[i].get<double>());
  }

  return true;
}

///
/// 構成ファイルの JSON オブジェクトから文字列を取得する
///
/// @param object 構成ファイルの JSON オブジェクト
/// @param key 取得する JSON オブジェクトのキー
/// @param string 取得した文字列を格納する変数
/// @return 取得に成功すれば true
///
inline
bool getString(const picojson::object& object,
  const std::string& key, std::string& string)
{
  // key に一致するオブジェクトを探す
  const auto value{ object.find(key) };

  // オブジェクトが無いか文字列でなかったら戻る
  if (value == object.end() || !value->second.is<std::string>()) return false;

  // 文字列として格納する
  string = value->second.get<std::string>();

  return true;
}

///
/// 構成ファイルの JSON オブジェクトから文字列の固定長配列を取得する
///
/// @tparam U 構成ファイルから取得する配列の要素の文字列の数
/// @param object 構成ファイルの JSON オブジェクト
/// @param key 取得する JSON オブジェクトのキー
/// @param strings 取得した文字列の配列を格納する変数
/// @return 取得に成功すれば true
///
template <std::size_t U>
bool getString(const picojson::object& object,
  const std::string& key, std::array<std::string, U>& strings)
{
  // key に一致するオブジェクトを探す
  const auto value{ object.find(key) };

  // オブジェクトが無いか配列でなかったら戻る
  if (value == object.end() || !value->second.is<picojson::array>()) return false;

  // 配列を取り出す
  const auto& array{ value->second.get<picojson::array>() };

  // 配列の要素数とデータの格納先の要素数の少ない方の数
  const auto n{ std::min(U, array.size()) };

  // 配列の要素について
  for (std::size_t i = 0; i < n; ++i)
  {
    // 要素が文字列なら文字列として格納する
    strings[i] = array[i].is<std::string>() ? array[i].get<std::string>() : "";
  }

  return true;
}

///
/// 構成ファイルの JSON オブジェクトから文字列の可変長配列を取得する
///
/// @param object 構成ファイルの JSON オブジェクト
/// @param key 取得する JSON オブジェクトのキー
/// @param strings 取得した文字列の配列を格納する変数
/// @return 取得に成功すれば true
///
inline
bool getString(const picojson::object& object,
  const std::string& key, std::vector<std::string>& strings)
{
  // key に一致するオブジェクトを探す
  const auto value{ object.find(key) };

  // オブジェクトが無いか配列でなかったら戻る
  if (value == object.end() || !value->second.is<picojson::array>()) return false;

  // 配列を取り出す
  const auto& array{ value->second.get<picojson::array>() };

  // メモリを確保してクリア
  strings.clear();
  strings.reserve(array.size());

  // 配列のすべての要素について
  for (const auto& element : array)
  {
    // 要素が文字列なら文字列として格納する
    strings.emplace_back(element.is<std::string>() ? element.get<std::string>() : "");
  }

  return true;
}

///
/// 構成ファイルの JSON オブジェクトに数値を設定する
///
/// @tparam T 構成ファイルに設定する数値のデータ型
/// @param object 構成ファイルの JSON オブジェクト
/// @param key 設定する JSON オブジェクトのキー
/// @param scalar 設定する数値
///
template <typename T>
void setValue(picojson::object& object,
  const std::string& key, const T& scalar)
{
  object[key] = picojson::value(static_cast<double>(scalar));
}

///
/// 構成ファイルの JSON オブジェクトに真偽値を設定する
///
/// @param object 構成ファイルの JSON オブジェクト
/// @param key 設定する JSON オブジェクトのキー
/// @param boolean 設定する真偽値
///
inline
void setValue(picojson::object& object,
  const std::string& key, bool boolean)
{
  object[key] = picojson::value(boolean);
}

///
/// 構成ファイルの JSON オブジェクトに数値の固定長配列を設定する
///
/// @tparam T 構成ファイルに設定する配列の要素の数値のデータ型
/// @tparam U 構成ファイルに設定する配列の要素の数値の数
/// @param object 構成ファイルの JSON オブジェクト
/// @param key 設定する JSON オブジェクトのキー
/// @param vector 設定する数値の配列
///
template <typename T, std::size_t U>
void setValue(picojson::object& object,
  const std::string& key, const std::array<T, U>& vector)
{
  // picojson の配列
  picojson::array array;
  array.reserve(vector.size());

  // 配列のすべての要素について
  for (const auto& element : vector)
  {
    // 要素を picojson::array に追加する
    array.emplace_back(picojson::value(static_cast<double>(element)));
  }

  // オブジェクトに設定する
  object[key] = picojson::value(std::move(array));
}

///
/// 構成ファイルの JSON オブジェクトに数値の可変長配列を設定する
///
/// @tparam T 構成ファイルに設定する配列の要素の数値のデータ型
/// @param object 構成ファイルの JSON オブジェクト
/// @param key 設定する JSON オブジェクトのキー
/// @param vector 設定する数値の配列
///
template <typename T>
void setValue(picojson::object& object,
  const std::string& key, const std::vector<T>& vector)
{
  // picojson の配列
  picojson::array array;
  array.reserve(vector.size());

  // 配列のすべての要素について
  for (const auto& element : vector)
  {
    // 要素を picojson::array に追加する
    array.emplace_back(picojson::value(static_cast<double>(element)));
  }

  // オブジェクトに設定する
  object[key] = picojson::value(std::move(array));
}

///
/// 構成ファイルの JSON オブジェクトに４要素の数値のベクトルを設定する
///
/// @param object 構成ファイルの JSON オブジェクト
/// @param key 設定する JSON オブジェクトのキー
/// @param vector 設定するベクトル
///
inline
void setVector(picojson::object& object,
  const std::string& key, const gg::GgVector& vector)
{
  // picojson の配列
  picojson::array array;
  array.reserve(vector.size());

  // ベクトルのすべての要素について
  for (const auto& element : vector)
  {
    // 要素を picojson::array に追加する
    array.emplace_back(picojson::value(static_cast<double>(element)));
  }

  // オブジェクトに設定する
  object[key] = picojson::value(std::move(array));
}

///
/// 構成ファイルの JSON オブジェクトに文字列を設定する
///
/// @param object 構成ファイルの JSON オブジェクト
/// @param key 設定する JSON オブジェクトのキー
/// @param string 設定する文字列
///
inline
void setString(picojson::object& object,
  const std::string& key, const std::string& string)
{
  object[key] = picojson::value(string);
}

///
/// 構成ファイルの JSON オブジェクトに文字列の固定長配列を設定する
///
/// @tparam U 構成ファイルに設定する配列の要素の文字列の数
/// @param object 構成ファイルの JSON オブジェクト
/// @param key 設定する JSON オブジェクトのキー
/// @param strings 設定する文字列の配列
///
template <std::size_t U>
void setString(picojson::object& object,
  const std::string& key, const std::array<std::string, U>& strings)
{
  // picojson の配列
  picojson::array array;
  array.reserve(strings.size());

  // 配列のすべての要素について
  for (const auto& string : strings)
  {
    // 要素を picojson::array に追加する
    array.emplace_back(picojson::value(string));
  }

  // オブジェクトに設定する
  object[key] = picojson::value(std::move(array));
}

///
/// 構成ファイルの JSON オブジェクトに文字列の可変長配列を設定する
///
/// @param object 構成ファイルの JSON オブジェクト
/// @param key 設定する JSON オブジェクトのキー
/// @param strings 設定する文字列の配列
///
inline
void setString(picojson::object& object,
  const std::string& key, const std::vector<std::string>& strings)
{
  // picojson の配列
  picojson::array array;
  array.reserve(strings.size());

  // 配列のすべての要素について
  for (const auto& string : strings)
  {
    // 要素を picojson::array に追加する
    array.emplace_back(picojson::value(string));
  }

  // オブジェクトに設定する
  object[key] = picojson::value(std::move(array));
}
