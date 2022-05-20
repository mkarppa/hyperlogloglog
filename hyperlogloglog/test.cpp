#define HYPERLOGLOGLOG_DEBUG
#include "HyperLogLogZstd.hpp"
#include "HyperLogLogLog.hpp"
#include "HyperLogLog.hpp"
#include "Hash.hpp"
#include "PackedMap.hpp"
#include "common.hpp"
#include "PackedVector.hpp"

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

static bool equals(const std::vector<uint8_t>& l, const std::vector<uint8_t>& r) {
  if (l.size() != r.size())
    return false;
  for (size_t i = 0; i < l.size(); ++i) {
    if (l[i] != r[i])
      return false;
  }
  return true;
}



TEST_CASE( "test_packed_vector", "[packedvector]" ) {
  hyperlogloglog::PackedVector pv(4,16);
  pv.set(0,0x0);
  pv.set(1,0x1);
  pv.set(2,0x2);
  pv.set(3,0x3);
  pv.set(4,0x4);
  pv.set(5,0x5);
  pv.set(6,0x6);
  pv.set(7,0x7);
  pv.set(8,0x8);
  pv.set(9,0x9);
  pv.set(10,0xa);
  pv.set(11,0xb);
  pv.set(12,0xc);
  pv.set(13,0xd);
  pv.set(14,0xe);
  pv.set(15,0xf);
  REQUIRE(pv.get(0) == 0x0);
  REQUIRE(pv.get(1) == 0x1);
  REQUIRE(pv.get(2) == 0x2);
  REQUIRE(pv.get(3) == 0x3);
  REQUIRE(pv.get(4) == 0x4);
  REQUIRE(pv.get(5) == 0x5);
  REQUIRE(pv.get(6) == 0x6);
  REQUIRE(pv.get(7) == 0x7);
  REQUIRE(pv.get(8) == 0x8);
  REQUIRE(pv.get(9) == 0x9);
  REQUIRE(pv.get(10) == 0xa);
  REQUIRE(pv.get(11) == 0xb);
  REQUIRE(pv.get(12) == 0xc);
  REQUIRE(pv.get(13) == 0xd);
  REQUIRE(pv.get(14) == 0xe);
  REQUIRE(pv.get(15) == 0xf);

  pv = hyperlogloglog::PackedVector(8,1024);
  
  for (uint64_t i = 0; i < 1024; ++i) {
    pv.set(i, i % 256);
  }

  for (uint64_t i = 0; i < 1024; ++i) {
    REQUIRE(pv.get(i) == i % 256);
  }

  pv = hyperlogloglog::PackedVector(10,10);
  pv.set(0, 0x01);
  pv.set(1, 0x03);
  pv.set(2, 0x05);
  pv.set(3, 0x07);
  pv.set(4, 0x09);
  pv.set(5, 0x10);
  pv.set(6, 0x2a);
  pv.set(7, 0x1b);
  pv.set(8, 0x09);
  pv.set(9, 0x12);
  REQUIRE(pv.get(0) == 0x01);  
  REQUIRE(pv.get(1) == 0x03);  
  REQUIRE(pv.get(2) == 0x05);  
  REQUIRE(pv.get(3) == 0x07);  
  REQUIRE(pv.get(4) == 0x09);  
  REQUIRE(pv.get(5) == 0x10);  
  REQUIRE(pv.get(6) == 0x2a);  
  REQUIRE(pv.get(7) == 0x1b);  
  REQUIRE(pv.get(8) == 0x09);  
  REQUIRE(pv.get(9) == 0x12);

  pv = hyperlogloglog::PackedVector(7,1024);
  for (uint64_t i = 0; i < 1024; ++i) {
    pv.set(i, i % 128);
  }
  for (uint64_t i = 0; i < 1024; ++i) {
    REQUIRE(pv.get(i) == i % 128);
  }

  pv = hyperlogloglog::PackedVector(4);
  REQUIRE(pv.size() == 0);
  REQUIRE(pv.capacity() == 0);
  pv.append(0x0);
  REQUIRE(pv.size() == 1);
  REQUIRE(pv.capacity() == 16);
  pv.append(0x1);
  pv.append(0x2);
  pv.append(0x3);
  pv.append(0x4);
  pv.append(0x5);
  pv.append(0x6);
  pv.append(0x7);
  pv.append(0x8);
  pv.append(0x9);
  pv.append(0xa);
  pv.append(0xb);
  pv.append(0xc);
  pv.append(0xd);
  pv.append(0xe);
  pv.append(0xf);
  REQUIRE(pv.size() == 16);
  REQUIRE(pv.capacity() == 16);
  pv.append(0xa);
  REQUIRE(pv.size() == 17);
  REQUIRE(pv.capacity() == 32);

  pv = hyperlogloglog::PackedVector(5);
  REQUIRE(pv.size() == 0);
  REQUIRE(pv.capacity() == 0);
  for (uint64_t i = 0; i < 1024; ++i) {
    pv.append(i % 32);
    REQUIRE(pv.size() == i+1);
    REQUIRE(pv.capacity() == (((i+1)*5+63)/64)*64/5);
  }
  for (uint64_t i = 0; i < 1024; ++i) {
    REQUIRE(pv.get(i) == i%32);
  }

  hyperlogloglog::PackedVector pv2;
  REQUIRE(pv2.size() == 0);
  REQUIRE(pv2.capacity() == 0);

  std::swap(pv,pv2);
  REQUIRE(pv.size() == 0);
  REQUIRE(pv.capacity() == 0);
  REQUIRE(pv2.size() == 1024);
  REQUIRE(pv2.capacity() == 1024);

  hyperlogloglog::PackedVector pv3(pv2);
  REQUIRE(pv3.size() == 1024);
  REQUIRE(pv3.capacity() == 1024);
  for (size_t i = 0; i < 1024; ++i) {
    REQUIRE(pv2.get(i) == pv3.get(i));
    REQUIRE(pv3.get(i) == i % 32);
    pv2.set(i,i%16);
  }
  for (size_t i = 0; i < 1024; ++i) {
    REQUIRE(pv3.get(i) == i % 32);
    REQUIRE(pv2.get(i) == i % 16);
  }

  hyperlogloglog::PackedVector pv4;
  REQUIRE(pv4.size() == 0);
  REQUIRE(pv4.capacity() == 0);
  pv4 = pv3;
  for (size_t i = 0; i < 1024; ++i) {
    REQUIRE(pv4.get(i) == i % 32);
    REQUIRE(pv3.get(i) == i % 32);
    pv3.set(i,i%15);
  }  
  for (size_t i = 0; i < 1024; ++i) {
    REQUIRE(pv4.get(i) == i % 32);
    REQUIRE(pv3.get(i) == i % 15);
  }

  pv3 = std::move(pv4);
  for (size_t i = 0; i < 1024; ++i) {
    REQUIRE(pv3.get(i) == i % 32);
    REQUIRE(pv4.get(i) == i % 15);
  }

  hyperlogloglog::PackedVector pv5(std::move(pv3));
  REQUIRE(pv3.size() == 0);
  REQUIRE(pv3.capacity() == 0);
  REQUIRE(pv5.size() == 1024);
  REQUIRE(pv5.capacity() == 1024);
  for (size_t i = 0; i < 1024; ++i) {
    REQUIRE(pv5.get(i) == i % 32);
  }
}



TEST_CASE( "test_packed_vector2", "[packedvector]" ) {
  std::mt19937 rng(0);
  size_t elemSize = 10;
  std::uniform_int_distribution<uint64_t> dist(0,0x3ff);
  size_t n = 1000;
  std::vector<uint64_t> correct(n,0);
  hyperlogloglog::PackedVector pv1(elemSize, n);
  hyperlogloglog::PackedVector pv2(elemSize);
  REQUIRE(pv1.size() == n);
  REQUIRE(pv2.size() == 0);
  for (size_t i = 0; i < n; ++i) {
    REQUIRE(pv1.get(i) == 0);
    REQUIRE(pv1.get(i) == correct[i]);
  }

  for (size_t i = 0; i < n; ++i) {
    uint64_t x = dist(rng);
    correct[i] = x;
    pv1.set(i,x);
    pv2.append(x);
  }

  auto equal = [](const std::vector<uint64_t>& v, const hyperlogloglog::PackedVector<uint64_t>& p)->bool {
    if (v.size() != p.size())
      return false;
    for (size_t i = 0; i < v.size(); ++i)
      if (v[i] != p.get(i))
        return false;
    return true;
  };
  
  REQUIRE(equal(correct, pv1));
  REQUIRE(equal(correct, pv2));

  for (size_t i = 0; i < n; ++i) {
    uint64_t x = dist(rng);
    correct.push_back(x);
    pv1.append(x);
    pv2.append(x);
  }

  REQUIRE(pv1.size() == 2*n);
  REQUIRE(pv2.size() == 2*n);
  REQUIRE(correct.size() == 2*n);

  REQUIRE(equal(correct, pv1));
  REQUIRE(equal(correct, pv2));

  auto randint = [&](size_t maxval)->size_t {
    return std::uniform_int_distribution<size_t>(0,maxval-1)(rng);
  };

  for (size_t i = 0; i < n; ++i) {
    size_t j = randint(correct.size());
    uint64_t x = dist(rng);
    correct.insert(correct.begin() + j, x);
    pv1.insert(j, x);
    pv2.insert(j, x);
  }
  REQUIRE(equal(correct, pv1));
  REQUIRE(equal(correct, pv2));

  REQUIRE(pv1.size() == 3*n);
  REQUIRE(pv2.size() == 3*n);
  REQUIRE(correct.size() == 3*n);
  for (size_t i = 0; i < n; ++i) {
    size_t j = randint(correct.size());
    correct.erase(correct.begin() + j);
    pv1.erase(j);
    pv2.erase(j);    
  }

  REQUIRE(correct.size() == 2*n);
  REQUIRE(pv1.size() == 2*n);
  REQUIRE(pv2.size() == 2*n);

  REQUIRE(equal(correct, pv1));
  REQUIRE(equal(correct, pv2));
}



TEST_CASE( "test_packed_vector3", "[packedvector]" ) {
  hyperlogloglog::PackedVector pv(8);
  REQUIRE(pv.size() == 0);
  pv.insert(0,0x12);
  REQUIRE(pv.size() == 1);
  REQUIRE(pv.get(0) == 0x12);

  pv.insert(0,0x34);
  REQUIRE(pv.size() == 2);
  REQUIRE(pv.get(0) == 0x34);
  REQUIRE(pv.get(1) == 0x12);

  pv.insert(0,0x56);
  REQUIRE(pv.size() == 3);
  REQUIRE(pv.get(0) == 0x56);
  REQUIRE(pv.get(1) == 0x34);
  REQUIRE(pv.get(2) == 0x12);

  pv.insert(2,0x78);
  REQUIRE(pv.size() == 4);
  REQUIRE(pv.get(0) == 0x56);
  REQUIRE(pv.get(1) == 0x34);
  REQUIRE(pv.get(2) == 0x78);
  REQUIRE(pv.get(3) == 0x12);

  pv.erase(2);
  REQUIRE(pv.size() == 3);
  REQUIRE(pv.get(0) == 0x56);
  REQUIRE(pv.get(1) == 0x34);
  REQUIRE(pv.get(2) == 0x12);

  pv.erase(2);
  REQUIRE(pv.size() == 2);
  REQUIRE(pv.get(0) == 0x56);
  REQUIRE(pv.get(1) == 0x34);

  pv.erase(0);
  REQUIRE(pv.size() == 1);
  REQUIRE(pv.get(0) == 0x34);

  pv.erase(0);
  REQUIRE(pv.size() == 0);
}



TEST_CASE( "test_packed_map", "[packedmap]" ) {
  int keySize = 10;
  int valueSize = 5;
  uint64_t keyMask = 0x3ff;
  uint64_t valueMask = 0x1f;
  hyperlogloglog::PackedMap pm(keySize, valueSize);
  REQUIRE(pm.size() == 0);

  std::map<uint64_t,uint64_t> map;

  std::mt19937 rng(42);
  std::uniform_int_distribution<uint64_t> keyDist(0,keyMask);
  std::uniform_int_distribution<uint64_t> valueDist(0,valueMask);

  int n = 1500;

  for (int i = 0; i < n; ++i) {
    uint64_t k = keyDist(rng);
    uint64_t v = valueDist(rng);
    map.insert_or_assign(k,v);
    pm.add(k,v);
    // std::cerr << k << " " << v << " " << ((k << valueSize) | v) << std::endl;
  }
  
  REQUIRE(map.size() == pm.size());
  // std::cerr << "size " << pm.size() << std::endl;
  
  for (auto [k, v] : map) {
    int i = pm.find(k);
    REQUIRE(i >= 0);
    REQUIRE(pm.at(i) == v);
    REQUIRE(pm.keyAt(i) == k);
  }

  std::vector<uint64_t> vec(map.size());
  std::transform(map.begin(), map.end(), vec.begin(),
                 [&](std::pair<uint64_t,uint64_t> p)->uint64_t {
                   return (p.first << valueSize) | p.second;
                 });
  std::sort(vec.begin(), vec.end());
  REQUIRE(vec.size() == pm.size());
  for (size_t i = 0; i < pm.size(); ++i) {
    uint64_t k = vec[i] >> valueSize;
    uint64_t v = vec[i]&valueMask;
    // std::cerr << k << " " << v << std::endl;
    REQUIRE(v == pm.at(i)); 
    REQUIRE(k == pm.keyAt(i));
  }

  std::set<uint64_t> keys;
  for (size_t i = 0; i < pm.size(); ++i)
    keys.insert(pm.keyAt(i));
  REQUIRE(keys.size() == pm.size());

  int m = 2;
  std::vector<uint64_t> toErase(m);
  std::vector<uint64_t> unErased(pm.size() - m);
  std::shuffle(vec.begin(), vec.end(), rng);
  std::transform(vec.begin(), vec.begin() + m, toErase.begin(),
                 [&](uint64_t x) {
                   return x >> valueSize;
                 });
  std::transform(vec.begin() + m, vec.end(), unErased.begin(),
                 [&](uint64_t x) {
                   return x >> valueSize;
                 });
  for (auto k : toErase)
    pm.erase(k);
  for (auto k : unErased) {
    int i = pm.find(k);
    REQUIRE(i >= 0);
    REQUIRE(pm.keyAt(i) == k);
    REQUIRE(pm.at(i) == map[k]);
  }
  for (auto k : toErase) {
    int i = pm.find(k);
    REQUIRE(i < 0);
  }
  toErase = std::vector<uint64_t>(unErased.begin(), unErased.begin() + m);
  unErased.erase(unErased.begin(), unErased.begin() + m);
  REQUIRE(toErase.size() == static_cast<size_t>(m));
  REQUIRE(unErased.size() == map.size() - 2*m);
  std::vector<size_t> eraseIdx(m);
  for (int i = 0; i < m; ++i) {
    int idx = pm.find(toErase[i]);
    REQUIRE(idx >= 0);
    pm.eraseAt(idx);
  }
  for (auto k : toErase)
    REQUIRE(pm.find(k) < 0);
  for (auto k : unErased) {
    int i = pm.find(k);
    REQUIRE(i >= 0);
    uint64_t v = map[k];
    REQUIRE(pm.at(i) == v);
    REQUIRE(pm.keyAt(i) == k);
  }

  for (size_t i = 1; i < pm.size(); ++i) {
    REQUIRE(pm.keyAt(i-1) < pm.keyAt(i));
  }
}



TEST_CASE( "test_farmhash", "[farmhash]" ) {
  REQUIRE(hyperlogloglog::farmhash(std::string("")) == 0x826e8074d1fa8def);
  REQUIRE(hyperlogloglog::farmhash(std::string("a")) == 0x06756523d617d714);
  REQUIRE(hyperlogloglog::farmhash(std::string("qwerty")) == 0xf0615cbf0f4109a7);
  REQUIRE(hyperlogloglog::farmhash(std::string("asdf")) == 0xcd80a1a405a2802a);
  REQUIRE(hyperlogloglog::farmhash(std::string("Hamburgevons")) == 0x3a0ede9a0a12fe52);
  REQUIRE(hyperlogloglog::farmhash(std::string("Etaoin shrdlu")) == 0x8e17742cf4450593);
  REQUIRE(hyperlogloglog::farmhash(std::string("The quick brown fox jumps over the lazy dog")) == 0x6466f758d91acaa);
  REQUIRE(hyperlogloglog::farmhash(std::string("Portez ce vieux whisky au juge blond qui fume")) == 0x5ecc8820954c4f9f);
  REQUIRE(hyperlogloglog::farmhash(std::string(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}")) == 0x5aaadcfad0cc5d66);
  REQUIRE(hyperlogloglog::farmhash(std::string("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.")) == 0xb8cb8886c1bb6d74);

  uint64_t xs[] = {
    0xfa8b7cc9187b3f05, 0x0c3d39ea865ce393, 0x88ec64e255f60c69,
    0x0b970d98db3ac6b3, 0x448ed96f193bc53c, 0xc979ba5fadd89285,
    0xdea1255738927cdb, 0x36cb7571d0d769ab, 0xdc58de1397c0b0f8,
    0x360ff1813c04971f
  };
  uint64_t hs[] = {
    0x5cb261a0ed7664da, 0x8a86effa42ce32b8, 0x2f89793a7519c66b,
    0x4dfdeac50dd41e03, 0x67c6987c2b20a7b6, 0xfc28201faab4f29e,
    0x8a59fb11ab06a2ca, 0x6e4669594468d072, 0x7ab355702b2b51a8,
    0x56dcf2491ed2f540
  };

  for (size_t i = 0; i < sizeof(xs)/sizeof(xs[0]); ++i) {
    REQUIRE(hyperlogloglog::farmhash(xs[i]) == hs[i]);
  }

  int bitCounts[64] = { 0 };
  int bitPairCounts[64][64] = { { } };
  for (uint64_t x = 0; x < 1000000; ++x) {
    uint64_t h = hyperlogloglog::farmhash(x);
    for (int i = 0; i < 64; ++i) {
      if ((h >> i) & 1) {
        ++bitCounts[i];
        for (int j = 0; j < 64; ++j) {
          bitPairCounts[i][j] += (h >> j) & 1;
        }
      }
    }
  }
  for (auto i : bitCounts) {
    REQUIRE(i >= 498000);
    REQUIRE(i <= 502000);
  }

  for (int i = 0; i < 64; ++i) {
    for (int j = 0; j < 64; ++j) {
      if (i == j) {
        REQUIRE(bitPairCounts[i][j] >= 498000);
        REQUIRE(bitPairCounts[i][j] <= 502000);
        REQUIRE(bitPairCounts[i][j] == bitCounts[i]);
      }
      else {
        REQUIRE(bitPairCounts[i][j] >= 248000);
        REQUIRE(bitPairCounts[i][j] <= 252000);
      }
    }
  }
}


TEST_CASE( "test_fibonaccihash", "[fibonaccihash]" ) {
  uint64_t xs[] = {
    0xfa8b7cc9187b3f05, 0x0c3d39ea865ce393, 0x88ec64e255f60c69,
    0x0b970d98db3ac6b3, 0x448ed96f193bc53c, 0xc979ba5fadd89285,
    0xdea1255738927cdb, 0x36cb7571d0d769ab, 0xdc58de1397c0b0f8,
    0x360ff1813c04971f
  };
  uint64_t hs[] = {
    0x8515c3db67149769, 0x5ecc354cb957df0f, 0x118f6a5ce88be09d,
    0x40c4e1a5b0cf00af, 0xacfa7cd1ccc83dec, 0x637f12757f2e70e9,
    0x7d82131a54cc51f7, 0x6f6f2ec1d9487f07, 0xdbfb45d2f836a458,
    0x38d2e55f3589698b
  };
  for (size_t i = 0; i < sizeof(xs)/sizeof(xs[0]); ++i)
    REQUIRE(hyperlogloglog::fibonacciHash(xs[i]) == hs[i]);
  
  int bitCounts[64] = { 0 };
  int bitPairCounts[64][64] = { { } };
  for (uint64_t x = 0; x < 1000000; ++x) {
    uint64_t h = hyperlogloglog::fibonacciHash(x);
    for (int i = 0; i < 64; ++i) {
      if ((h >> i) & 1) {
        ++bitCounts[i];
        for (int j = 0; j < 64; ++j) {
          bitPairCounts[i][j] += (h >> j) & 1;
        }
      }
    }
  }
  for (auto i : bitCounts) {
    REQUIRE(i >= 498000);
    REQUIRE(i <= 502000);
  }

  for (int i = 0; i < 64; ++i) {
    for (int j = 0; j < 64; ++j) {
      if (i == j) {
        REQUIRE(bitPairCounts[i][j] >= 498000);
        REQUIRE(bitPairCounts[i][j] <= 502000);
        REQUIRE(bitPairCounts[i][j] == bitCounts[i]);
      }
      else {
        REQUIRE(bitPairCounts[i][j] >= 248000);
        REQUIRE(bitPairCounts[i][j] <= 252000);
      }
    }
  }
}



TEST_CASE( "test_log2", "" ) {
  REQUIRE(hyperlogloglog::log2i(0ull) == 0);
  REQUIRE(hyperlogloglog::log2i(1ull) == 0);
  REQUIRE(hyperlogloglog::log2i(2ull) == 1);
  REQUIRE(hyperlogloglog::log2i(4ull) == 2);
  REQUIRE(hyperlogloglog::log2i(8ull) == 3);
  REQUIRE(hyperlogloglog::log2i(16ull) == 4);
  REQUIRE(hyperlogloglog::log2i(32ull) == 5);
  REQUIRE(hyperlogloglog::log2i(64ull) == 6);

  for (uint64_t i = 0; i < 64; ++i)
    REQUIRE(hyperlogloglog::log2i(1ull << i) == i);
}



TEST_CASE( "test_hyperloglog", "[hyperloglog]" ) {
  hyperlogloglog::HyperLogLog hll(16);
  REQUIRE(hll.bitSize() == 16*6);

  std::mt19937 rng(0);
  std::uniform_int_distribution<uint64_t> dist;

  std::vector<uint8_t> M(16,0);
  for (int i = 0; i < 1000; ++i) {
    uint64_t y = dist(rng);
    hll.add(y);
    uint64_t x = hyperlogloglog::farmhash(y);
    uint64_t j = hyperlogloglog::fibonacciHash(x,4);
    uint8_t r = hyperlogloglog::rho(x);
    M[j] = std::max(M[j], r);
  }

  auto M2 = hll.exportRegisters();
  for (int i = 0; i < 16; ++i) {
    REQUIRE(M[i] == M2[i]);
  }

  double E = 0;
  for (int j = 0; j < 16; ++j)
    E += 1.0 / (1llu << M[j]);
  E = 0.673 * 16 * 16 / E;
  REQUIRE(E == hll.estimate());

  hll = hyperlogloglog::HyperLogLog(1024);
  REQUIRE(hll.bitSize() == 1024*6);
  
  M = std::vector<uint8_t>(1024,0);
  for (int i = 0; i < 1000; ++i) {
    uint64_t y = dist(rng);
    hll.add(y);
    uint64_t x = hyperlogloglog::farmhash(y);
    uint64_t j = hyperlogloglog::fibonacciHash(x,10);
    uint8_t r = hyperlogloglog::rho(x);
    M[j] = std::max(M[j], r);
  }

  M2 = hll.exportRegisters();
  for (int i = 0; i < 1024; ++i) {
    REQUIRE(M[i] == M2[i]);
  }
  
  E = 0;
  for (int j = 0; j < 1024; ++j)
    E += 1.0 / (1llu << M[j]);
  E = 0.7213/(1+1.079/1024) * 1024 * 1024 / E;
  int V = 0;
  for (int j = 0; j < 1024; ++j)
    V += (M[j] == 0);
  REQUIRE(V > 0);
  E = 1024*log(1024.0/V);
  REQUIRE(E == hll.estimate());
}



TEST_CASE( "test_hyperloglog_uint64", "[hyperloglog]" ) {
  std::mt19937 rng(42);
  std::uniform_int_distribution<uint64_t> dist;

  int reps = 5000;
  double avg = 0.0;
  int n = 1000;
  int m = 64;

  for (int j = 0; j < reps; ++j) {
    hyperlogloglog::HyperLogLog hll(m);
    REQUIRE(static_cast<int>(hll.bitSize()) == m*6);
    for (int i = 0; i < n; ++i) {
      hll.add(dist(rng));
    }
    avg += hll.estimate();
  }
  avg /= reps;

  REQUIRE(std::abs(avg - n) < 1);
}



static std::string generateRandomString(std::mt19937& rng) {
  std::uniform_int_distribution<char> dist(0,25);
  std::string s;
  for (int i = 0; i < 8; ++i)
    s += 'a' + dist(rng);
  return s;
}



TEST_CASE( "test_hyperloglog_string", "[hyperloglog]" ) {
  std::mt19937 rng(1234);
  int n = 1000;
  int reps = 50000;
  int m = 32;
  double avg = 0.0;
  for (int j = 0; j < reps; ++j) {
    hyperlogloglog::HyperLogLog hll(m);
    REQUIRE(static_cast<int>(hll.bitSize()) == m*6);
    for (int i = 0; i < n; ++i) {
      hll.add(generateRandomString(rng));
    }
    avg += hll.estimate();
  }
  avg /= reps;

  REQUIRE(std::abs(avg - n) < 1);
}



TEST_CASE( "test_hyperloglog_merge", "[hyperloglog]" ) {
  int m = 128;
  hyperlogloglog::HyperLogLog hll1(m);
  hyperlogloglog::HyperLogLog hll2(m);
  hyperlogloglog::HyperLogLog hll3(m);
  int n = 10000;
  std::mt19937 rng(4321);
  std::uniform_int_distribution<uint64_t> dist;
  for (int i = 0; i < n; ++i) {
    uint64_t x = dist(rng);
    hll1.add(x);
    hll3.add(x);
  }
  for (int i = 0; i < n; ++i) {
    uint64_t x = dist(rng);
    hll2.add(x);
    hll3.add(x);
  }
  REQUIRE(hll1.estimate() != hll2.estimate());
  REQUIRE(hll2.estimate() != hll3.estimate());
  REQUIRE(hll1.estimate() != hll3.estimate());
  hyperlogloglog::HyperLogLog hll4 = hll1.merge(hll2);
  REQUIRE(hll3.estimate() == hll4.estimate());

  std::vector<uint8_t> M1 = hll1.exportRegisters();
  std::vector<uint8_t> M2 = hll2.exportRegisters();
  std::vector<uint8_t> M3 = hll3.exportRegisters();
  std::vector<uint8_t> M4 = hll4.exportRegisters();
  for (int i = 0; i < m; ++i) {
    REQUIRE(std::max(M1[i],M2[i]) == M3[i]);
    REQUIRE(M3[i] == M4[i]);
  }



  m = 64;
  hyperlogloglog::HyperLogLog hll5(m);
  hyperlogloglog::HyperLogLog hll6(m);
  hyperlogloglog::HyperLogLog hll7(m);
  n = 1000;
  for (int i = 0; i < n; ++i) {
    std::string x = generateRandomString(rng);
    hll5.add(x);
    hll7.add(x);
  }
  for (int i = 0; i < n; ++i) {
    std::string x = generateRandomString(rng);
    hll6.add(x);
    hll7.add(x);
  }
  REQUIRE(hll5.estimate() != hll6.estimate());
  REQUIRE(hll5.estimate() != hll7.estimate());
  REQUIRE(hll6.estimate() != hll7.estimate());
  hyperlogloglog::HyperLogLog hll8 = hll5.merge(hll6);
  REQUIRE(hll7.estimate() == hll8.estimate());

  std::vector<uint8_t> M5 = hll5.exportRegisters();
  std::vector<uint8_t> M6 = hll6.exportRegisters();
  std::vector<uint8_t> M7 = hll7.exportRegisters();
  std::vector<uint8_t> M8 = hll8.exportRegisters();
  for (int i = 0; i < m; ++i) {
    REQUIRE(std::max(M5[i],M6[i]) == M7[i]);
    REQUIRE(M7[i] == M8[i]);
  }
}



TEST_CASE( "test_hyperloglog_adds", "[hyperloglog]" ) {
  int m = 64;
  std::mt19937 rng(11332211);
  std::uniform_int_distribution<uint64_t> dist;
  int n = 1000;

  hyperlogloglog::HyperLogLog hll1(m);
  hyperlogloglog::HyperLogLog hll2(m);
  hyperlogloglog::HyperLogLog hll3(m);

  for (int i = 0; i < n; ++i) {
    uint64_t x = dist(rng);
    hll1.add(x);
    hll2.addHash(hyperlogloglog::farmhash(x));
    hll3.addJr(hyperlogloglog::fibonacciHash(hyperlogloglog::farmhash(x),
                                             hyperlogloglog::log2i(m)),
               hyperlogloglog::rho(hyperlogloglog::farmhash(x)));
  }
  REQUIRE(hll1.estimate() == hll2.estimate());
  REQUIRE(hll1.estimate() == hll3.estimate());
  REQUIRE(hll2.estimate() == hll3.estimate());

  REQUIRE(equals(hll1.exportRegisters(), hll2.exportRegisters()));
  REQUIRE(equals(hll1.exportRegisters(), hll3.exportRegisters()));
  REQUIRE(equals(hll2.exportRegisters(), hll3.exportRegisters()));

  for (int i = 0; i < n; ++i) {
    std::string x = generateRandomString(rng);
    hll1.add(x);
    hll2.addHash(hyperlogloglog::farmhash(x));
    hll3.addJr(hyperlogloglog::fibonacciHash(hyperlogloglog::farmhash(x),
                                             hyperlogloglog::log2i(m)),
               hyperlogloglog::rho(hyperlogloglog::farmhash(x)));
  }
  REQUIRE(hll1.estimate() == hll2.estimate());
  REQUIRE(hll1.estimate() == hll3.estimate());
  REQUIRE(hll2.estimate() == hll3.estimate());

  REQUIRE(equals(hll1.exportRegisters(), hll2.exportRegisters()));
  REQUIRE(equals(hll1.exportRegisters(), hll3.exportRegisters()));
  REQUIRE(equals(hll2.exportRegisters(), hll3.exportRegisters()));
}



TEST_CASE( "test_minimum_bits", "" ) {
  std::vector<uint8_t> M { 8, 4, 2, 1, 4, 2, 5, 3, 5, 4, 6, 2, 5, 4, 3, 4 };
  int minBits = 48;
  int mBits = 3;
  int sBits = 6;
  REQUIRE(hyperlogloglog::minimumBits(M, mBits, sBits) == minBits);

  M = std::vector<uint8_t> {7, 4, 9, 5, 10, 8, 6, 3, 6, 9, 4, 6, 6, 5, 6, 6, 9,
                            4, 7, 5, 6, 7, 7, 6, 7, 5, 5, 8, 7, 6, 6, 5};
  minBits = 96;
  REQUIRE(hyperlogloglog::minimumBits(M, mBits, sBits) == minBits);

  M = std::vector<uint8_t> {12, 10, 9, 9, 8, 8, 8, 11, 9, 8, 9, 6, 9, 9, 7, 9,
                            9, 9, 9, 9, 8, 6, 11, 9, 10, 9, 9, 13, 10, 13, 8,
                            10, 7, 7, 6, 7, 11, 9, 7, 13, 9, 8, 8, 6, 9, 8, 8,
                            10, 6, 9, 8, 9, 9, 9, 8, 9, 9, 9, 9, 9, 7, 6, 7, 12};
  minBits = 192;
  REQUIRE(hyperlogloglog::minimumBits(M, mBits, sBits) == minBits);

  M = std::vector<uint8_t> {11, 10, 13, 8, 11, 12, 11, 9, 11, 10, 10, 12, 11,
                            11, 10, 17, 8, 12, 11, 11, 11, 12, 9, 10, 10, 13,
                            9, 12, 13, 10, 9, 9, 12, 9, 16, 13, 10, 9, 11, 10,
                            11, 11, 12, 10, 9, 16, 10, 10, 10, 10, 9, 13, 12,
                            12, 11, 9, 10, 12, 8, 12, 11, 9, 12, 14, 15, 10, 9,
                            9, 11, 14, 10, 13, 12, 12, 9, 10, 11, 10, 10, 15,
                            11, 10, 7, 11, 13, 13, 12, 8, 11, 11, 11, 12, 11,
                            10, 12, 13, 10, 12, 12, 10, 11, 13, 13, 12, 13, 10,
                            12, 7, 10, 10, 11, 11, 11, 12, 11, 11, 11, 11, 10,
                            9, 10, 10, 9, 13, 11, 11, 11, 10};
  minBits = 449;
  REQUIRE(hyperlogloglog::minimumBits(M, mBits, sBits) == minBits);
}



TEST_CASE( "test_hyperlogloglog", "[hyperlogloglog]" ) {
  int m = 16;
  hyperlogloglog::HyperLogLogLog hlll(m);
  REQUIRE(hlll.bitSize() == 48);
  REQUIRE(hlll.getM().size() == 16);
  REQUIRE(hlll.getS().size() == 0);
  REQUIRE(hlll.getB() == 0);
  REQUIRE(hlll.getLowerBound() == 0);
  REQUIRE(hlll.getCompressCount() == 0);
  REQUIRE(hlll.getRebaseCount() == 0);

  hlll.addJr(0,1);
  REQUIRE(hlll.bitSize() == 48);
  REQUIRE(hlll.getM().size() == 16);
  REQUIRE(hlll.getS().size() == 0);
  REQUIRE(hlll.getB() == 0);
  REQUIRE(hlll.getM().get(0) == 1);
  for (int i = 1; i < 16; ++i)
    REQUIRE(hlll.getM().get(i) == 0);
  REQUIRE(hlll.getLowerBound() == 0);
  REQUIRE(hlll.getCompressCount() == 1);
  REQUIRE(hlll.getRebaseCount() == 0);

  hlll.addJr(1,7);
  REQUIRE(hlll.bitSize() == 48);
  REQUIRE(hlll.getM().size() == 16);
  REQUIRE(hlll.getS().size() == 0);
  REQUIRE(hlll.getB() == 0);
  REQUIRE(hlll.getM().get(0) == 1);
  REQUIRE(hlll.getM().get(1) == 7);
  for (int i = 2; i < 16; ++i)
    REQUIRE(hlll.getM().get(i) == 0);
  REQUIRE(hlll.getLowerBound() == 0);
  REQUIRE(hlll.getCompressCount() == 2);
  REQUIRE(hlll.getRebaseCount() == 0);

  hlll.addJr(2,8);
  REQUIRE(hlll.bitSize() == 58);
  REQUIRE(hlll.getM().size() == 16);
  REQUIRE(hlll.getS().size() == 1);
  REQUIRE(hlll.getB() == 0);
  REQUIRE(hlll.getM().get(0) == 1);
  REQUIRE(hlll.getM().get(1) == 7);
  for (int i = 2; i < 16; ++i)
    REQUIRE(hlll.getM().get(i) == 0);
  REQUIRE(hlll.getS().keyAt(0) == 2);
  REQUIRE(hlll.getS().at(0) == 8);
  REQUIRE(hlll.getLowerBound() == 0);
  REQUIRE(hlll.getCompressCount() == 3);
  REQUIRE(hlll.getRebaseCount() == 0);

  hlll.addJr(3,8);
  REQUIRE(hlll.bitSize() == 68);
  REQUIRE(hlll.getM().size() == 16);
  REQUIRE(hlll.getS().size() == 2);
  REQUIRE(hlll.getB() == 0);
  REQUIRE(hlll.getM().get(0) == 1);
  REQUIRE(hlll.getM().get(1) == 7);
  for (int i = 2; i < 16; ++i)
    REQUIRE(hlll.getM().get(i) == 0);
  REQUIRE(hlll.getS().keyAt(0) == 2);
  REQUIRE(hlll.getS().at(0) == 8);
  REQUIRE(hlll.getS().keyAt(1) == 3);
  REQUIRE(hlll.getS().at(1) == 8);
  REQUIRE(hlll.getLowerBound() == 0);
  REQUIRE(hlll.getCompressCount() == 4);
  REQUIRE(hlll.getRebaseCount() == 0);

  hlll.addJr(2,9);
  REQUIRE(hlll.bitSize() == 68);
  REQUIRE(hlll.getM().size() == 16);
  REQUIRE(hlll.getS().size() == 2);
  REQUIRE(hlll.getB() == 0);
  REQUIRE(hlll.getM().get(0) == 1);
  REQUIRE(hlll.getM().get(1) == 7);
  for (int i = 2; i < 16; ++i)
    REQUIRE(hlll.getM().get(i) == 0);
  REQUIRE(hlll.getS().keyAt(0) == 2);
  REQUIRE(hlll.getS().at(0) == 9);
  REQUIRE(hlll.getS().keyAt(1) == 3);
  REQUIRE(hlll.getS().at(1) == 8);
  REQUIRE(hlll.getLowerBound() == 0);
  REQUIRE(hlll.getCompressCount() == 5);
  REQUIRE(hlll.getRebaseCount() == 0);

  hlll.addJr(4,9);
  REQUIRE(hlll.bitSize() == 78);
  REQUIRE(hlll.getM().size() == 16);
  REQUIRE(hlll.getS().size() == 3);
  REQUIRE(hlll.getB() == 0);
  REQUIRE(hlll.getLowerBound() == 0);
  REQUIRE(hlll.getCompressCount() == 6);
  REQUIRE(hlll.getRebaseCount() == 0);

  hlll.addJr(5,9);
  hlll.addJr(6,9);
  REQUIRE(hlll.bitSize() == 98);
  REQUIRE(hlll.getM().size() == 16);
  REQUIRE(hlll.getS().size() == 5);
  REQUIRE(hlll.getB() == 0);
  REQUIRE(hlll.getLowerBound() == 0);
  REQUIRE(hlll.getCompressCount() == 8);
  REQUIRE(hlll.getRebaseCount() == 0);

  hlll.addJr(7,9);
  hlll.addJr(8,9);
  REQUIRE(hlll.bitSize() == 118);
  REQUIRE(hlll.getM().size() == 16);
  REQUIRE(hlll.getS().size() == 7);
  REQUIRE(hlll.getB() == 0);
  REQUIRE(hlll.getLowerBound() == 0);
  REQUIRE(hlll.getCompressCount() == 10);
  REQUIRE(hlll.getRebaseCount() == 0);

  hlll.addJr(9,9);
  REQUIRE(hlll.getB() == 7);
  REQUIRE(hlll.getM().size() == 16);
  REQUIRE(hlll.getS().size() == 7);
  REQUIRE(hlll.bitSize() == 118);
  REQUIRE(hlll.getLowerBound() == 0);
  REQUIRE(hlll.getCompressCount() == 11);
  REQUIRE(hlll.getRebaseCount() == 1);
  
  hlll.addJr(10,9);
  REQUIRE(hlll.getM().size() == 16);
  REQUIRE(hlll.getS().size() == 6);
  REQUIRE(hlll.bitSize() == 108);
  REQUIRE(hlll.getB() == 7);
  REQUIRE(hlll.getLowerBound() == 0);
  REQUIRE(hlll.getCompressCount() == 12);
  REQUIRE(hlll.getRebaseCount() == 1);

  hlll.addJr(11,2);
  hlll.addJr(12,2);
  hlll.addJr(13,2);
  hlll.addJr(14,2);
  hlll.addJr(15,2);
  REQUIRE(hlll.getM().size() == 16);
  REQUIRE(hlll.getS().size() == 1);
  REQUIRE(hlll.bitSize() == 58);
  REQUIRE(hlll.getB() == 2);
  REQUIRE(hlll.getLowerBound() == 1);
  REQUIRE(hlll.getCompressCount() == 17);
  REQUIRE(hlll.getRebaseCount() == 2);

  hlll.addJr(0,2);
  REQUIRE(hlll.getM().size() == 16);
  REQUIRE(hlll.getS().size() == 0);
  REQUIRE(hlll.bitSize() == 48);
  REQUIRE(hlll.getB() == 2);
  REQUIRE(hlll.getLowerBound() == 2);  
  REQUIRE(hlll.getCompressCount() == 18);
  REQUIRE(hlll.getRebaseCount() == 2);

  hlll.addJr(0,1);
  hlll.addJr(15,1);
  hlll.addJr(10,5);
  REQUIRE(hlll.getCompressCount() == 18);
  REQUIRE(hlll.getRebaseCount() == 2);  
}



TEST_CASE( "test_hyperlogloglog_partial", "[hyperlogloglog]" ) {
  int m = 16;
  hyperlogloglog::HyperLogLogLog hlll1(m, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_DEFAULT);
  hyperlogloglog::HyperLogLogLog hlll2(m, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_WHEN_APPEND);
  REQUIRE(hlll1.getCompressCount() == 0);
  REQUIRE(hlll1.getRebaseCount() == 0);
  REQUIRE(hlll2.getCompressCount() == 0);
  REQUIRE(hlll2.getRebaseCount() == 0);

  for (int i = 0; i < 8; ++i) {
    hlll1.addJr(i,i);
    hlll2.addJr(i,i);
  }
  REQUIRE(hlll1.getCompressCount() == 7);
  REQUIRE(hlll1.getRebaseCount() == 0);
  REQUIRE(hlll2.getCompressCount() == 0);
  REQUIRE(hlll2.getRebaseCount() == 0);

  hlll1.addJr(1,8);
  hlll2.addJr(1,8);
  REQUIRE(hlll1.getCompressCount() == 8);
  REQUIRE(hlll1.getRebaseCount() == 0);
  REQUIRE(hlll2.getCompressCount() == 1);
  REQUIRE(hlll2.getRebaseCount() == 0);
  for (int i = 0; i < 8; ++i) {
    hlll1.addJr(i,8);
    hlll2.addJr(i,8);
  }
  REQUIRE(hlll1.getCompressCount() == 15);
  REQUIRE(hlll1.getRebaseCount() == 0);
  REQUIRE(hlll2.getCompressCount() == 8);
  REQUIRE(hlll2.getRebaseCount() == 0);

  hlll1.addJr(0,9);
  hlll2.addJr(0,9);
  REQUIRE(hlll1.getCompressCount() == 16);
  REQUIRE(hlll1.getRebaseCount() == 0);
  REQUIRE(hlll2.getCompressCount() == 8);
  REQUIRE(hlll2.getRebaseCount() == 0);

  REQUIRE(hlll1.getS().size() == 8);
  REQUIRE(hlll1.getS().keyAt(0) == 0);
  REQUIRE(hlll1.getS().keyAt(1) == 1);
  REQUIRE(hlll1.getS().keyAt(2) == 2);
  REQUIRE(hlll1.getS().keyAt(3) == 3);
  REQUIRE(hlll1.getS().keyAt(4) == 4);
  REQUIRE(hlll1.getS().keyAt(5) == 5);
  REQUIRE(hlll1.getS().keyAt(6) == 6);
  REQUIRE(hlll1.getS().keyAt(7) == 7);
  REQUIRE(hlll1.getS().at(0) == 9);
  REQUIRE(hlll1.getS().at(1) == 8);
  REQUIRE(hlll1.getS().at(2) == 8);
  REQUIRE(hlll1.getS().at(3) == 8);
  REQUIRE(hlll1.getS().at(4) == 8);
  REQUIRE(hlll1.getS().at(5) == 8);
  REQUIRE(hlll1.getS().at(6) == 8);
  REQUIRE(hlll1.getS().at(7) == 8);
  
  hlll1.addJr(8,9);
  hlll2.addJr(8,9);
  REQUIRE(hlll1.getB() == 8);
  REQUIRE(hlll1.getCompressCount() == 17);
  REQUIRE(hlll1.getRebaseCount() == 1);
  REQUIRE(hlll2.getCompressCount() == 9);
  REQUIRE(hlll2.getRebaseCount() == 1);

  hlll1.addJr(9,7);
  hlll2.addJr(9,7);
  REQUIRE(hlll1.getB() == 7);
  REQUIRE(hlll1.getCompressCount() == 18);
  REQUIRE(hlll1.getRebaseCount() == 2);
  REQUIRE(hlll2.getB() == 8);
  REQUIRE(hlll2.getCompressCount() == 9);
  REQUIRE(hlll2.getRebaseCount() == 1);
  REQUIRE(hlll1.bitSize() == 108);
  REQUIRE(hlll2.bitSize() == 118);

  hyperlogloglog::HyperLogLogLog hlll3(m, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_TYPE_INCREASE);
  REQUIRE(hlll3.getB() == 0);
  REQUIRE(hlll3.getCompressCount() == 0);
  REQUIRE(hlll3.getRebaseCount() == 0);
  hlll3.addJr(1,1);
  for (int i = 2; i < 16; ++i)
    hlll3.addJr(i,9);
  REQUIRE(hlll3.getB() == 0);
  REQUIRE(hlll3.getCompressCount() == 15);
  REQUIRE(hlll3.getRebaseCount() == 0);
  hlll3.addJr(0,8);
  REQUIRE(hlll3.getB() == 1);
  REQUIRE(hlll3.getCompressCount() == 16);
  REQUIRE(hlll3.getRebaseCount() == 1);
}



TEST_CASE( "test_hyperlogloglog_uint64", "[hyperlogloglog]" ) {
  int m = 128;
  hyperlogloglog::HyperLogLogLog hlll1(m, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_DEFAULT);
  hyperlogloglog::HyperLogLogLog hlll2(m, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_WHEN_APPEND);
  hyperlogloglog::HyperLogLogLog hlll3(m, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_TYPE_INCREASE);
  hyperlogloglog::HyperLogLog hll(m);
  std::mt19937 rng(1001100);
  std::uniform_int_distribution<uint64_t> dist;
  int n = 10000;
  for (int i = 0; i < n; ++i) {
    uint64_t x = dist(rng);
    hlll1.add(x);
    hlll2.add(x);
    hlll3.add(x);
    hll.add(x);
    REQUIRE(hlll1.estimate() == hll.estimate());
    REQUIRE(hlll2.estimate() == hll.estimate());
    REQUIRE(hlll3.estimate() == hll.estimate());
    REQUIRE(equals(hlll1.exportRegisters(), hll.exportRegisters()));
    REQUIRE(equals(hlll2.exportRegisters(), hll.exportRegisters()));
    REQUIRE(equals(hlll3.exportRegisters(), hll.exportRegisters()));
    REQUIRE(static_cast<int>(hlll1.bitSize()) == hyperlogloglog::minimumBits(hlll1.exportRegisters(),3,6));
  }
}


TEST_CASE( "test_hyperlogloglog_string", "[hyperlogloglog]" ) {
  int m = 32;
  hyperlogloglog::HyperLogLogLog hlll1(m, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_DEFAULT);
  hyperlogloglog::HyperLogLogLog hlll2(m, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_WHEN_APPEND);
  hyperlogloglog::HyperLogLogLog hlll3(m, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_TYPE_INCREASE);
  hyperlogloglog::HyperLogLog hll(m);
  std::mt19937 rng(1001100);
  int n = 1000;
  for (int i = 0; i < n; ++i) {
    std::string x = generateRandomString(rng);
    hlll1.add(x);
    hlll2.add(x);
    hlll3.add(x);
    hll.add(x);
    REQUIRE(hlll1.estimate() == hll.estimate());
    REQUIRE(hlll2.estimate() == hll.estimate());
    REQUIRE(hlll3.estimate() == hll.estimate());
    REQUIRE(equals(hlll1.exportRegisters(), hll.exportRegisters()));
    REQUIRE(equals(hlll2.exportRegisters(), hll.exportRegisters()));
    REQUIRE(equals(hlll3.exportRegisters(), hll.exportRegisters()));
    REQUIRE(static_cast<int>(hlll1.bitSize()) == hyperlogloglog::minimumBits(hlll1.exportRegisters(),3,6));
  }
}



TEST_CASE( "test_hyperlogloglog_merge", "[hyperlogloglog]" ) {
  int m = 1024;
  hyperlogloglog::HyperLogLogLog hlll1(m);
  hyperlogloglog::HyperLogLogLog hlll2(m);
  hyperlogloglog::HyperLogLog hll(m);
  std::mt19937 rng(123874);
  std::uniform_int_distribution<uint64_t> dist;
  int n = 10000;
  for (int i = 0; i < n; ++i) {
    uint64_t x = dist(rng);
    hlll1.add(x);
    hll.add(x);
  }
  for (int i = 0; i < n; ++i) {
    uint64_t x = dist(rng);
    hlll2.add(x);
    hll.add(x);
  }
  n = 1200;
  for (int i = 0; i < n; ++i) {
    std::string x = generateRandomString(rng);
    hlll1.add(x);
    hll.add(x);
  }
  for (int i = 0; i < n; ++i) {
    std::string x = generateRandomString(rng);
    hlll2.add(x);
    hll.add(x);
  }
  hyperlogloglog::HyperLogLogLog hlll3 = hlll1.merge(hlll2);
  REQUIRE(hlll1.estimate() != hll.estimate());
  REQUIRE(hlll2.estimate() != hll.estimate());
  REQUIRE(hlll3.estimate() == hll.estimate());
  REQUIRE(equals(hlll3.exportRegisters(), hll.exportRegisters()));
  REQUIRE(hyperlogloglog::minimumBits(hll.exportRegisters(), 3, 6) == static_cast<int>(hlll3.bitSize()));
  int N = 100;
  m = 2048;
  for (int i = 0; i < N; ++i) {
    hlll1 = hyperlogloglog::HyperLogLogLog(m);
    hlll2 = hyperlogloglog::HyperLogLogLog(m);
    hll = hyperlogloglog::HyperLogLog(m);
    std::uniform_int_distribution<uint64_t> rdist(1,63);
    for (int j = 0; j < m; ++j) {
      uint64_t x = rdist(rng);
      hlll1.addJr(j, x);
      hll.addJr(j,x);
      x = rdist(rng);
      hlll2.addJr(j, x);
      hll.addJr(j,x);
    }
    hlll3 = hlll1.merge(hlll2);
    REQUIRE(hlll3.estimate() == hll.estimate());
    REQUIRE(equals(hll.exportRegisters(), hlll3.exportRegisters()));
  }
}



TEST_CASE( "test_hyperloglogzstd", "[hyperloglogzstd]" ) {
  int m = 128;
  hyperlogloglog::HyperLogLog hll(m);
  hyperlogloglog::HyperLogLogZstd hllz(m);
  std::mt19937 rng(123874);
  std::uniform_int_distribution<uint64_t> dist;
  int n = 1000;
  for (int i = 0; i < n; ++i) {
    uint64_t x = dist(rng);
    hll.add(x);
    hllz.add(x);
    REQUIRE(hll.estimate() == hllz.estimate());
    std::vector<uint8_t> M1 = hll.exportRegisters();
    std::vector<uint8_t> M2 = hllz.exportRegisters();
    REQUIRE(equals(M1,M2));
  }

  m = 256;
  hll = hyperlogloglog::HyperLogLog(m);
  hllz = hyperlogloglog::HyperLogLogZstd(m);
  for (int i = 0; i < n; ++i) {
    std::string x = generateRandomString(rng);
    hll.add(x);
    hllz.add(x);
    REQUIRE(hll.estimate() == hllz.estimate());
    std::vector<uint8_t> M1 = hll.exportRegisters();
    std::vector<uint8_t> M2 = hllz.exportRegisters();
    REQUIRE(equals(M1,M2));
  }

  m = 64;
  hll = hyperlogloglog::HyperLogLog(m);
  hyperlogloglog::HyperLogLogZstd hllz1(m);
  hyperlogloglog::HyperLogLogZstd hllz2(m);
  for (int i = 0; i < n; ++i) {
    uint64_t x = dist(rng);
    hll.add(x);
    hllz1.add(x);
  }
  REQUIRE(hll.estimate() == hllz1.estimate());
  REQUIRE(equals(hll.exportRegisters(), hllz1.exportRegisters()));
  for (int i = 0; i < n; ++i) {
    std::string x = generateRandomString(rng);
    hll.add(x);
    hllz2.add(x);
  }
  REQUIRE(hll.estimate() != hllz1.estimate());
  REQUIRE(hll.estimate() != hllz2.estimate());
  REQUIRE(!equals(hll.exportRegisters(), hllz1.exportRegisters()));
  REQUIRE(!equals(hll.exportRegisters(), hllz2.exportRegisters()));
  hllz = hllz1.merge(hllz2);
  REQUIRE(hll.estimate() == hllz.estimate());
  REQUIRE(equals(hll.exportRegisters(), hllz.exportRegisters()));
}

TEST_CASE( "test_hyperlogloglog_bottom_uint64", "[hyperlogloglog]" ) {
  int m = 256;

  REQUIRE_THROWS_AS(hyperlogloglog::HyperLogLogLog(m, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_BOTTOM |
                                                   hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_DEFAULT),
                    std::invalid_argument);
  REQUIRE_THROWS_AS(hyperlogloglog::HyperLogLogLog(m, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_BOTTOM |
                                                   hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_WHEN_ALWAYS),
                    std::invalid_argument);
  REQUIRE_THROWS_AS(hyperlogloglog::HyperLogLogLog(m, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_BOTTOM |
                                                   hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_WHEN_APPEND),
                    std::invalid_argument);
  REQUIRE_THROWS_AS(hyperlogloglog::HyperLogLogLog(m, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_BOTTOM |
                                                   hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_TYPE_FULL),
                    std::invalid_argument);
  REQUIRE_THROWS_AS(hyperlogloglog::HyperLogLogLog(m, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_BOTTOM |
                                                   hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_TYPE_INCREASE),
                    std::invalid_argument);
  REQUIRE_NOTHROW(hyperlogloglog::HyperLogLogLog(m, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_BOTTOM));

  hyperlogloglog::HyperLogLogLog hll(m);
  hyperlogloglog::HyperLogLogLog hlll1(m, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_BOTTOM);
  hyperlogloglog::HyperLogLogLog hlll2(m, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_DEFAULT);
  std::mt19937 rng(10110011);
  std::uniform_int_distribution<uint64_t> dist;
  int n = 10000;
  for (int i = 0; i < n; ++i) {
    uint64_t x = dist(rng);
    hll.add(x);
    hlll1.add(x);
    hlll2.add(x);
    REQUIRE(hlll1.estimate() == hll.estimate());
    REQUIRE(hlll2.estimate() == hll.estimate());
    std::vector<uint8_t> M = hll.exportRegisters();
    REQUIRE(equals(hlll1.exportRegisters(), M));
    REQUIRE(equals(hlll2.exportRegisters(), M));
    REQUIRE(static_cast<int>(hlll2.bitSize()) == hyperlogloglog::minimumBits(hlll1.exportRegisters(),3,6));
    REQUIRE(hlll2.bitSize() <= hlll1.bitSize());
    uint8_t minimumM = 255;
    for (uint8_t Mj : M)
      minimumM = std::min(minimumM, Mj);
    REQUIRE(hlll1.getB() == minimumM);
  }
}



TEST_CASE( "test_hyperlogloglog_bottom_str", "[hyperlogloglog]" ) {
  int m = 32;

  hyperlogloglog::HyperLogLogLog hll(m);
  hyperlogloglog::HyperLogLogLog hlll1(m, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_BOTTOM);
  hyperlogloglog::HyperLogLogLog hlll2(m, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_DEFAULT);
  std::mt19937 rng(0x2def25ad);
  int n = 10000;
  for (int i = 0; i < n; ++i) {
    std::string x = generateRandomString(rng);
    hll.add(x);
    hlll1.add(x);
    hlll2.add(x);
    REQUIRE(hlll1.estimate() == hll.estimate());
    REQUIRE(hlll2.estimate() == hll.estimate());
    std::vector<uint8_t> M = hll.exportRegisters();
    REQUIRE(equals(hlll1.exportRegisters(), M));
    REQUIRE(equals(hlll2.exportRegisters(), M));
    REQUIRE(static_cast<int>(hlll2.bitSize()) == hyperlogloglog::minimumBits(hlll1.exportRegisters(),3,6));
    REQUIRE(hlll2.bitSize() <= hlll1.bitSize());
    uint8_t minimumM = 255;
    for (uint8_t Mj : M)
      minimumM = std::min(minimumM, Mj);
    REQUIRE(hlll1.getB() == minimumM);
  }
}


TEST_CASE( "test_hyperlogloglog_bottom_jr", "[hyperlogloglog]" ) {
  int m = 1024;
  hyperlogloglog::HyperLogLogLog hll(m);
  hyperlogloglog::HyperLogLogLog hlll1(m, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_BOTTOM);
  hyperlogloglog::HyperLogLogLog hlll2(m, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_DEFAULT);
  std::mt19937 rng(0x2f9820f0);
  std::uniform_int_distribution<uint64_t> jDist(0,m-1);
  std::uniform_int_distribution<uint64_t> rDist(1,63);
  int n = 10000;
  for (int i = 0; i < n; ++i) {
    uint64_t j = jDist(rng);
    uint64_t r = rDist(rng);
    hll.addJr(j,r);
    hlll1.addJr(j,r);
    hlll2.addJr(j,r);
    REQUIRE(hlll1.estimate() == hll.estimate());
    REQUIRE(hlll2.estimate() == hll.estimate());
    std::vector<uint8_t> M = hll.exportRegisters();
    REQUIRE(equals(hlll1.exportRegisters(), M));
    REQUIRE(equals(hlll2.exportRegisters(), M));
    REQUIRE(static_cast<int>(hlll2.bitSize()) == hyperlogloglog::minimumBits(hlll1.exportRegisters(),3,6));
    REQUIRE(hlll2.bitSize() <= hlll1.bitSize());
    uint8_t minimumM = 255;
    for (uint8_t Mj : M)
      minimumM = std::min(minimumM, Mj);
    REQUIRE(hlll1.getB() == minimumM);
  }
}



TEST_CASE( "test_hyperlogloglog_bottom_merge", "[hyperlogloglog]" ) {
  const int m = 128;
  const int N = 100;
  std::mt19937 rng(0x4f992ab4);
  std::uniform_int_distribution<uint64_t> dist;
  for (int i = 0; i < N; ++i) {
    hyperlogloglog::HyperLogLogLog hlll1(m, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_BOTTOM);
    hyperlogloglog::HyperLogLogLog hlll2(m, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_BOTTOM);
    hyperlogloglog::HyperLogLog hll(m);
    int n = 10000;
    for (int i = 0; i < n; ++i) {
      uint64_t x = dist(rng);
      hlll1.add(x);
      hll.add(x);
    }
    
    REQUIRE(hlll1.estimate() == hll.estimate());
    
    std::vector<uint8_t> M = hlll1.exportRegisters();
    uint8_t minimumM = 255;
    for (auto Mj : M)
      minimumM = Mj < minimumM ? Mj : minimumM;
    REQUIRE(hlll1.getB() == minimumM);
    
    REQUIRE(equals(hll.exportRegisters(), M));
    
    for (int i = 0; i < n; ++i) {
      uint64_t x = dist(rng);
      hlll2.add(x);
      hll.add(x);
    }
    
    M = hlll2.exportRegisters();
    minimumM = 255;
    for (auto Mj : M)
      minimumM = Mj < minimumM ? Mj : minimumM;
    REQUIRE(hlll2.getB() == minimumM);
    
    hyperlogloglog::HyperLogLogLog hlll3 = hlll1.merge(hlll2);
    M = hlll3.exportRegisters();
    minimumM = 255;
    for (auto Mj : M)
      minimumM = Mj < minimumM ? Mj : minimumM;
    REQUIRE(hlll3.getB() == minimumM);
    REQUIRE(hlll1.estimate() != hll.estimate());
    REQUIRE(hlll2.estimate() != hll.estimate());
    REQUIRE(hlll3.estimate() == hll.estimate());
    REQUIRE(equals(M, hll.exportRegisters()));
    REQUIRE(hyperlogloglog::minimumBits(hll.exportRegisters(), 3, 6) <= static_cast<int>(hlll3.bitSize()));
  }
}



TEST_CASE( "test_hyperlogloglog_conversion", "[hyperlogloglog]" ) {
  const int m = 128;
  const int n = 10000;
  std::mt19937 rng(0x27bae2f3);
  std::uniform_int_distribution<uint64_t> dist;
  hyperlogloglog::HyperLogLog hll(m);
  hyperlogloglog::HyperLogLogLog hllld(m, 3,
                       hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_DEFAULT);
  hyperlogloglog::HyperLogLogLog hlllai(m, 3,
                       hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_WHEN_APPEND |
                        hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_TYPE_INCREASE);
  hyperlogloglog::HyperLogLogLog hlllb(m, 3,
                       hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_BOTTOM);

  for (int i = 0; i < n; ++i) {
    uint64_t y = dist(rng);
    hll.add(y);
    hllld.add(y);
    hlllai.add(y);
    hlllb.add(y);
  }

  REQUIRE(hll.estimate() == hllld.estimate());
  REQUIRE(hll.estimate() == hlllai.estimate());
  REQUIRE(hll.estimate() == hlllb.estimate());
  std::vector<uint8_t> M = hll.exportRegisters();
  std::vector<uint8_t> Md = hllld.exportRegisters();
  std::vector<uint8_t> Mai = hlllai.exportRegisters();
  std::vector<uint8_t> Mb = hlllb.exportRegisters();
  for (int j = 0; j < m; ++j) {
    REQUIRE(M[j] == Md[j]);
    REQUIRE(M[j] == Mai[j]);
    REQUIRE(M[j] == Mb[j]);
  }

  hyperlogloglog::HyperLogLog hlld = hllld.toHyperLogLog();
  hyperlogloglog::HyperLogLog hllai = hlllai.toHyperLogLog();
  hyperlogloglog::HyperLogLog hllb = hlllb.toHyperLogLog();
  REQUIRE(hll.estimate() == hlld.estimate());
  REQUIRE(hll.estimate() == hllai.estimate());
  REQUIRE(hll.estimate() == hllb.estimate());
  Md = hlld.exportRegisters();
  Mai = hllai.exportRegisters();
  Mb = hllb.exportRegisters();
  for (int j = 0; j < m; ++j) {
    REQUIRE(Md[j] == M[j]);
    REQUIRE(Mai[j] == M[j]);
    REQUIRE(Mb[j] == M[j]);
  }

  hyperlogloglog::HyperLogLogLog hllld2 = hyperlogloglog::HyperLogLogLog<uint64_t>::fromHyperLogLog(hll, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_DEFAULT);
  hyperlogloglog::HyperLogLogLog hlllai2 = hyperlogloglog::HyperLogLogLog<uint64_t>::fromHyperLogLog(hll, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_TYPE_INCREASE | hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_WHEN_APPEND);
  hyperlogloglog::HyperLogLogLog hlllb2 = hyperlogloglog::HyperLogLogLog<uint64_t>::fromHyperLogLog(hll, 3, hyperlogloglog::HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_BOTTOM);

  REQUIRE(hllld.getB() == hllld2.getB());
  REQUIRE(hllld.getLowerBound() == hllld2.getLowerBound());
  REQUIRE(hllld.getM().size() == hllld2.getM().size());
  REQUIRE(hllld.getS().size() == hllld2.getS().size());
  for (size_t j = 0; j < hllld.getS().size(); ++j)
    REQUIRE(hllld.getS().at(j) == hllld2.getS().at(j));
  Md = hllld.exportRegisters();
  for (int j = 0; j < m; ++j)
    REQUIRE(M[j] == Md[j]);

  REQUIRE(hlllai.getB() == hlllai2.getB());
  // the lower bound may not be updated accurately
  REQUIRE(hlllai.getLowerBound() >= hlllai2.getLowerBound());
  REQUIRE(hlllai.getM().size() == hlllai2.getM().size());
  REQUIRE(hlllai.getS().size() == hlllai2.getS().size());
  for (size_t j = 0; j < hlllai.getS().size(); ++j)
    REQUIRE(hlllai.getS().at(j) == hlllai2.getS().at(j));
  Md = hlllai.exportRegisters();
  for (int j = 0; j < m; ++j)
    REQUIRE(M[j] == Md[j]);

  REQUIRE(hlllb.getB() == hlllb2.getB());
  REQUIRE(hlllb.getLowerBound() == hlllb2.getLowerBound());
  REQUIRE(hlllb.getM().size() == hlllb2.getM().size());
  REQUIRE(hlllb.getS().size() == hlllb2.getS().size());
  for (size_t j = 0; j < hlllb.getS().size(); ++j)
    REQUIRE(hlllb.getS().at(j) == hlllb2.getS().at(j));
  Md = hlllb.exportRegisters();
  for (int j = 0; j < m; ++j)
    REQUIRE(M[j] == Md[j]);
}

