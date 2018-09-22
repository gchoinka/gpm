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

#include <outcome.hpp>
namespace outcome = OUTCOME_V2_NAMESPACE;

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <cxxopts.hpp>

#include <gpm/io.hpp>
#include <gpm/utils/fmtutils.hpp>

#include "common/ant_board_simulation.hpp"
#include "common/nodes.hpp"
#include "common/santa_fe_board.hpp"
#include "common/visitor.hpp"
#include "nodes_opp.hpp"

#include "code_generators/viarant_ctstatic.hpp"
#include "code_generators/variant_dynamic.hpp"
#include "code_generators/unwrapped_visitorcalling_ctstatic.hpp"
#include "code_generators/unwrapped_direct_ctstatic.hpp"
#include "code_generators/oop_tree_dynamic.hpp"
#include "code_generators/opp_tree_ctstatic.hpp"

using namespace fmt::literals;



decltype(auto) getRandomAnt() {
  int minHeight = 1;
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


enum class HandleCLIErrrc
{
  Success     = 0, // 0 should not represent an error
  MissingArg  = 1, 
};



template<typename ErrorMessageSinkT>
outcome::checked<std::tuple<cxxopts::ParseResult, cxxopts::Options>, HandleCLIErrrc> handleCLI(int argc, char ** argv, ErrorMessageSinkT errorMessageSink)
{
  cxxopts::Options options("generate_tree_for_benchmark", "");
  options.allow_unrecognised_options().add_options()
  // clang-format off
  ("a,antrpndef", "File name", cxxopts::value<std::string>())
  ("o,outfile", "File name", cxxopts::value<std::string>());
  // clang-format on  
  auto cliArgs = options.parse(argc, argv);
  
  if (!cliArgs.count("antrpndef") || !cliArgs.count("outfile")) {
    errorMessageSink(options.help({"", ""}));
    return outcome::failure(HandleCLIErrrc::MissingArg);
  }
  return std::make_tuple(cliArgs, options);
}

int main(int argc, char** argv) {

  auto cliArgsOutcome = handleCLI(argc, argv, [](auto const & s){ std::cerr << s << "\n"; } );
  if(!cliArgsOutcome)
    exit(1);
  auto cliArgs = std::get<cxxopts::ParseResult>(cliArgsOutcome.value());
    
  
  auto filename = cliArgs["antrpndef"].as<std::string>();
  auto ant = getOptAntFromFile(filename.c_str());
  
  
  // auto ant = getRandomAnt();

//   auto antBoardSimName = "antBoardSim";
//   auto antBoardSimVisitorName = "antBoardSimVisitor";
// 
//   auto recursiveVariantNotation =
//       boost::apply_visitor(AsRecursiveVariantNotation{}, ant);
//   auto cppFixedNotation =
//       boost::apply_visitor(AsCPPFixedNotation{antBoardSimName}, ant);
//   auto cppFixedWithVisitorNotation = boost::apply_visitor(
//       AsCPPFixedWithVisitorNotation{antBoardSimName, antBoardSimVisitorName},
//       ant);
//   auto oopNotation = boost::apply_visitor(AsOOPNotation{antBoardSimName}, ant);
// 
//   auto antRPN = boost::apply_visitor(gpm::RPNPrinter<std::string>{}, ant);
// 
//   auto outFileName = cliArgs["outfile"].as<std::string>();
//   std::ofstream outf(outFileName.c_str());
//   outf << fmt::format(
//       R"""(
// static char const antRPNString[] = {{"{antRPN}"}};    
// 
// 
// template<typename AntBoardSimT>
// decltype(auto) getAllTreeBenchmarks()
// {{
//   return std::make_tuple(
//       std::make_tuple(&recursiveVariantTreeFromString<AntBoardSimT>, "recursiveVariantTreeFromString")
//     , std::make_tuple(&recursiveVariantTree<AntBoardSimT>, "recursiveVariantTree")
//     , std::make_tuple(&cppFixedTree<AntBoardSimT>, "cppFixedTree")
//     , std::make_tuple(&cppFixedWithVisitor<AntBoardSimT>, "cppFixedWithVisitor")
//     , std::make_tuple(&oopTree<AntBoardSimT>, "oopTree")
//     , std::make_tuple(&oopTreeFromString<AntBoardSimT>, "oopTreeFromString")
//   );
// }}
// 
// )""",
//       "antRPN"_a = antRPN, "antBoardSimName"_a = antBoardSimName,
//       "recursiveVariantNotation"_a = recursiveVariantNotation,
//       "cppFixedNotation"_a = cppFixedNotation,
//       "cppFixedWithVisitorNotation"_a = cppFixedWithVisitorNotation,
//       "oopNotation"_a = oopNotation
//               );
}
