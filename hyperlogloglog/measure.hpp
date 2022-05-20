#ifndef HYPERLOGLOGLOG_MEASURE
#define HYPERLOGLOGLOG_MEASURE

#include "common.hpp"
#include <vector>
#include <cstdint>
#include <chrono>
#include <iostream>

namespace hyperlogloglog {
  template<typename T>
  std::vector<T> readData(size_t n, size_t len);



  template<>
  inline std::vector<uint64_t> readData(size_t n, size_t) {
    auto start = std::chrono::steady_clock::now();
    std::vector<uint64_t> v(n);
    std::cin.read(reinterpret_cast<char*>(&v[0]), n*sizeof(uint64_t));
    for (auto it = v.begin(); it != v.end(); ++it)
      *it = ntohll(*it);
    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;
    double seconds = std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count()/1e9;
    std::cerr << "data reading took " << seconds << std::endl;
    return v;
  }



  template<>
  inline std::vector<std::string> readData(size_t n, size_t len) {
    auto start = std::chrono::steady_clock::now();
    std::vector<char> temp(n*len);
    std::cin.read(&temp[0], n*len);
    std::vector<std::string> v(n);
    for (size_t i = 0; i < n; ++i)
      v[i] = std::string(temp.begin() + i*len, temp.begin() + (i+1)*len);
    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;
    double seconds = std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count()/1e9;
    std::cerr << "data reading took " << seconds << std::endl;
    return v;
  }



  template<>
  inline std::vector<std::pair<int,int>> readData(size_t n, size_t) {
    auto start = std::chrono::steady_clock::now();
    std::vector<uint32_t> temp(2*n);
    std::vector<std::pair<int,int>> v(n);
    std::cin.read(reinterpret_cast<char*>(&temp[0]), 2*n*sizeof(uint32_t));
    int j, r;
    for (size_t i = 0; i < n; ++i) {
      j = ntohl(temp[2*i]);
      r = ntohl(temp[2*i+1]);
      v[i].first = j;
      v[i].second = r;
    }
    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;
    double seconds = std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count()/1e9;
    std::cerr << "data reading took " << seconds << std::endl;
    return v;
  }
}
#endif // HYPERLOGLOGLOG_MEASURE

