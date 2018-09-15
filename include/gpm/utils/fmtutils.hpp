/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <cstddef>  //size_t
#include <tuple>
#include <utility>  //std::forward std::integer_sequence

#include <fmt/format.h>

namespace gpm::utils {

namespace detail {
template <typename StrT, typename TupleT, typename SizeType, SizeType... Idx>
decltype(auto) formatN_imp(StrT&& fstr, TupleT&& packedArgs,
                           std::integer_sequence<SizeType, Idx...>) {
  // test if std::get<Idx*2+0>(std::forward<Tuple>(packedArgs)) is suitable for
  // named arguments
  return fmt::format(
      std::forward<StrT>(fstr),
      fmt::arg(std::get<Idx * 2 + 0>(std::forward<TupleT>(
                   packedArgs)),  // selects all the even arguments
               std::get<Idx * 2 + 1>(std::forward<TupleT>(
                   packedArgs))  // selects all the odd arguments
               )...);
}
}  // namespace detail

struct argsnamed_t {};
constexpr auto argsnamed = argsnamed_t{};

template <typename StrT, typename... ArgsT>
decltype(auto) format(StrT&& fstr, argsnamed_t, ArgsT&&... args) {
  return detail::formatN_imp(
      std::forward<StrT>(fstr),
      std::tuple<ArgsT...>{std::forward<ArgsT>(args)...},
      std::make_integer_sequence<decltype(sizeof...(args) / 2),
                                 sizeof...(args) / 2>());
}

template <typename StrT, typename... ArgsT>
decltype(auto) format(StrT&& fstr, ArgsT&&... args) {
  return fmt::format(std::forward<StrT>(fstr), std::forward<ArgsT>(args)...);
}

}  // namespace gpm::utils
