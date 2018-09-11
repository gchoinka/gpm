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

compiler=["clang++-6.0", "g++-7"]
for c in compiler:
    compilerDir = os.path.join(buildDir, c)
    if not os.path.exists(compilerDir):
        os.mkdir(compilerDir)
        
    os.chdir(compilerDir)
    check_call(["cmake", "-GNinja", "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_CXX_COMPILER="+c, gpmDir])
    check_call(["cmake", "--build", compilerDir, "--target", "run_tree_benchmark"])


