#include <iostream>
#include <boost/hana.hpp>
#include <boost/mp11.hpp>
#include <string_view>
#include <type_traits>
#include <boost/variant.hpp>
#include <boost/type_index.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <iterator>

#include "../examples/ant/common/nodes.hpp"
#include <gpm/io.hpp>

namespace hana = boost::hana;
using namespace hana::literals;  // contains the _c suffix



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
  return hana::tuple<typename boost::unwrap_recursive<T>::type...>{};
}

template<typename VariantType, typename IterType>
using VariantTypeCreateFunction = std::add_pointer_t<VariantType(IterType&)>;

template<typename VariantType, typename IterType>
std::array<VariantTypeCreateFunction<VariantType, IterType>, maxHash> nodeFactoryField;



template <typename VariantType, typename Iter>
VariantType ffactory_imp(Iter &);



template<typename VariantType, typename IterType>
decltype(auto) makeHashToNode(VariantType vt)
{
  auto tup = asTuple(vt);
  boost::hana::for_each(tup, [](auto n){
    using NodeType = decltype(n);
    nodeFactoryField<VariantType, IterType>[cthash(n.name, std::end(n.name) - 1)] = [](IterType & iter) -> VariantType  {
      auto node = NodeType{};
      std::cout << ">>"<<*iter<<"<<" << boost::typeindex::type_id<NodeType>().pretty_name() <<"\n";
      for(auto & children: node.children)
      {
        ++iter;
        auto token = *iter;
        std::cout << ">>"<<token<<"<<"<<"\n";
        children = nodeFactoryField<VariantType, IterType>[cthash(std::begin(token), std::end(token))](iter);
      }
      return node; 
      
    };
  });
  return 0;
}



// template <typename VariantType, typename IterType>
// std::array<VariantTypeCreateFunction<VariantType, IterType>, maxHash> HashToNode = makeHashToNode<VariantType, IterType>(VariantType{});
  
  
  
//   template <class T>
//   void operator()(T) {
//     factoryMap[T::name] = [](Iter &tokenIter) {
//       T ret;
//       if constexpr (ret.children.size() != 0)
//         for (auto &n : ret.children) n = factory_imp<VariantType>(++tokenIter);
//         return ret;
//     };
//   }
//   
//   template <class T>
//   void operator()(boost::recursive_wrapper<T>) {
//     factoryMap[T::name] = [](Iter &tokenIter) {
//       T ret;
//       if constexpr (ret.children.size() != 0)
//         for (auto &n : ret.children) n = factory_imp<VariantType>(++tokenIter);
//         return ret;
//     };
//   }
// };

// constexpr auto tup = hana::tuple<M, L, R, IF, P2, P3>{};


// namespace {
//   namespace detail {
//     template<typename ...T>
//     constexpr int checkForColision(boost::variant<T...>)
//     {
//       using hashes = boost::mp11::mp_list<std::integral_constant<uint8_t, cthash(T::name, std::end(T::name)-1)>...>;
//       
//       using unique_hashes = boost::mp11::mp_unique<hashes>;
//       
//       using size_hashes = boost::mp11::mp_size<hashes>;
//       using size_unique_hashes = boost::mp11::mp_size<unique_hashes>;
//       
//       static_assert(std::is_same_v<size_hashes, size_unique_hashes>, "colision detected in hash function, please change hash function");
//       return 0;
//     }
//     
//     constexpr auto checkForColisionV =  checkForColision(ant::ant_nodes);
//     
//   }
// }

ant::ant_nodes pp(gpm::RPNToken_iterator &){ return ant::prog3{}; }



int main()
{
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
// //     std::cout << n.name << " " << std::hex << (int)cthash(n.name, std::end(n.name)-1) << "\n";
// //   });
// //   std::cout << std::hex << (int)cthash(p3.name, std::end(p3.name)-1) << "\n";
  
  gpm::RPNToken_iterator titer{"m l m if l l p3 m if l p3 m if"};
//   std::string_view token{"if"};
//   VariantTypeCreateFunction<ant::ant_nodes, gpm::RPNToken_iterator> f = [](gpm::RPNToken_iterator&) -> ant::ant_nodes {return ant::prog3{};};
//   auto tmp = f(titer);
  auto token = *titer;
  makeHashToNode<ant::ant_nodes, gpm::RPNToken_iterator> (ant::ant_nodes{});
  auto foo = nodeFactoryField<ant::ant_nodes, gpm::RPNToken_iterator>[cthash(token.begin(), token.end())](titer);
  std::cout << boost::apply_visitor(gpm::RPNPrinter<std::string>{}, foo);
//   boost::apply_visitor([](auto const & obj){
//     std::cout << boost::typeindex::type_id<decltype(obj)>().pretty_name() << "\n";
//   },
//   foo
//   );
//   
}
