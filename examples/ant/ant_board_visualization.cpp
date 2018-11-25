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
#include "nodes_implicit_tree.hpp"

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

// auto crossover = [](ant::NodesVariant& idv1, ant::NodesVariant& idv2,
//                     auto randomGen) {
//   std::array<std::reference_wrapper<ant::NodesVariant>, 2> indis = {
//       std::ref(idv1), std::ref(idv2)};
//
//   std::array<std::size_t, 2> crossoverPointsIndex;
//   for (std::size_t i = 0; i < 2; ++i) {
//     auto treeSize = boost::apply_visitor(gpm::CountNodes{}, indis[i].get());
//     crossoverPointsIndex[i] =
//         std::uniform_int_distribution<std::size_t>{1, treeSize -
//         1}(randomGen);
//   }
//
//   std::array<ant::NodesVariant, 2> crossoverPoints{};
//   for (std::size_t i = 0; i < 2; ++i) {
//     std::size_t idx = 1;
//     std::size_t const toFind = crossoverPointsIndex[i];
//     boost::apply_visitor(
//         gpm::CallSinkOnNodes{
//             [&idx, toFind, &crossoverPoints, i](ant::NodesVariant& n) -> bool
//             {
//               if (idx++ == toFind) {
//                 crossoverPoints[i] = n;
//                 return false;
//               }
//               return true;
//             }},
//         indis[i].get());
//   }
//
//   for (std::size_t i = 0; i < 2; ++i) {
//     std::size_t idx = 1;
//     std::size_t const toFind = crossoverPointsIndex[i];
//     boost::apply_visitor(
//         gpm::CallSinkOnNodes{
//             [&idx, toFind, &crossoverPoints, i](ant::NodesVariant& n) -> bool
//             {
//               if (idx++ == toFind) {
//                 n = crossoverPoints[1 - i];
//                 return false;
//               }
//               return true;
//             }},
//         indis[i].get());
//   }
// };

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

template <typename VariantType>
class GetNodeByIndex : public boost::static_visitor<VariantType&> {
 public:
  std::size_t& index_;
  VariantType& defaultVariant_;
  GetNodeByIndex(VariantType& defaultVariant, std::size_t& index)
      : index_{index}, defaultVariant_{defaultVariant} {
    assert(index > 0);
  }

  template <typename T>
  VariantType& operator()(T& node) {
    index_--;
    if constexpr (std::tuple_size<decltype(node.children)>::value != 0) {
      for (auto& n : node.children) {
        if (index_ == 0) {
          return n;
        } else {
          return boost::apply_visitor(*this, n);
        }
        index_--;
      }
    }
    throw std::runtime_error{"OK should never be here"};
    return defaultVariant_;
  }
};

template <typename HashType, HashType kMaxHashValue_>
struct NodeNameHash {
  static constexpr HashType kMaxHashValue = kMaxHashValue_;

  template <typename BeginIterType, typename EndIterType>
  static constexpr HashType get(BeginIterType begin, EndIterType end) {
    HashType r = 0;
    for (; begin != end; ++begin) {
      r = (r + 7) ^ *begin;
    }
    return r & (kMaxHashValue - 1);
  }

  template <typename RangeType>
  static constexpr HashType get(RangeType range) {
    return get(std::begin(range), std::end(range));
  }
};

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
  auto antBoardSim =
      makeAntBoardSimDecorator(getAntBoardSim(cliArgs.boarddef.c_str()));
  //
  //
  //
  char const* p = "m l m if l l p3 m if l p3 m if";
  //   auto optAnt2 = gpm::factory<ant::NodesVariant>(gpm::RPNTokenCursor{p});

  auto rpnTokenCursor = gpm::RPNTokenCursor{p};

  using HashFunction = NodeNameHash<uint8_t, 16>;

  implicit_tree::initNodesBehavior<HashFunction, decltype(rpnTokenCursor),
                                   decltype(antBoardSim)>();

  while (!antBoardSim.is_finish()) {
    implicit_tree::eval<HashFunction>(rpnTokenCursor, antBoardSim);
  }

  //   implizit::FooBar<gpm::RPNTokenCursor, decltype(antBoardSim),
  //                    implizit::HashHelper<uint8_t, 16>>
  //       f;
  //       f.fillNodesLUT();;

  //
  //   fmt::print("{}\n",
  //              boost::apply_visitor(gpm::RPNPrinter<std::string>{}, optAnt));
  //   fmt::print("{}\n",
  //              boost::apply_visitor(gpm::RPNPrinter<std::string>{},
  //              optAnt2));
  //
  //   std::size_t s = boost::apply_visitor(gpm::CountNodes{}, optAnt2);
  //   auto sel = std::uniform_int_distribution<std::size_t>{1, s - 1};
  //   std::size_t rndSeed = std::random_device{}();
  //   auto pRnd = std::mt19937{rndSeed};
  //   auto defaultNode = ant::NodesVariant{};

  //   crossover(optAnt, optAnt2, pRnd);
  //
  //   fmt::print("{}\n",
  //              boost::apply_visitor(gpm::RPNPrinter<std::string>{}, optAnt));
  //   fmt::print("{}\n",
  //              boost::apply_visitor(gpm::RPNPrinter<std::string>{},
  //              optAnt2));

  //   std::size_t idx = 1;
  //   boost::apply_visitor(
  //       gpm::CallSinkOnNodes{[&idx](ant::NodesVariant& n) {
  //         fmt::print("{} {}\n", idx++,
  //                    boost::apply_visitor(gpm::RPNPrinter<std::string>{},
  //                    n));
  //         return true;
  //       }},
  //       optAnt2);
  //   while (true) {
  // //     [[gnu::unused]] auto toSelect = sel(pRnd);
  //     //     auto s = GetNodeByIndex{defaultNode, toSelect};
  //     //     auto & node = boost::apply_visitor(s, optAnt2);
  //     //     fmt::print("{} {}\n", toSelect,
  //     //     boost::apply_visitor(gpm::RPNPrinter<std::string>{}, node ));
  //   }
}
