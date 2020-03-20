#!/usr/bin/env python3
#
# Plot results of relaxation oscillator simulation
#
# Copyright 2020  Jason Leake
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use, copy,
# modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
# BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

import csv
import matplotlib.pyplot as plt
import numpy as np
import sys
import os

descrFile = "description.dat"
if os.path.exists(descrFile):

    dict = {}
    with open(descrFile, "rt", newline="\n") as descr:
        for line in descr:
            key, equal, value = line.strip().split(" ")
            dict[key] = value

    filename = dict["FILE"]
    resistance = float(dict["RESISTANCE"])
    capacitance = float(dict["CAPACITANCE"])

    data = np.genfromtxt(filename, delimiter=",")
    plt.title(f"Relaxation oscillator, R = {resistance}, C = {capacitance}")
    plt.xlabel("seconds")
    plt.ylabel("volts")

    cap = plt.plot(data[:,0],data[:,1],label="capacitor")
    out = plt.plot(data[:,0],data[:,2],label="output")
    plt.legend()
    outputFile = "plot.png"
    plt.savefig(outputFile)
    plt.draw()
    plt.close()
    print(f"Output file is {outputFile}")
    
else:
    print(f"Description file {descrFile} not found", file=sys.stderr)
    sys.exit(1)


