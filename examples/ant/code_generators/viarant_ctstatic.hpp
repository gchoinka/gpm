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

class AsRecursiveVariantNotation : public boost::static_visitor<std::string> {
 public:
  std::string operator()(ant::IfFoodAhead const& node) const {
    return fmt::format(
        R"""(
{nodeName}{{
  {true_branch}
  , {false_branch}
}}
)""",
        "nodeName"_a = boost::typeindex::type_id_runtime(node).pretty_name(),
        "true_branch"_a = boost::apply_visitor(*this, node.get(true)),
        "false_branch"_a = boost::apply_visitor(*this, node.get(false)));
  }

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
        "{nodeName}{{{nodeChildren}}}\n",
        "nodeName"_a = boost::typeindex::type_id_runtime(node).pretty_name(),
        "nodeChildren"_a = res);
    return res;
  }
};

struct VariantCTStatic {
  std::string name() const { return "variantCTStatic"; }
  std::string functionName() const { return "variantCTStatic"; }
  std::string body(ant::NodesVariant ant) const {
    return fmt::format(R"""(
template<typename AntBoardSimT, typename CursorType>
static int variantCTStatic(AntBoardSimT antBoardSim, CursorType, BenchmarkPart toMessure)
{{    
  auto anAnt = ant::NodesVariant{{{recursiveVariantNotation}}};
  if(toMessure == BenchmarkPart::Create) {{
    benchmark::DoNotOptimize(anAnt);
    return 0;
  }}
  auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{{antBoardSim}};
  
  while(!antBoardSim.is_finish())
  {{
    boost::apply_visitor(antBoardSimVisitor, anAnt);
  }}
  benchmark::DoNotOptimize(antBoardSim.score());
  return antBoardSim.score(); 
}}
)""",
                       "recursiveVariantNotation"_a = boost::apply_visitor(
                           AsRecursiveVariantNotation{}, ant)

    );
  }
};
