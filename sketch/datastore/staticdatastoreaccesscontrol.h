/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include "staticdatastore.h"

namespace sds {
namespace detail {
template <typename KeyType, KeyType... keys>
struct AccessControlList {
  constexpr static size_t keysSize_ = sizeof...(keys);
  constexpr static KeyType keys_[] = {
      keys...,
  };
};

template <typename KeyType, KeyType toSearch, typename ACL,
          size_t index = ACL::keysSize_ - 1>
struct AccessControlListSearch {
  constexpr static bool hasKey =
      (toSearch == ACL::keys_[index]) ||
      AccessControlListSearch<KeyType, toSearch, ACL, index - 1>::hasKey;
};

template <typename KeyType, KeyType toSearch, typename ACL>
struct AccessControlListSearch<KeyType, toSearch, ACL, 0> {
  constexpr static bool hasKey = toSearch == ACL::keys_[0];
};

template <typename Who>
struct AccessControlListMap {};
}  // namespace detail

template <typename T>
struct BaseType {
  using type = typename std::remove_cv<typename std::remove_pointer<
      typename std::remove_reference<T>::type>::type>::type;
};

namespace ac {

template <KeyTag keyTag, typename Accessor, class... Args>
auto get([[gnu::unused]] Accessor const* thisPtr,
         [[gnu::unused]] std::tuple<Args...>& t)
    -> boost::optional<typename std::tuple_element<
        size_t(keyTag),
        typename std::decay<std::tuple<Args...>>::type>::type::Type>& {
  static_assert(
      detail::AccessControlListSearch<
          KeyTag, keyTag,
          typename detail::AccessControlListMap<
              typename std::remove_cv<Accessor>::type>::allowed>::hasKey,
      "the class is not allowed to access this key in datastore");
  return std::get<size_t(keyTag)>(t).value;
}

template <KeyTag keyTag, typename Accessor, class... Args>
auto get([[gnu::unused]] Accessor const* thisPtr,
         [[gnu::unused]] std::tuple<Args...> const& t)
    -> boost::optional<typename std::tuple_element<
        size_t(keyTag),
        typename std::decay<std::tuple<Args...>>::type>::type::Type> const& {
  static_assert(
      detail::AccessControlListSearch<
          KeyTag, keyTag,
          typename detail::AccessControlListMap<
              typename std::remove_cv<Accessor>::type>::allowed>::hasKey,
      "the class is not allowed to access this key in datastore");
  return std::get<size_t(keyTag)>(t).value;
}

template <typename Accessor, KeyTag keyTag, class... Args>
auto get([[gnu::unused]] std::tuple<Args...>& t)
    -> boost::optional<typename std::tuple_element<
        size_t(keyTag),
        typename std::decay<std::tuple<Args...>>::type>::type::Type>& {
  static_assert(detail::AccessControlListSearch<
                    KeyTag, keyTag,
                    typename detail::AccessControlListMap<
                        typename BaseType<Accessor>::type>::allowed>::hasKey,
                "the class is not allowed to access this key in datastore");
  return std::get<size_t(keyTag)>(t).value;
}

template <typename Accessor, KeyTag keyTag, class... Args>
auto get([[gnu::unused]] std::tuple<Args...> const& t)
    -> boost::optional<typename std::tuple_element<
        size_t(keyTag),
        typename std::decay<std::tuple<Args...>>::type>::type::Type> const& {
  static_assert(detail::AccessControlListSearch<
                    KeyTag, keyTag,
                    typename detail::AccessControlListMap<
                        typename BaseType<Accessor>::type>::allowed>::hasKey,
                "the class is not allowed to access this key in datastore");
  return std::get<size_t(keyTag)>(t).value;
}
}  // namespace ac
}  // namespace sds
#define EA_SETACCESS(Accessor, ...)                         \
  namespace sds {                                           \
  namespace detail {                                        \
  template <>                                               \
  struct AccessControlListMap<Accessor> {                   \
    using allowed = AccessControlList<KeyTag, __VA_ARGS__>; \
  };                                                        \
  }                                                         \
  }

namespace MyLib {
class MyClass;
}
EA_SETACCESS(MyLib::MyClass, sds::KeyTag::AntDirection, sds::KeyTag::AntPos)

#undef EA_SETACCESS
