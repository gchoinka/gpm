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
#include <vector>

struct VariantDynamic2 {
  std::vector<std::string> includes() const { return {"common/nodes.hpp"}; }
  std::string name() const { return "variantDynamic2"; }
  std::string functionName() const { return "variantDynamic2"; }
  std::string body(ant::NodesVariant) const {
    return fmt::format(R"""(
template<typename AntBoardSimT, typename CursorType>
static int variantDynamic2(AntBoardSimT antBoardSim, CursorType cursor, BenchmarkPart toMessure)
{{   
  constexpr auto maxHash = 16;
    
  auto anAnt =
    gpm::experimental::FactoryV2<ant::NodesVariant, CursorType, maxHash>::factory(cursor);

  if(toMessure == BenchmarkPart::Create) {{
    benchmark::DoNotOptimize(anAnt);
    return 0;
  }}
  auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{{antBoardSim}};

  while(!antBoardSim.is_finish())
  {{
    boost::apply_visitor(antBoardSimVisitor, anAnt);
    benchmark::DoNotOptimize(antBoardSim.score());
  }}
  return antBoardSim.score(); 
}}
    )""");
  }
};
