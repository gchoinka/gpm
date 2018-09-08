/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <utility> //std::forward std::integer_sequence
#include <tuple>
#include <cstddef> //size_t

#include <fmt/format.h>

namespace gpm::utils
{
namespace detail
{
template<typename StrT, typename Tuple, size_t...Idx>
decltype(auto) formatNamed_imp(StrT && fstr, Tuple&& args, std::integer_sequence<size_t, Idx...>) 
{
    return fmt::format(std::forward<StrT>(fstr), fmt::arg(std::get<Idx*2>(std::forward<Tuple>(args)), std::get<Idx*2+1>(std::forward<Tuple>(args)))...);
}
}


template<typename StrT, typename ...T>
decltype(auto) formatNamed(StrT && fstr, T && ... args)
{
    return detail::formatNamed_imp(std::forward<StrT>(fstr), std::tuple<T...>{std::forward<T>(args)...}, std::make_integer_sequence<size_t, sizeof...(args)/2>() );
}

}
