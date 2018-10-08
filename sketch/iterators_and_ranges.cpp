#include <iostream>
#include <iterator>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <vector>
#include <regex>
#include <fmt/printf.h>

namespace detail {
template <typename IterTupleT, auto... Idx>
constexpr bool isSameInput_imp(IterTupleT iterTuple,
                               std::index_sequence<Idx...>) {
  auto findValidCharMindingBackspace = [](auto beginIter, auto endIter) {
    int backspaceCount = 0;
    auto it = beginIter;
    for (; it != endIter && *it == '\b'; ++it) ++backspaceCount;

    while (it != endIter && backspaceCount--) ++it;
    return it;
  };

  auto walkIters = std::make_tuple(std::get<Idx * 2 + 0>(iterTuple)...);
  auto endIters = std::make_tuple(std::get<Idx * 2 + 1>(iterTuple)...);

  while (true) {
    (..., (std::get<Idx>(walkIters) = findValidCharMindingBackspace(
               std::get<Idx>(walkIters), std::get<Idx>(endIters))));

    bool allIterAtEnd =
        (... && (std::get<Idx>(walkIters) == std::get<Idx>(endIters)));
    if (allIterAtEnd) return true;

    bool someIterAtEnd =
        (... || (std::get<Idx>(walkIters) == std::get<Idx>(endIters)));
    if (someIterAtEnd) return false;

    auto currentChar = *std::get<0>(walkIters);
    bool allPointingToSameChar =
        (... && (*std::get<Idx>(walkIters) == currentChar));
    if (!allPointingToSameChar) return false;

    (++std::get<Idx>(walkIters), ...);
  }

  return false;
}

template <typename TupleT, auto... Idx>
constexpr auto convertToReverseIter(TupleT t, std::index_sequence<Idx...>) {
  return std::tuple_cat(
      std::make_tuple(std::make_reverse_iterator(std::get<Idx * 2 + 1>(t)),
                      std::make_reverse_iterator(std::get<Idx * 2 + 0>(t)))...);
}

template <typename TupleT0, typename TupleT1, auto... Idx>
constexpr auto interleaveTuple(TupleT0 t0, TupleT1 t1,
                               std::index_sequence<Idx...>) {
  return std::tuple_cat(
      std::make_tuple(std::get<Idx>(t0), std::get<Idx>(t1))...);
}

template <typename TupleT0, typename TupleT1>
constexpr auto interleaveTuple(TupleT0 t0, TupleT1 t1) {
  return interleaveTuple(
      t0, t1, std::make_index_sequence<std::tuple_size_v<TupleT0>>());
}

}  // namespace detail

template <typename... IterT>
constexpr bool isSameInput(IterT... iters) {
  static_assert(sizeof...(IterT) >= 4,
                "Not enouth iterators pairs given, at least 2 pairs needed.");
  static_assert(
      (sizeof...(IterT) % 2) == 0,
      "Iterators count is odd, pairs of begin and end iterators needed.");
  auto itPack = detail::convertToReverseIter(
      std::make_tuple(iters...),
      std::make_index_sequence<sizeof...(IterT) / 2>());
  return detail::isSameInput_imp(
      itPack, std::make_index_sequence<sizeof...(IterT) / 2>());
}

template <typename... RangeT,
          typename std::enable_if<std::tuple_size<decltype(std::make_tuple(
                                      std::crbegin(RangeT{})...))>::value,
                                  int>::type = 0>
constexpr bool isSameInput(RangeT... range) {
  static_assert(sizeof...(range) >= 2,
                "Usefull result only posibible with at least 2 ranges");
  auto itPackBegins = std::make_tuple(std::crbegin(range)...);
  auto itPackEnds = std::make_tuple(std::crend(range)...);
  return detail::isSameInput_imp(
      detail::interleaveTuple(itPackBegins, itPackEnds),
      std::make_index_sequence<sizeof...(RangeT)>());
}


bool isSameBruteforce(std::string s1, std::string s2)
{
    auto removeBackspaces = [](std::string s){
      std::regex removeBackspacesRegExp {"^\b*|[^\b]\b"};
      std::size_t lengthBefore = 0;
      std::size_t lengthAfter = s.length();
      do
      {
        lengthBefore = lengthAfter; 
        s  = std::regex_replace(s, removeBackspacesRegExp, ""); 
        lengthAfter = s.length(); 
      }while(lengthBefore != lengthAfter);
      return s;
    };
    
    s1 = removeBackspaces(s1);
    s2 = removeBackspaces(s2);

    return s1 == s2;
}

int main() {
  using namespace std;
  constexpr auto seq1 = string_view{"\b\bas\b\bas\bc"};
  constexpr auto seq2 = string_view{"asas\b\b\b"};

  constexpr auto isSameIter = isSameInput(begin(seq1), end(seq1),
                                          begin(seq2), end(seq2));
  constexpr auto isSameRange = isSameInput(seq1, seq2);
  

  auto isSameBruteforceResult =  isSameBruteforce(std::string{seq1}, std::string{seq2});
  cout << isSameIter << "\n";
  cout << isSameRange << "\n";
  cout << isSameBruteforceResult << "\n";

  return 0;
}
