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

}  // namespace detail

template <typename ContexType, typename GetNodesDefType,
          typename CursorTypeType>
NodeVectorType<ContexType> factory(CursorTypeType tokenCursorType) {
  NodeVectorType<ContexType> nodeVector;
  detail::FactoryMapBuilder<ContexType, GetNodesDefType,
                            CursorTypeType>::factory(tokenCursorType,
                                                     nodeVector);
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

template <typename ContexType>
std::vector<typename Node<ContexType>::SizeType> makeParentMatrix(
    NodeVectorType<ContexType>& tree) {
  using SizeT = typename ipt::Node<ContexType>::SizeType;
  auto const kParentNotSet = std::numeric_limits<SizeT>::max();
  std::vector<SizeT> parents(tree.size(), kParentNotSet);

  SizeT riStart = tree.size() - 1;
  while (tree[0].childrenCount != 0) {
    SizeT ri = riStart;
    riStart = tree.size() - 1;
    for (; ri > 1; --ri) {
      if (tree[ri].childrenCount == 0 &&
          (tree[ri - 1].childrenCount != 0 ||
           parents[ri - 1] != kParentNotSet) &&
          parents[ri] == kParentNotSet) {
        break;
      }
    }

    SizeT sri = ri - 1;
    for (; sri >= 0; --sri) {
      if (tree[sri].childrenCount != 0) {
        tree[sri].childrenCount -= 1;
        parents[ri] = sri;
        if (tree[sri].childrenCount == 0) riStart = sri;
        break;
      }
    }
  }
  return parents;
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
                 [](NodeVectorT const&,
                    SizeT,
                    ContexType& c) { c.move(); },
                 ChildrenNT{0}},
        NodeDesT{"l"sv,
                 [](NodeVectorT const&,
                    SizeT,
                    ContexType& c) { c.left(); },
                 ChildrenNT{0}},

        NodeDesT{"r"sv,
                 [](NodeVectorT const&,
                    SizeT,
                    ContexType& c) { c.right(); },
                 ChildrenNT{0}},

        NodeDesT{
            "p2"sv,
            [](NodeVectorType<ContexType> const& n,
               SizeT currentNodePos,
               ContexType& c) {
              for (auto childIdx :
                   getChildrenIndex<ContexType, 2>(n, currentNodePos))
                n[childIdx].behavior(n, childIdx, c);
            },
            2},
        NodeDesT{
            "p3"sv,
            [](NodeVectorT const& n,
               SizeT currentNodePos,
               ContexType& c) {
              for (auto childIdx :
                   getChildrenIndex<ContexType, 3>(n, currentNodePos))
                n[childIdx].behavior(n, childIdx, c);
            },
            ChildrenNT{3}},
        NodeDesT{
            "if"sv,
            [](NodeVectorT const& n,
               SizeT currentNodePos,
               ContexType& c) {
              auto childIdxArray =
                  getChildrenIndex<ContexType, 2>(n, currentNodePos);

              auto childIdx = childIdxArray[c.is_food_in_front() ? 0 : 1];
              n[childIdx].behavior(n, childIdx, c);
            },
            ChildrenNT{2}}

    };
  }
};

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
  //
  //
  // char const* p = "m l m if l l p3 m if l p3 m if";
  char const* p = "l l m m m p3 p3 l m p2 if";
  //   auto optAnt2 = gpm::factory<ant::NodesVariant>(gpm::RPNTokenCursor{p});

  auto rpnTokenCursor = gpm::RPNTokenCursor{p};

  using ContexT = decltype(antBoardSim);

  //   auto nodes = ipt::IptAntNodesDef<ContexT>::get();
  auto tree =
      ipt::factory<ContexT, iptexample::AntNodesDef<ContexT>>(rpnTokenCursor);

  auto parents = ipt::makeParentMatrix<ContexT>(tree);
  //   using SizeT = ipt::Node<ContexT>::SizeType;
  //   auto const kParentNotSet = std::numeric_limits<SizeT>::max();
  //   std::vector<SizeT> parents(tree.size(), kParentNotSet);
  //
  //
  //   int riStart = tree.size() - 1;
  //   while(tree[0].childrenCount != 0){
  //     int ri = riStart;
  //     riStart = tree.size() - 1;
  //     for(; ri > 1; --ri){
  //       if(tree[ri].childrenCount == 0 && (tree[ri-1].childrenCount != 0 ||
  //       parents[ri-1] != kParentNotSet) && parents[ri] == kParentNotSet){
  //         break;
  //       }
  //     }
  //
  //     int sri = ri - 1;
  //     for(; sri >= 0; --sri){
  //       if(tree[sri].childrenCount != 0){
  //         tree[sri].childrenCount -= 1;
  //         parents[ri] = sri;
  //         if(tree[sri].childrenCount == 0)
  //           riStart = sri;
  //         break;
  //       }
  //     }
  //

  for (auto& n : tree) {
    fmt::print(
        "{:>4}",
        ipt::getNodeDescription<ContexT, iptexample::AntNodesDef<ContexT>>(
            n.behavior)
            .name);
  }
  fmt::print("\n");
  for (auto& n : tree) {
    fmt::print("{:>4}", n.childrenCount);
  }
  fmt::print("\n");
  for (auto& n : parents) {
    if (n > parents.size())
      fmt::print("{:>4}", -1);
    else
      fmt::print("{:>4}", n);
  }
  fmt::print("\n");

  // //
  // //     }
  //   }

  //   SizeT cursor = 0;
  //   while(tree[cursor].childrenCount != 0){
  //     parents[cursor+1] = cursor;
  //     childProcessed[cursor] = 1;
  //     cursor += 1;
  //   }
  //
  // //     while(true){
  //       auto candidate = parents[cursor];
  //
  //       if(tree[candidate].childrenCount > 1 && childProcessed[candidate] <
  //       tree[candidate].childrenCount){
  //         auto subCursor = candidate;
  //
  //         while(parents[++subCursor] != kParentNotSet){}
  //
  //         parents[subCursor] = candidate;
  //         childProcessed[candidate] += 1;
  // //         while(tree[subCursor].childrenCount != 0){
  // //           parents[subCursor+1] = subCursor;
  // //           childProcessed[subCursor] = 1;
  // //           subCursor += 1;
  // //         }
  //
  //       }
  //
  // //     }

  /*
    if(b[0].childrenCount > 0){
      parents[1] = 0;
      if(b[1].childrenCount > 0)
        parents[2] = 1;
    }

    std::vector<std::size_t> sizeInfo(b.size(), 0);
    struct Branch {
      std::size_t pos;
      std::size_t childNumber;
    };
    std::stack<Branch> branchesTodo;


    std::size_t cur = 0;
    while(true){
      if(b[cur].childrenCount == 0){
        for(auto p: parents)
          sizeInfo[p] += 1;
        if(branchesTodo.size() == 0)
          break;
      }
      else {
        for(int i = 1; i < b[cur].childrenCount; ++i){
          branchesTodo.push(Branch{cur, b[cur].childrenCount - 1 - i});
        }
        cur++;
      }

    }*/

  std::cout << tree.size() << "\n";
}
