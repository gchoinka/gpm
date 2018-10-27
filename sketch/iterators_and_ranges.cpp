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

  cout << isSameWithIterators << "\n";
  cout << isSameWithRanges << "\n";
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
struct ReturnAfterPrematurelyStopRequest {};
struct NoDifferenceFound {};
struct DifferenceFound {
  std::string message;
};

using Return_t = std::variant<ReturnAfterPrematurelyStopRequest,
                              NoDifferenceFound, DifferenceFound>;

}  // namespace CheckerResult

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...)->overloaded<Ts...>;

void bruteForceTest() {
  constexpr auto kSequenceCount = 3;
  constexpr auto kSequenceMaxLength = 4;
  constexpr std::array<char, 4> charSet = {'a', 'Z', '\b', '\0'};
  constexpr auto kIntputStatesN =
      int(std::pow(std::size(charSet), kSequenceMaxLength));
  constexpr auto kIterationsNeeded =
      int(std::pow(kIntputStatesN, kSequenceCount));

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

  std::atomic<int> iterationDone = 0;
  std::atomic<bool> prematurelyStop = false;

  auto algorithmusChecker = [makeInputSet, &iterationDone, &prematurelyStop](
                                auto indexRange) -> CheckerResult::Return_t {
    auto progressCounter = 0;
    auto progressCounterMax = 5000;
    for (auto i : indexRange) {
      auto sequences = makeInputSet(i);
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
          sequences, std::make_index_sequence<kSequenceCount>());
      if (isSameInputRegExpResult != isSameInputResult) {
        return CheckerResult::DifferenceFound{
            formatDiff(sequences, isSameInputRegExpResult)};
      }
      if (++progressCounter == progressCounterMax && prematurelyStop.load()) {
        return CheckerResult::ReturnAfterPrematurelyStopRequest{};
      }

      if (++progressCounter == progressCounterMax) {
        progressCounter = 0;
        iterationDone.fetch_add(progressCounterMax);
      }
    }
    return CheckerResult::NoDifferenceFound{};
  };
  auto asyncWorkersCount =
      std::min(int(std::thread::hardware_concurrency()), kIterationsNeeded);
  auto algorithmusCheckerAsyncWrapper =
      [algorithmusChecker, asyncWorkersCount](int rangeBeginOffset) {
        return algorithmusChecker(boost::irange(
            0 + rangeBeginOffset, kIterationsNeeded, asyncWorkersCount));
      };

  std::vector<std::future<CheckerResult::Return_t>> asyncWokers;
  for (auto workerNumber : boost::irange(0, asyncWorkersCount)) {
    asyncWokers.emplace_back(std::async(
        std::launch::async, algorithmusCheckerAsyncWrapper, workerNumber));
  }

  int oldValue = iterationDone.load();
  constexpr int updateIntervalMs = 1000;
  auto const singelWorkerWaitTime =
      std::chrono::milliseconds(updateIntervalMs / std::size(asyncWokers));
  auto allHaveFinished = [&singelWorkerWaitTime](auto& workerCont) {
    return std::all_of(std::begin(workerCont), std::end(workerCont),
                       [&singelWorkerWaitTime](auto& fu) {
                         return fu.wait_for(singelWorkerWaitTime) ==
                                std::future_status::ready;
                       });
  };

  auto checkForErrors = [&prematurelyStop](auto& workerCont,
                                           auto errorSink) -> bool {
    bool errorFound = false;
    for (auto& w : workerCont) {
      auto workerResult = w.wait_for(std::chrono::seconds(0));
      if (workerResult == std::future_status::ready) {
        std::visit(
            overloaded{[](CheckerResult::ReturnAfterPrematurelyStopRequest) {},
                       [](CheckerResult::NoDifferenceFound) {},
                       [&prematurelyStop, &errorSink,
                        &errorFound](CheckerResult::DifferenceFound const& df) {
                         errorSink(df.message);
                         prematurelyStop.store(true);
                         errorFound = true;
                       }},
            w.get());
      }
    }
    return errorFound;
  };
  while (!allHaveFinished(asyncWokers)) {
    auto errorFound = checkForErrors(asyncWokers, [](auto const& errorMessage) {
      fmt::print("{}\n", errorMessage);
    });
    if (errorFound) break;

    if (oldValue != iterationDone.load()) {
      oldValue = iterationDone.load();
      fmt::print("{:f}\n", double(iterationDone.load()) / kIterationsNeeded);
    }
  }
}

int main() {
  manualTest();
  bruteForceTest();

  return 0;
}
