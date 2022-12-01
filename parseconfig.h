#pragma once

///
/// 構成ファイルの読み取り補助
///
/// @file
/// @author Kohe Tokoi
/// @date November 15, 2022
///

// JSON
#include "picojson.h"

///
/// 構成ファイルの JSON オブジェクトから数値を取得する
///
/// @tparam T 構成ファイルから取得する数値のデータ型
/// @param object 構成ファイルの JSON オブジェクト
/// @param key 取得する JSON オブジェクトのキー
/// @param scalar 取得した JSON オブジェクトの数値を格納する変数
///
template <typename T>
bool getValue(const picojson::object& object,
  const std::string& key, T& scalar)
{
  // key に一致するオブジェクトを探す
  const auto&& value{ object.find(key) };

  // オブジェクトが無いか数値でなかったら戻る
  if (value == object.end() || !value->second.is<double>()) return false;

  // 数値として格納する
  scalar = static_cast<T>(value->second.get<double>());

  return true;
}

///
/// 構成ファイルの JSON オブジェクトから数値の配列を取得する
///
/// @tparam T 構成ファイルから取得する配列の要素の数値のデータ型
/// @tparam U 構成ファイルから取得する配列の要素の数値の数
/// @param object 構成ファイルの JSON オブジェクト
/// @param key 取得する JSON オブジェクトのキー
/// @param vector 取得した数値の配列を格納する変数
///
template <typename T, std::size_t U>
bool getValue(const picojson::object& object,
  const std::string& key, std::array<T, U>& vector)
{
  // key に一致するオブジェクトを探す
  const auto&& value{ object.find(key) };

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
    if (array[i].is<double>()) vector[i] = static_cast<T>(array[i].get<double>());
  }

  return true;
}

///
/// 構成ファイルの JSON オブジェクトから４要素の数値のベクトルを取得する
///
/// @param object 構成ファイルの JSON オブジェクト
/// @param key 取得する JSON オブジェクトのキー
/// @param vector 取得した数値の配列を格納する変数
///
inline
bool getVector(const picojson::object& object,
  const std::string& key, GgVector& vector)
{
  // key に一致するオブジェクトを探す
  const auto&& value{ object.find(key) };

  // オブジェクトが無いか配列でなかったら戻る
  if (value == object.end() || !value->second.is<picojson::array>()) return false;

  // 配列を取り出す
  const auto& array{ value->second.get<picojson::array>() };

  // 配列の要素数とデータの格納先の要素数の少ない方の数
  const auto n{ std::min(sizeof(GgVector) / sizeof(GLfloat), array.size()) };

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
///
inline
bool getString(const picojson::object& object,
  const std::string& key, std::string& string)
{
  // key に一致するオブジェクトを探す
  const auto&& value{ object.find(key) };

  // オブジェクトが無いか文字列でなかったら戻る
  if (value == object.end() || !value->second.is<std::string>()) return false;

  // 文字列として格納する
  string = value->second.get<std::string>();

  return true;
}

///
/// 構成ファイルの JSON オブジェクトから文字列の配列を取得する
///
/// @tparam U 構成ファイルから取得する配列の要素の文字列の数
/// @param object 構成ファイルの JSON オブジェクト
/// @param key 取得する JSON オブジェクトのキー
/// @param strings 取得した文字列の配列を格納する変数
///
template <std::size_t U>
bool getString(const picojson::object& object,
  const std::string& key, std::array<std::string, U>& strings)
{
  // key に一致するオブジェクトを探す
  const auto&& value{ object.find(key) };

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
/// 構成ファイルの JSON オブジェクトから文字列のベクトルを取得する
///
/// @param object 構成ファイルの JSON オブジェクト
/// @param key 取得する JSON オブジェクトのキー
/// @param strings 取得した文字列の配列を格納する変数
///
inline
bool getString(const picojson::object& object,
  const std::string& key, std::vector<std::string>& strings)
{
  // key に一致するオブジェクトを探す
  const auto&& value{ object.find(key) };

  // オブジェクトが無いか配列でなかったら戻る
  if (value == object.end() || !value->second.is<picojson::array>()) return false;

  // 配列を取り出す
  const auto& array{ value->second.get<picojson::array>() };

  // 配列のすべての要素について
  for (auto& element : array)
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
  object.emplace(key, picojson::value(static_cast<double>(scalar)));
}

///
/// 構成ファイルの JSON オブジェクトに数値の配列を設定する
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

  // 配列のすべての要素について
  for (const auto& element : vector)
  {
    // 要素を picojson::array に追加する
    array.emplace_back(picojson::value(static_cast<double>(element)));
  }

  // オブジェクトに追加する
  object.emplace(key, array);
}

///
/// 構成ファイルの JSON オブジェクトに４要素の数値のベクトルを設定する
///
/// @param object 構成ファイルの JSON オブジェクト
/// @param key 設定する JSON オブジェクトのキー
/// @param vector 設定する数値の配列
///
inline
void setVector(picojson::object& object,
  const std::string& key, const GgVector& vector)
{
  // picojson の配列
  picojson::array array;

  // ベクトルのすべての要素について
  for (const auto& element : vector)
  {
    // 要素を picojson::array に追加する
    array.emplace_back(picojson::value(static_cast<double>(element)));
  }

  // オブジェクトに追加する
  object.emplace(key, array);
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
  object.emplace(key, picojson::value(string));
}

///
/// 構成ファイルの JSON オブジェクトに文字列の配列を設定する
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

  // 配列のすべての要素について
  for (const auto& string : strings)
  {
    // 要素を picojson::array に追加する
    array.emplace_back(picojson::value(string));
  }

  // オブジェクトに追加する
  object.emplace(key, array);
}

///
/// 構成ファイルの JSON オブジェクトに文字列のベクトルを設定する
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

  // 配列のすべての要素について
  for (auto& string : strings)
  {
    // 要素を picojson::array に追加する
    array.emplace_back(picojson::value(string));
  }

  // オブジェクトに追加する
  object.emplace(key, array);
}
