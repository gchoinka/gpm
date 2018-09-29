#include <iostream>
#include <boost/hana.hpp>
#include <boost/mp11.hpp>
#include <string_view>
#include <type_traits>


namespace hana = boost::hana;
using namespace hana::literals;  // contains the _c suffix

template<auto ...ch>
struct Base{
  static constexpr char name[] = {ch..., '\0'};
};

struct M : public Base<'m'> {};
struct L : public Base<'l'> {};
struct R : public Base<'r'> {};
struct IF : public Base<'i','f'> {};
struct P2 : public Base<'p', '2'> {};
struct P3 : public Base<'p', '3'> {};
struct Colision : public Base<'a', 'Z'> {};
struct Colision0 : public Base<'p', 'K'> {};


template<auto ch0, auto ...ch>
constexpr uint8_t cthash(Base<ch0, ch...>)
{  
  return ch0 ^ cthash(Base<ch...>{});
}


constexpr uint8_t cthash( Base<>  )
{  
  return 0;
}


constexpr auto tup = hana::tuple<M, L, R, IF, P2, P3>{};


namespace {
namespace detail {
template<typename ...T>
constexpr int checkForColision(hana::tuple<T...>)
{
  using hashes = boost::mp11::mp_list<std::integral_constant<uint8_t, cthash(T{})>...>;
  
  using unique_hashes = boost::mp11::mp_unique<hashes>;
  
  using size_hashes = boost::mp11::mp_size<hashes>;
  using size_unique_hashes = boost::mp11::mp_size<unique_hashes>;
 
  static_assert(std::is_same_v<size_hashes, size_unique_hashes>, "colision detected in hash function, please change hash function");
  return 0;
}

constexpr auto checkForColisionV =  checkForColision(tup);

}
}


int main()
{

  boost::hana::for_each(tup, [](auto & n){
    std::cout << n.name << " " << std::hex << (int)cthash(n) << "\n";
  });
}
