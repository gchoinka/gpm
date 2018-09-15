/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include <cstddef>

namespace ant::santa_fe {

constexpr size_t const x_size = 32;
constexpr size_t const y_size = 32;

constexpr size_t const x_size = 32;
constexpr size_t const y_size = 32;

// clang-format off
constexpr char const * const board[] = {
" XXX                            ",
"   X                            ",
"   X                    .XXX..  ",
"   X                    X    X  ",
"   X                    X    X  ",
"   XXXX.XXXXX       .XX..    .  ",
"            X       .        X  ",
"            X       X        .  ",
"            X       X        .  ",
"            X       X        X  ",
"            .       X        .  ",
"            X       .        .  ",
"            X       .        X  ",
"            X       X        .  ",
"            X       X  ...XXX.  ",
"            .   .X...  X        ",
"            .   .      .        ",
"            X   .      .        ",
"            X   X      .X...    ",
"            X   X          X    ",
"            X   X          .    ",
"            X   X          .    ",
"            X   .      ...X.    ",
"            X   .      X        ",
" ..XX..XXXXX.   X               ",
" X              X               ",
" X              X               ",
" X     .XXXXXXX..               ",
" X     X                        ",
" .     X                        ",
" .XXXX..                        ",
"                                "
};
// clang-format on

namespace detail {

constexpr int length(const char* str) { return *str ? 1 + length(str + 1) : 0; }

static_assert(
    sizeof(board) / sizeof(board[0]) == y_size,
    "Constant y_size is different from the y size of ant::santa_fe::board "
    "array, please update x_size or check the array.");

template <int XRow>
constexpr int checkYSize() {
  static_assert(
      length(board[XRow]) == x_size,
      "Constant x_size is different from the x size of ant::santa_fe::board "
      "array, please update y_size or check the array");
  if constexpr ((XRow - 1) >= 0)
    return checkYSize<XRow - 1>();
  else
    return 0;
}

constexpr auto dummyValue = checkYSize<sizeof(board) / sizeof(board[0]) - 1>();
}  // namespace detail

}  // namespace ant::santa_fe
