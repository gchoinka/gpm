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
#include <vector>
#include <tuple>

#include <boost/container/flat_map.hpp>
#include <boost/mp11.hpp>
#include <boost/type_index.hpp>
#include <boost/variant.hpp>

#include <gpm/io.hpp>

namespace gpm {

template <auto ... ch>
struct NodeToken {
  constexpr static char name[] = {ch..., '\0'};
};

template <typename VariantType, size_t NodeCount_, typename CTString>
struct BaseNode : public CTString {
  template <typename... Args>
  BaseNode(Args &&... args) : children{std::forward<Args>(args)...} {}
  
  std::array<VariantType, NodeCount_> children;
  
  constexpr char const *nameStr() const { return CTString::name; }
};
  
  
class GPMException : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
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
  if (!nodeCreateFunMap.count(token)) {
    throw GPMException{std::string{"cant find factory function for token >>"} +
                       std::string{token} + "<<"};
  }

  return nodeCreateFunMap[token](tokenIter);
}
}  // namespace detail

template <typename VariantType, typename Iter>
VariantType factory(Iter tokenIter) {
  return detail::factory_imp<VariantType>(tokenIter);
}

template <typename VariantType>
struct Builder : public boost::static_visitor<void> {
  Builder(int height, std::function<VariantType()> termialGen,
          std::function<VariantType()> noneTermialGen)
      : height_{height},
        termialGen_{termialGen},
        noneTermialGen_{noneTermialGen} {}

  int height_ = 0;
  std::function<VariantType()> termialGen_;
  std::function<VariantType()> noneTermialGen_;

  template <typename T>
  void operator()(T &node) const {
    if (height_ > 0) {
      if constexpr (std::tuple_size<decltype(node.children)>::value != 0)
        for (auto &childNode : node.children) {
          childNode = noneTermialGen_();
          auto sub =
              Builder<VariantType>{height_ - 1, termialGen_, noneTermialGen_};
          boost::apply_visitor(sub, childNode);
        }
    } else {
      for (auto &childNode : node.children) {
        childNode = termialGen_();
      }
    }
  }
};



struct AnyTypeNullSink
{
  template<typename T>
  AnyTypeNullSink(T const &&) {}
  template<typename T>
  AnyTypeNullSink const & operator=(T const &&) { return *this;}
};

template<typename T>
constexpr size_t ChildrenSize = 0;


template<typename VariantType, typename TerminalFactory, typename NoneTerminalFactory>
struct NodeChildrenInsert
{
  VariantType dummy_;
  TerminalFactory & terminalFactory_;
  NoneTerminalFactory & noneTerminalFactory_;
  int height_;
  
  
  template<typename NodeT>
  void operator()(NodeT & node)
  {
    if constexpr (ChildrenSize<NodeT> != 0)
      for (auto &childNode : node.children) {
        Builder<VariantType> b{height_, terminalFactory_, noneTerminalFactory_};
        childNode = noneTerminalFactory_();
        boost::apply_visitor(b, childNode);
      }
  }
};
  
template<typename VariantType, typename TerminalFactory, typename NoneTerminalFactory>
NodeChildrenInsert(VariantType, TerminalFactory &, NoneTerminalFactory &, int height) -> NodeChildrenInsert<VariantType, TerminalFactory, NoneTerminalFactory>;

template <typename VariantType>
class BasicGenerator {
 public:
  BasicGenerator(int minHeight, int maxHeight, unsigned int rndSeed = 5489u)
      : minHeight_{minHeight}, maxHeight_{maxHeight}, rnd_{rndSeed} {
    OrderNodesHelper orderNodesHelper{terminalNodes_, noneTerminalNodes_};
    boost::mp11::mp_for_each<VariantType>(orderNodesHelper);
  }

  VariantType operator()() {
    std::uniform_int_distribution<> height{minHeight_, maxHeight_};
    std::uniform_int_distribution<size_t> randomNoneTermSelector{
        0, noneTerminalNodes_.size() - 1};
    std::uniform_int_distribution<size_t> randomTermSelector{
        0, terminalNodes_.size() - 1};

    auto makeTermial = [&]() {
      return terminalNodes_[randomTermSelector(rnd_)];
    };
    auto makeNoneTermial = [&]() {
      return noneTerminalNodes_[randomNoneTermSelector(rnd_)];
    };

    VariantType rootNode = makeNoneTermial();
    
    NodeChildrenInsert ni{rootNode, makeTermial, makeNoneTermial, height(rnd_) - 1};

    boost::apply_visitor(
      ni,
      rootNode);
  
    return rootNode;
  }

 private:
  struct OrderNodesHelper {
    std::vector<VariantType> &terminalNodes;
    std::vector<VariantType> &noneTerminalNodes;

    template <class T>
    void operator()(T) {
      terminalNodes.push_back(T{});
    }

    template <class T>
    void operator()(boost::recursive_wrapper<T>) {
      noneTerminalNodes.push_back(T{});
    }
  };
  int minHeight_;
  int maxHeight_;
  std::vector<VariantType> terminalNodes_;
  std::vector<VariantType> noneTerminalNodes_;
  std::mt19937 rnd_;
};



}  // namespace gpm


