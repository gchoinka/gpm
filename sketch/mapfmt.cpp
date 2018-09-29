/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <iostream>
#include <utility>
#define FMT_HEADER_ONLY
#include "fmt/format.h"

#include <initializer_list>
#include <tuple>

#include <gpm/utils/fmtutils.hpp>

bool replacePlaceHolderSingel(std::string& str, std::string_view const& key,
                              std::string_view const& to) {
  std::string from = "{" + std::string{key} + "}";
  if (str.empty()) return false;
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos + 1, from.length() - 2, to);
    start_pos += to.length();  // In case 'to' contains 'from', like replacing
                               // 'x' with 'yx'
  }
  return true;
}

template <typename Tuple, size_t... Idx>
decltype(auto) fmtinvoke(std::string fstr, const Tuple& args,
                         std::integer_sequence<size_t, Idx...>) {
  // TODO replace  std::to_string with compile time to_string
  auto dummy = {
      (replacePlaceHolderSingel(fstr, std::get<Idx * 2>(args),
                                std::to_string(Idx)),
       0)...,
  };
  (void)dummy;
  return fmt::format(fstr, std::get<Idx * 2 + 1>(args)...);
}

template <typename StrT, typename... T>
decltype(auto) mapformat(StrT&& fstr, T... args) {
  return fmtinvoke(std::forward<StrT>(fstr), std::tuple<T const&...>{args...},
                   std::make_integer_sequence<size_t, sizeof...(args) / 2>());
}



template <typename KeyT, typename ValueT>
struct argHolder {
  KeyT k;
  ValueT v;
};




int main() {
  using namespace gpm::utils;
  std::cout << mapformat(R"""(
{indent}if({cond})
{indent}{{
{indent}    {true_case}
{indent}}}
{indent}else
{indent}{{
{indent}    {false_case}
{indent}}}
)""",
                         "cond", 42, "true_case", "p3", "false_case", "Nope",
                         "indent", "    ");

  std::cout << format(R"""(
{indent}if({cond})
{indent}{{
{indent}    {true_case}
{indent}}}
{indent}else
{indent}{{
{indent}    {false_case}
{indent}}}
)""",
                      argsnamed, "cond", 42, "true_case", "p3", "false_case",
                      "Nope", "indent", "    ", "su");

  std::cout << fmt::format(
                   "Hello, {name}! The answer is {number}. Goodbye, {name}.",
                   fmt::arg("name", "World"), fmt::arg("number", 42))
            << "\n";

  // std::cout<< mf("Hello, {name}! The answer is {number}. Goodbye, {name}.",
  // toArgs2({"name", "World"},{"number", 42}));
}
