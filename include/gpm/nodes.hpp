/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include <array>
#include <boost/variant.hpp>
#include <utility>

namespace gpm {

template <auto... ch>
struct NodeToken {
  constexpr static char name[] = {ch..., '\0'};
};

template <typename VariantType, size_t NodeCount_, typename CTString>
struct BaseNode : public CTString {
  template <typename... Args>
  BaseNode(Args &&... args) : children{std::forward<Args>(args)...} {}

  std::array<VariantType, NodeCount_> children;
};

struct AnyTypeNullSink {
  AnyTypeNullSink() {}

  template <typename T>
  AnyTypeNullSink(T const &&) {}
  template <typename T>
  AnyTypeNullSink const &operator=(T const &&) {
    return *this;
  }
};

}  // namespace gpm
