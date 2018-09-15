

static char const antRPNString[] = {"m r m if l l p3 r m if if p2 r p2 m if"};

template <typename AntBoardSimT>
static int recursiveVariantTreeFromString(AntBoardSimT antBoardSim) {
  auto optAnt =
      gpm::factory<ant::ant_nodes>(gpm::RPNToken_iterator{antRPNString});

  auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{antBoardSim};

  while (!antBoardSim.is_finish()) {
    boost::apply_visitor(antBoardSimVisitor, optAnt);
  }
  return antBoardSim.score();
}

static void BM_recursiveVariantTreeFromString(benchmark::State& state) {
  for (auto _ : state) {
    state.counters["score"] = recursiveVariantTreeFromString(getAntBoardSim());
  }
}
BENCHMARK(BM_recursiveVariantTreeFromString);

template <typename AntBoardSimT>
static int recursiveVariantTree(AntBoardSimT antBoardSim) {
  auto optAnt = ant::ant_nodes{ant::if_food_ahead{
      ant::move{},
      ant::prog<2, gpm::NodeToken<(char)112, (char)50>>{
          ant::right{},
          ant::prog<2, gpm::NodeToken<(char)112, (char)50>>{

              ant::if_food_ahead{
                  ant::if_food_ahead{ant::move{}, ant::right{}},
                  ant::prog<3, gpm::NodeToken<(char)112, (char)51>>{
                      ant::left{}, ant::left{},
                      ant::if_food_ahead{ant::move{}, ant::right{}}

                  }},
              ant::move{}

          }

      }}};

  auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{antBoardSim};

  while (!antBoardSim.is_finish()) {
    boost::apply_visitor(antBoardSimVisitor, optAnt);
  }
  return antBoardSim.score();
}

static void BM_recursiveVariantTree(benchmark::State& state) {
  for (auto _ : state) {
    state.counters["score"] = recursiveVariantTree(getAntBoardSim());
  }
}
BENCHMARK(BM_recursiveVariantTree);

template <typename AntBoardSimT>
int cppFixedTree(AntBoardSimT antBoardSim) {
  while (!antBoardSim.is_finish()) {
    if (antBoardSim.is_food_in_front()) {
      antBoardSim.move();

    } else {
      antBoardSim.right();

      if (antBoardSim.is_food_in_front()) {
        if (antBoardSim.is_food_in_front()) {
          antBoardSim.move();

        } else {
          antBoardSim.right();
        }

      } else {
        antBoardSim.left();
        antBoardSim.left();

        if (antBoardSim.is_food_in_front()) {
          antBoardSim.move();

        } else {
          antBoardSim.right();
        }
      }
      antBoardSim.move();
    }
  }
  return antBoardSim.score();
}

static void BM_cppFixedTree(benchmark::State& state) {
  for (auto _ : state) {
    state.counters["score"] = cppFixedTree(getAntBoardSim());
  }
}
BENCHMARK(BM_cppFixedTree);

template <typename AntBoardSimT>
int cppFixedWithVisitor(AntBoardSimT antBoardSim) {
  auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{antBoardSim};

  while (!antBoardSim.is_finish()) {
    if (antBoardSim.is_food_in_front()) {
      antBoardSimVisitor(ant::move{});

    } else {
      antBoardSimVisitor(ant::right{});

      if (antBoardSim.is_food_in_front()) {
        if (antBoardSim.is_food_in_front()) {
          antBoardSimVisitor(ant::move{});

        } else {
          antBoardSimVisitor(ant::right{});
        }

      } else {
        antBoardSimVisitor(ant::left{});
        antBoardSimVisitor(ant::left{});

        if (antBoardSim.is_food_in_front()) {
          antBoardSimVisitor(ant::move{});

        } else {
          antBoardSimVisitor(ant::right{});
        }
      }
      antBoardSimVisitor(ant::move{});
    }
  }
  return antBoardSim.score();
}

static void BM_cppFixedWithVisitor(benchmark::State& state) {
  for (auto _ : state) {
    state.counters["score"] = cppFixedWithVisitor(getAntBoardSim());
  }
}
BENCHMARK(BM_cppFixedWithVisitor);

template <typename AntBoardSimT>
int oopTree(AntBoardSimT antBoardSim) {
  auto oopTree = std::make_unique<antoop::IfFoodAhead<decltype(antBoardSim)>>(
      std::make_unique<antoop::Move<decltype(antBoardSim)>>()

          ,
      std::make_unique<antoop::Prog2<decltype(antBoardSim)>>(
          std::make_unique<antoop::Right<decltype(antBoardSim)>>(),
          std::make_unique<antoop::Prog2<decltype(antBoardSim)>>(

              std::make_unique<antoop::IfFoodAhead<decltype(antBoardSim)>>(

                  std::make_unique<antoop::IfFoodAhead<decltype(antBoardSim)>>(
                      std::make_unique<antoop::Move<decltype(antBoardSim)>>()

                          ,
                      std::make_unique<antoop::Right<decltype(antBoardSim)>>()

                          )

                      ,
                  std::make_unique<antoop::Prog3<decltype(antBoardSim)>>(
                      std::make_unique<antoop::Left<decltype(antBoardSim)>>(),
                      std::make_unique<antoop::Left<decltype(antBoardSim)>>(),
                      std::make_unique<
                          antoop::IfFoodAhead<decltype(antBoardSim)>>(
                          std::make_unique<
                              antoop::Move<decltype(antBoardSim)>>()

                              ,
                          std::make_unique<
                              antoop::Right<decltype(antBoardSim)>>()

                              )

                          )

                      ),
              std::make_unique<antoop::Move<decltype(antBoardSim)>>()

                  )

              )

  );

  while (!antBoardSim.is_finish()) {
    (*oopTree)(antBoardSim);
  }
  return antBoardSim.score();
}

static void BM_oopTree(benchmark::State& state) {
  for (auto _ : state) {
    state.counters["score"] = oopTree(getAntBoardSim());
  }
}
BENCHMARK(BM_oopTree);

template <typename AntBoardSimT>
int oopTreeFromString(AntBoardSimT antBoardSim) {
  auto oopTree =
      antoop::factory<AntBoardSimT>(gpm::RPNToken_iterator{antRPNString});

  while (!antBoardSim.is_finish()) {
    (*oopTree)(antBoardSim);
  }
  return antBoardSim.score();
}

static void BM_oopTreeFromString(benchmark::State& state) {
  for (auto _ : state) {
    state.counters["score"] = oopTreeFromString(getAntBoardSim());
  }
}
BENCHMARK(BM_oopTreeFromString);

template <typename AntBoardSimT>
int oopTreeFromExtString(AntBoardSimT antBoardSim) {
  auto oopTree = antoop::factory<AntBoardSimT>(
      gpm::RPNToken_iterator{ant::optimalAntRPNExt});

  while (!antBoardSim.is_finish()) {
    (*oopTree)(antBoardSim);
  }
  return antBoardSim.score();
}

static void BM_oopTreeFromExtString(benchmark::State& state) {
  for (auto _ : state) {
    state.counters["score"] = oopTreeFromExtString(getAntBoardSim());
  }
}
BENCHMARK(BM_oopTreeFromExtString);
