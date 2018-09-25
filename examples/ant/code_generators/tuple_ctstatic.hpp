/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#pragma once
#include <fmt/format.h>
#include <boost/type_index.hpp>
#include <boost/variant.hpp>
#include <string>

#include "../common/nodes.hpp"

using namespace fmt::literals;

class AsTupletNotation : public boost::static_visitor<std::string> {
 public:
  std::string operator()(ant::if_food_ahead const& node) const {
    return fmt::format(
        R"""(
  hana::tuple<tag::IfFood, {true_branch}, {false_branch}>
)""",
        "true_branch"_a = boost::apply_visitor(*this, node.get(true)),
        "false_branch"_a = boost::apply_visitor(*this, node.get(false)));
  }

  std::string operator()(ant::prog2 const& node) const {
    return fmt::format(
        R"""(
  hana::tuple<tag::Prog2, {branch0}, {branch1}>
)""",
        "branch0"_a = boost::apply_visitor(*this, node.children[0]),
        "branch1"_a = boost::apply_visitor(*this, node.children[1]));
  }

  std::string operator()(ant::prog3 const& node) const {
    return fmt::format(
        R"""(
  hana::tuple<tag::Prog3, {branch0}, {branch1}, {branch2}>
)""",
        "branch0"_a = boost::apply_visitor(*this, node.children[0]),
        "branch1"_a = boost::apply_visitor(*this, node.children[1]),
        "branch2"_a = boost::apply_visitor(*this, node.children[2]));
  }

  std::string operator()(ant::move const&) const {
    return "hana::tuple<tag::Move>";
  }
  std::string operator()(ant::left const&) const {
    return "hana::tuple<tag::Left>";
  }
  std::string operator()(ant::right const&) const {
    return "hana::tuple<tag::Right>";
  }
};

struct TupleCTStatic {
  std::string name() const { return "tupleCTStatic"; }
  std::string functionName() const { return "tupleCTStatic"; }
  std::string body(ant::ant_nodes ant) const {
    return fmt::format(
        R"""(
template<typename AntBoardSimT>
static int tupleCTStatic(AntBoardSimT antBoardSim, std::string_view const &)
{{ 
  using namespace tup;
  constexpr auto optAnt = {tupleNotation}{{}};

  while(!antBoardSim.is_finish())
  {{
    tup::eval(antBoardSim, optAnt);
  }}
  return antBoardSim.score(); 
}}
)""",
        "tupleNotation"_a = boost::apply_visitor(AsTupletNotation{}, ant)

    );
  }
};
