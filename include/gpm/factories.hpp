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
