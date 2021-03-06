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
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <boost/hana.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
namespace hana = boost::hana;

#include <outcome.hpp>
namespace outcome = OUTCOME_V2_NAMESPACE;

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <gpm/io.hpp>
#include <gpm/utils/fmtutils.hpp>

#include "common/ant_board_simulation.hpp"
#include "common/nodes.hpp"
#include "common/santa_fe_board.hpp"
#include "common/visitor.hpp"
#include "nodes_funcptr.hpp"
#include "nodes_hana_tuple.hpp"
#include "nodes_opp.hpp"

#include "code_generators/funcptr_dynamic.hpp"
#include "code_generators/implicit_tree_dynamic.hpp"
#include "code_generators/oop_tree_dynamic.hpp"
#include "code_generators/tuple_ctstatic.hpp"
#include "code_generators/variant_dynamic.hpp"

namespace {

// decltype(auto) getRandomAnt() {
//   int minHeight = 2;
//   int maxHeight = 17;
//   // std::random_device rd;
//
//   return gpm::BasicGenerator<ant::NodesVariant>{minHeight, maxHeight}();
// }
//
// decltype(auto) getOptAnt() {
//   char const* optimalAntRPNdef = "m r m if l l p3 r m if if p2 r p2 m if";
//   return
//   gpm::factory<ant::NodesVariant>(gpm::RPNTokenCursor{optimalAntRPNdef});
// }

decltype(auto) getAntFromFile(char const* filename) {
  std::ifstream f(filename);
  std::string str;
  std::getline(f, str);
  return gpm::factory<ant::NodesVariant>(gpm::RPNTokenCursor{str});
}

struct CLIArgs {
  using ErrorMessage = std::string;
  std::string outfile;
  std::string antrpndef;
  std::vector<std::string> benchmark;
  bool listBenchmarks = false;
};

outcome::unchecked<CLIArgs, CLIArgs::ErrorMessage> handleCLI(int argc,
                                                             char** argv) {
  using namespace fmt::literals;
  namespace po = boost::program_options;
  auto args = CLIArgs{};
  po::options_description simOptions("Simulation Settings");
  simOptions.add_options()
      // clang-format off
    ("help,h", "produce help message")
    ("antrpndef,a", po::value<std::string>(&args.antrpndef), "")
    ("outfile,o", po::value<std::string>(&args.outfile), "")
    ;
  // clang-format on
  po::options_description bmOptions("Benchmark Settings");
  bmOptions.add_options()
      // clang-format off
    ("benchmark",po::value<std::vector<std::string>>(&args.benchmark)->multitoken(),"")
    ("list-benchmarks", po::bool_switch(&args.listBenchmarks), "")
    ;
  // clang-format on

  po::options_description allOptions("Allowed options");
  allOptions.add(simOptions).add(bmOptions);
  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, allOptions), vm);
    po::notify(vm);
  } catch (std::exception const& e) {
    if (vm.count("help")) {
      return outcome::failure(boost::lexical_cast<std::string>(allOptions));
    } else if (args.listBenchmarks) {
      return args;
    }
    return outcome::failure(e.what());
  }

  return args;
}
}  // namespace

int main(int argc, char** argv) {
  auto cliArgsOutcome = handleCLI(argc, argv);
  if (!cliArgsOutcome) {
    std::cerr << cliArgsOutcome.error() << "\n";
    exit(1);
  }
  auto cliArgs = cliArgsOutcome.value();

  auto bm =
      hana::make_tuple(VariantDynamic{}, OOPTreeDynamic{}, TupleCTStatic{},
                       ImplicitTreeDynamic{}, FuncPtrDynamic{});

  if (cliArgs.listBenchmarks) {
    std::cout << "\n";
    hana::for_each(bm, [&](auto const& codeGenerator) {
      std::cout << codeGenerator.name() << "\n";
    });

    exit(0);
  }

  auto ant = getAntFromFile(cliArgs.antrpndef.c_str());
  std::ofstream outf(cliArgs.outfile.c_str());

  outf << fmt::format(
      "static inline char const * getAntPN() {{ return \"{antPN}\"; }}\n",
      "antPN"_a = boost::apply_visitor(gpm::PNPrinter<std::string>{}, ant));

  outf << fmt::format(
      "static inline char const * getAntRPN() {{ return \"{antRPN}\"; }}\n",
      "antRPN"_a = boost::apply_visitor(gpm::RPNPrinter<std::string>{}, ant));

  hana::for_each(bm, [&](auto const& codeGenerator) {
    auto found = std::any_of(cliArgs.benchmark.begin(), cliArgs.benchmark.end(),
                             [&](auto const& bm) {
                               return bm == "all" || bm == codeGenerator.name();
                             });
    if (found) {
      for (auto inc : codeGenerator.includes())
        outf << fmt::format("#include \"{}\"\n", inc);
    }
  });
  hana::for_each(bm, [&](auto const& codeGenerator) {
    auto found = std::any_of(cliArgs.benchmark.begin(), cliArgs.benchmark.end(),
                             [&](auto const& bm) {
                               return bm == "all" || bm == codeGenerator.name();
                             });
    if (found) {
      outf << codeGenerator.body(ant);
    }
  });

  std::string tupleElements;
  auto delimiter = "\n      ";

  hana::for_each(bm, [&](auto const& codeGenerator) {
    auto found = std::any_of(cliArgs.benchmark.begin(), cliArgs.benchmark.end(),
                             [&](auto const& bm) {
                               return bm == "all" || bm == codeGenerator.name();
                             });
    if (!found) return;
    tupleElements += fmt::format(
        R"""({Delimiter} std::make_tuple(&{FunctionPointer}<AntBoardSimT, CursorType>, "{BenchmareName}"))""",
        "Delimiter"_a = delimiter,
        "FunctionPointer"_a = codeGenerator.functionName(),
        "BenchmareName"_a = codeGenerator.name());
    delimiter = "\n    , ";
  });

  fmt::print(outf,
             R"""(
template<typename AntBoardSimT, typename CursorType>
decltype(auto) getAllTreeBenchmarks()
{{
  return std::make_tuple({tupleElements});
}}
  
)""",
             "tupleElements"_a = tupleElements);
}
