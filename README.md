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
```console
#clang++-6.0
-----------------------------------------------------------------------------------------
Benchmark                                  Time           CPU Iterations UserCounters...
-----------------------------------------------------------------------------------------
BM_recursiveVariantTreeFromString     226637 ns     226637 ns       3020 score=78
BM_recursiveVariantTree                74096 ns      74094 ns       9531 score=78
BM_cppFixedTree                          794 ns        794 ns     869607 score=78
BM_cppFixedWithVisitor                   790 ns        790 ns     823121 score=78
BM_oopTree                             16317 ns      16317 ns      42673 score=78
BM_oopTreeFromString                   45595 ns      45585 ns      15123 score=78
#g++7
-----------------------------------------------------------------------------------------
Benchmark                                  Time           CPU Iterations UserCounters...
-----------------------------------------------------------------------------------------
BM_recursiveVariantTreeFromString     173610 ns     173610 ns       4070 score=78
BM_recursiveVariantTree                75690 ns      75688 ns       9230 score=78
BM_cppFixedTree                          880 ns        880 ns     806511 score=78
BM_cppFixedWithVisitor                   872 ns        872 ns     805375 score=78
BM_oopTree                             18368 ns      18368 ns      36963 score=78
BM_oopTreeFromString                   45878 ns      45878 ns      15162 score=78
```
# Notes
At this stage I’m not sure if this library can deliver the promise “close to Metal”, so the M will maybe change meaning.

