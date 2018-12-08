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

struct FuncPtr2Dynamic {
  std::string fileName() const { return __FILE__; }
  std::vector<std::string> includes() const { return {"nodes_funcptr2.hpp"}; }
  std::string name() const { return "funcPtr2Dynamic"; }
  std::string functionName() const { return "funcPtr2Dynamic"; }
  std::string body(ant::NodesVariant) const {
    return fmt::format(R"""(
template<typename AntBoardSimT, typename CursorType>
static int funcPtr2Dynamic(AntBoardSimT antBoardSim, CursorType cursor, BenchmarkPart toMessure)
{{    
auto anAnt = funcptr2::factory<AntBoardSimT, funcptr2::GetAntNodes<AntBoardSimT>>(cursor);
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
