#!/usr/bin/env python

import os
import shutil 
from subprocess import check_call

thisDir = os.path.dirname(os.path.abspath(__file__))
gpmDir = os.path.abspath(os.path.join(thisDir, ".."))
buildDir = os.path.join(gpmDir, "build-benchmarks")

if os.path.exists(buildDir):
    shutil.rmtree(buildDir)
    
os.mkdir(buildDir)

compiler=[
    {"compiler":"g++-7", "dir":"g++-7", "cmake_args":["-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_CXX_FLAGS=-DGPM_BENCHMARK_NAME_PREFIX=gcc7"]}, 
    {"compiler":"clang++-6.0", "dir":"clang++-6.0", "cmake_args":["-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_CXX_FLAGS=-DGPM_BENCHMARK_NAME_PREFIX=clang6"]}, 
    {"compiler":"clang++-6.0", "dir":"clang++-6.0-ASAN", "cmake_args":["-DCMAKE_BUILD_TYPE=Debug", "-DCMAKE_CXX_FLAGS=-fsanitize=address -fno-omit-frame-pointer -DGPM_BENCHMARK_NAME_PREFIX=clang6ASAN",  "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address"]}, 
    {"compiler":"clang++-6.0", "dir":"clang++-6.0-USAN", "cmake_args":["-DCMAKE_BUILD_TYPE=Debug", "-DCMAKE_CXX_FLAGS=-fsanitize=undefined -fno-omit-frame-pointer -DGPM_BENCHMARK_NAME_PREFIX=clang6USAN",  "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=undefined"]}, 
]
for c in compiler:
    compilerDir = os.path.join(buildDir, c["dir"])
    if not os.path.exists(compilerDir):
        os.mkdir(compilerDir)
        
    os.chdir(compilerDir)
    check_call(["cmake", "-GNinja", "-DGPM_FORMAT_GENERATED_FILES=ON", "-DCMAKE_CXX_COMPILER="+c["compiler"]] + c["cmake_args"] + [gpmDir])
    check_call(["cmake", "--build", compilerDir, "--target", "run_tree_benchmark"])
    check_call(["cmake", "--build", compilerDir])
    check_call(["ctest", "--output-on-failure",  "-VV"])
    shutil.copy(os.path.join(compilerDir, "tree_benchmark.json"), os.path.join(thisDir, "benchmark/%s-tree_benchmark.json" % c["dir"]))


