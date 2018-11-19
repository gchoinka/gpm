/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once
#include <fmt/format.h>
#include <string>

#include <boost/variant.hpp>

#include "../common/nodes.hpp"

using namespace fmt::literals;

class AsDynoNotation : public boost::static_visitor<std::string> {
 public:
  std::string operator()(ant::IfFoodAhead const& node) const {
    return fmt::format(
        R"""(
antdyno::IfFood<AntBoardSimT>(
  {true_branch}
  , {false_branch}
)
)""",
        "true_branch"_a = boost::apply_visitor(*this, node.get(true)),
        "false_branch"_a = boost::apply_visitor(*this, node.get(false)));
  }

  struct AntNodeToClassName {
    static char const* name(ant::Move) { return "Move"; }
    static char const* name(ant::Left) { return "Left"; }
    static char const* name(ant::Right) { return "Right"; }
    static char const* name(ant::Prog2) { return "Prog2"; }
    static char const* name(ant::Prog3) { return "Prog3"; }
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

    res = fmt::format("antdyno::{nodeName}<AntBoardSimT>({nodeChildren})\n",
                      "nodeName"_a = AntNodeToClassName::name(node),
                      "nodeChildren"_a = res);
    return res;
  }
};

struct DynoTreeCTStatic {
  std::string name() const { return "dynoTreeCTStatic"; }
  std::string functionName() const { return "dynoTreeCTStatic"; }

  std::string body(ant::NodesVariant ant) const {
    return fmt::format(
        R"""(
template<typename AntBoardSimT>
int dynoTreeCTStatic(AntBoardSimT antBoardSim, std::string_view const &, BenchmarkPart toMessure)
{{                
  auto dynoTree = {dynoNotation};
  if(toMessure == BenchmarkPart::Create) {{
    benchmark::DoNotOptimize(dynoTree);
    return 0;
  }}
  while(!antBoardSim.is_finish())
  {{
    dynoTree.eval(antBoardSim);
  }}
  benchmark::DoNotOptimize(antBoardSim.score());
  return antBoardSim.score(); 
}}
)""",
        "dynoNotation"_a = boost::apply_visitor(AsDynoNotation{}, ant));
  }
};
