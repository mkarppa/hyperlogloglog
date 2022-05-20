CXX=c++
CXXFLAGS=-std=c++17 -O3 -march=native -pedantic -Wall -Wextra -I../external
LDFLAGS=-L../external/zstd/ -lzstd
HDR=PackedVector.hpp PackedMap.hpp Hash.hpp HyperLogLog.hpp HyperLogLogLog.hpp common.hpp

all: measure

measure: measure.o farmhash.o
	$(CXX) -o measure measure.o farmhash.o $(LDFLAGS)

test: test.o farmhash.o
	$(CXX) -o test test.o farmhash.o $(LDFLAGS) 

measure.o: measure.cpp $(HDR)
	$(CXX) $(CXXFLAGS) -c measure.cpp -o measure.o

test.o: test.cpp $(HDR)
	$(CXX) $(CXXFLAGS) -c test.cpp -o test.o

farmhash.o: ../external/farmhash/farmhash.cc ../external/farmhash/farmhash.h
	$(CXX) $(CXXFLAGS) -Wno-overflow -c -o farmhash.o ../external/farmhash/farmhash.cc

clean:
	rm -vf *.o test measure
