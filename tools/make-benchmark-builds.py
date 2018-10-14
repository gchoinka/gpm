#!/usr/bin/env python

import os
import shutil
import glob
import re
import json
from subprocess import check_call

thisDir = os.path.dirname(os.path.abspath(__file__))
gpmDir = os.path.abspath(os.path.join(thisDir, ".."))
buildDir = os.path.join(gpmDir, "build-benchmarks")
fastRun = False

compiler=[
    {"compiler":"g++-8.2", "dir":"g++-8.2", "cmake_args":["-DCMAKE_BUILD_TYPE=Release"]}, 
    {"compiler":"clang++-7", "dir":"clang++-7", "cmake_args":["-DCMAKE_BUILD_TYPE=Release"]}, 
    {"compiler":"clang++-7", "dir":"clang++-7-ASAN", "cmake_args":["-DCMAKE_BUILD_TYPE=Debug", "-DCMAKE_CXX_FLAGS=-fsanitize=address -fno-omit-frame-pointer",  "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address"]}, 
    {"compiler":"clang++-7", "dir":"clang++-7-USAN", "cmake_args":["-DCMAKE_BUILD_TYPE=Debug", "-DCMAKE_CXX_FLAGS=-fsanitize=undefined -fno-omit-frame-pointer",  "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=undefined"]}, 
]


if not fastRun:
    if os.path.exists(buildDir):
        shutil.rmtree(buildDir)
        
    os.mkdir(buildDir)

    for c in compiler:
        compilerDir = os.path.join(buildDir, c["dir"])
        if not os.path.exists(compilerDir):
            os.mkdir(compilerDir)
            
        os.chdir(compilerDir)
        check_call(["cmake", "-GNinja", "-DGPM_FORMAT_GENERATED_FILES=ON", "-DCMAKE_CXX_COMPILER="+c["compiler"]] + c["cmake_args"] + [gpmDir])
    
for c in compiler:
    compilerDir = os.path.join(buildDir, c["dir"])
    os.chdir(compilerDir)

    if not fastRun:
        check_call(["cmake", "--build", compilerDir, "--target", "run_tree_benchmark"])
        check_call(["cmake", "--build", compilerDir])
        check_call(["ctest", "--output-on-failure",  "-VV"])
    #shutil.copy(os.path.join(compilerDir, "examples/ant/tree_benchmark.json"), os.path.join(thisDir, "benchmark/%s-tree_benchmark.json" % c["dir"]))
    if not fastRun:
        check_call(["cmake", "--build", compilerDir, "--target", "timeing_build_tree_benchmark_all"])
    treeBenchmark = json.load(open(os.path.join(compilerDir, "examples/ant/tree_benchmark.json"), "r"))
    treeBenchmark["BuildTimes"] = {}
    for filename in glob.iglob(os.path.join(compilerDir, "examples/ant/buildtime_*.txt")):
        #shutil.copy(filename, os.path.join(thisDir, "benchmark/%s-%s" % (c["dir"], os.path.basename(filename))))
        fileContent = open(filename).read()
        wallTime = re.findall("Elapsed \(wall clock\) time \(h:mm:ss or m:ss\): (\d:\d\d.\d\d)", fileContent)
        bmName = re.findall("buildtime_(.*).txt", filename)[0]
        (minPart, secPart) = wallTime[0].split((":"))
        treeBenchmark["BuildTimes"][bmName] = int(minPart) * 60 + float(secPart)
    
    treeBenchmark["BuildSize"] = {}
    for filename in glob.iglob(os.path.join(compilerDir, "examples/ant/bin_size_*.txt")):
        bmName = re.findall("bin_size_(.*).txt", filename)[0]
        treeBenchmark["BuildSize"][bmName] = int(open(filename).read())
        #shutil.copy(filename, os.path.join(thisDir, "benchmark/%s-%s" % (c["dir"], os.path.basename(filename))))
    with open(os.path.join(thisDir, "benchmark/%s-tree_benchmark.json" % c["dir"]), "w") as f:
      json.dump(treeBenchmark, f, indent=4)
    
