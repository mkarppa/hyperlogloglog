#ifndef HYPERLOGLOGLOG_HYPERLOGLOG_ZSTD
#define HYPERLOGLOGLOG_HYPERLOGLOG_ZSTD

#include "HyperLogLog.hpp"
#include <zstd/zstd.h>

namespace hyperlogloglog {
  /**
   * Zstd-compressed Basic HyperLogLog. The template parameter Word
   * determines the word type and length (that is, the length of the
   * hashes).
   */
  template<typename Word = uint64_t>
  class HyperLogLogZstd {
  public:
    /**
     * Basic constructor
     * m : the number of registers
     */
    explicit HyperLogLogZstd(int m) :
      m(m), logM(log2i(m)), compressedSize(0),
      Mcompressed(ZSTD_compressBound(m)), Mtemp(m,0) {
      compress();
    }

    

    /**
     * Returns the size of the sketch (the number of bits)
     */
    inline size_t bitSize() const {
      return compressedSize * CHAR_BIT;
    }



    /**
     * Adds a new element to the sketch
     */
    template<typename Object,
             typename XHashFun = decltype(farmhash<Object>),
             typename JHashFun = decltype(fibonacciHash<Word>)>
    inline void add(const Object& o, XHashFun h = farmhash<Object>,
                    JHashFun f = fibonacciHash<Word>) {
      static_assert(std::is_same<decltype(h(o)),Word>::value,
                    "Hash function type does not match the Word type of the class");
      addHash(h(o), f);
    }



    /**
     * Adds a new hash to the sketch. Potentially useful if a
     * different kind of hashing scheme is used outside the class.
     */
    template<typename JHashFun = decltype(fibonacciHash<Word>)>
    inline void addHash(Word x, JHashFun f = fibonacciHash<Word>) {
      static_assert(std::is_same<decltype(f(x,logM)),Word>::value,
                    "Hash function type does not match the Word type of the class");
      addJr(f(x,logM), rho(x));
    }



    /**
     * Adds the specific j and r values to the sketch. This may be
     * useful if full control is required of the hashing faculties.
     * j must satisfy 0 <= j < m but no checks are made
     * r must satisfy 0 <= r < log(word length) (64 for uint64_t) but no checks are made
     */
    inline void addJr(Word j, Word r) {
      if (r < lowerBound)
        return;
      
      decompress();
      Word r0 = Mtemp[j];
      if (r > r0) {
        Mtemp[j] = r;
        compress();
      }
    }
    


    /**
     * Returns a vector that contains the register values
     */
    std::vector<uint8_t> exportRegisters() const {
      decompress();
      return std::vector<uint8_t>(Mtemp.begin(), Mtemp.begin() + m);
    }



    /**
     * Returns the present estimate
     */
    double estimate() const {
      decompress();
      double E = 0;
      int V = 0;
      for (int j = 0; j < m; ++j) {
        Word r = Mtemp[j];
        V += (r == 0);
        E += 1.0 / (1ull << r);
      }
      E = HyperLogLog<Word>::alpha(m) * m * m / E;
      if (E <= 5.0 / 2.0 * m && V != 0) {
        return m*log(static_cast<double>(m)/V);
      }
      else if (E <= (1ull << 32)/30) {
        return E;
      }
      else {
        return -(1ll << 32) * log(1-E/(1ll << 32));
      }
    }


    /**
     * Merges this sketch with the other sketch and returns a new sketch
     *
     * Note: if the sketches were constructed with different hash
     * functions, the result will be nonsensical. It is up to the
     * caller to ensure that the exact same hash functions were used.
     */
    HyperLogLogZstd merge(const HyperLogLogZstd& that) const {
      if (m != that.m)
        throw std::invalid_argument("Mismatch in the number of registers");
      HyperLogLogZstd H(m);
      decompress();
      that.decompress();
      for (int j = 0; j < m; ++j)
        H.Mtemp[j] = std::max(Mtemp[j], that.Mtemp[j]);
      H.compress();
      return H;
    }


    
  private:
    void decompress() const {
      ZSTD_decompress(&Mtemp[0], Mtemp.size(), &Mcompressed[0], compressedSize);
    }

    void compress() {
      compressedSize = ZSTD_compress(&Mcompressed[0], Mcompressed.size(),
                                     &Mtemp[0], m, 1);
      lowerBound = sizeof(Word)*CHAR_BIT;
      for (Word Mj : Mtemp)
        if (Mj < lowerBound)
          lowerBound = Mj;
    }

    
    int m;
    int logM; // register address length
    size_t compressedSize;
    std::vector<char> Mcompressed;
    mutable std::vector<char> Mtemp;
    Word lowerBound = 0;
  };
}

#endif // HYPERLOGLOGLOG_HYPERLOGLOG_ZSTD
