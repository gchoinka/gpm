/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <variant>
#include <vector>

#include <boost/hana.hpp>
#include <boost/hana/ext/std/tuple.hpp>
#include <boost/program_options.hpp>
#include <boost/type_index.hpp>

#include <benchmark/benchmark.h>

#include <outcome.hpp>
namespace outcome = OUTCOME_V2_NAMESPACE;

#include <fmt/format.h>
#include <gpm/gpm.hpp>
#include <gpm/io.hpp>

#include "common/ant_board_simulation.hpp"
#include "common/nodes.hpp"
#include "common/santa_fe_board.hpp"
#include "common/visitor.hpp"

decltype(auto) getAntSataFeStaticBoardSim() {
  using namespace ant;
  auto max_steps = 400;
  auto max_food = 89;
  auto antSim =
      sim::AntBoardSimulationStaticSize<santa_fe::x_size, santa_fe::y_size>{
          max_steps, max_food, sim::Pos2d{0, 0}, sim::Direction::east,
          [](sim::AntBoardSimulationStaticSize<
              santa_fe::x_size, santa_fe::y_size>::FieldType& board) {
            for (size_t x = 0; x < board.size(); ++x) {
              for (size_t y = 0; y < board[x].size(); ++y) {
                board[x][y] = santa_fe::board[x][y] == 'X'
                                  ? sim::BoardState::food
                                  : sim::BoardState::empty;
              }
            }
          }};

  return antSim;
}

decltype(auto) getAntRandomBoardSim() {
  using namespace ant;
  auto max_steps = 400;
  auto max_food = 89;

  static auto rndSeed = std::random_device{}();
  auto rnd = std::mt19937{rndSeed};
  auto intdist = std::uniform_int_distribution<>{0, 10};
  auto antSim =
      sim::AntBoardSimulationStaticSize<santa_fe::x_size, santa_fe::y_size>{
          max_steps, max_food, sim::Pos2d{0, 0}, sim::Direction::east,
          [&](sim::AntBoardSimulationStaticSize<
              santa_fe::x_size, santa_fe::y_size>::FieldType& board) {
            for (size_t x = 0; x < board.size(); ++x) {
              for (size_t y = 0; y < board[x].size(); ++y) {
                board[x][y] = intdist(rnd) == 0 ? sim::BoardState::food
                                                : sim::BoardState::empty;
              }
            }
          }};

  return antSim;
}

outcome::unchecked<ant::sim::AntBoardSimulationStaticSize<
                       ant::santa_fe::x_size, ant::santa_fe::y_size>,
                   std::string>
getAntBoardSimFromFileName(char const* filename) {
  using namespace ant;
  std::string errorMessage;

  auto boardInitFunction = [filename, &errorMessage](auto& board) mutable {
    std::ifstream boardFile(filename);
    if (!boardFile.good()) {
      (errorMessage += "Could not open board file: ") += filename;
      return false;
    }
    size_t x = 0;
    for (std::string line; std::getline(boardFile, line);) {
      if (line.size() != board[x].size()) {
        errorMessage = "line length does not match with the board";
        return false;
      }
      for (size_t y = 0; y < board[x].size(); ++y) {
        board[x][y] =
            line[y] == 'X' ? sim::BoardState::food : sim::BoardState::empty;
      }
      ++x;
    }
    if (x != board.size()) {
      errorMessage = "not enough lines in the file.";
      return false;
    }
    return true;
  };

  auto max_steps = 400;
  auto max_food = 89;
  auto antBoardSim =
      sim::AntBoardSimulationStaticSize<santa_fe::x_size, santa_fe::y_size>{
          max_steps, max_food, sim::Pos2d{0, 0}, sim::Direction::east,
          boardInitFunction};

  if (!errorMessage.empty()) return outcome::failure(errorMessage);

  return antBoardSim;
}

enum class BenchmarkPart { Full, Create };

#if __has_include("ant_simulation_benchmark_generated_functions.cpp")
#include "ant_simulation_benchmark_generated_functions.cpp"
#else
#pragma message \
    "run artificial_ant_generate and copy ant_simulation_benchmark_generated_functions.cpp to the same folder, touch this file and then rerun this target"

template <typename AntBoardSimT, typename CursorType>
decltype(auto) getAllTreeBenchmarks() {
  return std::make_tuple();
}

[[gnu::unused]] static char const* getAntPN() { return ""; }
#endif

namespace {

struct CLIArgs {
  using ErrorMessage = std::string;
  std::string boarddef;
};

outcome::unchecked<CLIArgs, CLIArgs::ErrorMessage> handleCLI(int argc,
                                                             char** argv) {
  namespace po = boost::program_options;
  auto args = CLIArgs{};
  po::options_description simOptions("Simulation options");
  simOptions.add_options()
      // clang-format off
  ("help,h", "produce help message")
  ("boarddef,b", po::value<std::string>(&args.boarddef), "")
  ;
  // clang-format on

  //   po::options_description bmOptions("Benchmark Settings");
  //   bmOptions.add_options()
  //   // clang-format off
  //   ;
  //   // clang-format on

  po::options_description allOptions("Allowed options");
  allOptions.add(simOptions);

  po::parsed_options parsed = po::command_line_parser(argc, argv)
                                  .options(allOptions)
                                  .allow_unregistered()
                                  .run();

  po::variables_map vm;
  po::store(parsed, vm);
  po::notify(vm);
  return args;
}
}  // namespace

int main(int argc, char** argv) {
  namespace hana = boost::hana;
  auto cliArgsOutcome = handleCLI(argc, argv);

  if (!cliArgsOutcome) {
    std::cerr << cliArgsOutcome.error() << "\n";
    exit(1);
  }
  auto cliArgs = cliArgsOutcome.value();

  auto const resultAntBoardSimOutcome =
      getAntBoardSimFromFileName(cliArgs.boarddef.c_str());
  if (!resultAntBoardSimOutcome) {
    std::cerr << resultAntBoardSimOutcome.error() << "\n";
    exit(1);
  }
  auto theAntBoardSim = resultAntBoardSimOutcome.value();

  using CursorType = gpm::PNTokenCursor;
  auto getAntString = getAntPN;

  auto allTreeBechmarks =
      getAllTreeBenchmarks<decltype(theAntBoardSim), CursorType>();

  boost::hana::for_each(
      allTreeBechmarks,
      [theAntBoardSim, cliArgs, getAntString](auto& treeBenchmarkTuple) {
        auto nameFull = std::string{std::get<1>(treeBenchmarkTuple)} + "Full";
        auto toCall = std::get<0>(treeBenchmarkTuple);
        auto BM_lambdaFull = [toCall, theAntBoardSim,
                              getAntString](benchmark::State& state) {
          auto theAntBoardSimCopy = theAntBoardSim;
          for (auto _ : state)
            state.counters["score"] =
                toCall(theAntBoardSimCopy, CursorType{getAntString()},
                       BenchmarkPart::Full);
        };
        benchmark::RegisterBenchmark(nameFull.c_str(), BM_lambdaFull);

        auto nameCreateOnly =
            std::string{std::get<1>(treeBenchmarkTuple)} + "CreateOnly";
        auto BM_lambdaCreateOnly = [toCall, theAntBoardSim,
                                    getAntString](benchmark::State& state) {
          auto theAntBoardSimCopy = theAntBoardSim;
          for (auto _ : state)
            state.counters["score"] =
                toCall(theAntBoardSimCopy, CursorType{getAntString()},
                       BenchmarkPart::Create);
        };
        benchmark::RegisterBenchmark(nameCreateOnly.c_str(),
                                     BM_lambdaCreateOnly);
      });

  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
}
