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


namespace gpm::experimental
{
  constexpr uint8_t cthash(char const * begin, char const * end)
  {
    uint8_t r = 0;
    while( begin < end)
      r = r ^ *begin++;
    return r & 0b0001'1111;
  }
  
  constexpr auto maxHash = 2^5;
  
  
  
  template<typename ...T>
  decltype(auto) asTuple(boost::variant<T...>)
  {
    return boost::hana::tuple<typename boost::unwrap_recursive<T>::type...>{};
  }
  
  
  template<typename VariantType, typename IterType>
  class FactoryV2
  {
    
    using VariantTypeCreateFunction = std::add_pointer_t<VariantType(IterType&)>;
    
    static inline std::array<VariantTypeCreateFunction, maxHash> nodeFactoryField;
    
    static int makeHashToNode()
    {
      auto tup = asTuple(VariantType{});
      boost::hana::for_each(tup, [](auto n){
        using NodeType = decltype(n);
        nodeFactoryField[cthash(n.name, std::end(n.name) - 1)] = [](IterType & iter) -> VariantType  {
          auto node = NodeType{};
          for(auto & children: node.children)
          {
            ++iter;
            auto token = *iter;
            children = nodeFactoryField[cthash(std::begin(token), std::end(token))](iter);
          }
          return node; 
        };
      });
      return 0;
    }
    
    

    
  public:
    static VariantType factory(IterType iter)
    {
      static int dummyValue = makeHashToNode();
      (void)dummyValue;
      auto token = *iter;
      return nodeFactoryField[cthash(token.begin(), token.end())](iter);
    }
    
  };
}
