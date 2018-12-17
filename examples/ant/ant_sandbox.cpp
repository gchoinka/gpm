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

#include <gpm/tree_utils.hpp>
#include "nodes_funcptr.hpp"
#include "nodes_implicit_tree.hpp"

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

namespace ipt {

template <typename ContexType>
struct Node;

template <typename ContexType>
using NodeVector = boost::container::vector<Node<ContexType>>;

template <typename ContexType>
using BehaviorFunctionPtr = std::add_pointer_t<void(
    NodeVector<ContexType> const&, typename NodeVector<ContexType>::size_type,
    ContexType&)>;

template <typename ContexType>
struct Node {
  BehaviorFunctionPtr<ContexType> behavior;
  typename NodeVector<ContexType>::size_type childrenCount;
};

template <typename ContexType>
struct NodeDescription {
  std::string_view name;
  BehaviorFunctionPtr<ContexType> behavior;
  typename NodeVector<ContexType>::size_type childrenCount;
};

template <typename ContexType, typename NodeVector<ContexType>::size_type N>
std::array<typename NodeVector<ContexType>::size_type, N> getChildrenIndex(
    NodeVector<ContexType> const& n,
    typename NodeVector<ContexType>::size_type currentNodePos) {
  std::array<typename NodeVector<ContexType>::size_type, N> childrenIdx;
  if (N > 0) {
    childrenIdx[0] = currentNodePos + 1;
    for (typename NodeVector<ContexType>::size_type i = 1; i < N; ++i) {
      auto nextNodePos = n[childrenIdx[i - 1]].childrenCount + 1;
      childrenIdx[i] = nextNodePos;
    }
  }
  return childrenIdx;
}

template <typename ContexType>
auto getNodes() -> decltype(auto) {
  using namespace std::literals;
  using NodeDesT = NodeDescription<ContexType>;
  return std::array{
      NodeDesT{"m"sv,
               [](NodeVector<ContexType> const&,
                  typename NodeVector<ContexType>::size_type,
                  ContexType& c) { c.move(); },
               0},
      NodeDesT{"p2"sv,
               [](NodeVector<ContexType> const& n,
                  typename NodeVector<ContexType>::size_type currentNodePos,
                  ContexType& c) {
                 for (auto childIdx :
                      getChildrenIndex<ContexType, 2>(n, currentNodePos))
                   n[childIdx].behavior(n, childIdx, c);
               },
               2}};
}

}  // namespace ipt

int main(int argc, char* argv[]) {
  auto cliArgsOutcome = handleCLI(argc, argv);
  if (!cliArgsOutcome) {
    std::cerr << cliArgsOutcome.error() << "\n";
    exit(1);
  }
  auto cliArgs = cliArgsOutcome.value();

  //   char const* optimalAntRPNdef = "m r m if l l p3 r m if if p2 r p2 m if";
  //   auto optAnt =
  //       gpm::factory<ant::NodesVariant>(gpm::RPNTokenCursor{optimalAntRPNdef});
  auto antBoardSim = getAntBoardSim(cliArgs.boarddef.c_str());
  //
  //
  char const* p = "m l m if l l p3 m if l p3 m if";
  //   auto optAnt2 = gpm::factory<ant::NodesVariant>(gpm::RPNTokenCursor{p});

  auto rpnTokenCursor = gpm::RPNTokenCursor{p};

  using ContexType = decltype(antBoardSim);

  auto nodes = ipt::getNodes<ContexType>();
}
