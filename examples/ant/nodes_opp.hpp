/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once

#include <array>
#include <memory>
#include <string_view>
#include <utility>

#include <boost/container/flat_map.hpp>
#include <boost/mp11.hpp>

namespace antoop {

template <typename ContexType>
class BaseNode {
 public:
  virtual void operator()(ContexType &contex) const = 0;
  virtual ~BaseNode() = 0;
};

template <typename ContexType>
BaseNode<ContexType>::~BaseNode() {}

template <typename ContexType, int ChildrenCount, char... CTName>
class BaseNodeWithChildren : public BaseNode<ContexType> {
 public:
  static constexpr char const name[] = {CTName..., '\0'};
  template <typename... Args>
  BaseNodeWithChildren(Args &&... args) : children_{std::move(args)...} {}

  std::array<std::unique_ptr<BaseNode<ContexType>>, ChildrenCount> children_;
};

template <typename ContexType>
class Prog3 : public BaseNodeWithChildren<ContexType, 3, 'p', '3'> {
 public:
  using BaseNodeWithChildren<ContexType, 3, 'p', '3'>::BaseNodeWithChildren;

  virtual void operator()(ContexType &contex) const override {
    for (auto &c : BaseNodeWithChildren<ContexType, 3, 'p', '3'>::children_)
      (*c)(contex);
  }
};

template <typename ContexType>
class Prog2 : public BaseNodeWithChildren<ContexType, 2, 'p', '2'> {
 public:
  using BaseNodeWithChildren<ContexType, 2, 'p', '2'>::BaseNodeWithChildren;

  virtual void operator()(ContexType &contex) const override {
    for (auto &c : BaseNodeWithChildren<ContexType, 2, 'p', '2'>::children_)
      (*c)(contex);
  }
};

template <typename ContexType>
class IfFoodAhead : public BaseNodeWithChildren<ContexType, 2, 'i', 'f'> {
 public:
  using BaseNodeWithChildren<ContexType, 2, 'i', 'f'>::BaseNodeWithChildren;

  virtual void operator()(ContexType &contex) const override {
    if (contex.is_food_in_front())
      (*(BaseNodeWithChildren<ContexType, 2, 'i', 'f'>::children_[0]))(contex);
    else
      (*(BaseNodeWithChildren<ContexType, 2, 'i', 'f'>::children_[1]))(contex);
  }
};

template <typename ContexType>
class Move : public BaseNodeWithChildren<ContexType, 0, 'm'> {
 public:
  virtual void operator()(ContexType &contex) const override { contex.move(); }
};

template <typename ContexType>
class Left : public BaseNodeWithChildren<ContexType, 0, 'l'> {
 public:
  virtual void operator()(ContexType &contex) const override { contex.left(); }
};

template <typename ContexType>
class Right : public BaseNodeWithChildren<ContexType, 0, 'r'> {
 public:
  virtual void operator()(ContexType &contex) const override { contex.right(); }
};

namespace detail {
template <typename ContexType, typename CursorType>
std::unique_ptr<BaseNode<ContexType>> factory_imp(CursorType &);

template <typename ContexType, typename CursorType>
using FactoryMap = boost::container::flat_map<
    std::string_view,
    std::function<std::unique_ptr<BaseNode<ContexType>>(CursorType &)>>;

template <typename ContexType, typename CursorType>
struct FactoryMapInsertHelper {
  FactoryMap<ContexType, CursorType> &factoryMap;

  template <class T>
  void operator()(T) {
    factoryMap[T::name] = [](CursorType &tokenCursor) {
      auto ret = std::make_unique<T>();

      for (auto &n : ret->children_)
        n = std::move(factory_imp<ContexType>(tokenCursor.next()));
      return std::move(ret);
    };
  }
};

template <typename ContexType, typename CursorType>
FactoryMap<ContexType, CursorType> makeFactoryMap() {
  FactoryMap<ContexType, CursorType> factoryMap;
  auto insertHelper =
      FactoryMapInsertHelper<ContexType, CursorType>{factoryMap};
  boost::mp11::mp_for_each<boost::mp11::mp_list<
      Prog3<ContexType>, Prog2<ContexType>, IfFoodAhead<ContexType>,
      Move<ContexType>, Left<ContexType>, Right<ContexType>>>(insertHelper);
  return factoryMap;
}

template <typename ContexType, typename CursorType>
std::unique_ptr<BaseNode<ContexType>> factory_imp(CursorType &tokenCursor) {
  static auto nodeCreateFunMap = makeFactoryMap<ContexType, CursorType>();
  auto token = tokenCursor.token();
//   if (!nodeCreateFunMap.count(token)) {
//     throw std::runtime_error{
//         std::string{"cant find factory function for token >>"} +
//         std::string{token} + "<<"};
//   }

  return nodeCreateFunMap[token](tokenCursor);
}
}  // namespace detail

template <typename ContexType, typename CursorType>
std::unique_ptr<BaseNode<ContexType>> factory(CursorType tokenCursor) {
  return detail::factory_imp<ContexType>(tokenCursor);
}
}  // namespace antoop
