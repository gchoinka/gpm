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
  template<uint8_t kMaxHash, typename BeginIterType, typename EndIterType>
  constexpr uint8_t simpleHash(BeginIterType begin, EndIterType end) {
    uint8_t r = 0;
    while (begin != end) r = r ^ *begin++;
    return r & (kMaxHash-1);
  }
  
  
  template<typename RangeType>
  constexpr uint8_t simpleHash(RangeType range) {
    return simpleHash(std::begin(range), std::end(range));
  }
  
  template<uint8_t kMaxHash, typename ...T>
  constexpr int checkForColision(boost::hana::tuple<T...>)
  {
    using hashes = boost::mp11::mp_list<std::integral_constant<uint8_t,
    simpleHash<kMaxHash>(std::begin(T::name), std::end(T::name)-1)>...>;
    
    using unique_hashes = boost::mp11::mp_unique<hashes>;
    
    using size_hashes = boost::mp11::mp_size<hashes>;
    using size_unique_hashes = boost::mp11::mp_size<unique_hashes>;
    
    static_assert(std::is_same_v<size_hashes, size_unique_hashes>,
                  "colision detected in hash function, please change hash function");
    return 0;
  }
  
  template <typename... T>
  decltype(auto) variantToTuple(boost::variant<T...>) {
    return boost::hana::tuple<typename boost::unwrap_recursive<T>::type...>{};
  }
  
  template <typename VariantType, typename CursorType, uint8_t kMaxHash>
  class FactoryV2 {
    
    using VariantTypeCreateFunction = std::add_pointer_t<VariantType(CursorType&)>;
    
    static std::array<VariantTypeCreateFunction, kMaxHash> makeHashToNode() {
      std::array<VariantTypeCreateFunction, kMaxHash> creatFunctionLUT{nullptr};
      auto nodesAsTuple = variantToTuple(VariantType{});
      checkForColision<kMaxHash>(nodesAsTuple);
      boost::hana::for_each(nodesAsTuple, [&creatFunctionLUT](auto n) {
        using NodeType = decltype(n);
        auto hash = simpleHash<kMaxHash>(std::begin(n.name), std::end(n.name) - 1);
        
        creatFunctionLUT[hash] = [](CursorType& tokenCursor) -> VariantType {
          auto node = NodeType{};
          for (auto& child : node.children) {
            tokenCursor.next();
            child = FactoryV2<VariantType,CursorType,kMaxHash>::factory(tokenCursor);
          }
          return node;
        };
      });
      return creatFunctionLUT;
    }
    
    
    template<typename TokenType>
    static VariantTypeCreateFunction getCreateFuntion(TokenType token){
      static std::array<VariantTypeCreateFunction, kMaxHash> creatFunctionLUT = makeHashToNode();
      
      return creatFunctionLUT[simpleHash<kMaxHash>(token.begin(), token.end())];
    }
    
  public:
    static VariantType factory(CursorType tokenCursor) {
      auto token = tokenCursor.token();
      return getCreateFuntion(token)(tokenCursor);
    }
  };
}  // namespace gpm::experimental
