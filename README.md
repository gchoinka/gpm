[![Build Status](https://travis-ci.org/gchoinka/gpm.svg?branch=master)](https://travis-ci.org/gchoinka/gpm) [![Build status](https://ci.appveyor.com/api/projects/status/rxqqyeshsx9jn5p1?svg=true)](https://ci.appveyor.com/project/gchoinka/gpm)
# Genetic Programming close to Metal 

Genetic Programming is an optimizing strategy inspired by genetic and evolution theory. 

GPM is a genetic programming library written in C++17.
It started as a modernisation of https://github.com/Ambrosys/gpcxx , the goal was to replace object polymorphism with std::variant polymorphism.
Aa a result of benchmarking, it became clear that std::variant did not have runtime advantages (in the tested use case). Even std::variant offers value semantic and double dispatch abilities. The decision was made against using std::variant as the base for the tree structure. 
 * std::variant lacks some features for recursive data structures (the benchmarks used boost::variant as a workaround) https://stackoverflow.com/questions/39454347/using-stdvariant-with-recursion-without-using-boostrecursive-wrapper
 * our variant implementation of the tree leaked implementation details to the users
The current goal is to implement gpm with nodes in a continues chunk of memory and minimal overhead inspired the way binary heaps are implemented.

## Goals
* use modern C++ futures to make GP more easily accessible with C++. 

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

Documentation
=============
Browse the [documentation](https://gchoinka.github.io/gpm/#/).


