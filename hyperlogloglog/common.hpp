#ifndef HYPERLOGLOGLOG_COMMON
#define HYPERLOGLOGLOG_COMMON

#include <arpa/inet.h>
#include <cstdint>

namespace hyperlogloglog {
  template<typename T>
  inline int clz(T x);

  template<>
  inline int clz(unsigned int x) {
    return __builtin_clz(x);
  }

  template<>
  inline int clz(unsigned long x) {
    return __builtin_clzl(x);
  }

  template<>
  inline int clz(unsigned long long x) {
    return __builtin_clzll(x);
  }

  template<typename T>
  int rho(T x) {
    return clz(x) + 1;
  }

  template<typename T>
  constexpr T log2i(T x) {
    return x < 2 ? 0 : 1 + log2i(x >> 1);
  }



#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#ifndef htonll // MacOS X defines this as a macro
    inline uint64_t htonll(uint64_t x) {
    return (static_cast<uint64_t>(htonl(x & 0xffffffff)) << 32) |
      (htonl(x >> 32));
  }

  inline uint64_t ntohll(uint64_t x) {
    return (static_cast<uint64_t>(ntohl(x & 0xffffffff)) << 32) |
      (htonl(x >> 32));
  }
#endif // htonll
#endif // __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
}

#endif // HYPERLOGLOGLOG_COMMON
