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

template <typename ContextT, typename Child0T, typename Child1T,
          typename Child2T>
void eval(ContextT& contex,
          hana::tuple<tag::Prog3, Child0T, Child1T, Child2T> node);

template <typename ContextT, typename Child0T, typename Child1T>
void eval(ContextT& contex, hana::tuple<tag::Prog2, Child0T, Child1T> node);

template <typename ContextT, typename TrueTreeT, typename FalseTreeT>
void eval(ContextT& contex,
          hana::tuple<tag::IfFood, TrueTreeT, FalseTreeT> node);

template <typename ContextT>
void eval(ContextT& contex, hana::tuple<tag::Move>) {
  contex.move();
}

template <typename ContextT>
void eval(ContextT& contex, hana::tuple<tag::Left>) {
  contex.left();
}

template <typename ContextT>
void eval(ContextT& contex, hana::tuple<tag::Right>) {
  contex.right();
}

template <typename ContextT, typename TrueTreeT, typename FalseTreeT>
void eval(ContextT& contex,
          hana::tuple<tag::IfFood, TrueTreeT, FalseTreeT> node) {
  if (contex.is_food_in_front())
    eval(contex, node[1_c]);
  else
    eval(contex, node[2_c]);
}

template <typename ContextT, typename Child0T, typename Child1T>
void eval(ContextT& contex, hana::tuple<tag::Prog2, Child0T, Child1T> node) {
  eval(contex, node[1_c]);
  eval(contex, node[2_c]);
}

template <typename ContextT, typename Child0T, typename Child1T,
          typename Child2T>
void eval(ContextT& contex,
          hana::tuple<tag::Prog3, Child0T, Child1T, Child2T> node) {
  eval(contex, node[1_c]);
  eval(contex, node[2_c]);
  eval(contex, node[3_c]);
}

}  // namespace tup
