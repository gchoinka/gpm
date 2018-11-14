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

struct FuncPtrDynamic {
  std::string name() const { return "funcPtrDynamic"; }
  std::string functionName() const { return "funcPtrDynamic"; }
  std::string body(ant::ant_nodes) const {
    return fmt::format(R"""(
template<typename AntBoardSimT>
static int funcPtrDynamic(AntBoardSimT antBoardSim, std::string_view const & sv, BenchmarkPart toMessure)
{{    
  auto anAnt = funcptr::factory<AntBoardSimT>(gpm::RPNTokenCursor{{sv}});
  if(toMessure == BenchmarkPart::Create) 
    return anAnt.children.size();

  while(!antBoardSim.is_finish())
  {{
    anAnt(anAnt, antBoardSim);
  }}
  return antBoardSim.score(); 
}}
    )""");
  }
};
