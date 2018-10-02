[![Build Status](https://travis-ci.org/gchoinka/gpm.svg?branch=master)](https://travis-ci.org/gchoinka/gpm) [![Build status](https://ci.appveyor.com/api/projects/status/rxqqyeshsx9jn5p1?svg=true)](https://ci.appveyor.com/project/gchoinka/gpm)
# Genetic Programming close to Metal 

Genetic Programming is an optimizing strategy inspired by genetic and evolution theory. 

GPM is a genetic programming library written in C++17.
It is heavily influenced by gpcxx but it useses boost::variant and std::array for its tree and node representation.  Gpcxx, on the other hand, uses object polymorphism and custom tree containers (which is not slower, but value semantic and visitor pattern are preferable in my view).
# Goals
* use modern C++ futures to make it more easily accessible. 
* use external compiler to converts runtime polymorphism to static compile-time polymorphism. 

# Install

*Requiements*
 * cmake
 * gcc (tested with gcc-7) or clang (tested with clang-6)
 
*Procedure*
```console
git clone https://github.com/gchoinka/gpm.git
mkdir gpm/build && cd gpm/build
cmake .. && cmake --build .
cmake --build . --target run_tree_benchmark
```

# Status
 * Evaluating different ways to implement a tree structures in C++. 
   * run target cmake `--build . --target generate_tree_for_benchmark` in the build directory and view the `./generated_includes/ant_simulation_benchmark_generated_functions.cpp` file for an example
 * Implementing C++ code generation from C++ itself. See [generate_tree...](examples/ant/generate_tree_for_benchmark_main.cpp) for the code that generated `ant_simulation_benchmark_generated_functions.cpp`

# Benchmarks
## variantDynamic
The ant is build from reverse polish notation string
```cpp
template <typename AntBoardSimT>
int variantDynamic(AntBoardSimT antBoardSim, std::string_view const &sv) {
    auto anAnt = gpm::Factory<ant::ant_nodes, gpm::RPNToken_iterator>::factory(gpm::RPNToken_iterator{sv});
    auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{antBoardSim};
    while (!antBoardSim.is_finish()) {
        boost::apply_visitor(antBoardSimVisitor, anAnt);
    }
    return antBoardSim.score();
}
```
## variantCTStatic
the tree structure is defined at compile time
```cpp
template <typename AntBoardSimT>
int variantCTStatic(AntBoardSimT antBoardSim, std::string_view const &) {
    auto anAnt = ant::ant_nodes{
        ant::if_food_ahead{
            ant::move{},
            ant::prog3{
                ant::left{},
                ant::if_food_ahead{
                    ant::move{},
                    ant::prog3{
                        ant::left{}, 
                        ant::left{},
                        ant::if_food_ahead{
                            ant::move{},
                            ant::left{}
                        }
                    }
                },
                ant::move{}
          }
      }
  };
  auto antBoardSimVisitor = ant::AntBoardSimulationVisitor{antBoardSim};
  while (!antBoardSim.is_finish()) {
        boost::apply_visitor(antBoardSimVisitor, anAnt);
  }
  return antBoardSim.score();
}
```
## unwrappedDirectCTStatic
```cpp
template <typename AntBoardSimT>
int unwrappedDirectCTStatic(AntBoardSimT antBoardSim, std::string_view const &) {
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
```
# Notes
At this stage I’m not sure if this library can deliver the promise “close to Metal”, so the M will maybe change meaning.

