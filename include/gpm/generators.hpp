/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include <boost/mp11.hpp>
#include <boost/variant.hpp>
#include <functional>
#include <random>
#include <vector>

namespace gpm {


template <typename T>
struct UnpackRecursiveWrapper {
  static T get(T t) { return t; }
};

template <typename T>
struct UnpackRecursiveWrapper<boost::recursive_wrapper<T>> {
  static T get(boost::recursive_wrapper<T> t) { return t.get(); }
};

template <typename VariantType, typename TerminalNodeFactoryT,
          typename NotTerminalNodeFactoryT, typename NodeFactoryT>
struct ChildrenInserter : boost::static_visitor<VariantType> {
  TerminalNodeFactoryT &terminalNodeFactory_;
  NotTerminalNodeFactoryT &notTerminalNodeFactory_;
  NodeFactoryT &nodeFactory_;
  int const minHeight_;
  int const maxHeight_;
  int currentHeight_;

  ChildrenInserter(VariantType, TerminalNodeFactoryT &terminalNodeFactory,
                   NotTerminalNodeFactoryT &notTerminalNodeFactory,
                   NodeFactoryT &nodeFactory, int minHeight, int maxHeight,
                   int currentHeight)
      : terminalNodeFactory_{terminalNodeFactory},
        notTerminalNodeFactory_{notTerminalNodeFactory},
        nodeFactory_{nodeFactory},
        minHeight_{minHeight},
        maxHeight_{maxHeight},
        currentHeight_{currentHeight} {}

  template <typename NodeT>
  VariantType operator()(NodeT node) const {
    if constexpr (node.children.size() != 0) {
      for (auto &childNode : node.children) {
        if (currentHeight_ < maxHeight_) {
          if (currentHeight_ >= minHeight_)
            childNode = nodeFactory_();
          else
            childNode = notTerminalNodeFactory_();
          auto nextLevel = *this;
          nextLevel.currentHeight_++;
          childNode = boost::apply_visitor(nextLevel, childNode);
        } else if (currentHeight_ == maxHeight_) {
          childNode = terminalNodeFactory_();
        }
      }
    }
    return node;
  }
};

template <typename VariantType, typename TerminalNodeFactoryT,
          typename NotTerminalNodeFactoryT, typename NodeFactoryT>
ChildrenInserter(VariantType, TerminalNodeFactoryT &, NotTerminalNodeFactoryT &,
                 NodeFactoryT &, int, int, int)
    ->ChildrenInserter<VariantType, TerminalNodeFactoryT,
                       NotTerminalNodeFactoryT, NodeFactoryT>;

template <typename VariantType>
class BasicGenerator {
 public:
  BasicGenerator(int minHeight, int maxHeight, unsigned int rndSeed = 5489u)
      : minHeight_{minHeight}, maxHeight_{maxHeight}, rnd_{rndSeed} {
    std::vector<VariantType> terminalNodes;
    std::vector<VariantType> notTerminalNodes;
    std::vector<VariantType> allNodes;

    boost::mp11::mp_for_each<VariantType>([&](auto node) {
      auto unpacked = UnpackRecursiveWrapper<decltype(node)>::get(node);
      allNodes.push_back(unpacked);
      if (unpacked.children.size() == 0)
        terminalNodes.push_back(unpacked);
      else
        notTerminalNodes.push_back(unpacked);
    });

    BOOST_ASSERT_MSG(minHeight > 1, "minHeight needs to be bigger that 1");
    BOOST_ASSERT_MSG(terminalNodes.size() > 0, "no terminatin nodes defined");
    BOOST_ASSERT_MSG(notTerminalNodes.size() > 0,
                     "no none terminatin nodes defined");

    std::uniform_int_distribution<size_t> randomTermNodeSelector{
        0, terminalNodes.size() - 1};
    randomTerminalNode_ = [=]() mutable {
      return terminalNodes[randomTermNodeSelector(rnd_)];
    };

    std::uniform_int_distribution<size_t> randomNotTermNodeSelector{
        0, notTerminalNodes.size() - 1};
    randomNotTerminalNode_ = [=]() mutable {
      return notTerminalNodes[randomNotTermNodeSelector(rnd_)];
    };

    std::uniform_int_distribution<size_t> randomNodeSelector{
        0, allNodes.size() - 1};
    randomNode_ = [=]() mutable { return allNodes[randomNodeSelector(rnd_)]; };
  }

  VariantType operator()() {
    auto rootNode = randomNotTerminalNode_();

    return boost::apply_visitor(
        ChildrenInserter{rootNode, randomTerminalNode_, randomNotTerminalNode_,
                         randomNode_, minHeight_, maxHeight_, 1},
        rootNode);
  }

 private:
  int const minHeight_;
  int const maxHeight_;

  std::function<VariantType()> randomTerminalNode_;
  std::function<VariantType()> randomNotTerminalNode_;
  std::function<VariantType()> randomNode_;

  std::mt19937 rnd_;
};

}  // namespace gpm
