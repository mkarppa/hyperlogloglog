#ifndef HYPERLOGLOGLOG_HYPERLOGLOGLOG
#define HYPERLOGLOGLOG_HYPERLOGLOGLOG

#include "HyperLogLog.hpp"
#include "Hash.hpp"
#include "common.hpp"
#include "PackedMap.hpp"
#include <cstdint>
#include <set>

namespace hyperlogloglog {
  /**
   * HyperLogLogLog. The template parameter Word determines the
   * word type and length (that is, the length of the hashes).
   */
  template<typename Word = uint64_t>
  class HyperLogLogLog {
  public:
    // these are flags
    // always compress
    static const uint8_t HYPERLOGLOGLOG_COMPRESS_WHEN_ALWAYS = 0x1;
    // only compress when an entry is added to S
    static const uint8_t HYPERLOGLOGLOG_COMPRESS_WHEN_APPEND = 0x2;
    // perform full compression
    static const uint8_t HYPERLOGLOGLOG_COMPRESS_TYPE_FULL = 0x4;
    // only allow rebasing by increasing the base slightly
    static const uint8_t HYPERLOGLOGLOG_COMPRESS_TYPE_INCREASE = 0x8;
    // maintain the invariant that the base is always the bottom value
    // (cannot be combined with any other flag)
    static const uint8_t HYPERLOGLOGLOG_COMPRESS_BOTTOM = 0x10;
    static const uint8_t HYPERLOGLOGLOG_COMPRESS_DEFAULT =
      HYPERLOGLOGLOG_COMPRESS_WHEN_ALWAYS | HYPERLOGLOGLOG_COMPRESS_TYPE_FULL;

    
    
    /**
     * Basic constructor
     * m : number of registers
     * mBits : log2(log2(n)) bits for offsets in the M registers; 
     *         should be 2 or 3
     * flags : how to perform compression (default value gives theoretical 
     *         guarantees, but might be slow initially)
     */
    explicit HyperLogLogLog(int m, int mBits = 3, 
                            int flags_ = HYPERLOGLOGLOG_COMPRESS_DEFAULT) :
      m(m), logM(log2i(m)), mBits(mBits),
      sBits(log2i(sizeof(Word)*CHAR_BIT)), flags(flags_),
      M(mBits,m), S(log2i(m), sBits), minValueCount(m),
      maxOffset((1u << mBits) - 1) {
      if (m != 1 << log2i(m))
        throw std::invalid_argument("m must be a power of two");
      
      if (flags == HYPERLOGLOGLOG_COMPRESS_TYPE_FULL ||
          flags == HYPERLOGLOGLOG_COMPRESS_TYPE_INCREASE)
        flags |= HYPERLOGLOGLOG_COMPRESS_WHEN_ALWAYS;
      if (flags == HYPERLOGLOGLOG_COMPRESS_WHEN_ALWAYS ||
          flags == HYPERLOGLOGLOG_COMPRESS_WHEN_APPEND)
        flags |= HYPERLOGLOGLOG_COMPRESS_TYPE_FULL;

      if ((flags & HYPERLOGLOGLOG_COMPRESS_BOTTOM) && (flags != HYPERLOGLOGLOG_COMPRESS_BOTTOM))
        throw std::invalid_argument("invalid flags");

      if (flags != HYPERLOGLOGLOG_COMPRESS_BOTTOM) {
        if (!((flags & HYPERLOGLOGLOG_COMPRESS_TYPE_FULL) ||
              (flags & HYPERLOGLOGLOG_COMPRESS_TYPE_INCREASE)) ||
            !((flags & HYPERLOGLOGLOG_COMPRESS_WHEN_ALWAYS) ||
              (flags & HYPERLOGLOGLOG_COMPRESS_WHEN_APPEND)))
          throw std::invalid_argument("invalid flags");
      } 
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
      if (r <= lowerBound)
        return;

      bool updated = false;
      bool sizeIncreased = false;
      int idx = S.find(j);
      Word r0 = idx >= 0 ? S.at(idx) : M.get(j) + B;
      if (r0 < r) {
        if (B <= r && r <= B + maxOffset) {
          if (idx >= 0)
            S.eraseAt(idx);
          M.set(j, r - B);
        }
        else {
          S.add(j,r);
          sizeIncreased = idx < 0;
        }
        
        if (r0 == lowerBound)
          --minValueCount;
        
        updated = true;
      }

      
      if ((updated && (flags & HYPERLOGLOGLOG_COMPRESS_WHEN_ALWAYS)) ||
          (sizeIncreased && (flags & HYPERLOGLOGLOG_COMPRESS_WHEN_APPEND)) ||
          (minValueCount == 0 && (flags == HYPERLOGLOGLOG_COMPRESS_BOTTOM)))
        compress();
    }

    

    /**
     * Returns the size of the sketch (the number of bits)
     */
    inline size_t bitSize() const {
      return M.bitSize() + S.bitSize();
    }



    /**
     * Returns a vector that contains the register values
     */
    std::vector<uint8_t> exportRegisters() const {
      std::vector<uint8_t> v(m);
      for (int i = 0; i < m; ++i)
        v[i] = get(i);
      return v;
    }



    /**
     * Returns the present estimate
     */
    double estimate() const {
      double E = 0;
      int V = 0;
      iterate([&](Word, Word r) {
          V += (r == 0);
          E += 1.0 / (1ull << r);
        });
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
     * Merges the two sketches. It is up to the user to make sure that
     * both sketches have been constructed with the same hash
     * functions; otherwise the operation is meaningless.
     */
    HyperLogLogLog merge(const HyperLogLogLog& that) const {
      if (m != that.m)
        throw std::invalid_argument("Mismatch in the number of registers");
      assert(logM == that.logM);
      if (mBits != that.mBits)
        throw std::invalid_argument("Mismatch in the number of M bits");
      if (sBits != that.sBits)
        throw std::invalid_argument("Mismatch in the number of S bits");
      if (flags != that.flags)
        throw std::invalid_argument("Mismatch in the flags");

      HyperLogLogLog H(m, mBits, flags);
      H.B = std::max(B, that.B);
      Word j = 0;
      size_t i1 = 0;
      size_t i2 = 0;
      Word r1, r2, r;
      while (i1 < S.size() && i2 < that.S.size()) {
        Word k1 = S.keyAt(i1);
        Word k2 = that.S.keyAt(i2);
        Word k = std::min(k1,k2);
        while (j < k) {
          r1 = M.get(j)+B;
          r2 = that.M.get(j)+that.B;
          H.M.set(j, std::max(r1,r2)-H.B);
          ++j;
        }
        if (k1 == k) {
          r1 = S.at(i1);
          ++i1;
        }
        else {
          r1 = M.get(j)+B;
        }
        if (k2 == k) {
          r2 = that.S.at(i2);
          ++i2;
        }
        else {
          r2 = that.M.get(j)+that.B;
        }
        r = std::max(r1,r2);
        if (H.B <= r && r <= H.B + H.maxOffset)
          H.M.set(j, r - H.B);
        else
          H.S.add(j,r);
        ++j;
      }
      while (i1 < S.size()) {
        Word k = S.keyAt(i1);
        while (j < k) {
          r1 = M.get(j)+B;
          r2 = that.M.get(j)+that.B;
          H.M.set(j, std::max(r1,r2)-H.B);
          ++j;
        }
        r1 = S.at(i1++);
        r2 = that.M.get(j)+that.B;
        r = std::max(r1,r2);
        if (H.B <= r && r <= H.B + H.maxOffset)
          H.M.set(j, r - H.B);
        else
          H.S.add(j,r);
        ++j;
      }
      while (i2 < that.S.size()) {
        Word k = that.S.keyAt(i2);
        while (j < k) {
          r1 = M.get(j)+B;
          r2 = that.M.get(j)+that.B;
          H.M.set(j, std::max(r1,r2)-H.B);
          ++j;
        }
        r1 = M.get(j) + B;
        r2 = that.S.at(i2++);
        r = std::max(r1,r2);
        if (H.B <= r && r <= H.B + H.maxOffset)
          H.M.set(j, r - H.B);
        else
          H.S.add(j,r);
        ++j;
      }
      while (j < static_cast<Word>(m)) {
        r1 = M.get(j)+B;
        r2 = that.M.get(j)+that.B;
        H.M.set(j, std::max(r1,r2)-H.B);
        ++j;
      }

      H.compress();
      return H;
    }



    /**
     * Returns the number of compression routine calls
     */
    int getCompressCount() const {
      return compressCount;
    }



    /**
     * Returns the number of rebase routine calls
     */
    int getRebaseCount() const {
      return rebaseCount;
    }



    /**
     * Converts the sketch into an uncompressed, vanilla HyperLogLog sketch.
     */
    HyperLogLog<Word> toHyperLogLog() const {
      HyperLogLog hll(m);
      iterate([&](Word j, Word r) {
          hll.addJr(j,r);
        });
      return hll;
    }


    
    /**
     * Converts a vanilla HyperLogLog sketch into a HyperLogLogLog sketch
     */
    static
    HyperLogLogLog fromHyperLogLog(const HyperLogLog<Word>& hll,
                                   int mBits = 3,
                                   int flags =
                                   HYPERLOGLOGLOG_COMPRESS_DEFAULT) {
      HyperLogLogLog hlll(hll.getM(), mBits, flags);
      Word j = 0;
      for (auto r : hll.exportRegisters())
        hlll.addJr(j++, r);
      return hlll;
    }

    
    
#ifdef HYPERLOGLOGLOG_DEBUG
    // debug getters
    const PackedVector<Word>& getM() const {
      return M;
    }

    const PackedMap<Word>& getS() const {
      return S;
    }

    uint8_t getB() const {
      return B;
    }



    uint8_t getLowerBound() const {
      return lowerBound;
    }
#endif // HYPERLOGLOGLOG_DEBUG


    
  private:
    /**
     * Performs the rebase operation, that is, adjusts things to a new base.
     */
    void rebase(uint8_t newB) {
      for (int i = 0; i < m; ++i) {
        int idx = S.find(i);
        Word r = idx >= 0 ? S.at(idx) : M.get(i) + B;
        if (newB <= r && r <= newB + maxOffset) {
          M.set(i, r - newB);
          if (idx >= 0)
            S.eraseAt(idx);
        }
        else {
          S.add(i, r);
        }
      }
      B = newB;
      ++rebaseCount;
    }

    
    
    /**
     * Iterates over all registers and applies the function to the (j,r) pairs
     */
    template<typename Fun>
    void iterate(Fun f) const {
      Word j = 0;
      for (size_t i = 0; i < S.size(); ++i) {
        Word k = S.keyAt(i);
        while (j < k) {
          f(j, M.get(j) + B);
          ++j;
        }
        f(j,S.at(i));
        ++j;
      }
      while (j < static_cast<Word>(m)) {
        f(j, M.get(j) + B);
        ++j;
      }
    }

    
    void compress() {
      if (flags & HYPERLOGLOGLOG_COMPRESS_TYPE_FULL)
        compressFull();
      else if (flags & HYPERLOGLOGLOG_COMPRESS_TYPE_INCREASE)
        compressIncrease();
      else if (flags == HYPERLOGLOGLOG_COMPRESS_BOTTOM)
        compressBottom();
      else
        assert(false && "Invalid flags");
      compressCount++;
    }



    void compressFull() {
      size_t bestNs = S.size();
      uint8_t bestPotentialBase = B;

      uint8_t potentialBase = (1u << sBits);
      uint8_t nextPotentialBase = potentialBase;

      iterate([&](Word, Word r) {
          if (r < potentialBase) {
            nextPotentialBase = potentialBase;
            potentialBase = r;
          }
          else if (r < nextPotentialBase) {
            nextPotentialBase = r;
          }
        });
      lowerBound = potentialBase;

      size_t nBelowB = 0; // this is a lower bound on ns
      while (nBelowB < bestNs && potentialBase < (1u << sBits)) {
        nextPotentialBase = (1u << sBits);
        size_t ns = 0;
        iterate([&](Word, Word r) {
            if ((r < potentialBase) || (r > potentialBase + maxOffset))
              ++ns;
            if (r == potentialBase)
              ++nBelowB;
            if (r > potentialBase && r < nextPotentialBase)
              nextPotentialBase = r;
          });

        if (ns < bestNs) {
          bestNs = ns;
          bestPotentialBase = potentialBase;
        }

        potentialBase = nextPotentialBase;
      }

      if (bestPotentialBase != B)
        rebase(bestPotentialBase);
    }


    
    void compressIncrease() {
      uint8_t potentialBase = (1u << sBits);
      lowerBound = potentialBase;
      iterate([&](Word, Word r) {
          if (B < r && r < potentialBase) 
            potentialBase = r;
          if (r < lowerBound)
            lowerBound = r;
        });

      size_t ns = 0;
      iterate([&](Word, Word r) {
          ns += (r < potentialBase) || (r > potentialBase + maxOffset);
        });

      if (ns < S.size()) {
        rebase(potentialBase);
      }
    }

    
      
    void compressBottom() {
      lowerBound = (1u << sBits);
      iterate([&](Word, Word r) {
          lowerBound = r < lowerBound ? r : lowerBound;
        });
      minValueCount = 0;
      iterate([&](Word, Word r) {
          if (r == lowerBound)
            ++minValueCount;
        });

      if (lowerBound > B) {
        rebase(lowerBound);
      }
    }


    
    /**
     * Returns the register value at register j
     */
    inline Word get(Word j) const {
      int idx = S.find(j);
      if (idx >= 0)
        return S.at(idx);
      else
        return M.get(j) + B;
    }

    
    
    int m;
    int logM;
    uint8_t mBits;
    uint8_t sBits;
    uint8_t flags;
    PackedVector<Word> M;
    PackedMap<Word> S;
    uint8_t lowerBound = 0; // Lower bound on the register values
    int minValueCount = 0; // number of minimum-valued registers
    uint8_t B = 0; // Current base value
    uint8_t maxOffset;
    int compressCount = 0;
    int rebaseCount = 0;
  };



  /**
   * Returns the minimum number of bits for the sketch with an optimal
   * dense/sparse split
   * M : an unpacked HyperLogLog sketch
   * mBits : number of bits / offset (2 or 3)
   * sBits : number of bits per sparse element (5 or 6)
   */
  int minimumBits(const std::vector<uint8_t>& M, int mBits, int sBits) {
    int m = M.size();
    int logM = log2i(m);
    std::set<uint8_t> bases(M.begin(), M.end());
    uint8_t maxOffset = (1 << mBits) - 1;
    int bestNs = M.size();
    for (uint8_t B : bases) {
      int ns = 0;
      for (int Mj : M)
        ns += Mj < B || Mj > B + maxOffset;
      bestNs = std::min(bestNs, ns);
    }
    int elemSize = logM + sBits;
    return bestNs * elemSize + m*mBits;
  }
}

#endif // HYPERLOGLOGLOG_HYPERLOGLOGLOG
