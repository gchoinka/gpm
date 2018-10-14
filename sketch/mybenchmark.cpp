#include <array>
#include <memory>
#include <variant>

#include <benchmark/benchmark.h>

struct Move {};
struct Left {};
struct Right {};
struct If {
  std::array<std::variant<Move, Left, Right>, 2> arr;
};

struct Prog3 {
  std::array<std::variant<If, Move, Left, Right>, 3> arr;
};

using antNode = std::variant<If, Move, Left, Right, std::shared_ptr<Prog3>>;

static void StringCreation(benchmark::State& state) {
  // Code inside this loop is measured repeatedly
  for (auto _ : state) {
    std::array<antNode, 3> children{};
    children[0] = If{};
    children[1] = Move{};
    children[2] = std::make_shared<Prog3>();

    benchmark::DoNotOptimize(children);
  }
}
// Register the function as a benchmark
BENCHMARK(StringCreation);

static void StringCopy(benchmark::State& state) {
  for (auto _ : state) {
    std::array<antNode, 3> children{If{}, Move{}, std::make_shared<Prog3>()};
    benchmark::DoNotOptimize(children);
  }
}
BENCHMARK(StringCopy);

BENCHMARK_MAIN();
