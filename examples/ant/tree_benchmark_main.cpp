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
#include <boost/type_index.hpp>

#include <benchmark/benchmark.h>

#include <cxxopts.hpp>

#include <gpm/gpm.hpp>
#include <gpm/io.hpp>

#include "common/ant_board_simulation.hpp"
#include "common/nodes.hpp"
#include "common/santa_fe_board.hpp"
#include "common/visitor.hpp"

#include "nodes_opp.hpp"

template <typename AntBoardSimT>
class AntBoardSimDecorator {
  AntBoardSimT orgAntBoardSim_;

 public:
  AntBoardSimDecorator(AntBoardSimT& other) : orgAntBoardSim_{other} {}

  template <typename F>
  struct BefreAndAfterTask {
    F f_;
    BefreAndAfterTask(F f) : f_{f} { f_(); }
    ~BefreAndAfterTask() { f_(); }
  };

  void move() {
    BefreAndAfterTask b{[this]() {
      orgAntBoardSim_.get_board_as_str([](auto s) { std::cout << s << "\n"; });
    }};
    orgAntBoardSim_.move();
  }

  void left() {
    BefreAndAfterTask b{[this]() {
      orgAntBoardSim_.get_board_as_str([](auto s) { std::cout << s << "\n"; });
    }};
    orgAntBoardSim_.left();
  }

  void right() {
    BefreAndAfterTask b{[this]() {
      orgAntBoardSim_.get_board_as_str([](auto s) { std::cout << s << "\n"; });
    }};
    orgAntBoardSim_.right();
  }

  bool is_food_in_front() const { return orgAntBoardSim_.is_food_in_front(); }

  bool is_finish() const { return orgAntBoardSim_.is_finish(); }

  int score() const { return orgAntBoardSim_.score(); }

  std::string get_status_line() const {
    return orgAntBoardSim_.get_status_line();
  }

  template <typename LineSinkF>
  void get_board_as_str(LineSinkF lineSink) const {
    orgAntBoardSim_.get_board_as_str(lineSink);
  }

  auto xSize() const { return orgAntBoardSim_.xSize(); }

  auto ySize() const { return orgAntBoardSim_.ySize(); }
};

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

decltype(auto) getAntBoardSimFromFileName(char const* filename) {
  using namespace ant;
  std::string errorMessage;

  auto boardInitFunction = [filename, &errorMessage](auto& board) mutable {
    std::ifstream boardFile(filename);
    if (!boardFile.good()) {
      (errorMessage += "Could not open the file: ") += filename;
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
      errorMessage = "not enoth lines int the file.";
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

  boost::variant<std::string, decltype(antBoardSim)> result{antBoardSim};
  if (!errorMessage.empty()) result = errorMessage;

  return result;
}

#if __has_include("ant_simulation_benchmark_generated_functions.cpp")
#include "ant_simulation_benchmark_generated_functions.cpp"
#else
#pragma message \
    "run artificial_ant_generate and copy ant_simulation_benchmark_generated_functions.cpp to the same folder, touch this file and then rerun this target"

template <typename AntBoardSimT>
decltype(auto) getAllTreeBenchmarks() {
  return std::make_tuple();
}
#endif

struct RegisterIfPosible : public boost::static_visitor<void> {
  void operator()(std::string errorMessage) const {
    std::cout << errorMessage << "\n";
  }

  template <typename T>
  void operator()(T theAntBoardSim) const {
    auto allTreeBechmarks = getAllTreeBenchmarks<decltype(theAntBoardSim)>();

    boost::hana::for_each(
        allTreeBechmarks, [theAntBoardSim](auto& treeBenchmarkFunktion) {
          auto BM_lambda = [treeBenchmarkFunktion,
                            theAntBoardSim](benchmark::State& state) {
            auto theAntBoardSimCopy = theAntBoardSim;
            for (auto _ : state)
              state.counters["score"] =
                  std::get<0>(treeBenchmarkFunktion)(theAntBoardSimCopy);
          };
          benchmark::RegisterBenchmark(std::get<1>(treeBenchmarkFunktion),
                                       BM_lambda);
        });
  }
};

int main(int argc, char** argv) {
  namespace hana = boost::hana;

  cxxopts::Options options("tree_benchmark", "");
  options.allow_unrecognised_options().add_options()(
      "b,boarddef", "File name", cxxopts::value<std::string>());

  auto cliArgs = options.parse(argc, argv);

  if (!cliArgs.count("boarddef")) {
    std::cout << options.help({"", ""}) << std::endl;
    exit(0);
  }

  auto filename = cliArgs["boarddef"].as<std::string>();

  auto const resultAntBoardSim = getAntBoardSimFromFileName(filename.c_str());

  boost::apply_visitor(RegisterIfPosible{}, resultAntBoardSim);

  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
}
