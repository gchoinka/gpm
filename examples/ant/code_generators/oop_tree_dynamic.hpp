/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include <string>

#include <fmt/format.h>

struct OOPTreeDynamic {
  std::string name() const { return "oopTreeDynamic"; }
  std::string functionName() const { return "oopTreeDynamic"; }

  std::string body(ant::ant_nodes) const {
    return fmt::format(R"""(
template<typename AntBoardSimT>
static int oopTreeDynamic(AntBoardSimT antBoardSim, std::string_view const & sv, BenchmarkPart toMessure)
{{                
  auto oopTree = antoop::factory<AntBoardSimT>(gpm::RPNTokenCursor{{sv}});
  if(toMessure == BenchmarkPart::Create) 
    return 0;
  while(!antBoardSim.is_finish())
  {{
    (*oopTree)(antBoardSim);
  }}
  return antBoardSim.score(); 
}}
)""");
  }
};
