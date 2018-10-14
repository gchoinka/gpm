#include <boost/hana.hpp>
#include <boost/mp11.hpp>
#include <boost/type_index.hpp>
#include <boost/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <iostream>
#include <iterator>
#include <string_view>
#include <type_traits>

#include <gpm/io.hpp>
#include "../examples/ant/common/nodes.hpp"

namespace hana = boost::hana;
using namespace hana::literals;  // contains the _c suffix

constexpr uint8_t cthash(char const* begin, char const* end) {
  uint8_t r = 0;
  while (begin < end) r = r ^ *begin++;
  return r & 0b0001'1111;
}

constexpr auto maxHash = 2 ^ 5;

template <typename... T>
decltype(auto) asTuple(boost::variant<T...>) {
  return hana::tuple<typename boost::unwrap_recursive<T>::type...>{};
}

template <typename VariantType, typename IterType>
class FactoryV2 {
  using VariantTypeCreateFunction = std::add_pointer_t<VariantType(IterType&)>;

  static std::array<VariantTypeCreateFunction, maxHash> makeHashToNode() {
    std::array<VariantTypeCreateFunction, maxHash> result;
    auto tup = asTuple(VariantType{});
    boost::hana::for_each(tup, [&result](auto n) {
      using NodeType = decltype(n);
      result[cthash(n.name, std::end(n.name) - 1)] =
          [](IterType& iter) -> VariantType {
        auto node = NodeType{};
        for (auto& children : node.children) {
          ++iter;
          auto token = *iter;
          children =
              nodeFactoryField[cthash(std::begin(token), std::end(token))](
                  iter);
        }
        return node;
      };
    });
    return result;
  }

  static inline std::array<VariantTypeCreateFunction, maxHash>
      nodeFactoryField = makeHashToNode();

 public:
  static VariantType factory(IterType& iter) {
    auto token = *iter;
    return nodeFactoryField[cthash(token.begin(), token.end())](iter);
  }
};

// template <typename VariantType, typename IterType>
// std::array<VariantTypeCreateFunction<VariantType, IterType>, maxHash>
// HashToNode = makeHashToNode<VariantType, IterType>(VariantType{});

//   template <class T>
//   void operator()(T) {
//     factoryMap[T::name] = [](Iter &tokenIter) {
//       T ret;
//       if constexpr (ret.children.size() != 0)
//         for (auto &n : ret.children) n =
//         factory_imp<VariantType>(++tokenIter); return ret;
//     };
//   }
//
//   template <class T>
//   void operator()(boost::recursive_wrapper<T>) {
//     factoryMap[T::name] = [](Iter &tokenIter) {
//       T ret;
//       if constexpr (ret.children.size() != 0)
//         for (auto &n : ret.children) n =
//         factory_imp<VariantType>(++tokenIter); return ret;
//     };
//   }
// };

// constexpr auto tup = hana::tuple<M, L, R, IF, P2, P3>{};

// namespace {
//   namespace detail {
//     template<typename ...T>
//     constexpr int checkForColision(boost::variant<T...>)
//     {
//       using hashes = boost::mp11::mp_list<std::integral_constant<uint8_t,
//       cthash(T::name, std::end(T::name)-1)>...>;
//
//       using unique_hashes = boost::mp11::mp_unique<hashes>;
//
//       using size_hashes = boost::mp11::mp_size<hashes>;
//       using size_unique_hashes = boost::mp11::mp_size<unique_hashes>;
//
//       static_assert(std::is_same_v<size_hashes, size_unique_hashes>,
//       "colision detected in hash function, please change hash function");
//       return 0;
//     }
//
//     constexpr auto checkForColisionV =  checkForColision(ant::ant_nodes);
//
//   }
// }

int main() {
  //   auto va = toVariant(tup);
  //
  //   boost::hana::for_each(tup, [](auto & n){
  //     std::cout << n.name << " " << std::hex << (int)cthash(n) << "\n";
  //   });
  //
  //   boost::apply_visitor([](auto & n){
  //     std::cout << n.name << "\n";
  //     return 0;
  //   }, hashToObject[cthash(Base<'p', '3'>{})]);
  // //   ant::prog3 p3{};
  // //   auto at = asTuple(ant::ant_nodes{});
  // //   boost::hana::for_each(at, [](auto & n){
  // //     std::cout << n.name << " " << std::hex << (int)cthash(n.name,
  // std::end(n.name)-1) << "\n";
  // //   });
  // //   std::cout << std::hex << (int)cthash(p3.name, std::end(p3.name)-1) <<
  // "\n";

  gpm::RPNToken_iterator optAntIter{"m l m if l l p3 m if l p3 m if"};
  //   std::string_view token{"if"};
  //   VariantTypeCreateFunction<ant::ant_nodes, gpm::RPNToken_iterator> f =
  //   [](gpm::RPNToken_iterator&) -> ant::ant_nodes {return ant::prog3{};};
  //   auto tmp = f(titer);

  auto optAnt =
      FactoryV2<ant::ant_nodes, gpm::RPNToken_iterator>::factory(optAntIter);
  std::cout << boost::apply_visitor(gpm::RPNPrinter<std::string>{}, optAnt);
  //   boost::apply_visitor([](auto const & obj){
  //     std::cout << boost::typeindex::type_id<decltype(obj)>().pretty_name()
  //     << "\n";
  //   },
  //   foo
  //   );
  //
}
