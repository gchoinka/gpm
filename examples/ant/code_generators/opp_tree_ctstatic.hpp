/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once
#include <fmt/format.h>
#include <boost/variant.hpp>
#include <string>

#include "../common/nodes.hpp"

using namespace fmt::literals;

class AsOOPNotation : public boost::static_visitor<std::string> {
  std::string simulationName_;

 public:
  AsOOPNotation(std::string const& simulationName)
      : simulationName_{simulationName} {}

  std::string operator()(ant::if_food_ahead const& node) const {
    return fmt::format(
        R"""(
        std::make_unique<antoop::IfFoodAhead<decltype({simulationName})>>(
          {true_branch}
          , {false_branch}
    )
    )""",
        "simulationName"_a = simulationName_,
        "true_branch"_a = boost::apply_visitor(*this, node.get(true)),
        "false_branch"_a = boost::apply_visitor(*this, node.get(false)));
  }

  struct AntNodeToClassName {
    static char const* name(ant::move) { return "Move"; }
    static char const* name(ant::left) { return "Left"; }
    static char const* name(ant::right) { return "Right"; }
    static char const* name(ant::prog2) { return "Prog2"; }
    static char const* name(ant::prog3) { return "Prog3"; }
  };

  template <typename NodeT>
  std::string operator()(NodeT node) const {
    std::string res;
    if constexpr (node.children.size() != 0) {
      auto delimi = "\n";
      for (auto& n : node.children) {
        (res += delimi) += boost::apply_visitor(*this, n);
        delimi = ",";
      }
      res += "\n";
    }

    res = fmt::format(
        "std::make_unique<antoop::{nodeName}<AntBoardSimT>>({"
        "nodeChildren})\n",
        "nodeName"_a = AntNodeToClassName::name(node), "nodeChildren"_a = res,
        "simulationName"_a = simulationName_);
    return res;
  }
};

struct OPPTreeCTStatic {
  std::string name() const { return "oppTreeCTStatic"; }
  std::string functionName() const { return "oppTreeCTStatic"; }

  std::string body(ant::ant_nodes ant) const {
    return fmt::format(R"""(
template<typename AntBoardSimT>
int oppTreeCTStatic(AntBoardSimT antBoardSim, std::string_view const &)
{{                
  auto oopTree = {oopNotation};
  
  while(!antBoardSim.is_finish())
  {{
    (*oopTree)(antBoardSim);
  }}
  return antBoardSim.score(); 
}}
)""",
                       "oopNotation"_a = boost::apply_visitor(
                           AsOOPNotation{"antBoardSim"}, ant));
  }
};
