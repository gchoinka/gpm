/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include <string>
#include <type_traits>

#include <fmt/format.h>
#include <boost/variant.hpp>

#include "../common/nodes.hpp"

using namespace fmt::literals;

class AsCPPFixedNotation : public boost::static_visitor<std::string> {
  std::string simulationName_;

 public:
  AsCPPFixedNotation(std::string const& simulationName)
      : simulationName_{simulationName} {}

  struct AntNodeToSimulationMethodName {
    static char const* name(ant::move) { return "move"; }
    static char const* name(ant::left) { return "left"; }
    static char const* name(ant::right) { return "right"; }
    template <typename NodeT>
    static char const* name(NodeT) {
      return "";
    }
  };

  template <typename NodeT>
  std::string operator()(NodeT node) const {
    std::string res;
    if constexpr (std::is_same_v<ant::if_food_ahead, NodeT>) {
      return fmt::format(
          R"""(
if({simulationName}.is_food_in_front()){{
  {true_branch}
}}else{{
  {false_branch}
}}
)""",
          "simulationName"_a = simulationName_,
          "true_branch"_a = boost::apply_visitor(*this, node.get(true)),
          "false_branch"_a = boost::apply_visitor(*this, node.get(false)));
    } else if constexpr (node.children.size() == 0) {
      res += fmt::format(
          "{simulationName}.{methodName}();\n",
          "simulationName"_a = simulationName_,
          "methodName"_a = AntNodeToSimulationMethodName::name(node));
    } else if constexpr (node.children.size() != 0) {
      for (auto& n : node.children) res += boost::apply_visitor(*this, n);
    }
    return res;
  }
};

struct UnwrappedDirectCTStatic {
  std::string name() const { return "unwrappedDirectCTStatic"; }
  std::string functionName() const { return "unwrappedDirectCTStatic"; }
  std::string body(ant::ant_nodes ant) const {
    return fmt::format(R"""(
template<typename AntBoardSimT>
int unwrappedDirectCTStatic(AntBoardSimT antBoardSim, std::string_view const &, BenchmarkPart toMessure)
{{
if(toMessure == BenchmarkPart::Create) 
    return 0;
  while(!antBoardSim.is_finish()){{
    {cppFixedNotation}
  }}
  return antBoardSim.score();
}}
)""",
                       "cppFixedNotation"_a = boost::apply_visitor(
                           AsCPPFixedNotation{"antBoardSim"}, ant)

    );
  }
};
