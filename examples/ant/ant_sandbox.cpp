/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <fstream>
#include <iostream>
#include <limits>
#include <stack>
#include <string_view>
#include <vector>

#include <boost/algorithm/string/join.hpp>
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
using NodeVectorType = boost::container::vector<Node<ContexType>>;

template <typename ContexType>
struct Node {
  using SizeType = typename NodeVectorType<ContexType>::size_type;
  using BehaviorFunctionPtrType = std::add_pointer_t<void(
      NodeVectorType<ContexType> const&,
      typename NodeVectorType<ContexType>::size_type, ContexType&)>;

  BehaviorFunctionPtrType behavior;
  SizeType childrenCount;
};

template <typename ContexType>
struct NodeDescription {
  std::string_view name;
  typename Node<ContexType>::BehaviorFunctionPtrType behavior;
  typename Node<ContexType>::SizeType childrenCount;
};

template <typename ContexType, typename NodeVectorType<ContexType>::size_type N>
std::array<typename NodeVectorType<ContexType>::size_type, N> getChildrenIndex(
    NodeVectorType<ContexType> const& n,
    typename NodeVectorType<ContexType>::size_type currentNodePos) {
  std::array<typename NodeVectorType<ContexType>::size_type, N> childrenIdx;
  if (N > 0) {
    childrenIdx[0] = currentNodePos + 1;
    for (typename NodeVectorType<ContexType>::size_type i = 1; i < N; ++i) {
      auto nextNodePos = n[childrenIdx[i - 1]].childrenCount + 1;
      childrenIdx[i] = nextNodePos;
    }
  }
  return childrenIdx;
}

namespace detail {

template <typename ContexType, typename GetNodesDefType, typename CursorType>
struct FactoryMapBuilder {
  using FactoryFunc =
      std::function<void(CursorType&, NodeVectorType<ContexType>&)>;
  using FactoryMap = frozen::unordered_map<frozen::string, FactoryFunc, 6>;

  static FactoryFunc makeFactoryFunc(NodeDescription<ContexType> templateNode) {
    return [templateNode](CursorType& tokenCursor,
                          NodeVectorType<ContexType>& nodeVector) {
      nodeVector.emplace_back(
          Node<ContexType>{templateNode.behavior, templateNode.childrenCount});
      for (std::size_t i = 0; i < templateNode.childrenCount; ++i) {
        tokenCursor.next();
        auto token = tokenCursor.token();
        auto key = frozen::string{token.data(), token.size()};
        factoryMap.at(key)(tokenCursor, nodeVector);
      }
    };
  }

  template <typename NodesT, auto... Idx>
  static FactoryMap makeFactoryMapImpl(NodesT nodes,
                                       std::index_sequence<Idx...>) {
    auto toFrozenString = [](NodeDescription<ContexType> templateNode) {
      return frozen::string{templateNode.name.data(), templateNode.name.size()};
    };
    return frozen::unordered_map<frozen::string, FactoryFunc,
                                 std::tuple_size_v<NodesT>>{
        {toFrozenString(std::get<Idx>(nodes)),
         makeFactoryFunc(std::get<Idx>(nodes))}...};
  }

  static inline FactoryMap factoryMap = []() {
    auto antNodes = GetNodesDefType::get();
    return FactoryMapBuilder<ContexType, GetNodesDefType, CursorType>::
        makeFactoryMapImpl(
            antNodes,
            std::make_index_sequence<std::tuple_size_v<decltype(antNodes)>>());
  }();

  static void factory(CursorType& tokenCursor,
                      NodeVectorType<ContexType>& nodeVector) {
    auto token = tokenCursor.token();
    auto key = frozen::string{token.data(), token.size()};
    factoryMap.at(key)(tokenCursor, nodeVector);
  }
};

template <typename ContexType, typename GetNodesDefType>
struct NodeDescriptionMapBilder {
  using NameToNodeDescriptionMapType =
      boost::container::flat_map<std::string_view, NodeDescription<ContexType>>;
  using BehaviorToNodeDescriptionMapType = boost::container::flat_map<
      typename Node<ContexType>::BehaviorFunctionPtrType,
      NodeDescription<ContexType>>;

  static inline NameToNodeDescriptionMapType nameToNodeDescriptionMap = []() {
    NameToNodeDescriptionMapType m{};
    auto antNodesDes = GetNodesDefType::get();
    for (auto& nDes : antNodesDes) m[nDes.name] = nDes;
    return m;
  }();

  static inline BehaviorToNodeDescriptionMapType
      behaviorToNodeDescriptionMapType = []() {
        BehaviorToNodeDescriptionMapType m{};
        auto antNodesDes = GetNodesDefType::get();
        for (auto& nDes : antNodesDes) m[nDes.behavior] = nDes;
        return m;
      }();

  static NodeDescription<ContexType> const& getNodeDescription(
      std::string_view name) {
    return NodeDescriptionMapBilder<
        ContexType, GetNodesDefType>::nameToNodeDescriptionMap[name];
  }
  static NodeDescription<ContexType> const& getNodeDescription(
      typename Node<ContexType>::BehaviorFunctionPtrType behavior) {
    return NodeDescriptionMapBilder<ContexType, GetNodesDefType>::
        behaviorToNodeDescriptionMapType[behavior];
  }
};

template <typename ContexType>
std::vector<typename Node<ContexType>::SizeType> makeParentMatrix(
    NodeVectorType<ContexType>& tree) {
  using SizeT = typename ipt::Node<ContexType>::SizeType;
  auto const kParentNotSet = 666;  // std::numeric_limits<SizeT>::max();
  std::vector<SizeT> parents(tree.size(), kParentNotSet);

  auto getChildrenCount = [&tree](SizeT index) mutable -> SizeT& {
    return tree[index].childrenCount;
  };

  SizeT nextParentSearchPos = tree.size() - 2;
  while (getChildrenCount(0) != 0) {
    SizeT ri = nextParentSearchPos;
    for (; ri > 1; --ri) {
      if (getChildrenCount(ri) == 0 && getChildrenCount(ri - 1) != 0) {
        break;
      }
    }

    SizeT parentPos = ri - 1;
    for (SizeT i = parentPos + 1;
         i < tree.size() && getChildrenCount(parentPos) != 0; ++i) {
      if (getChildrenCount(i) == 0 && parents[i] == kParentNotSet) {
        getChildrenCount(parentPos) -= 1;
        parents[i] = parentPos;
      }
    }
    nextParentSearchPos = parentPos;
  }
  parents[0] = 0;
  return parents;
}

template <typename ContexType>
std::tuple<std::vector<typename Node<ContexType>::SizeType>, std::vector<typename Node<ContexType>::SizeType>> makeChildCountMatrix(
    NodeVectorType<ContexType>& tree) {
  using SizeT = typename ipt::Node<ContexType>::SizeType;
  auto const kParentNotSet = 666;  // std::numeric_limits<SizeT>::max();
  std::vector<SizeT> parents(tree.size(), kParentNotSet);

  auto getChildrenCount = [&tree](SizeT index) mutable -> SizeT& {
    return tree[index].childrenCount;
  };

  SizeT nextParentSearchPos = tree.size() - 2;
  while (getChildrenCount(0) != 0) {
    SizeT ri = nextParentSearchPos;
    for (; ri > 1; --ri) {
      if (getChildrenCount(ri) == 0 && getChildrenCount(ri - 1) != 0) {
        break;
      }
    }

    SizeT parentPos = ri - 1;
    for (SizeT i = parentPos + 1;
         i < tree.size() && getChildrenCount(parentPos) != 0; ++i) {
      if (getChildrenCount(i) == 0 && parents[i] == kParentNotSet) {
        getChildrenCount(parentPos) -= 1;
        parents[i] = parentPos;
      }
    }
    nextParentSearchPos = parentPos;
  }
  parents[0] = 0;

  std::vector<SizeT> childCountAllLevel(tree.size(), kParentNotSet);
  return std::make_tuple(parents, childCountAllLevel);
}


template <typename ContexType>
void setChildrenCount(NodeVectorType<ContexType>& tree) {
  using SizeT = typename ipt::Node<ContexType>::SizeType;

  std::vector<SizeT> parents{makeParentMatrix(tree)};

  auto getChildrenCount = [&tree](SizeT index) mutable -> SizeT& {
    return tree[index].childrenCount;
  };

  for (SizeT i = 1; i < tree.size(); ++i) {
    SizeT cursor = i;
    getChildrenCount(cursor) = 0;
    do {
      cursor = parents[cursor];
      getChildrenCount(cursor) += 1;
    } while (cursor != 0);
  }
}

}  // namespace detail

template <typename ContexType, typename GetNodesDefType, typename CursorType>
NodeVectorType<ContexType> factory(CursorType tokenCursorType) {
  NodeVectorType<ContexType> nodeVector;
  detail::FactoryMapBuilder<ContexType, GetNodesDefType, CursorType>::factory(
      tokenCursorType, nodeVector);
  detail::setChildrenCount(nodeVector);
  return nodeVector;
}

template <typename ContexType, typename GetNodesDefType>
NodeDescription<ContexType> getNodeDescription(std::string_view name) {
  return detail::NodeDescriptionMapBilder<
      ContexType, GetNodesDefType>::getNodeDescription(name);
}

template <typename ContexType, typename GetNodesDefType>
NodeDescription<ContexType> getNodeDescription(
    typename Node<ContexType>::BehaviorFunctionPtrType behavior) {
  return detail::NodeDescriptionMapBilder<
      ContexType, GetNodesDefType>::getNodeDescription(behavior);
}

}  // namespace ipt

namespace iptexample {

template <typename ContexType>
struct AntNodesDef {
  static auto get() -> decltype(auto) {
    using namespace ipt;
    using namespace std::literals;
    using NodeDesT = NodeDescription<ContexType>;
    using NodeVectorT = NodeVectorType<ContexType>;
    using SizeT = typename NodeVectorType<ContexType>::size_type;
    using ChildrenNT = SizeT;

    return std::array{
        NodeDesT{"m"sv,
                 [](NodeVectorT const&, SizeT, ContexType& c) { c.move(); },
                 ChildrenNT{0}},
        NodeDesT{"l"sv,
                 [](NodeVectorT const&, SizeT, ContexType& c) { c.left(); },
                 ChildrenNT{0}},

        NodeDesT{"r"sv,
                 [](NodeVectorT const&, SizeT, ContexType& c) { c.right(); },
                 ChildrenNT{0}},

        NodeDesT{"p2"sv,
                 [](NodeVectorType<ContexType> const& n, SizeT currentNodePos,
                    ContexType& c) {
                   for (auto childIdx :
                        getChildrenIndex<ContexType, 2>(n, currentNodePos))
                     n[childIdx].behavior(n, childIdx, c);
                 },
                 2},
        NodeDesT{"p3"sv,
                 [](NodeVectorT const& n, SizeT currentNodePos, ContexType& c) {
                   for (auto childIdx :
                        getChildrenIndex<ContexType, 3>(n, currentNodePos))
                     n[childIdx].behavior(n, childIdx, c);
                 },
                 ChildrenNT{3}},
        NodeDesT{"if"sv,
                 [](NodeVectorT const& n, SizeT currentNodePos, ContexType& c) {
                   auto childIdxArray =
                       getChildrenIndex<ContexType, 2>(n, currentNodePos);

                   auto childIdx = childIdxArray[c.is_food_in_front() ? 0 : 1];
                   n[childIdx].behavior(n, childIdx, c);
                 },
                 ChildrenNT{2}}

    };
  }
};

enum class TestState { Success, Fail };

template <typename ContexT, typename OutputFunctionT>
bool test_parentMatrix(const char* antDef,
                       std::vector<std::size_t> const& expected,
                       OutputFunctionT out) {
  auto vecToStr = [](auto const& vec) {
    std::string s;
    for (auto& v : vec) {
      s += fmt::format("{:>4}", v);
    }
    return s;
  };

  auto pnTokenCursor = gpm::PNTokenCursor{antDef};
  auto tree =
      ipt::factory<ContexT, iptexample::AntNodesDef<ContexT>>(pnTokenCursor);

  std::string treeStringWithPadding;
  std::string childCount;
  std::string nodeIndex;
  int i = 0;
  for (auto& n : tree) {
    n.childrenCount =
        ipt::getNodeDescription<ContexT, iptexample::AntNodesDef<ContexT>>(
            n.behavior)
            .childrenCount;
    treeStringWithPadding += fmt::format(
        "{:>4}",
        ipt::getNodeDescription<ContexT, iptexample::AntNodesDef<ContexT>>(
            n.behavior)
            .name);
    childCount += fmt::format(
        "{:>4}",
        ipt::getNodeDescription<ContexT, iptexample::AntNodesDef<ContexT>>(
            n.behavior)
            .childrenCount);
    nodeIndex += fmt::format("{:>4}", i++);
  }
  auto [parents, childCoundAllLevel] = ipt::detail::makeChildCountMatrix(tree);

  auto parentsAsStr = vecToStr(parents);
  auto expectedAsStr = vecToStr(expected);
  auto childCoundAllLevelStr = vecToStr(childCoundAllLevel);

  out(fmt::format(
          "\n{} : index\n{} : node names\n{} : node child count (1th "
          "level)\n{} : parents index\n{} : expexted parents index (manual)\n{} : child count all level\n",
          nodeIndex, treeStringWithPadding, childCount, parentsAsStr, expectedAsStr, childCoundAllLevelStr),
      parentsAsStr != expectedAsStr ? TestState::Fail : TestState::Success);
  return (parentsAsStr == expectedAsStr);
}

}  // namespace iptexample

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
  using ContexT = decltype(antBoardSim);

  iptexample::test_parentMatrix<ContexT>(
      "if p2 m l p3 p3 m m m l l", {0, 0, 1, 1, 0, 4, 5, 5, 5, 4, 4},
      [](std::string const& s, iptexample::TestState /*ts*/) {
        std::cout << s;
      });
  iptexample::test_parentMatrix<ContexT>(
      "if m p3 l if m p3 l l if m l m", {0, 0, 0, 2, 2, 4, 4, 6, 6, 6, 9, 9, 2},
      [](std::string const& s, iptexample::TestState /*ts*/) {
        std::cout << s;
      });
  // iptexample::test_parentMatrix<ContexT>("if m p2 r p2 if if m r p3 l l if m
  // r m",
  //                                         { });
}
