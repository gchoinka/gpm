#include <fmt/printf.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <boost/range.hpp>
#include <boost/range/irange.hpp>
#include <future>
#include <iostream>
#include <iterator>
#include <regex>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <variant>
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

  if (isSameWithIterators != isSameWithRanges) {
    cout << isSameWithIterators << "\n";
    cout << isSameWithRanges << "\n";
  }
}

template <std::size_t N>
std::string formatDiff(std::array<std::string, N> const& inputs,
                       bool regExpCallReturnCode) {
  std::stringstream sstr;
  fmt::print(sstr, "found diff regExpCallReturnCode:{}\n\t",
             regExpCallReturnCode);
  for (auto const& i : inputs) {
    for (auto c : i) fmt::print(sstr, "{:02x} ", c);
    fmt::print(sstr, "\n\t");
  }
  return sstr.str();
}

namespace CheckerResult {
struct NoDifferenceFound {
  int processedCount = 0;
};
struct DifferenceFound {
  std::string message;
};

using Return_t = std::variant<NoDifferenceFound, DifferenceFound>;

}  // namespace CheckerResult

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...)->overloaded<Ts...>;

void bruteForceTest() {
  constexpr auto kSequencesCount = 3;
  constexpr auto kSequenceMaxLength = 4;
  constexpr std::array<char, 4> charSet = {'a', 'Z', '\b', '\0'};
  constexpr auto kSequenceStates =
      int(std::pow(std::size(charSet), kSequenceMaxLength));
  constexpr auto kIterationsNeeded =
      int(std::pow(kSequenceStates, kSequencesCount));

  auto makeInput =
      [&](int stateNumber) -> std::array<std::string, kSequencesCount> {
    std::array<int, kSequencesCount> subStateNumbers;
    int currentNumber = stateNumber;
    for (auto& n : subStateNumbers) {
      n = currentNumber % kSequenceStates;
      currentNumber /= kSequenceStates;
    }

    std::array<std::string, kSequencesCount> sequences;
    auto subStateNumberIter = subStateNumbers.cbegin();
    for (auto& i : sequences)
      i = numTestString(*subStateNumberIter++, kSequenceMaxLength, charSet);
    return sequences;
  };

  auto algorithmusChecker =
      [makeInput](auto indexRange) -> CheckerResult::Return_t {
    auto processedCounter = 0;
    for (auto i : indexRange) {
      auto sequences = makeInput(i);
      auto isSameInputRegExpResult = true;
      for (auto const& i : sequences) {
        isSameInputRegExpResult =
            isSameInputRegExpResult && isSameInputRegExp(sequences[0], i);
      }
      auto isSameInputArryCall =
          []<auto... Idx>(auto const& Cont, std::index_sequence<Idx...>) {
        return isSameInput(std::get<Idx>(Cont)...);
      };

      auto isSameInputResult = isSameInputArryCall(
          sequences, std::make_index_sequence<kSequencesCount>());
      if (isSameInputRegExpResult != isSameInputResult) {
        return CheckerResult::DifferenceFound{
            formatDiff(sequences, isSameInputRegExpResult)};
      }
      ++processedCounter;
    }
    return CheckerResult::NoDifferenceFound{processedCounter};
  };

  auto asyncWorkersCount = std::min(
      int(std::thread::hardware_concurrency()) * 1000, kIterationsNeeded);
  auto algorithmusCheckerAsyncWrapper =
      [algorithmusChecker, asyncWorkersCount](int rangeBeginOffset) {
        return algorithmusChecker(boost::irange(
            0 + rangeBeginOffset, kIterationsNeeded, asyncWorkersCount));
      };

  std::vector<std::function<std::future<CheckerResult::Return_t>()>>
      asyncWokersFactory;
  for (auto workerNumber : boost::irange(0, asyncWorkersCount)) {
    asyncWokersFactory.emplace_back(
        [algorithmusCheckerAsyncWrapper, workerNumber]() {
          return std::async(std::launch::async, algorithmusCheckerAsyncWrapper,
                            workerNumber);
        });
  }

  auto concurentRunningWorkersCount =
      std::min(int(std::thread::hardware_concurrency()), kIterationsNeeded);
  std::vector<std::future<CheckerResult::Return_t>> asyncWokers;

  auto asyncWokersFactoryIter = std::begin(asyncWokersFactory);
  while (asyncWokersFactoryIter != std::end(asyncWokersFactory) &&
         int(asyncWokers.size()) < concurentRunningWorkersCount)
    asyncWokers.emplace_back((*asyncWokersFactoryIter++)());

  constexpr int updateIntervalMicro = 20'000'000;
  auto const singelWorkerWaitTime =
      std::chrono::microseconds(updateIntervalMicro / std::size(asyncWokers));

  auto checkResultsAndRemove = [singelWorkerWaitTime](auto& workerCont,
                                             auto errorSink) -> int {
    int proccessedCount = 0;
    for (auto& w : workerCont) {
      if (!w.valid()) continue;
      auto workerResult = w.wait_for(singelWorkerWaitTime);
      if (workerResult == std::future_status::ready) {
        std::visit(
            overloaded{
                [&proccessedCount](
                    CheckerResult::NoDifferenceFound const& noDiffFound) {
                  proccessedCount += noDiffFound.processedCount;
                },
                [&errorSink](CheckerResult::DifferenceFound const& df) {
                  errorSink(df.message);
                }},
            w.get());
      }
    }
    workerCont.erase(
        std::remove_if(std::begin(workerCont), std::end(workerCont),
                       [](auto const& w) { return !w.valid(); }),
        std::end(workerCont));
    return proccessedCount;
  };
  int proccessedCount = 0;
  while (asyncWokers.size() > 0) {
    auto errorFound = false;
    proccessedCount +=
    checkResultsAndRemove(asyncWokers, [&errorFound](auto const& errorMessage) {
          fmt::print("\n{}\n", errorMessage);
          errorFound = true;
        });
    if (errorFound) break;

    fmt::print("{:f}\n", double(proccessedCount) / kIterationsNeeded);
    while (asyncWokersFactoryIter != std::end(asyncWokersFactory) &&
           int(asyncWokers.size()) < concurentRunningWorkersCount)
      asyncWokers.emplace_back((*asyncWokersFactoryIter++)());
  }
}

int main() {
  manualTest();
  bruteForceTest();

  return 0;
}
