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

struct OOPTreeDynamic {
  std::string fileName() const { return __FILE__; }
  std::vector<std::string> includes() const { return {"nodes_opp.hpp"}; }
  std::string name() const { return "oopTreeDynamic"; }
  std::string functionName() const { return "oopTreeDynamic"; }

  std::string body(ant::NodesVariant) const {
    return fmt::format(R"""(
template<typename AntBoardSimT, typename CursorType>
static int oopTreeDynamic(AntBoardSimT antBoardSim, CursorType cursor, BenchmarkPart toMessure)
{{                
  auto oopTree = antoop::factory<AntBoardSimT>(cursor);
  if(toMessure == BenchmarkPart::Create) {{
    benchmark::DoNotOptimize(oopTree);
    return 0;
  }}
  while(!antBoardSim.is_finish())
  {{
    (*oopTree)(antBoardSim);
  }}
  benchmark::DoNotOptimize(antBoardSim.score());
  return antBoardSim.score(); 
}}
)""");
  }
};
