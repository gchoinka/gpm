/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <fstream>
#include <iostream>

#include <boost/program_options.hpp>

#include <gpm/gpm.hpp>
#include <gpm/io.hpp>

#include <fmt/format.h>

#include <outcome.hpp>
namespace outcome = OUTCOME_V2_NAMESPACE;

#include "common/ant_board_simulation.hpp"
#include "common/nodes.hpp"
#include "common/santa_fe_board.hpp"
#include "common/visitor.hpp"

#include "nodes_funcptr.hpp"

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

decltype(auto) getAntBoardSim(char const* filename) {
  using namespace ant;
  auto max_steps = 400;
  auto max_food = 89;
  auto antSim =
      sim::AntBoardSimulationStaticSize<santa_fe::x_size, santa_fe::y_size>{
          max_steps, max_food, sim::Pos2d{0, 0}, sim::Direction::east,
          [filename](auto& board) {
            std::ifstream boardFile(filename);
            size_t x = 0;
            for (std::string line; std::getline(boardFile, line);) {
              if (line.size() != board[x].size())
                throw std::runtime_error{
                    "line length does not match with the board"};
              for (size_t y = 0; y < board[x].size(); ++y) {
                board[x][y] = line[y] == 'X' ? sim::BoardState::food
                                             : sim::BoardState::empty;
              }
              ++x;
            }
            if (x != board.size())
              throw std::runtime_error{"not enoth lines int the file."};
          }};
  return antSim;
}

namespace {

struct CLIArgs {
  using ErrorMessage = std::string;
  std::string boarddef;
  std::string outfile;
  std::string antrpndef;
};

outcome::unchecked<CLIArgs, CLIArgs::ErrorMessage> handleCLI(int argc,
                                                             char** argv) {
  namespace po = boost::program_options;
  auto args = CLIArgs{};
  po::options_description desc("Allowed options");
  desc.add_options()
      // clang-format off
    ("help", "produce help message")
    ("boarddef", po::value<std::string>(&args.boarddef)->required(), "")
    ("outfile", po::value<std::string>(&args.outfile)->required(), "")
    ("antrpndef", po::value<std::string>(&args.antrpndef), "");
  // clang-format on
  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  } catch (std::exception const& e) {
    if (vm.count("help")) {
      return outcome::failure(boost::lexical_cast<std::string>(desc));
    }
    return outcome::failure(e.what());
  }

  return args;
}
}  // namespace

int main(int argc, char* argv[]) {
  auto cliArgsOutcome = handleCLI(argc, argv);
  if (!cliArgsOutcome) {
    std::cerr << cliArgsOutcome.error() << "\n";
    exit(1);
  }
  auto cliArgs = cliArgsOutcome.value();

  char const* optimalAntRPNdef = "m r m if l l p3 r m if if p2 r p2 m if";
  auto optAnt =
      gpm::factory<ant::ant_nodes>(gpm::RPNTokenCursor{optimalAntRPNdef});
  auto antBoardSim =
      makeAntBoardSimDecorator(getAntBoardSim(cliArgs.boarddef.c_str()));

  //   using namespace funcptr;
  //   using ContexType = decltype(antBoardSim);
  //   auto aNode = make_Node<ContexType>("if", [](Node<ContexType> const & n,
  //   ContexType & c){
  //         if(c.is_food_in_front())
  //           n.children_[0](n.children_[0], c);
  //         else
  //           n.children_[1](n.children_[1], c);
  //       },
  //       2
  //   );

  auto funcNode = funcptr::factory<decltype(antBoardSim)>(
      gpm::RPNTokenCursor{optimalAntRPNdef});
  auto funcNod2e = funcptr::factory<decltype(antBoardSim)>(
      gpm::RPNTokenCursor{optimalAntRPNdef});

  // funcNode(funcNode, antBoardSim);

  //   auto fmap = funcptr::getAntNodesMap<decltype(antBoardSim)>();
  //
  //   fmt::print("{}\n", fmap.at("m").name);

  while (!antBoardSim.is_finish()) {
    funcNode(funcNode, antBoardSim);
  }

  //   auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{antBoardSim};
  //
  //   while (!antBoardSim.is_finish()) {
  //     boost::apply_visitor(antBoardSimVisitor, optAnt);
  //   }
  //   return antBoardSim.score();
}
