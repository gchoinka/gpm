/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include <fmt/format.h>
#include <boost/type_index.hpp>
#include <boost/variant.hpp>
#include <string>

#include "../common/nodes.hpp"

using namespace fmt::literals;

class AsCPPFixedWithVisitorNotation
    : public boost::static_visitor<std::string> {
 public:
  std::string operator()(ant::IfFoodAhead const& node) const {
    return fmt::format(
        R"""(
if(antBoardSim.is_food_in_front()){{
  {true_branch}
}}else{{
  {false_branch}
}}
)""",
        "true_branch"_a = boost::apply_visitor(*this, node.get(true)),
        "false_branch"_a = boost::apply_visitor(*this, node.get(false)));
  }

  template <typename NodeT>
  std::string operator()(NodeT node) const {
    auto nodeType = boost::typeindex::type_id_runtime(node).pretty_name();
    std::string res;
    if constexpr (node.children.size() == 0) {
      res += fmt::format("antBoardSimVisitor({nodeType}{{}});\n",
                         "nodeType"_a = nodeType);
    }
    if constexpr (node.children.size() != 0)
      for (auto& n : node.children) res += boost::apply_visitor(*this, n);
    return res;
  }
};

struct UnwrappedVisitorCallingCTStatic {
  std::string name() const { return "unwrappedVisitorCallingCTStatic"; }
  std::string functionName() const { return "unwrappedVisitorCallingCTStatic"; }
  std::string body(ant::NodesVariant ant) const {
    return fmt::format(R"""(
template<typename AntBoardSimT>
int unwrappedVisitorCallingCTStatic(AntBoardSimT antBoardSim, std::string_view const &, BenchmarkPart toMessure)
{{                
  if(toMessure == BenchmarkPart::Create) 
      return 0;
  auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{{antBoardSim}};
  while(!antBoardSim.is_finish())
  {{
    {cppFixedWithVisitorNotation}
  }}
  benchmark::DoNotOptimize(antBoardSim.score());
  return antBoardSim.score(); 
}}
)""",
                       "cppFixedWithVisitorNotation"_a = boost::apply_visitor(
                           AsCPPFixedWithVisitorNotation{}, ant));
  }
};
