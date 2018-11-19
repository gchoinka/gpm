#include <array>
#include <memory>
#include <variant>

#include <benchmark/benchmark.h>




static void FrozenMap(benchmark::State& state) {
  // Code inside this loop is measured repeatedly
  for (auto _ : state) {

  }
}
// Register the function as a benchmark
BENCHMARK(FrozenMap);

static void HandcraftedHash(benchmark::State& state) {
  for (auto _ : state) {

  }
}
BENCHMARK(HandcraftedHash);

BENCHMARK_MAIN();
