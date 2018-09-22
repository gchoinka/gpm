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

struct VariantDynamic
{
  static std::string name() { return "oopTreeFromString"; }
  static std::string body() {
    return R"""(
template<typename AntBoardSimT>
static int recursiveVariantTreeFromString(AntBoardSimT {antBoardSimName})
{{    
  auto optAnt = gpm::factory<ant::ant_nodes>(gpm::RPNToken_iterator{{antRPNString}});

  auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{{{antBoardSimName}}};

  while(!{antBoardSimName}.is_finish())
  {{
    boost::apply_visitor(antBoardSimVisitor, optAnt);
  }}
  return {antBoardSimName}.score(); 
}}
)""";
  }
};
