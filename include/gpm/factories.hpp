/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include <functional>
#include <string_view>
#include <vector>

#include <boost/assert.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/mp11.hpp>
#include <boost/variant.hpp>

#include <boost/hana.hpp>

namespace gpm {

namespace detail {
template <typename VariantType, typename CursorType>
VariantType factory_imp(CursorType &);

template <typename VariantType, typename CursorType>
using FactoryMap =
    boost::container::flat_map<std::string_view,
                               std::function<VariantType(CursorType &)>>;

template <typename VariantType, typename CursorType>
struct FactoryMapInsertHelper {
  FactoryMap<VariantType, CursorType> &factoryMap;

  template <class T>
  void operator()(T) {
    factoryMap[T::name] = [](CursorType &tokenCursor) {
      T ret;
      if constexpr (ret.children.size() != 0)
        for (auto &n : ret.children)
          n = factory_imp<VariantType>(tokenCursor.next());
      return ret;
    };
  }

  template <class T>
  void operator()(boost::recursive_wrapper<T>) {
    factoryMap[T::name] = [](CursorType &tokenCursor) {
      T ret;
      if constexpr (ret.children.size() != 0)
        for (auto &n : ret.children)
          n = factory_imp<VariantType>(tokenCursor.next());
      return ret;
    };
  }
};

template <typename VariantType, typename CursorType>
FactoryMap<VariantType, CursorType> makeFactoryMap() {
  FactoryMap<VariantType, CursorType> factoryMap;
  auto insertHelper =
      FactoryMapInsertHelper<VariantType, CursorType>{factoryMap};
  boost::mp11::mp_for_each<VariantType>(insertHelper);
  return factoryMap;
}

template <typename VariantType, typename CursorType>
VariantType factory_imp(CursorType &tokenCursor) {
  static auto nodeCreateFunMap = makeFactoryMap<VariantType, CursorType>();
  auto token = tokenCursor.token();
  BOOST_ASSERT_MSG(nodeCreateFunMap.count(token) > 0,
                   "can not find factory function for token");

  return nodeCreateFunMap[token](tokenCursor);
}
}  // namespace detail

template <typename VariantType, typename CursorType>
VariantType factory(CursorType tokenCursor) {
  return detail::factory_imp<VariantType>(tokenCursor);
}
}  // namespace gpm

namespace gpm::experimental {
constexpr uint8_t cthash(char const *begin, char const *end) {
  uint8_t r = 0;
  while (begin < end) r = r ^ *begin++;
  return r & 0b1111'1111;
}

constexpr auto maxHash = 2 ^ 5;

template <typename... T>
decltype(auto) asTuple(boost::variant<T...>) {
  return boost::hana::tuple<typename boost::unwrap_recursive<T>::type...>{};
}

template <typename VariantType, typename CursorType>
class FactoryV2 {
  using VariantTypeCreateFunction =
      std::add_pointer_t<VariantType(CursorType &)>;

  static inline std::array<VariantTypeCreateFunction, maxHash> nodeFactoryField{
      nullptr};

  static int makeHashToNode() {
    auto tup = asTuple(VariantType{});
    boost::hana::for_each(tup, [](auto n) {
      using NodeType = decltype(n);
      auto ind = cthash(n.name, std::end(n.name) - 1);
      nodeFactoryField[ind] = [](CursorType &tokenCursor) -> VariantType {
        auto node = NodeType{};
        for (auto &children : node.children) {
          tokenCursor.next();
          auto token = tokenCursor.token();
          auto index = cthash(std::begin(token), std::end(token));
          children = nodeFactoryField[index](tokenCursor);
        }
        return node;
      };
    });
    return 0;
  }

 public:
  static VariantType factory(CursorType tokenCursor) {
    static int dummyValue = makeHashToNode();
    (void)dummyValue;
    auto token = tokenCursor.token();
    auto index = cthash(std::begin(token), std::end(token));
    return nodeFactoryField[index](tokenCursor);
  }
};
}  // namespace gpm::experimental
