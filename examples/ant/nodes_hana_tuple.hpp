/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include <boost/hana.hpp>

namespace tup {

namespace hana = boost::hana;
using namespace hana::literals;  // contains the _c suffix

namespace tag {
struct Move {};
struct Left {};
struct Right {};
struct IfFood {};
struct Prog2 {};
struct Prog3 {};
}  // namespace tag

template <typename ContextType, typename Child0T, typename Child1T,
          typename Child2T>
void eval(hana::tuple<tag::Prog3, Child0T, Child1T, Child2T> node,
          ContextType& c);

template <typename ContextType, typename Child0T, typename Child1T>
void eval(hana::tuple<tag::Prog2, Child0T, Child1T> node, ContextType& c);

template <typename ContextType, typename TrueBranchT, typename FalseBranchT>
void eval(hana::tuple<tag::IfFood, TrueBranchT, FalseBranchT> node,
          ContextType& c);

template <typename ContextType>
void eval(hana::tuple<tag::Move>, ContextType& c) {
  c.move();
}

template <typename ContextType>
void eval(hana::tuple<tag::Left>, ContextType& c) {
  c.left();
}

template <typename ContextType>
void eval(hana::tuple<tag::Right>, ContextType& c) {
  c.right();
}

template <typename ContextType, typename TrueBranchT, typename FalseBranchT>
void eval(hana::tuple<tag::IfFood, TrueBranchT, FalseBranchT> node,
          ContextType& c) {
  if (c.is_food_in_front())
    eval(node[1_c], c);
  else
    eval(node[2_c], c);
}

template <typename ContextType, typename Child0T, typename Child1T>
void eval(hana::tuple<tag::Prog2, Child0T, Child1T> node, ContextType& c) {
  eval(node[1_c], c);
  eval(node[2_c], c);
}

template <typename ContextType, typename Child0T, typename Child1T,
          typename Child2T>
void eval(hana::tuple<tag::Prog3, Child0T, Child1T, Child2T> node,
          ContextType& c) {
  eval(node[1_c], c);
  eval(node[2_c], c);
  eval(node[3_c], c);
}

}  // namespace tup
