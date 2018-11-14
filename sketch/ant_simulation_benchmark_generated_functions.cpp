static inline char const *getAntRPN() {
  return "m l m if l l p3 m if l p3 m if";
}

template <typename AntBoardSimT>
static int variantDynamic(AntBoardSimT antBoardSim, std::string_view const &sv,
                          BenchmarkPart toMessure) {
  auto optAnt = gpm::experimental::FactoryV2<
      ant::ant_nodes, gpm::RPNTokenCursor>::factory(gpm::RPNTokenCursor{sv});
  if (toMessure == BenchmarkPart::Create) return 0;
  auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{antBoardSim};

  while (!antBoardSim.is_finish()) {
    boost::apply_visitor(antBoardSimVisitor, optAnt);
  }
  return antBoardSim.score();
}

template <typename AntBoardSimT>
static int variantCTStatic(AntBoardSimT antBoardSim, std::string_view const &,
                           BenchmarkPart toMessure) {
  auto optAnt = ant::ant_nodes{ant::if_food_ahead{
      ant::move{}

      ,
      ant::prog3{ant::left{},
                 ant::if_food_ahead{ant::move{}

                                    ,
                                    ant::prog3{ant::left{}, ant::left{},
                                               ant::if_food_ahead{ant::move{}

                                                                  ,
                                                                  ant::left{}

                                               }

                                    }

                 },
                 ant::move{}

      }

  }};
  if (toMessure == BenchmarkPart::Create) return 0;
  auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{antBoardSim};

  while (!antBoardSim.is_finish()) {
    boost::apply_visitor(antBoardSimVisitor, optAnt);
  }
  return antBoardSim.score();
}

template <typename AntBoardSimT>
static int oopTreeDynamic(AntBoardSimT antBoardSim, std::string_view const &sv,
                          BenchmarkPart toMessure) {
  auto oopTree = antoop::factory<AntBoardSimT>(gpm::RPNTokenCursor{sv});
  if (toMessure == BenchmarkPart::Create) return 0;
  while (!antBoardSim.is_finish()) {
    (*oopTree)(antBoardSim);
  }
  return antBoardSim.score();
}

template <typename AntBoardSimT>
int oppTreeCTStatic(AntBoardSimT antBoardSim, std::string_view const &,
                    BenchmarkPart toMessure) {
  auto oopTree = std::make_unique<antoop::IfFoodAhead<decltype(antBoardSim)>>(
      std::make_unique<antoop::Move<AntBoardSimT>>()

          ,
      std::make_unique<antoop::Prog3<AntBoardSimT>>(
          std::make_unique<antoop::Left<AntBoardSimT>>(),
          std::make_unique<antoop::IfFoodAhead<decltype(antBoardSim)>>(
              std::make_unique<antoop::Move<AntBoardSimT>>()

                  ,
              std::make_unique<antoop::Prog3<AntBoardSimT>>(
                  std::make_unique<antoop::Left<AntBoardSimT>>(),
                  std::make_unique<antoop::Left<AntBoardSimT>>(),
                  std::make_unique<antoop::IfFoodAhead<decltype(antBoardSim)>>(
                      std::make_unique<antoop::Move<AntBoardSimT>>()

                          ,
                      std::make_unique<antoop::Left<AntBoardSimT>>()

                          )

                      )

                  ),
          std::make_unique<antoop::Move<AntBoardSimT>>()

              )

  );
  if (toMessure == BenchmarkPart::Create) return 0;
  while (!antBoardSim.is_finish()) {
    (*oopTree)(antBoardSim);
  }
  return antBoardSim.score();
}

template <typename AntBoardSimT>
int unwrappedDirectCTStatic(AntBoardSimT antBoardSim, std::string_view const &,
                            BenchmarkPart toMessure) {
  if (toMessure == BenchmarkPart::Create) return 0;
  while (!antBoardSim.is_finish()) {
    if (antBoardSim.is_food_in_front()) {
      antBoardSim.move();

    } else {
      antBoardSim.left();

      if (antBoardSim.is_food_in_front()) {
        antBoardSim.move();

      } else {
        antBoardSim.left();
        antBoardSim.left();

        if (antBoardSim.is_food_in_front()) {
          antBoardSim.move();

        } else {
          antBoardSim.left();
        }
      }
      antBoardSim.move();
    }
  }
  return antBoardSim.score();
}

template <typename AntBoardSimT>
int unwrappedVisitorCallingCTStatic(AntBoardSimT antBoardSim,
                                    std::string_view const &,
                                    BenchmarkPart toMessure) {
  if (toMessure == BenchmarkPart::Create) return 0;
  auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{antBoardSim};
  while (!antBoardSim.is_finish()) {
    if (antBoardSim.is_food_in_front()) {
      antBoardSimVisitor(ant::move{});

    } else {
      antBoardSimVisitor(ant::left{});

      if (antBoardSim.is_food_in_front()) {
        antBoardSimVisitor(ant::move{});

      } else {
        antBoardSimVisitor(ant::left{});
        antBoardSimVisitor(ant::left{});

        if (antBoardSim.is_food_in_front()) {
          antBoardSimVisitor(ant::move{});

        } else {
          antBoardSimVisitor(ant::left{});
        }
      }
      antBoardSimVisitor(ant::move{});
    }
  }
  return antBoardSim.score();
}

template <typename AntBoardSimT>
static int tupleCTStatic(AntBoardSimT antBoardSim, std::string_view const &,
                         BenchmarkPart toMessure) {
  using namespace tup;
  constexpr auto optAnt = hana::tuple<
      tag::IfFood, hana::tuple<tag::Move>,
      hana::tuple<tag::Prog3, hana::tuple<tag::Left>,
                  hana::tuple<tag::IfFood, hana::tuple<tag::Move>,
                              hana::tuple<tag::Prog3, hana::tuple<tag::Left>,
                                          hana::tuple<tag::Left>,
                                          hana::tuple<tag::IfFood,
                                                      hana::tuple<tag::Move>,
                                                      hana::tuple<tag::Left>>>>,
                  hana::tuple<tag::Move>>>{};
  if (toMessure == BenchmarkPart::Create) return 0;
  while (!antBoardSim.is_finish()) {
    tup::eval(antBoardSim, optAnt);
  }
  return antBoardSim.score();
}

template <typename AntBoardSimT>
static int dynoTreeDynamic(AntBoardSimT antBoardSim, std::string_view const &sv,
                           BenchmarkPart toMessure) {
  auto dynoTree = antdyno::factory<AntBoardSimT>(gpm::RPNTokenCursor{sv});
  if (toMessure == BenchmarkPart::Create) return 0;
  while (!antBoardSim.is_finish()) {
    dynoTree.eval(antBoardSim);
  }
  return antBoardSim.score();
}

template <typename AntBoardSimT>
int dynoTreeCTStatic(AntBoardSimT antBoardSim, std::string_view const &,
                     BenchmarkPart toMessure) {
  auto dynoTree = antdyno::IfFood<AntBoardSimT>(
      antdyno::Move<AntBoardSimT>()

          ,
      antdyno::Prog3<AntBoardSimT>(
          antdyno::Left<AntBoardSimT>(),
          antdyno::IfFood<AntBoardSimT>(
              antdyno::Move<AntBoardSimT>()

                  ,
              antdyno::Prog3<AntBoardSimT>(
                  antdyno::Left<AntBoardSimT>(), antdyno::Left<AntBoardSimT>(),
                  antdyno::IfFood<AntBoardSimT>(antdyno::Move<AntBoardSimT>()

                                                    ,
                                                antdyno::Left<AntBoardSimT>()

                                                    )

                      )

                  ),
          antdyno::Move<AntBoardSimT>()

              )

  );
  if (toMessure == BenchmarkPart::Create) return 0;
  while (!antBoardSim.is_finish()) {
    dynoTree.eval(antBoardSim);
  }
  return antBoardSim.score();
}

template <typename AntBoardSimT>
decltype(auto) getAllTreeBenchmarks() {
  return std::make_tuple(
      std::make_tuple(&variantDynamic<AntBoardSimT>, "variantDynamic"),
      std::make_tuple(&variantCTStatic<AntBoardSimT>, "variantCTStatic"),
      std::make_tuple(&oopTreeDynamic<AntBoardSimT>, "oopTreeDynamic"),
      std::make_tuple(&oppTreeCTStatic<AntBoardSimT>, "oppTreeCTStatic"),
      std::make_tuple(&unwrappedDirectCTStatic<AntBoardSimT>,
                      "unwrappedDirectCTStatic"),
      std::make_tuple(&unwrappedVisitorCallingCTStatic<AntBoardSimT>,
                      "unwrappedVisitorCallingCTStatic"),
      std::make_tuple(&tupleCTStatic<AntBoardSimT>, "tupleCTStatic"),
      std::make_tuple(&dynoTreeDynamic<AntBoardSimT>, "dynoTreeDynamic"),
      std::make_tuple(&dynoTreeCTStatic<AntBoardSimT>, "dynoTreeCTStatic"));
}
