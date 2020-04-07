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

import argparse
import csv
import decimal
import matplotlib.pyplot as plt
import numpy as np
import os
import sys

OHM_SYMBOL = "\u03A9"
MICRO_SYMBOL = "\u00B5"

def test(a, b):
    assert a == b, f'"{a}" != "{b}"'
    print(f'"{a}" == "{b}"')

def makeFriendly(value, symbol):
    """ Convert component value to conventional notation. e.g. 10e6 to
    1 M.  """
    decimal.getcontext().prec = 5
    val1 = decimal.Decimal(value)
    order = 0
    while val1 >= 10:
        val1 = val1 / 10
        order = order + 1
    while val1 < 1:
        val1 = val1 * 10
        order = order - 1

    while order % 3 != 0:
        order = order - 1
        val1 = val1 * 10

    # No fractional part if not needed
    if val1 == int(val1):
        val1 = int(val1)

    # Trim trailing  zeros after decimal point
    val1 = str(val1)
    if "." in val1:
        while val1[-1] == '0':
            val1 = val1[:-1]

    if order == -12:
        returnValue = val1 + " p";
    elif order == -9:
        returnValue = val1 + " n";
    elif order == -6:
        returnValue = val1 + " " + MICRO_SYMBOL;
    elif order == -3:
        returnValue = val1 + " m";
    elif order == 3:
        returnValue = val1 + " k";
    elif order == 6:
        returnValue = val1 + " M";
    elif order == 9:
        returnValue = val1 + " G";
    elif order == 12:
        returnValue = val1 + " T";
    else:
        returnValue = str(value) + " ";
    return returnValue + symbol

parser = argparse.ArgumentParser(description="Plot relaxation program output")
parser.add_argument('--test', action='store_true',
                    help="Test makeFriendly() function")
parser.add_argument('--png', action = "store_true",
                    help="Produce PNG file instead of PDF")

args = parser.parse_args()
if args.test:
    test(makeFriendly(3300000000000, OHM_SYMBOL),"3.3 T" + OHM_SYMBOL)
    test(makeFriendly(2200000000, OHM_SYMBOL), "2.2 G" + OHM_SYMBOL)
    test(makeFriendly(2400000, OHM_SYMBOL), "2.4 M" + OHM_SYMBOL)
    test(makeFriendly(1000, OHM_SYMBOL), "1 k" + OHM_SYMBOL)
    test(makeFriendly(1, "F"), "1 F")
    test(makeFriendly(0.0016, "F"), "1.6 mF")
    test(makeFriendly(0.000008, "F"), "8 " + MICRO_SYMBOL + "F")
    test(makeFriendly(0.00000000123, "F"), "1.23 nF")
    test(makeFriendly(0.0000000000011, "F"), "1.1 pF")
    print("Tests passed")
    sys.exit(0)

if args.png:
    suffix = ".png"
else:
    suffix = ".pdf"

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
    friendlyResistance = makeFriendly(resistance, OHM_SYMBOL)
    friendlyCapacitance = makeFriendly(capacitance, "F")
    plt.title(f"Relaxation oscillator, R = {friendlyResistance}, " +
              f"C = {friendlyCapacitance}")
    plt.xlabel("seconds")
    plt.ylabel("volts")

    cap = plt.plot(data[:,0],data[:,1],label="capacitor voltage")
    out = plt.plot(data[:,0],data[:,2],label="inverter output voltage")
    plt.legend()
    outputFile = "plot" + suffix
    plt.savefig(outputFile)
    plt.draw()
    plt.close()
    print(f"Output file is {outputFile}")
    
else:
    print(f"Description file {descrFile} not found", file=sys.stderr)
    sys.exit(1)
