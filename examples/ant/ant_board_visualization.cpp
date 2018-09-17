/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <iostream>
#include <fstream>

#include <cxxopts.hpp>

#include <gpm/gpm.hpp>
#include <gpm/io.hpp>

#include "common/ant_board_simulation.hpp"
#include "common/nodes.hpp"
#include "common/santa_fe_board.hpp"
#include "common/visitor.hpp"

template <typename AntBoardSimT>
class AntBoardSimDecorator {
  AntBoardSimT orgAntBoardSim_;

 public:
  AntBoardSimDecorator(AntBoardSimT& other) : orgAntBoardSim_{other} {}

  void move() {
    orgAntBoardSim_.get_board_as_str([](auto s) { std::cout << s << "\n"; });
    orgAntBoardSim_.move();
    orgAntBoardSim_.get_board_as_str([](auto s) { std::cout << s << "\n"; });
  }

  void left() {
    orgAntBoardSim_.get_board_as_str([](auto s) { std::cout << s << "\n"; });
    orgAntBoardSim_.left();
    orgAntBoardSim_.get_board_as_str([](auto s) { std::cout << s << "\n"; });
  }

  void right() {
    orgAntBoardSim_.get_board_as_str([](auto s) { std::cout << s << "\n"; });
    orgAntBoardSim_.right();
    orgAntBoardSim_.get_board_as_str([](auto s) { std::cout << s << "\n"; });
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

template <typename AntBoardSimT>
AntBoardSimDecorator<AntBoardSimT> makeAntBoardSimDecorator(
    AntBoardSimT toDecorateAntBoardSim) {
  return AntBoardSimDecorator<AntBoardSimT>{toDecorateAntBoardSim};
}



decltype(auto) getAntBoardSim(char const * filename) {
  using namespace ant;
  auto max_steps = 400;
  auto max_food = 89;
  auto antSim =
      sim::AntBoardSimulationStaticSize<santa_fe::x_size, santa_fe::y_size>{
          max_steps, max_food, sim::Pos2d{0, 0}, sim::Direction::east,
          [filename](auto & board) {
            std::ifstream boardFile(filename);
            int x = 0;
            for (std::string line; std::getline(boardFile, line); ) {
              if(line.size() != board[x].size())
                throw std::runtime_error{"line length does not match with the board"};
              for (size_t y = 0; y < board[x].size(); ++y) {
                board[x][y] = line[y] == 'X'
                                  ? sim::BoardState::food
                                  : sim::BoardState::empty;
              }
              ++x;
            }
            if(x != board.size())
              throw std::runtime_error{"not enoth lines int the file."};
          }};
  return antSim;
}

int main(int argc, char * argv[]) {
  cxxopts::Options options("ant_board_visualization", "One line description of MyProgram");
  options.add_options()
    ("b,boarddef", "File name", cxxopts::value<std::string>())
//("f,file", "File name", cxxopts::value<std::string>())
  ;
  
  auto cliArgs = options.parse(argc, argv);
  
  if (!cliArgs.count("boarddef")){
    std::cout << options.help({"", ""}) << std::endl;
    exit(0);
  }
  
  char const* optimalAntRPNdef = "m r m if l l p3 r m if if p2 r p2 m if";
  auto optAnt =
      gpm::factory<ant::ant_nodes>(gpm::RPNToken_iterator{optimalAntRPNdef});
      auto antBoardSim = makeAntBoardSimDecorator(getAntBoardSim(cliArgs["boarddef"].as<std::string>().c_str()));

  auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{antBoardSim};

  while (!antBoardSim.is_finish()) {
    boost::apply_visitor(antBoardSimVisitor, optAnt);
  }
  return antBoardSim.score();
}
