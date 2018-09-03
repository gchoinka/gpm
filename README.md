[![Build Status](https://travis-ci.org/gchoinka/gpm.svg?branch=master)](https://travis-ci.org/gchoinka/gpm)
# gpm
Genetic Programming close to Metal

Genetic Programming is an optimizing strategy inspired by genetic and evolution theory. 

GPM is a genetic programming library written with C++17.
It is heavily influenced by gpcxx but uses boost::variant and std::array for its tree and node representation.  Gpcxx, on the other hand, uses object polymorphism and custom tree structures
#Goals
* use modern C++ futures to make it more easily accessible. 
* external compiler which converts runtime polymorphism to static compile-time polymorphism.
