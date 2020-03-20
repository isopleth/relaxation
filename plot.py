
import csv
import matplotlib.pyplot as plt
import numpy as np
import sys
import os

descrFile = "description.dat"
if os.path.exists(descrFile):

    with open(descrFile, "rt", newline="\n") as descr:
        filename = descr.readline().strip().split(" ")[2]
        resistance = float(descr.readline().strip().split(" ")[2])
        capacitance = float(descr.readline().strip().split(" ")[2])

        data = np.genfromtxt(filename, delimiter=",")
        plt.title(f"Relaxation oscillator, R = {resistance}, C = {capacitance}")
        plt.xlabel("seconds")
        plt.ylabel("volts")

        cap = plt.plot(data[:,0],data[:,1],label="capacitor")
        out = plt.plot(data[:,0],data[:,2],label="output")
        plt.legend()
        outputFile = "plot.pdf"
        plt.savefig(outputFile)
        plt.draw()
        plt.close()
        print(f"Output file is {outputFile}")

        
else:
    print(f"Description file {descrFile} not found", file=sys.stderr)
    sys.exit(1)


