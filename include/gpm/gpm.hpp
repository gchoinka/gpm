/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include <array>
#include <exception>

#include <functional>
#include <iterator>
#include <random>
#include <string_view>
#include <tuple>
#include <vector>

#include <boost/assert.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/mp11.hpp>
#include <boost/type_index.hpp>
#include <boost/variant.hpp>

#include <gpm/io.hpp>

namespace gpm {

template <auto... ch>
struct NodeToken {
  constexpr static char name[] = {ch..., '\0'};
};

template <typename VariantType, size_t NodeCount_, typename CTString>
struct BaseNode : public CTString {
  template <typename... Args>
  BaseNode(Args &&... args) : children{std::forward<Args>(args)...} {}

  std::array<VariantType, NodeCount_> children;
};

namespace detail {
template <typename VariantType, typename Iter>
VariantType factory_imp(Iter &);

template <typename VariantType, typename Iter>
using FactoryMap =
    boost::container::flat_map<std::string_view,
                               std::function<VariantType(Iter &)>>;

template <typename VariantType, typename Iter>
struct FactoryMapInsertHelper {
  FactoryMap<VariantType, Iter> &factoryMap;

  template <class T>
  void operator()(T) {
    factoryMap[T::name] = [](Iter &tokenIter) {
      T ret;
      if constexpr (ret.children.size() != 0)
        for (auto &n : ret.children) n = factory_imp<VariantType>(++tokenIter);
      return ret;
    };
  }

  template <class T>
  void operator()(boost::recursive_wrapper<T>) {
    factoryMap[T::name] = [](Iter &tokenIter) {
      T ret;
      if constexpr (ret.children.size() != 0)
        for (auto &n : ret.children) n = factory_imp<VariantType>(++tokenIter);
      return ret;
    };
  }
};

template <typename VariantType, typename Iter>
FactoryMap<VariantType, Iter> makeFactoryMap() {
  FactoryMap<VariantType, Iter> factoryMap;
  auto insertHelper = FactoryMapInsertHelper<VariantType, Iter>{factoryMap};
  boost::mp11::mp_for_each<VariantType>(insertHelper);
  return factoryMap;
}

template <typename VariantType, typename Iter>
VariantType factory_imp(Iter &tokenIter) {
  static auto nodeCreateFunMap = makeFactoryMap<VariantType, Iter>();
  auto token = *tokenIter;
  BOOST_ASSERT_MSG(nodeCreateFunMap.count(token) > 0,
                   "can not find factory function for token");

  return nodeCreateFunMap[token](tokenIter);
}
}  // namespace detail

template <typename VariantType, typename Iter>
VariantType factory(Iter tokenIter) {
  return detail::factory_imp<VariantType>(tokenIter);
}

struct AnyTypeNullSink {
  template <typename T>
  AnyTypeNullSink(T const &&) {}
  template <typename T>
  AnyTypeNullSink const &operator=(T const &&) {
    return *this;
  }
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

template <typename T>
struct IsInRecursiveWrapper {
  static constexpr auto value = 0;
};

template <typename T>
struct IsInRecursiveWrapper<boost::recursive_wrapper<T>> {
  static constexpr auto value = 1;
};

template <typename T>
constexpr auto isInRecursiveWrapper_v = IsInRecursiveWrapper<T>::value;

template <typename T>
struct UnpackRecursiveWrapper {
  static decltype(auto) get(T t) { return t; }
};

template <typename T>
struct UnpackRecursiveWrapper<boost::recursive_wrapper<T>> {
  static decltype(auto) get(boost::recursive_wrapper<T> t) { return t.get(); }
};

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
