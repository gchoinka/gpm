/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include <boost/variant.hpp>
#include <string_view>

namespace gpm {

template <typename StringT>
struct Printer : public boost::static_visitor<StringT> {
  template <typename T>
  StringT operator()(T const& node) const {
    char const* delimiter = "";
    char const* begin_delimiter = "";
    char const* end_delimiter = "";
    StringT children;
    if constexpr (std::tuple_size<decltype(node.children)>::value != 0)
      for (auto const& n : node.children) {
        children += delimiter + boost::apply_visitor(*this, n);
        delimiter = " , ";
        begin_delimiter = "( ";
        end_delimiter = " )";
      }
    return StringT{T::name} + begin_delimiter + children + end_delimiter;
  }
};

template <typename StringT>
struct RPNPrinter : public boost::static_visitor<StringT> {
  template <typename T>
  StringT operator()(T const& node) const {
    StringT children;
    if constexpr (std::tuple_size<decltype(node.children)>::value != 0)
      for (auto const& n : node.children) {
        children = boost::apply_visitor(*this, n) + " " + children;
      }
    return children + T::name;
  }
};

template <typename StringT>
struct PNPrinter : public boost::static_visitor<StringT> {
  template <typename T>
  StringT operator()(T const& node) const {
    StringT children;
    if constexpr (std::tuple_size<decltype(node.children)>::value != 0)
      for (auto const& n : node.children) {
        children = children + " " + boost::apply_visitor(*this, n);
      }
    return T::name + children;
  }
};

class RPNTokenCursor {
  std::string_view sv_;
  std::string_view::size_type tokenBegin_;
  std::string_view::size_type tokenEnd_;

 public:
  RPNTokenCursor(std::string_view sv) : sv_{sv} {
    tokenBegin_ = sv_.size();
    --tokenBegin_;
    for (; tokenBegin_ > 0; --tokenBegin_) {
      if (sv_[tokenBegin_] == ' ') {
        tokenBegin_++;
        break;
      }
    }

    tokenEnd_ = tokenBegin_ + 1;
    for (; tokenEnd_ != sv_.size(); ++tokenEnd_) {
      if (sv_[tokenEnd_] == ' ') {
        break;
      }
    }
  }

  std::string_view token() {
    return sv_.substr(tokenBegin_, tokenEnd_ - tokenBegin_);
  }

  RPNTokenCursor& next() {
    --tokenBegin_;
    if (tokenBegin_ > 0) {
      --tokenBegin_;
      for (; tokenBegin_ > 0; --tokenBegin_) {
        if (sv_[tokenBegin_] == ' ') {
          ++tokenBegin_;
          break;
        }
      }
    }
    tokenEnd_ = tokenBegin_ + 1;
    for (; tokenEnd_ != sv_.size(); ++tokenEnd_) {
      if (sv_[tokenEnd_] == ' ') {
        break;
      }
    }
    return *this;
  }
};

class PNTokenCursor {
  std::string_view sv_;
  std::string_view::size_type tokenBegin_;
  std::string_view::size_type tokenEnd_;

 public:
  PNTokenCursor(std::string_view sv) : sv_{sv}, tokenBegin_{0} {
    tokenEnd_ = tokenBegin_ + 1;
    while (tokenEnd_ < sv_.size() && sv_[tokenEnd_] != ' ') ++tokenEnd_;
  }

  std::string_view token() const {
    return sv_.substr(tokenBegin_, tokenEnd_ - tokenBegin_);
  }

  PNTokenCursor& next() {
    while (tokenBegin_ < sv_.size() && sv_[tokenBegin_] != ' ') ++tokenBegin_;
    ++tokenBegin_;
    tokenEnd_ = tokenBegin_ + 1;
    while (tokenEnd_ < sv_.size() && sv_[tokenEnd_] != ' ') ++tokenEnd_;
    return *this;
  }
};

}  // namespace gpm
