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

namespace gpm {

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
}  // namespace gpm
