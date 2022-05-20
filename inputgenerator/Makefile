CXX=c++
CXXFLAGS=-std=c++17 -O3 -march=native -pedantic -Wall -Wextra -I../external
LDFLAGS=
HDR=

all: inputgenerator

inputgenerator: inputgenerator.o
	$(CXX) -o inputgenerator inputgenerator.o $(LDFLAGS)

inputgenerator.o: inputgenerator.cpp
	$(CXX) -c $(CXXFLAGS) -o inputgenerator.o inputgenerator.cpp

clean:
	rm -vf *.o inputgenerator
