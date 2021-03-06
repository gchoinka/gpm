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

struct VariantDynamic {
  std::string fileName() const { return __FILE__; }
  std::vector<std::string> includes() const { return {"common/nodes.hpp"}; }
  std::string name() const { return "variantDynamic"; }
  std::string functionName() const { return "variantDynamic"; }
  std::string body(ant::NodesVariant) const {
    return fmt::format(R"""(
      template<typename AntBoardSimT, typename CursorType>
      static int variantDynamic(AntBoardSimT antBoardSim, CursorType cursor, BenchmarkPart toMessure)
    {{    
    auto anAnt = gpm::factory<ant::NodesVariant>(cursor);
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
