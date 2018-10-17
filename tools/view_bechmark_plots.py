#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
import json

bmdata = json.load(open("g++-8.2-tree_benchmark.json"))

dictionary = plt.figure()
D = bmdata["BuildTimes"]

plt.rcParams.update({'font.size': 22})
plt.xticks(range(len(D)), D.keys(), rotation=-15)
plt.bar(range(len(D)), list(D.values()), align='center')

plt.show()


