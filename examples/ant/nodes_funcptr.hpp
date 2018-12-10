/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include <frozen/string.h>
#include <frozen/unordered_map.h>

#include <array>
#include <boost/container/flat_map.hpp>
#include <boost/container/vector.hpp>
#include <functional>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/hana.hpp>

namespace funcptr {

template <typename ContexType>
struct Node {
  using BehaviorPtr =
      std::add_pointer_t<void(Node<ContexType> const &, ContexType &)>;

  void operator()(Node<ContexType> const &n, ContexType &c) const {
    behavior(n, c);
  }

  BehaviorPtr behavior = nullptr;
  boost::container::vector<Node<ContexType>> children;
};

template <typename ContexType>
struct NodeDescription {
  std::string_view name;
  typename Node<ContexType>::BehaviorPtr behavior;
  std::size_t childCount;
};

template <typename ContexType>
struct GetAntNodes {
  static constexpr auto get() -> decltype(auto) {
    using NodeT = Node<ContexType>;
    using NodeDesT = NodeDescription<ContexType>;
    using namespace std::literals;
    return std::array{
        NodeDesT{"if"sv,
                 [](NodeT const &self, ContexType &c) {
                   if (c.is_food_in_front())
                     self.children[0](self.children[0], c);
                   else
                     self.children[1](self.children[1], c);
                 },
                 2},
        NodeDesT{"m"sv, [](NodeT const &, ContexType &c) { c.move(); }, 0},
        NodeDesT{"l"sv, [](NodeT const &, ContexType &c) { c.left(); }, 0},
        NodeDesT{"r"sv, [](NodeT const &, ContexType &c) { c.right(); }, 0},
        NodeDesT{"p2"sv,
                 [](NodeT const &self, ContexType &c) {
                   for (auto &child : self.children) child(child, c);
                 },
                 2},
        NodeDesT{"p3"sv,
                 [](NodeT const &self, ContexType &c) {
                   for (auto &child : self.children) child(child, c);
                 },
                 3}};
  }
};

namespace detail {

template <typename ContexType, typename GetNodesDefType, typename CursorType>
struct FactoryMapBuilder {
  using FactoryFunc = std::function<Node<ContexType>(CursorType &)>;
  using FactoryMap = frozen::unordered_map<frozen::string, FactoryFunc, 6>;

  static FactoryFunc makeFactoryFunc(NodeDescription<ContexType> templateNode) {
    return [templateNode](CursorType &tokenCursor) {
      auto currentNode = Node<ContexType>{templateNode.behavior, {}};
      for (std::size_t i = 0; i < templateNode.childCount; ++i) {
        tokenCursor.next();
        auto token = tokenCursor.token();
        auto key = frozen::string{token.data(), token.size()};
        currentNode.children.emplace_back(factoryMap.at(key)(tokenCursor));
      }
      return currentNode;
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

  static Node<ContexType> factory(CursorType &tokenCursor) {
    auto token = tokenCursor.token();
    auto key = frozen::string{token.data(), token.size()};
    return factoryMap.at(key)(tokenCursor);
  }
};

template <typename ContexType, typename GetNodesDefType>
struct NodeDescriptionMapBilder {
  using NameToNodeDescriptionMapType =
      boost::container::flat_map<std::string_view, NodeDescription<ContexType>>;
  using BehaviorToNodeDescriptionMapType =
      boost::container::flat_map<typename Node<ContexType>::BehaviorPtr,
                                 NodeDescription<ContexType>>;

  static inline NameToNodeDescriptionMapType nameToNodeDescriptionMap = []() {
    NameToNodeDescriptionMapType m{};
    auto antNodesDes = GetNodesDefType::get();
    for (auto &nDes : antNodesDes) m[nDes.name] = nDes;
    return m;
  }();

  static inline BehaviorToNodeDescriptionMapType
      behaviorToNodeDescriptionMapType = []() {
        BehaviorToNodeDescriptionMapType m{};
        auto antNodesDes = GetNodesDefType::get();
        for (auto &nDes : antNodesDes) m[nDes.behavior] = nDes;
        return m;
      }();

  static NodeDescription<ContexType> const &getNodeDescription(
      std::string_view name) {
    return NodeDescriptionMapBilder<
        ContexType, GetNodesDefType>::nameToNodeDescriptionMap[name];
  }
  static NodeDescription<ContexType> const &getNodeDescription(
      typename Node<ContexType>::BehaviorPtr behavior) {
    return NodeDescriptionMapBilder<ContexType, GetNodesDefType>::
        behaviorToNodeDescriptionMapType[behavior];
  }
};

}  // namespace detail

template <typename ContexType, typename GetNodesDefType,
          typename CursorTypeType>
Node<ContexType> factory(CursorTypeType tokenCursorType) {
  return detail::FactoryMapBuilder<ContexType, GetNodesDefType,
                                   CursorTypeType>::factory(tokenCursorType);
}

template <typename ContexType, typename GetNodesDefType>
NodeDescription<ContexType> getNodeDescription(std::string_view name) {
  return detail::NodeDescriptionMapBilder<
      ContexType, GetNodesDefType>::getNodeDescription(name);
}

template <typename ContexType, typename GetNodesDefType>
NodeDescription<ContexType> getNodeDescription(
    typename Node<ContexType>::BehaviorPtr behavior) {
  return detail::NodeDescriptionMapBilder<
      ContexType, GetNodesDefType>::getNodeDescription(behavior);
}

}  // namespace funcptr

namespace funcptr::io {

template <typename ContexType, typename GetNodesDefType, typename SinkT>
void printRPN(funcptr::Node<ContexType> const &n, SinkT sink) {
  for (auto b = n.children.rbegin(); b != n.children.rend(); ++b)
    printRPN<ContexType, GetNodesDefType, SinkT>(*b, sink);
  sink(getNodeDescription<ContexType, GetNodesDefType>(n.behavior).name);
}

}  // namespace funcptr::io
