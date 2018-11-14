/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <array>
#include <dyno.hpp>
#include <functional>

#include <boost/container/flat_map.hpp>
#include <boost/mp11.hpp>

using namespace dyno::literals;

template <typename ContexT>
struct AntBoardSimEvaluable
    : decltype(dyno::requires("eval"_s = dyno::method<void(ContexT &) const>)) {
};

// Define how concrete types can fulfill that interface
template <typename ContexT, typename T>
auto const dyno::default_concept_map<AntBoardSimEvaluable<ContexT>, T> =
    dyno::make_concept_map("eval"_s = [](T const &self, ContexT &c) {
      self.eval(c);
    });

template <typename ContexT>
struct evalable {
  template <typename T>
  evalable(T x) : poly_{x} {}

  void eval(ContexT &c) const { poly_.virtual_("eval"_s)(c); }

 private:
  using Storage = dyno::shared_remote_storage;
  using VTable = dyno::vtable<dyno::remote<dyno::everything>>;
  dyno::poly<AntBoardSimEvaluable<ContexT>, Storage, VTable> poly_;
};

namespace antdyno {

template <typename ContexT>
struct Move {
  static constexpr char name[] = {"m"};
  std::array<evalable<ContexT>, 0> children;
  void eval(ContexT &c) const { c.move(); }
};

template <typename ContexT>
struct Left {
  static constexpr char name[] = {"l"};
  std::array<evalable<ContexT>, 0> children;
  void eval(ContexT &c) const { c.left(); }
};

template <typename ContexT>
struct Right {
  static constexpr char name[] = {"r"};
  std::array<evalable<ContexT>, 0> children;
  void eval(ContexT &c) const { c.right(); }
};

template <typename ContexT>
struct IfFood {
  static constexpr char name[] = {"if"};
  std::array<evalable<ContexT>, 2> children;
  IfFood(evalable<ContexT> &&trueCase, evalable<ContexT> &&falseCase)
      : children{trueCase, falseCase} {}
  IfFood() : children{Move<ContexT>{}, Move<ContexT>{}} {}

  void eval(ContexT &c) const {
    if (c.is_food_in_front())
      children[0].eval(c);
    else
      children[1].eval(c);
  }
};

template <typename ContexT>
struct Prog2 {
  static constexpr char name[] = {"p2"};
  std::array<evalable<ContexT>, 2> children;
  Prog2(evalable<ContexT> &&child0, evalable<ContexT> &&child1)
      : children{child0, child1} {}
  Prog2() : children{Move<ContexT>{}, Move<ContexT>{}} {}

  void eval(ContexT &c) const {
    children[0].eval(c);
    children[1].eval(c);
  }
};

template <typename ContexT>
struct Prog3 {
  static constexpr char name[] = {"p3"};
  std::array<evalable<ContexT>, 3> children;
  Prog3(evalable<ContexT> &&child0, evalable<ContexT> &&child1,
        evalable<ContexT> &&child2)
      : children{child0, child1, child2} {}
  Prog3() : children{Move<ContexT>{}, Move<ContexT>{}, Move<ContexT>{}} {}

  void eval(ContexT &c) const {
    children[0].eval(c);
    children[1].eval(c);
    children[2].eval(c);
  }
};

namespace detail {
template <typename ContexT, typename CursorType>
evalable<ContexT> factory_imp(CursorType &);

template <typename ContexT, typename CursorType>
using FactoryMap =
    boost::container::flat_map<std::string_view,
                               std::function<evalable<ContexT>(CursorType &)>>;

template <typename ContexT, typename CursorType>
struct FactoryMapInsertHelper {
  FactoryMap<ContexT, CursorType> &factoryMap;

  template <class T>
  void operator()(T) {
    factoryMap[T::name] = [](CursorType &tokenCursor) {
      auto ret = T{};

      for (auto &n : ret.children) n = factory_imp<ContexT>(tokenCursor.next());
      return ret;
    };
  }
};

template <typename ContexT, typename CursorType>
FactoryMap<ContexT, CursorType> makeFactoryMap() {
  FactoryMap<ContexT, CursorType> factoryMap;
  auto insertHelper = FactoryMapInsertHelper<ContexT, CursorType>{factoryMap};
  boost::mp11::mp_for_each<
      boost::mp11::mp_list<IfFood<ContexT>, Prog2<ContexT>, Prog3<ContexT>,
                           Move<ContexT>, Left<ContexT>, Right<ContexT>>>(
      insertHelper);
  return factoryMap;
}

template <typename ContexT, typename CursorType>
evalable<ContexT> factory_imp(CursorType &tokenCursor) {
  static auto nodeCreateFunMap = makeFactoryMap<ContexT, CursorType>();
  auto token = tokenCursor.token();
  if (!nodeCreateFunMap.count(token)) {
    throw std::runtime_error{
        std::string{"cant find factory function for token >>"} +
        std::string{token} + "<<"};
  }

  return nodeCreateFunMap[token](tokenCursor);
}
}  // namespace detail

template <typename ContexT, typename CursorType>
evalable<ContexT> factory(CursorType tokenCursor) {
  return detail::factory_imp<ContexT>(tokenCursor);
}
}  // namespace antdyno
