#include <fmt/printf.h>
#include <algorithm>
#include <boost/range.hpp>
#include <iostream>
#include <iterator>
#include <regex>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <vector>

namespace detail {
template <typename IterTupleT, auto... Idx>
bool isSameInput_imp(IterTupleT iterTuple, std::index_sequence<Idx...>) {
  auto findNextCharMindingBackspace = [](auto iter, auto endIter) {
    int backspaceCount = 0;
    while (iter != endIter && (*iter == '\b' || backspaceCount > 0)) {
      for (; iter != endIter && *iter == '\b'; ++iter) ++backspaceCount;

      if (iter != endIter && backspaceCount > 0) {
        ++iter;
        --backspaceCount;
      }
    }
    return iter;
  };

  auto iters = std::make_tuple(std::get<Idx * 2 + 0>(iterTuple)...);
  auto endIters = std::make_tuple(std::get<Idx * 2 + 1>(iterTuple)...);

  while (true) {
    (..., (std::get<Idx>(iters) = findNextCharMindingBackspace(
               std::get<Idx>(iters), std::get<Idx>(endIters))));

    bool allItersAtEnd =
        (... && (std::get<Idx>(iters) == std::get<Idx>(endIters)));
    if (allItersAtEnd) return true;

    bool someItersAtEnd =
        (... || (std::get<Idx>(iters) == std::get<Idx>(endIters)));
    if (someItersAtEnd) return false;

    auto currentChar = *std::get<0>(iters);
    bool allPointingToSameChar =
        (... && (*std::get<Idx>(iters) == currentChar));
    if (!allPointingToSameChar) return false;

    (++std::get<Idx>(iters), ...);
  }

  return false;
}

template <typename TupleT, auto... Idx>
auto convertToReverseIter(TupleT t, std::index_sequence<Idx...>) {
  return std::tuple_cat(
      std::make_tuple(std::make_reverse_iterator(std::get<Idx * 2 + 1>(t)),
                      std::make_reverse_iterator(std::get<Idx * 2 + 0>(t)))...);
}

template <typename TupleT0, typename TupleT1, auto... Idx>
auto interleaveTuple(TupleT0 t0, TupleT1 t1, std::index_sequence<Idx...>) {
  return std::tuple_cat(
      std::make_tuple(std::get<Idx>(t0), std::get<Idx>(t1))...);
}

template <typename TupleT0, typename TupleT1>
auto interleaveTuple(TupleT0 t0, TupleT1 t1) {
  return interleaveTuple(
      t0, t1, std::make_index_sequence<std::tuple_size_v<TupleT0>>());
}

}  // namespace detail

template <typename... ItersT>
bool isSameInput(ItersT... iters) {
  static_assert(
      sizeof...(ItersT) >= 4,
      "Not enouth iterators pairs given, expecting 4 arguments (2 pairs) at "
      "least, e.g. isSameInput(v.begin(), v.end(), v2.begin(), v2.end())");
  static_assert((sizeof...(ItersT) % 2) == 0,
                "Iterators count is odd, this function need pairs of (begin, "
                "end) iterators.");
  auto itPack = detail::convertToReverseIter(
      std::make_tuple(iters...),
      std::make_index_sequence<sizeof...(ItersT) / 2>());
  return detail::isSameInput_imp(
      itPack, std::make_index_sequence<sizeof...(ItersT) / 2>());
}

template <typename... RangesT,
          typename std::enable_if<std::tuple_size<decltype(std::make_tuple(
                                      std::crbegin(RangesT{})...))>::value,
                                  int>::type = 0>
bool isSameInput(RangesT... ranges) {
  static_assert(sizeof...(RangesT) >= 2,
                "Usefull result only posibible with at least 2 ranges");
  auto itPackBegins = std::make_tuple(std::crbegin(ranges)...);
  auto itPackEnds = std::make_tuple(std::crend(ranges)...);
  return detail::isSameInput_imp(
      detail::interleaveTuple(itPackBegins, itPackEnds),
      std::make_index_sequence<sizeof...(RangesT)>());
}

bool isSameInputRegExp(std::string s1, std::string s2) {
  auto evalBackspaces = [](std::string s) {
    std::regex removeBackspacesRegExp{"^\b*|[^\b]\b"};
    std::size_t lengthBefore = 0;
    std::size_t lengthAfter = s.length();
    do {
      lengthBefore = lengthAfter;
      s = std::regex_replace(s, removeBackspacesRegExp, "");
      lengthAfter = s.length();
    } while (lengthBefore != lengthAfter);
    return s;
  };

  s1 = evalBackspaces(s1);
  s2 = evalBackspaces(s2);

  return s1 == s2;
}

std::string& numTestString(std::string& buffer, int n, int length,
                           std::string const& charSet) {
  for (int i = 0; i < length; ++i) {
    auto toSelect = n % charSet.length();
    n = n / charSet.length();
    buffer[i] = charSet[toSelect];
  }
  return buffer;
}

void bruteForceTest() {
  using namespace std;
  auto seq1 = string_view{"\b\b\baaa"};
  auto seq2 = string_view{"aaa\bc\b"};

  auto isSameIter = isSameInput(begin(seq1), end(seq1), begin(seq2), end(seq2));
  auto isSameRange = isSameInput(seq1, seq2);

  cout << isSameIter << "\n";
  cout << isSameRange << "\n";
}

void printDiff(std::string s0, std::string s1, int numS0, int numS1,
               bool resultCall0, int resultCall1) {
  fmt::print("found diff {} {} resultCall0{} resultCall1{}\n\t", numS0, numS1,
             resultCall0, resultCall1);
  for (auto c : s0) fmt::print("{:02x} ", c);
  fmt::print("\n\t");
  for (auto c : s1) fmt::print("{:02x} ", c);
  fmt::print("\n");
}

void bruteForecTest() {
  auto length = 6;
  std::string charSet{"abc\b"};
  auto iterationsNeeded = std::pow(charSet.length(), length);
  std::string inputSequence[2];
  for (auto& s : inputSequence) s.resize(length);
  for (int i = 0; i < iterationsNeeded; ++i) {
    inputSequence[0] = numTestString(inputSequence[0], i, length, charSet);
    for (int k = 0; k < iterationsNeeded; ++k) {
      inputSequence[1] = numTestString(inputSequence[1], k, length, charSet);
      auto isSameInputRegExpResult =
          isSameInputRegExp(inputSequence[0], inputSequence[1]);
      auto isSameInputResult = isSameInput(inputSequence[0], inputSequence[1]);
      if (isSameInputRegExpResult != isSameInputResult) {
        printDiff(inputSequence[0], inputSequence[1], i, k,
                  isSameInputRegExpResult, isSameInputResult);
      }
    }
    if ((i % 100) == 0) {
      std::cout << i << "/" << iterationsNeeded << "\n";
    }
  }
}

int main() {
  manualTest();
  bruteForceTest();

  return 0;
}
