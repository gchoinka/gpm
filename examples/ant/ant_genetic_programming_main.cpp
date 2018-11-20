/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <functional>
#include <future>
#include <optional>
#include <thread>
#include <vector>

#include <boost/function_output_iterator.hpp>
#include <boost/range/irange.hpp>
#include <boost/variant.hpp>

#include <fmt/printf.h>
#define SPDLOG_TRACE_ON
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "common/ant_board_simulation.hpp"
#include "common/nodes.hpp"
#include "common/santa_fe_board.hpp"
#include "common/visitor.hpp"

#include "nodes_funcptr.hpp"

#include <vector>

class CountNodes : public boost::static_visitor<std::size_t> {
 public:
  template <typename T>
  std::size_t operator()(T const& node) const {
    std::size_t count = 0;
    if constexpr (std::tuple_size<decltype(node.children)>::value != 0) {
      for (auto const& n : node.children) {
        count += boost::apply_visitor(*this, n);
      }
    }
    return 1 + count;
  }
};

// class FlattenTree : public
// boost::static_visitor<std::vector<std::reference_wrapper<ant::NodesVariant>>>
// {
// public:
//   mutable ant::NodesVariant * rootNode_ = nullptr;
//   FlattenTree(ant::NodesVariant & rootNode):rootNode_{&rootNode}{}
//
//   template <typename T>
//   std::vector<std::reference_wrapper<ant::NodesVariant>> operator()(T & node)
//   const {
//     std::vector<std::reference_wrapper<ant::NodesVariant>> toReturn_;
//     if(rootNode_ != nullptr)
//     {
//       toReturn_.push_back(*rootNode_);
//       rootNode_ = nullptr;
//     }
//     if constexpr (std::tuple_size<decltype(node.children)>::value != 0)
//     {
//       for (auto & n : node.children) {
//         toReturn_.push_back(n);
//         auto toAppend = boost::apply_visitor(*this, n);
//         toReturn_.insert(std::end(toReturn_), std::begin(toAppend),
//         std::end(toAppend));
//       }
//     }
//     return toReturn_;
//   }
// };

template <typename OutputIterT>
class FlattenTree : public boost::static_visitor<OutputIterT> {
 public:
  OutputIterT sink_;
  bool firstNode_ = true;
  FlattenTree(OutputIterT sink) : sink_{sink} {}

  template <typename T>
  OutputIterT operator()(T& node) {
    if constexpr (std::tuple_size<decltype(node.children)>::value != 0) {
      for (auto& n : node.children) {
        sink_(&n);
        boost::apply_visitor(*this, n);
      }
    }
    return sink_;
  }
};

decltype(auto) getAntSataFeStaticBoardSim() {
  using namespace ant;
  auto max_steps = 400;
  auto max_food = 89;
  auto antSim =
      sim::AntBoardSimulationStaticSize<santa_fe::x_size, santa_fe::y_size>{
          max_steps, max_food, sim::Pos2d{0, 0}, sim::Direction::east,
          [](sim::AntBoardSimulationStaticSize<
              santa_fe::x_size, santa_fe::y_size>::FieldType& board) {
            for (size_t x = 0; x < board.size(); ++x) {
              for (size_t y = 0; y < board[x].size(); ++y) {
                board[x][y] = santa_fe::board[x][y] == 'X'
                                  ? sim::BoardState::food
                                  : sim::BoardState::empty;
              }
            }
          }};

  return antSim;
}

decltype(auto) getAntRandomBoardSim(
    int xSize, int ySize, std::size_t rndSeed = std::random_device{}()) {
  using namespace ant;
  auto foodCount = 0;
  auto pRnd = std::mt19937{rndSeed};
  auto intdist = std::uniform_int_distribution<>{0, 10};
  auto board = std::vector<std::vector<sim::BoardState>>(
      xSize, std::vector<sim::BoardState>(ySize, sim::BoardState::empty));

  for (auto& xDim : board) {
    for (auto& yDim : xDim) {
      bool placeFood = intdist(pRnd) == 0;
      if (!placeFood) continue;
      ++foodCount;
      yDim = sim::BoardState::food;
    }
  }
  auto maxSteps = foodCount * 5;
  auto antSim =
      sim::AntBoardSimulation<std::vector<std::vector<sim::BoardState>>>{
          maxSteps, foodCount, sim::Pos2d{0, 0}, sim::Direction::east,
          [&board](auto& b) { b = std::move(board); }};

  return antSim;
}

int main() {
  auto console = spdlog::stdout_color_mt("console");
  console->info("Welcome to spdlog version {}.{}.{} !", SPDLOG_VER_MAJOR,
                SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);

  constexpr auto populationSize = 5000;
  constexpr auto minHeight = 2;
  constexpr auto maxHeight = 11;

  constexpr auto generationMax = 5000;
  constexpr auto numberOfElite = std::min(4, populationSize);
  //   double const mutation_rate = 0.8;
  //   double const crossover_rate = 0.8;
  //   double const crossover_internal_point_rate = 0.9;
  //   double const reproduction_rate = 0.1;
  //   size_t const min_tree_height = 1;
  //   size_t const init_max_tree_height  = 6;
  //   size_t const max_tree_height = 17;
  size_t const tournamentSize = 4;

  auto fittnessFun = [](auto const& anAnt) {
    //auto sim = getAntRandomBoardSim(1024, 1024, 42);
    auto sim = getAntSataFeStaticBoardSim();
    auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{sim};

    while (!sim.is_finish()) {
      boost::apply_visitor(antBoardSimVisitor, anAnt);
    }
    return sim.score();
  };

  using FittnessReturnType = decltype(fittnessFun(ant::NodesVariant{}));
  struct ScoreIdxPair {
    FittnessReturnType score;
    std::size_t index;
  };

  auto population = std::vector<ant::NodesVariant>{};
  population.reserve(populationSize);
  auto fitness = std::vector<ScoreIdxPair>{};
  fitness.reserve(populationSize);

  auto nextPopulation = std::vector<ant::NodesVariant>{};
  nextPopulation.reserve(population.size());
  auto nextFitness = std::vector<ScoreIdxPair>{};
  nextFitness.reserve(population.size());

  std::random_device rd;
  auto rndSeed = rd();
  auto pRndGen = std::mt19937{rndSeed};
  auto rndNodeGen =
      gpm::BasicGenerator<ant::NodesVariant>{minHeight, maxHeight, rndSeed};

  for (auto i = population.size(); i < populationSize; ++i)
    population.emplace_back(rndNodeGen());
  auto asyncWorkersCount =
      std::min(std::size_t(std::thread::hardware_concurrency()),
               std::size_t(populationSize));

  auto stridedRanges = std::vector<boost::strided_integer_range<std::size_t>>{};
  for (auto workerNum : boost::irange(asyncWorkersCount)) {
    stridedRanges.emplace_back(
        boost::irange(0 + workerNum, population.size(), asyncWorkersCount));
  }

  auto workFu = [fittnessFun, &population, &fitness](auto range) {
    for (auto i : range) {
      fitness[i] = ScoreIdxPair{fittnessFun(population[i]), i};
    }
  };

  auto crossover = [](ant::NodesVariant idv1, ant::NodesVariant idv2,
                      auto randomGen) {
    std::array<std::reference_wrapper<ant::NodesVariant>, 2> args = {
        std::ref(idv1), std::ref(idv2)};
    std::array<ant::NodesVariant*, 2> crossoverPoints;

    for (int i = 0; i < 2; ++i) {
      std::vector<ant::NodesVariant*> flatenTree;
      FlattenTree ft{[&flatenTree](ant::NodesVariant* n) mutable {
        flatenTree.push_back(n);
      }};
      boost::apply_visitor(ft, args[i].get());
      crossoverPoints[i] =
          flatenTree[std::uniform_int_distribution<std::size_t>{
              0, flatenTree.size() - 1}(randomGen)];
    }

    ant::NodesVariant old = *crossoverPoints[0];
    *crossoverPoints[0] = *crossoverPoints[1];
    *crossoverPoints[1] = old;

    return std::make_tuple(idv1, idv2);
  };

  for ([[gnu::unused]] auto generation : boost::irange(generationMax)) {
    console->info("fitness calc");
    fitness.resize(population.size());
    [&stridedRanges, &workFu]() {
      std::vector<std::future<void>> worker;
      for (auto subTaskRanke : stridedRanges) {
        worker.emplace_back(
            std::async(std::launch::async, workFu, subTaskRanke));
      }
    }();

    console->info("evaluation");
    std::sort(
        std::begin(fitness), std::end(fitness),
        [](auto const& lhs, auto const& rhs) { return lhs.score < rhs.score; });

    for (int i = 0; i < 5; ++i) {
      auto s = boost::apply_visitor(gpm::RPNPrinter<std::string>{},
                                    population[fitness[i].index]);
      console->info("{} : {}\n", fitness[i].score, s);
    }

    nextFitness.clear();
    nextPopulation.clear();
    for (std::size_t i = 0; i < numberOfElite; ++i) {
      nextFitness.emplace_back(ScoreIdxPair{fitness[i].score, i});
      nextPopulation.emplace_back(population[fitness[i].index]);
    }

    auto tournamentSelector =
        std::uniform_int_distribution<std::size_t>{0, population.size() - 1};
    auto indvIndex = std::array<std::size_t, 2>{tournamentSelector(pRndGen),
                                                tournamentSelector(pRndGen)};
    for (size_t i = 0; i < tournamentSize; ++i) {
      indvIndex[0] = std::min(indvIndex[0], tournamentSelector(pRndGen));
      indvIndex[1] = std::min(indvIndex[1], tournamentSelector(pRndGen));
    }

    auto [newIdv1, newIdv2] =
        crossover(population[fitness[indvIndex[0]].index],
                  population[fitness[indvIndex[1]].index], pRndGen);

    nextPopulation.emplace_back(newIdv1);
    nextPopulation.emplace_back(newIdv2);

    population.swap(nextPopulation);
    fitness.swap(nextFitness);

    console->info("refill");
    population.resize(populationSize);

    [&population, &rndNodeGen]() {
      auto asyncWorkersCount =
          std::min(std::size_t(std::thread::hardware_concurrency()),
                   std::size_t(populationSize) - population.size());

      std::vector<std::future<void>> worker;
      for (auto workerNum : boost::irange(asyncWorkersCount)) {
        worker.emplace_back(std::async(
            std::launch::async,
            [&population, &rndNodeGen](auto range) {
              for (auto i : range) {
                population[i] = rndNodeGen();
              }
            },
            boost::irange(workerNum, population.size(), asyncWorkersCount)));
      }
    }();
  }

  //
  //   s = boost::apply_visitor(gpm::RPNPrinter<std::string>{},
  //   population[std::get<1>(fitness.back())]); fmt::print("score:{} ant:{}\n",
  //   std::get<0>(fitness.back()), s);

  //   auto g2 = boost::apply_visitor(FlattenTree{optAnt}, optAnt);
  //   fmt::print("{}\n", g2.size());
  //   for(auto const & subTree: g2)
  //   {
  //
  //     fmt::print("{}\n", s);
  //   }
}
