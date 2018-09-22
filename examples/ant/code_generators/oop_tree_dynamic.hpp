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

struct OOPTreeDynamic
{
  static std::string name() { return "oopTreeFromString"; }
  static std::string body() {
    return R"""(
template<typename AntBoardSimT>
int oopTreeFromString(AntBoardSimT antBoardSim)
{{                
  auto oopTree = antoop::factory<AntBoardSimT>(gpm::RPNToken_iterator{{antRPNString}});
  
  while(!antBoardSim.is_finish())
  {{
    (*oopTree)(antBoardSim);
  }}
  return antBoardSim.score(); 
}}
)""";
  }
};
