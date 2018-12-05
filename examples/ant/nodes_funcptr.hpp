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
#include <functional>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/hana.hpp>

namespace funcptr {

template <typename ContexType>
struct Node {
  using BehaviorPtr = void (*)(Node<ContexType> const &, ContexType &);

  void operator()(Node<ContexType> const &n, ContexType &c) const {
    behavior(n, c);
  }

  std::string_view name = "unset";
  BehaviorPtr behavior = nullptr;
  std::vector<Node<ContexType>> children;
};

template <typename ContexType>
constexpr auto getAntNodes() -> decltype(auto) {
  using NodeT = Node<ContexType>;
  auto childCount = [](int childCount) {
    return std::vector<NodeT>(childCount, NodeT{});
  };

  return std::array{
      NodeT{"if",
            [](NodeT const &self, ContexType &c) {
              if (c.is_food_in_front())
                self.children[0](self.children[0], c);
              else
                self.children[1](self.children[1], c);
            },
            childCount(2)},
      NodeT{"m", [](NodeT const &, ContexType &c) { c.move(); }, childCount(0)},
      NodeT{"l", [](NodeT const &, ContexType &c) { c.left(); }, childCount(0)},
      NodeT{"r", [](NodeT const &, ContexType &c) { c.right(); },
            childCount(0)},
      NodeT{"p2",
            [](NodeT const &self, ContexType &c) {
              for (auto &child : self.children) child(child, c);
            },
            childCount(2)},
      NodeT{"p3",
            [](NodeT const &self, ContexType &c) {
              for (auto &child : self.children) child(child, c);
            },
            childCount(3)}};
}

namespace detail {

template <typename ContexType, typename CursorType>
struct FactoryBuilder {
  using FactoryFunc = std::function<Node<ContexType>(CursorType &)>;
  using FactoryMap = frozen::unordered_map<frozen::string, FactoryFunc, 6>;

  static FactoryFunc makeFactoryFunc(Node<ContexType> templateNode) {
    return [templateNode](CursorType &tokenCursor) {
      auto currentNode =
          Node<ContexType>{templateNode.name, templateNode.behavior, {}};
      for ([[gnu::unused]] auto const &foo : templateNode.children) {
        tokenCursor.next();
        auto token = tokenCursor.token();
        auto key = frozen::string{token.data(), token.size()};
        currentNode.children.emplace_back(factoryMap.at(key)(tokenCursor));
      }
      return currentNode;
    };
  }

  static frozen::string makeName(Node<ContexType> templateNode) {
    return frozen::string{templateNode.name.data(), templateNode.name.size()};
  }

  template <typename NodesT, auto... Idx>
  static FactoryMap makeFactoryMap2(NodesT nodes, std::index_sequence<Idx...>) {
    return frozen::unordered_map<frozen::string, FactoryFunc,
                                 std::tuple_size_v<NodesT>>{
        {makeName(std::get<Idx>(nodes)),
         makeFactoryFunc(std::get<Idx>(nodes))}...};
  }

  static FactoryMap makeFactoryMap() {
    auto antNodes = getAntNodes<ContexType>();
    return FactoryBuilder<ContexType, CursorType>::makeFactoryMap2(
        antNodes,
        std::make_index_sequence<std::tuple_size_v<decltype(antNodes)>>());
  }
  // std::make_index_sequence<std::tuple_size_v<TupleT0>>()
  static inline FactoryMap factoryMap =
      FactoryBuilder<ContexType, CursorType>::makeFactoryMap();

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
}  // namespace funcptr
