#include <boost/hana.hpp>
#include <boost/mp11.hpp>
#include <boost/type_index.hpp>
#include <boost/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <cassert>
#include <iostream>
#include <iterator>
#include <string_view>
#include <type_traits>

#include <gpm/io.hpp>
#include "../examples/ant/common/nodes.hpp"

// namespace hana = boost::hana;
// using namespace hana::literals;  // contains the _c suffix

template <uint8_t kMaxHash, typename BeginIterType, typename EndIterType>
constexpr uint8_t simpleHash(BeginIterType begin, EndIterType end) {
  uint8_t r = 0;
  for (; begin != end; ++begin) {
    r = (r + 7) ^ *begin;
  }
  return r & (kMaxHash - 1);
}

template <typename RangeType>
constexpr uint8_t simpleHash(RangeType range) {
  return simpleHash(std::begin(range), std::end(range));
}

template <uint8_t kMaxHash, typename... T>
constexpr int checkForColision(boost::hana::tuple<T...>) {
  using hashes = boost::mp11::mp_list<std::integral_constant<
      uint8_t,
      simpleHash<kMaxHash>(std::begin(T::name), std::end(T::name) - 1)>...>;

  using unique_hashes = boost::mp11::mp_unique<hashes>;

  using size_hashes = boost::mp11::mp_size<hashes>;
  using size_unique_hashes = boost::mp11::mp_size<unique_hashes>;

  static_assert(
      std::is_same_v<size_hashes, size_unique_hashes>,
      "colision detected in hash function, please change hash function");
  return 0;
}

template <typename... T>
decltype(auto) variantToTuple(boost::variant<T...>) {
  return boost::hana::tuple<typename boost::unwrap_recursive<T>::type...>{};
}

template <typename VariantType, typename CursorType, uint8_t kMaxHash>
class FactoryV2 {
  using VariantTypeCreateFunction =
      std::add_pointer_t<VariantType(CursorType&)>;

  static std::array<VariantTypeCreateFunction, kMaxHash> makeHashToNode() {
    std::array<VariantTypeCreateFunction, kMaxHash> creatFunctionLUT{nullptr};
    auto nodesAsTuple = variantToTuple(VariantType{});
    checkForColision<kMaxHash>(nodesAsTuple);
    boost::hana::for_each(nodesAsTuple, [&creatFunctionLUT](auto n) {
      using NodeType = decltype(n);
      auto hash =
          simpleHash<kMaxHash>(std::begin(n.name), std::end(n.name) - 1);

      creatFunctionLUT[hash] = [](CursorType& tokenCursor) -> VariantType {
        auto node = NodeType{};
        for (auto& child : node.children) {
          tokenCursor.next();
          child = FactoryV2<VariantType, CursorType, kMaxHash>::factory(
              tokenCursor);
        }
        return node;
      };
    });
    return creatFunctionLUT;
  }

  template <typename TokenType>
  static VariantTypeCreateFunction getCreateFuntion(TokenType token) {
    static std::array<VariantTypeCreateFunction, kMaxHash> creatFunctionLUT =
        makeHashToNode();

    return creatFunctionLUT[simpleHash<kMaxHash>(token.begin(), token.end())];
  }

 public:
  static VariantType factory(CursorType& tokenCursor) {
    auto token = tokenCursor.token();
    return getCreateFuntion(token)(tokenCursor);
  }
};

// template <typename VariantType, typename CursorType>
// std::array<VariantTypeCreateFunction<VariantType, CursorType>, maxHash>
// HashToNode = makeHashToNode<VariantType, CursorType>(VariantType{});

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

namespace {
namespace detail {

// constexpr auto checkForColisionV =  checkForColision(ant::NodesVariant);

}
}  // namespace

constexpr int ctpow(int base, int iexp) {
  return iexp == 0 ? 1 : base * ctpow(base, iexp - 1);
}

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
  // //   ant::Prog3 p3{};
  // //   auto at = asTuple(ant::NodesVariant{});
  // //   boost::hana::for_each(at, [](auto & n){
  // //     std::cout << n.name << " " << std::hex << (int)cthash(n.name,
  // std::end(n.name)-1) << "\n";
  // //   });
  // //   std::cout << std::hex << (int)cthash(p3.name, std::end(p3.name)-1) <<
  // "\n";

  gpm::RPNTokenCursor optAntIter{"m l m if l l p3 m if l p3 m if"};
  //   std::string_view token{"if"};
  //   VariantTypeCreateFunction<ant::NodesVariant, gpm::RPNTokenCursor> f =
  //   [](gpm::RPNTokenCursor&) -> ant::NodesVariant {return ant::Prog3{};};
  //   auto tmp = f(ttokenCursor);

  constexpr auto maxHash = ctpow(2, 4);
  auto optAnt =
      FactoryV2<ant::NodesVariant, gpm::RPNTokenCursor, maxHash>::factory(
          optAntIter);
  std::cout << boost::apply_visitor(gpm::RPNPrinter<std::string>{}, optAnt);
  //   boost::apply_visitor([](auto const & obj){
  //     std::cout << boost::typeindex::type_id<decltype(obj)>().pretty_name()
  //     << "\n";
  //   },
  //   foo
  //   );
  //
}
