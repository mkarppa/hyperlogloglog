CXX=c++
CXXFLAGS=-std=c++17 -O3 -march=native -pedantic -Wall -Wextra -I../external/datasketches/hll/ -I../external/datasketches/common/ -I../external/datasketches/cpc/ -I../external/ -Wno-type-limits
LDFLAGS=

all: measure

measure: measure.o 
	$(CXX) -o measure measure.o $(LDFLAGS)

measure.o: measure.cpp
	$(CXX) $(CXXFLAGS) -c measure.cpp -o measure.o

clean:
	rm -vf *.o measure
