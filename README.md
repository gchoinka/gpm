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


# Notes
At this stage I’m not sure if this library can deliver the promise “close to Metal”, so the M will maybe change meaning.

