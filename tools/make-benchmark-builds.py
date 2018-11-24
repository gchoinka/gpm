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
    {"compiler":"g++-8.2", "dir":"g++-8.2-minsize", "cmake_args":["-DCMAKE_BUILD_TYPE=MinSizeRel"]}, 
    {"compiler":"clang++-7", "dir":"clang++-7-minsize", "cmake_args":["-DCMAKE_BUILD_TYPE=MinSizeRel"]}, 
    
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
        
    if not fastRun:
        check_call(["cmake", "--build", compilerDir, "--target", "time_build_tree_benchmark_all"])
    treeBenchmark = json.load(open(os.path.join(compilerDir, "examples/ant/tree_benchmark.json"), "r"))
    treeBenchmark["BuildTimes"] = {}
    for filename in glob.iglob(os.path.join(compilerDir, "examples/ant/buildtime_*.txt")):

        fileContent = open(filename).read()
        wallTime = re.findall("Elapsed \(wall clock\) time \(h:mm:ss or m:ss\): (\d:\d\d.\d\d)", fileContent)
        bmName = re.findall("buildtime_(.*).txt", filename)[0]
        (minPart, secPart) = wallTime[0].split((":"))
        treeBenchmark["BuildTimes"][bmName] = int(minPart) * 60 + float(secPart)
    
    treeBenchmark["BuildSize"] = {}
    for filename in glob.iglob(os.path.join(compilerDir, "examples/ant/bin_size_*.txt")):
        bmName = re.findall("bin_size_(.*).txt", filename)[0]
        treeBenchmark["BuildSize"][bmName] = int(open(filename).read())

    createOnlyOrFull = {"Full_median":{}, "CreateOnly_median":{}, "Eval_median":{}}
    for b in treeBenchmark["benchmarks"]:
      name = re.findall("(.*)(CreateOnly_median|Full_median)", b["name"])
      if len(name) == 0:
        continue
      createOnlyOrFull[name[0][1]][name[0][0]] = b;
      
    for bmName in createOnlyOrFull["Full_median"]:
      createOnlyOrFull["Eval_median"][bmName] = {"cpu_time": createOnlyOrFull["Full_median"][bmName]["cpu_time"] - createOnlyOrFull["CreateOnly_median"][bmName]["cpu_time"]}
      
    
    treeBenchmark["EvalTimes"] = createOnlyOrFull
    with open(os.path.join(thisDir, "benchmark/%s-tree_benchmark.json" % c["dir"]), "w") as f:
      json.dump(treeBenchmark, f, indent=4)
    
