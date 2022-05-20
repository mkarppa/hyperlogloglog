#ifndef HYPERLOGLOGLOG_HASH
#define HYPERLOGLOGLOG_HASH

#include <farmhash/farmhash.h>
#include <climits>
#include <cstdint>
#include <string>

namespace hyperlogloglog {
  template<typename T, typename Word = uint64_t>
  Word fibonacciHash(const T& x, int b = CHAR_BIT*sizeof(Word));

  template<>
  inline uint64_t fibonacciHash(const uint64_t& x, int b) {
    static_assert(CHAR_BIT*sizeof(uint64_t) == 64);
    return 0x9e3779b97f4a7c15*x >> (64-b);
  }

  template<typename T, typename Word = uint64_t>
  Word farmhash(const T& x);

  template<>
  inline uint64_t farmhash(const std::string& x) {
    return farmhash::Hash64(x);
  }

  template<>
  inline uint64_t farmhash(const uint64_t& x) {
    return farmhash::Fingerprint(x);
  }
}

#endif // HYPERLOGLOGLOG_HASH
