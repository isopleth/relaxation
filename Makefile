all: relaxation

relaxation: relaxation.cpp
	g++ -std=c++17 -o relaxation relaxation.cpp

run: plot.pdf

clean:
	- rm relaxation
	- rm plot.pdf
	- rm description.dat

plot.pdf : relaxation plot.py
	./relaxation 1e4 1e-6 0 5 0.6 2.5
	python3 plot.py


