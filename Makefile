all:
	g++ -std=c++17 -o relaxation relaxation.cpp

run:
	./relaxation 1e4 1e-6 0 5 0.6 2.5
	python3 plot.py


