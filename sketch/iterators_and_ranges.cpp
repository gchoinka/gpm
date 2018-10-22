#include <fmt/printf.h>
#include <algorithm>
#include <array>
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
constexpr bool isSameInputReverseIterImp(IterTupleT iterTuple,
                                         std::index_sequence<Idx...>) noexcept {
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

template <typename... ItersT>
constexpr auto isSameInput(ItersT... iters) -> typename std::enable_if<
    std::tuple_size<decltype(std::make_tuple(
        typename std::iterator_traits<ItersT>::pointer{}...))>::value != 0,
    bool>::type {
  static_assert(
      sizeof...(ItersT) >= 4,
      "Not enouth iterators pairs given, expecting 4 arguments (2 pairs) at "
      "least, e.g. isSameInput(v.begin(), v.end(), v2.begin(), v2.end())");
  static_assert((sizeof...(ItersT) % 2) == 0,
                "Iterators count is odd, this function need pairs of (begin, "
                "end) iterators.");
  auto iteratorPack = detail::convertToReverseIter(
      std::make_tuple(iters...),
      std::make_index_sequence<sizeof...(ItersT) / 2>());

  return detail::isSameInputReverseIterImp(
      iteratorPack, std::make_index_sequence<sizeof...(ItersT) / 2>());
}

template <typename... RangesT>
constexpr auto isSameInput(RangesT... ranges) ->
    typename std::enable_if<(std::tuple_size<decltype(std::make_tuple(
                                 std::crbegin(ranges)...))>::value +
                             std::tuple_size<decltype(std::make_tuple(
                                 std::crend(ranges)...))>::value) != 0,
                            bool>::type {
  static_assert(sizeof...(RangesT) >= 2,
                "Usefull result only posibible with at least 2 ranges");
  auto itPackBegins = std::make_tuple(std::crbegin(ranges)...);
  auto itPackEnds = std::make_tuple(std::crend(ranges)...);
  return detail::isSameInputReverseIterImp(
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

template <typename CharSetT>
auto numTestString(int n, int length, CharSetT charSet) {
  std::string buffer;
  buffer.reserve(length);
  for (int i = 0; i < length; ++i) {
    auto toSelect = n % std::size(charSet);
    n = n / std::size(charSet);
    if (charSet[toSelect] != '\0') buffer.push_back(charSet[toSelect]);
  }
  return buffer;
}

void manualTest() {
  using namespace std;
  constexpr auto seq1 = string_view{"\b\b\baaa"};
  constexpr auto seq2 = string_view{"aaa\bc\b"};

  constexpr auto isSameWithIterators =
      isSameInput(begin(seq1), end(seq1), begin(seq2), end(seq2));
  constexpr auto isSameWithRanges = isSameInput(seq1, seq2);

  cout << isSameWithIterators << "\n";
  cout << isSameWithRanges << "\n";
}

template <std::size_t N>
void printDiff(std::array<std::string, N> const& inputs, bool resultCall0,
               int resultCall1) {
  fmt::print("found diff resultCall0{} resultCall1{}\n\t", resultCall0,
             resultCall1);
  for (auto const& i : inputs) {
    for (auto c : i) fmt::print("{:02x} ", c);
    fmt::print("\n\t");
  }
}

void bruteForceTest() {
  constexpr auto kSequenceCount = 3;
  constexpr auto kSequenceMaxLength = 4;
  constexpr std::array<char, 4> charSet = {'a', 'Z', '\b', '\0'};
  constexpr auto kIntputStatesN =
      int(std::pow(std::size(charSet), kSequenceMaxLength));
  constexpr auto kIterationsNeeded = std::pow(kIntputStatesN, kSequenceCount);

  auto makeInputSet =
      [&](int stateNumber) -> std::array<std::string, kSequenceCount> {
    std::array<int, kSequenceCount> subStateNumbers;
    int currentNumber = stateNumber;
    for (auto& n : subStateNumbers) {
      n = currentNumber % kIntputStatesN;
      currentNumber /= kIntputStatesN;
    }

    std::array<std::string, kSequenceCount> sequences;
    auto subStateNumberIter = subStateNumbers.cbegin();
    for (auto& i : sequences)
      i = numTestString(*subStateNumberIter++, kSequenceMaxLength, charSet);
    return sequences;
  };

  for (int i = 0; i < kIterationsNeeded; ++i) {
    auto sequences = makeInputSet(i);
    auto isSameInputRegExpResult = true;
    for (auto const& i : sequences)
      isSameInputRegExpResult =
          isSameInputRegExpResult && isSameInputRegExp(sequences[0], i);

    auto isSameInputArryCall =
        []<auto... Idx>(auto const& Cont, std::index_sequence<Idx...>) {
      return isSameInput(std::get<Idx>(Cont)...);
    };

    auto isSameInputResult = isSameInputArryCall(
        sequences, std::make_index_sequence<kSequenceCount>());
    if (isSameInputRegExpResult != isSameInputResult) {
      printDiff(sequences, isSameInputRegExpResult, isSameInputResult);
      break;
    }
    if (!(i % 10000)) {
      fmt::print("{:f}\n", i / kIterationsNeeded);
    }
  }
}

int main() {
  manualTest();
  bruteForceTest();

  return 0;
}
