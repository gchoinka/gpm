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
  std::string name() const { return "variantDynamic"; }
  std::string functionName() const { return "variantDynamic"; }
  std::string body(ant::ant_nodes) const {
    return fmt::format(R"""(
template<typename AntBoardSimT>
static int variantDynamic(AntBoardSimT antBoardSim, std::string_view const & sv, BenchmarkPart toMessure)
{{    
  auto optAnt = gpm::factory<ant::ant_nodes>(gpm::RPNToken_iterator{{sv}});
  if(toMessure == BenchmarkPart::Create) 
    return 0;
  auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{{antBoardSim}};

  while(!antBoardSim.is_finish())
  {{
    boost::apply_visitor(antBoardSimVisitor, optAnt);
  }}
  return antBoardSim.score(); 
}}
)""");
  }
};
