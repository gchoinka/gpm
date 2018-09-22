/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include <string>
#include <boost/variant.hpp>
#include <boost/type_index.hpp>
#include <fmt/format.h>

#include "../common/nodes.hpp"

using namespace fmt::literals;

struct UnwrappedVisitorCallingCTStatic
{
  static std::string name() { return "oopTreeFromString"; }
  static std::string body() {
    return R"""(
template<typename AntBoardSimT>
int cppFixedWithVisitor(AntBoardSimT antBoardSim)
{{                
  auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{{antBoardSim}};

  while(!antBoardSim.is_finish())
  {{
    {cppFixedWithVisitorNotation}
  }}
  return antBoardSim.score(); 
}}
)""";
  }
};

class AsCPPFixedWithVisitorNotation
: public boost::static_visitor<std::string> {
  std::string simulationName_;
  std::string visitorName_;
  
public:
  AsCPPFixedWithVisitorNotation(std::string const& simulationName,
                                std::string const& visitorName)
  : simulationName_{simulationName}, visitorName_{visitorName} {}
  
  std::string operator()(ant::if_food_ahead const& node) const {
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
  }
  
  template <typename NodeT>
  std::string operator()(NodeT node) const {
    auto nodeType = boost::typeindex::type_id_runtime(node).pretty_name();
    std::string res;
    if constexpr (node.children.size() == 0) {
      res +=
      fmt::format("{visitorName}({nodeType}{{}});\n",
                  "visitorName"_a = visitorName_, "nodeType"_a = nodeType);
    }
    if constexpr (node.children.size() != 0)
      for (auto& n : node.children) res += boost::apply_visitor(*this, n);
      return res;
  }
};
