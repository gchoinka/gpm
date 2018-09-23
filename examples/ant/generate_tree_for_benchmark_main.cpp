/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <fstream>
#include <iostream>
#include <memory>
#include <type_traits>

#include <boost/algorithm/string/join.hpp>
#include <boost/hana.hpp>
namespace hana = boost::hana;

#include <outcome.hpp>
namespace outcome = OUTCOME_V2_NAMESPACE;

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>

#include <gpm/io.hpp>
#include <gpm/utils/fmtutils.hpp>

#include "common/ant_board_simulation.hpp"
#include "common/nodes.hpp"
#include "common/santa_fe_board.hpp"
#include "common/visitor.hpp"
#include "nodes_opp.hpp"
#include "nodes_hana_tuple.hpp"

#include "code_generators/viarant_ctstatic.hpp"
#include "code_generators/variant_dynamic.hpp"
#include "code_generators/unwrapped_visitorcalling_ctstatic.hpp"
#include "code_generators/unwrapped_direct_ctstatic.hpp"
#include "code_generators/oop_tree_dynamic.hpp"
#include "code_generators/opp_tree_ctstatic.hpp"
#include "code_generators/tuple_ctstatic.hpp"






decltype(auto) getRandomAnt() {
  int minHeight = 2;
  int maxHeight = 17;
  // std::random_device rd;

  return gpm::BasicGenerator<ant::ant_nodes>{minHeight, maxHeight}();
}

decltype(auto) getOptAnt() {
  char const* optimalAntRPNdef = "m r m if l l p3 r m if if p2 r p2 m if";
  return gpm::factory<ant::ant_nodes>(gpm::RPNToken_iterator{optimalAntRPNdef});
}

decltype(auto) getOptAntFromFile(char const* filename) {
  std::ifstream f(filename);
  std::string str;
  std::getline(f, str);
  return gpm::factory<ant::ant_nodes>(gpm::RPNToken_iterator{str});
}

struct CLIArgs
{
  using ErrorMessage = std::string;
  std::string outfile;
  std::string antrpndef;
};



outcome::unchecked<CLIArgs, CLIArgs::ErrorMessage> handleCLI(int argc, char ** argv)
{
  using namespace fmt::literals;
  namespace po = boost::program_options;
  auto args = CLIArgs{};
  po::options_description desc("Allowed options");
  desc.add_options()
  ("help", "produce help message")
  ("antrpndef", po::value<std::string>(&args.antrpndef)->required(), "")
  ("outfile", po::value<std::string>(&args.outfile)->required(), "")
  ;
  po::variables_map vm;
  try{
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  }
  catch(std::exception const & e){
    if(vm.count("help"))
      return outcome::failure(boost::lexical_cast<std::string>(desc));
    else
      return outcome::failure(e.what());
  }
   
  if (vm.count("help")) {
    return outcome::failure(boost::lexical_cast<std::string>(desc));
  }
  return args;
}

int main(int argc, char** argv) {

  auto cliArgsOutcome = handleCLI(argc, argv);
  if(!cliArgsOutcome)
  {
    std::cerr << cliArgsOutcome.error() << "\n";
    exit(1);
  }
  auto cliArgs = cliArgsOutcome.value();

  auto ant = getRandomAnt();
  
  auto bm = hana::make_tuple(VariantDynamic{}, VariantCTStatic{}, OOPTreeDynamic{}, OPPTreeCTStatic{}, UnwrappedDirectCTStatic{}, UnwrappedVisitorCallingCTStatic{}, TupleCTStatic{});
  
  std::ofstream outf(cliArgs.outfile.c_str());
  
  outf << fmt::format("static char const * getAntRPN() {{ return \"{antRPN}\"; }}\n", "antRPN"_a = boost::apply_visitor(gpm::RPNPrinter<std::string>{}, ant));
  hana::for_each(bm, [&](auto const & codeGenerator){
    outf << codeGenerator.body(ant);
  });
  
  
  
  std::string tupleElements;
  auto delimiter = "\n      ";
  
  hana::for_each(bm, [&](auto const & codeGenerator){
    tupleElements += fmt::format(R"""({Delimiter} std::make_tuple(&{FunctionPointer}<AntBoardSimT>, "{BenchmareName}"))""", "Delimiter"_a = delimiter, "FunctionPointer"_a = codeGenerator.functionName(), "BenchmareName"_a = codeGenerator.name());
    delimiter = "\n    , ";
  });
  
  fmt::print(outf, 
R"""(
template<typename AntBoardSimT>
decltype(auto) getAllTreeBenchmarks()
{{
  return std::make_tuple({tupleElements});
}}
  
)"""
       , "tupleElements"_a = tupleElements
  );
}
