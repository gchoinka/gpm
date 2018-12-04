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

struct FuncPtrDynamic {
  std::vector<std::string> includes() const { return {"nodes_funcptr.hpp"}; }
  std::string name() const { return "funcPtrDynamic"; }
  std::string functionName() const { return "funcPtrDynamic"; }
  std::string body(ant::NodesVariant) const {
    return fmt::format(R"""(
template<typename AntBoardSimT, typename CursorType>
static int funcPtrDynamic(AntBoardSimT antBoardSim, CursorType cursor, BenchmarkPart toMessure)
{{    
  auto anAnt = funcptr::factory<AntBoardSimT>(cursor);
  if(toMessure == BenchmarkPart::Create) {{
    benchmark::DoNotOptimize(anAnt);
    return 0;
  }}

  while(!antBoardSim.is_finish())
  {{
    anAnt(anAnt, antBoardSim);
  }}
  benchmark::DoNotOptimize(antBoardSim.score());
  return antBoardSim.score(); 
}}
    )""");
  }
};
