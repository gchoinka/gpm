/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include <array>
#include <functional>
#include <string_view>
#include <unordered_map>
#include <vector>

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
auto getAntNodes() -> decltype(auto) {
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
template <typename ContexType, typename IterType>
struct FactoryHelper {
  using FactoryMap =
      std::unordered_map<std::string_view,
                         std::function<Node<ContexType>(IterType &)>>;
  static FactoryMap makeFactoryMap() {
    FactoryMap toReturn;
    for (auto &templateNode : getAntNodes<ContexType>()) {
      toReturn[templateNode.name] = [templateNode](IterType &tokenIterType) {
        auto currentNode =
            Node<ContexType>{templateNode.name, templateNode.behavior, {}};
        for ([[gnu::unused]] auto const &foo : templateNode.children) {
          ++tokenIterType;
          currentNode.children.emplace_back(
              factoryMap[*tokenIterType](tokenIterType));
        }
        return currentNode;
      };
    }
    return toReturn;
  }

  static inline FactoryMap factoryMap =
      FactoryHelper<ContexType, IterType>::makeFactoryMap();

  static Node<ContexType> factory(IterType &tokenIterType) {
    auto token = *tokenIterType;
    return factoryMap[token](tokenIterType);
  }
};
}  // namespace detail

template <typename ContexType, typename IterTypeType>
Node<ContexType> factory(IterTypeType tokenIterType) {
  return detail::FactoryHelper<ContexType, IterTypeType>::factory(
      tokenIterType);
}
}  // namespace funcptr
