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

struct DynoTreeDynamic {
  std::string name() const { return "dynoTreeDynamic"; }
  std::string functionName() const { return "dynoTreeDynamic"; }

  std::string body(ant::ant_nodes) const {
    return fmt::format(R"""(
template<typename AntBoardSimT>
static int dynoTreeDynamic(AntBoardSimT antBoardSim, std::string_view const & sv, BenchmarkPart toMessure)
{{                
  auto dynoTree = antdyno::factory<AntBoardSimT>(gpm::RPNToken_iterator{{sv}});
  if(toMessure == BenchmarkPart::Create) 
    return 0;
  while(!antBoardSim.is_finish())
  {{
    dynoTree.eval(antBoardSim);
  }}
  return antBoardSim.score(); 
}}
)""");
  }
};
