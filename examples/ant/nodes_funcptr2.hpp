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
#include <boost/container/vector.hpp>
#include <functional>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/hana.hpp>

namespace funcptr2 {

template <typename ContexType>
struct Node {
  using BehaviorPtr = void (*)(Node<ContexType> const &, ContexType &);

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
constexpr auto getAntNodes() -> decltype(auto) {
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

namespace detail {

template <typename ContexType, typename CursorType>
struct FactoryBuilder {
  using FactoryFunc = std::function<Node<ContexType>(CursorType &)>;
  using FactoryMap = frozen::unordered_map<frozen::string, FactoryFunc, 6>;
  using NameMap = std::unordered_map<typename Node<ContexType>::BehaviorPtr,
                                     std::string_view>;

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

  static frozen::string makeName(NodeDescription<ContexType> templateNode) {
    return frozen::string{templateNode.name.data(), templateNode.name.size()};
  }

  template <typename NodesT, auto... Idx>
  static FactoryMap makeFactoryMapImpl(NodesT nodes,
                                       std::index_sequence<Idx...>) {
    return frozen::unordered_map<frozen::string, FactoryFunc,
                                 std::tuple_size_v<NodesT>>{
        {makeName(std::get<Idx>(nodes)),
         makeFactoryFunc(std::get<Idx>(nodes))}...};
  }

  template <typename NodesT, auto... Idx>
  static NameMap makeNameMapImpl(NodesT nodes, std::index_sequence<Idx...>) {
    return NameMap{
        {std::get<Idx>(nodes).behavior, makeName(std::get<Idx>(nodes))}...};
  }

  static FactoryMap makeFactoryMap() {
    auto antNodes = getAntNodes<ContexType>();
    return FactoryBuilder<ContexType, CursorType>::makeFactoryMapImpl(
        antNodes,
        std::make_index_sequence<std::tuple_size_v<decltype(antNodes)>>());
  }

  static NameMap makeNameMap() {
    auto antNodes = getAntNodes<ContexType>();
    NameMap nm;
    for (auto &ndef : antNodes) nm[ndef.behavior] = ndef.name;
    return nm;
  }

  static inline FactoryMap factoryMap =
      FactoryBuilder<ContexType, CursorType>::makeFactoryMap();

  static inline NameMap nameMap =
      FactoryBuilder<ContexType, CursorType>::makeNameMap();

  static Node<ContexType> factory(CursorType &tokenCursor) {
    auto token = tokenCursor.token();
    auto key = frozen::string{token.data(), token.size()};
    return factoryMap.at(key)(tokenCursor);
  }
};
}  // namespace detail

template <typename ContexType, typename CursorTypeType>
Node<ContexType> factory(CursorTypeType tokenCursorType) {
  return detail::FactoryBuilder<ContexType, CursorTypeType>::factory(
      tokenCursorType);
}

template <typename ContexType, typename CursorTypeType>
std::string_view getNodeName(Node<ContexType> const &nc) {
  return detail::FactoryBuilder<ContexType,
                                CursorTypeType>::nameMap[nc.behavior];
}

}  // namespace funcptr2
