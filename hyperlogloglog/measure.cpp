#include "measure.hpp"
#include "HyperLogLogZstd.hpp"
#include "HyperLogLogLog.hpp"
#include <tclap/CmdLine.h>
#include <memory>

using std::chrono::duration_cast;
using std::chrono::nanoseconds;
using std::chrono::steady_clock;
using std::make_unique;
using std::unique_ptr;
using std::make_pair;
using std::pair;
using std::vector;
using std::string;
using std::cerr;
using std::cout;
using std::endl;
using std::cin;
using TCLAP::CmdLine;
using TCLAP::SwitchArg;
using TCLAP::UnlabeledValueArg;
using TCLAP::ValuesConstraint;
using TCLAP::ValueArg;
using namespace hyperlogloglog;

/**
 * Helper class for emulating hyperloglog interface for hashing only
 */
class Hasher {
public:
  explicit Hasher(int m) : m(m) { }
  
  template<typename T>
  void add(const T& o) {
    uint64_t x = hyperlogloglog::farmhash<T>(o);
    M = hyperlogloglog::fibonacciHash<uint64_t>(x,m);
  }

  void addJr(int, int) {
    assert(false && "this is an unsupported operation");
  }

  Hasher merge(const Hasher&) {
    assert(false && "this is an unsupported operation");
    return Hasher(1);
  }
  
  static uint64_t M; // to prevent code deletion
  int m;
};
uint64_t Hasher::M = 0;



template<typename T>
static void adds(T& h, vector<string>::const_iterator begin, vector<string>::const_iterator end) {
  for (auto it = begin; it != end; ++it)
    h.add(*it);
}

template<typename T>
static void adds(T& h, vector<uint64_t>::const_iterator begin, vector<uint64_t>::const_iterator end) {
  for (auto it = begin; it != end; ++it)
    h.add(*it);
}

template<typename T>
static void adds(T& h, vector<pair<int,int>>::const_iterator begin, vector<pair<int,int>>::const_iterator end) {
  for (auto it = begin; it != end; ++it)
    h.addJr(it->first, it->second);
}

template<typename T>
static void adds(T& h, const vector<string>& v) {
  for (const string& o : v)
    h.add(o);
}

template<typename T>
static void adds(T& h, const vector<uint64_t>& v) {
  for (const uint64_t& o : v)
    h.add(o);
}

template<typename T>
static void adds(T& h, const vector<pair<int,int>>& v) {
  for (const pair<int,int>& o : v)
    h.addJr(o.first, o.second);
}



template<typename T>
string toString(const T& s);

template<>
string toString(const string& s) {
  return s;
}

template<>
string toString(const uint64_t& s) {
  return std::to_string(s);
}

template<>
string toString(const pair<int,int>& s) {
  return std::to_string(s.first) + " " + std::to_string(s.second);
}



template<typename T>
static double getEstimate(T& H) {
  return H.estimate();
}

template<>
double getEstimate(Hasher&) {
  return 0;
}

template<typename T>
static size_t getBitsize(T& H) {
  return H.bitSize();
}

template<>
size_t getBitsize(Hasher&) {
  return 0;
}

template<typename T>
static int getCompressCount(T&) {
  return 0;
}

template<>
int getCompressCount(HyperLogLogLog<uint64_t>& H) {
  return H.getCompressCount();
}

template<typename T>
static int getRebaseCount(T&) {
  return 0;
}

template<>
int getRebaseCount(HyperLogLogLog<uint64_t>& H) {
  return H.getRebaseCount();
}

template<typename T>
void report(double seconds, T& H) {
  double estimate = getEstimate(H);
  size_t bitsize = getBitsize(H);
  int compressCount = getCompressCount(H);
  int rebaseCount = getRebaseCount(H);
  
  fprintf(stdout, "time %g\n", seconds);
  fprintf(stdout, "estimate %f\n", estimate);
  fprintf(stdout, "bitsize %zu\n", bitsize);
  fprintf(stdout, "compressCount %d\n", compressCount);
  fprintf(stdout, "rebaseCount %d\n", rebaseCount);
}



template<typename AlgorithmType>
static unique_ptr<AlgorithmType> constructImplementation(int m, int flags);

template<>
unique_ptr<HyperLogLog<uint64_t>> constructImplementation(int m, int) {
  return make_unique<HyperLogLog<uint64_t>>(m);
}

template<>
unique_ptr<HyperLogLogZstd<uint64_t>> constructImplementation(int m, int) {
  return make_unique<HyperLogLogZstd<uint64_t>>(m);
}

template<>
unique_ptr<HyperLogLogLog<uint64_t>> constructImplementation(int m, int flags) {
  return make_unique<HyperLogLogLog<uint64_t>>(m, 3, flags);
}

template<>
unique_ptr<Hasher> constructImplementation(int m, int) {
  return make_unique<Hasher>(m);
}

template<typename DataType, typename AlgorithmType>
static void measureMerge(AlgorithmType& H1, AlgorithmType& H2,
                         const vector<DataType>& data) {
  size_t n1 = data.size() / 2;
  adds(H1, data.begin(), data.begin() + n1);
  adds(H2, data.begin() + n1, data.end());
  auto start = steady_clock::now();
  auto H = H1.merge(H2);
  auto end = steady_clock::now();
  auto diff = end - start;
  double seconds = duration_cast<nanoseconds>(diff).count()/1e9;
  report(seconds, H);
}

template<typename DataType, typename AlgorithmType>
static void measureMerge(int m, int flags,
                         const vector<DataType>& data) {
  unique_ptr<AlgorithmType> impl1 = constructImplementation<AlgorithmType>(m,flags);
  unique_ptr<AlgorithmType> impl2 = constructImplementation<AlgorithmType>(m,flags);
  measureMerge(*impl1, *impl2, data);
}



template<typename DataType, typename AlgorithmType>
static void measureQuery(AlgorithmType& H, const vector<DataType>& data) {
    auto start = steady_clock::now();
    adds(H, data);
    auto end = steady_clock::now();
    auto diff = end - start;
    double seconds = duration_cast<nanoseconds>(diff).count()/1e9;
    report(seconds, H);
}


template<typename DataType, typename AlgorithmType>
static void measureQuery(int m, int flags, const vector<DataType>& data) {
  unique_ptr<AlgorithmType> impl = constructImplementation<AlgorithmType>(m,flags);
  measureQuery(*impl, data);
}

template<typename DataType,typename AlgorithmType>
static void measure(const string& mode,
                    int m,
                    int flags,
                    const vector<DataType>& data) {
  if (mode == "merge")
    measureMerge<DataType,AlgorithmType>(m, flags, data);
  else if (mode == "query")
    measureQuery<DataType,AlgorithmType>(m, flags, data);
}

template<typename DataType>
static void measure(const string& mode,
                    const string& algo,
                    int m,
                    int flags,
                    size_t n,
                    size_t len) {
  vector<DataType> data = readData<DataType>(n, len);
  if (algo == "hyperloglog")
    measure<DataType,HyperLogLog<uint64_t>>(mode, m, flags, data);
  else if (algo == "hyperloglog")
    measure<DataType,HyperLogLog<uint64_t>>(mode, m, flags, data);
  else if (algo == "hyperloglogzstd")
    measure<DataType,HyperLogLogZstd<uint64_t>>(mode, m, flags, data);
  else if (algo == "hyperlogloglog")
    measure<DataType,HyperLogLogLog<uint64_t>>(mode, m, flags, data);  
  else if (algo == "hashonly")
    measure<DataType,Hasher>(mode, m, flags, data);
}


static void measure(const string& mode,
                    const string& algo,
                    const string& dt,
                    int m,
                    int flags,
                    size_t n,
                    size_t len) {
  if (dt == "uint64")
    measure<uint64_t>(mode, algo, m, flags, n, len);
  if (dt == "str") 
    measure<string>(mode, algo, m, flags, n, len);
  if (dt == "jr")
    measure<pair<int,int>>(mode, algo, m, flags, n, len);
}




int main(int argc, char* argv[]) {
  try {
    CmdLine cmd("Make measurements of hyperlogloglog.", ' ', "", false);
    SwitchArg helpSwitch("h", "help", "Print this message", cmd, false);
    vector<string> modeValues { "query", "merge" };
    ValuesConstraint<string> modeValuesConstraint(modeValues);
    UnlabeledValueArg<string> modeArg("mode", "measurement mode", true, "query",
                                      &modeValuesConstraint, cmd);
    vector<string> algorithmValues { "hyperloglog", "hyperloglogzstd", "hyperlogloglog", "hashonly" };
    ValuesConstraint<string> algorithmValuesConstraint(algorithmValues);
    UnlabeledValueArg<string> algorithmArg("algorithm", "algorithm to measure",
                                           true, "hyperloglog",
                                           &algorithmValuesConstraint, cmd);
    vector<string> datatypeValues { "uint64", "str", "jr" };
    ValuesConstraint<string> datatypeValuesConstraint(datatypeValues);
    UnlabeledValueArg<string> datatypeArg("datatype", "type of input data",
                                          true, "uint64",
                                          &datatypeValuesConstraint, cmd);
    
    UnlabeledValueArg<int> mArg("m", "number of registers", true, 1024,
                                "int power of two", cmd);

    UnlabeledValueArg<size_t> nArg("n", "number of values to read from stdin", true, 0,
                                "int", cmd);

    vector<string> flagValues { "default", "appendonly", "increaseonly",
        "appendincreaseonly", "bottom" };
    ValuesConstraint<string> flagValuesConstraint(flagValues);
    ValueArg<string> flagArg("", "flags", "flags for hyperlogloglog", false,
                             "default", &flagValuesConstraint, cmd);
    ValueArg<size_t> lenArg("", "len", "length of strings to read", false, 0, "int", cmd);
    cmd.parse(argc, argv);
    
    if (helpSwitch.getValue()) {
      TCLAP::StdOutput().usage(cmd);
      return EXIT_SUCCESS;
    }

    string mode = modeArg.getValue();
    string algo = algorithmArg.getValue();
    string dt = datatypeArg.getValue();
    int m = mArg.getValue();
    string flagsString = flagArg.getValue();
    size_t n = nArg.getValue();
    size_t len = lenArg.getValue();

    if (mode == "merge" && algo == "hashonly") {
      cerr << "hashonly does not support merging!" << endl;
      return EXIT_FAILURE;
    }

    if (algo == "hashonly" && dt == "jr") {
      cerr << "hashonly does not support jr datatype!" << endl;
      return EXIT_FAILURE;
    }

    if (m != (1 << log2i(m))) {
      cerr << "m must be a power of two!" << endl;
      return EXIT_FAILURE;
    }

    if (flagArg.isSet() && algo != "hyperlogloglog") {
      cerr << "flags are only supported for hyperlogloglog!" << endl;
      return EXIT_FAILURE;
    }

    int flags = flagsString == "default" ? 
      HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_DEFAULT :
      flagsString == "appendonly" ?
      HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_WHEN_APPEND :
      flagsString == "increaseonly" ?
      HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_TYPE_INCREASE :
      flagsString == "appendincreaseonly" ?
      HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_WHEN_APPEND |
      HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_TYPE_INCREASE :
      flagsString == "bottom" ?
      HyperLogLogLog<uint64_t>::HYPERLOGLOGLOG_COMPRESS_BOTTOM :
      -1;

    if (dt == "str" && !lenArg.isSet()) {
      cerr << "len must be set if datatype is string" << endl;
      return EXIT_FAILURE;
    }
    if (dt != "str" && lenArg.isSet()) {
      cerr << "len must not be set if datatype is not string" << endl;
      return EXIT_FAILURE;
    }
    
    measure(mode, algo, dt, m, flags, n, len);
  }
  catch (TCLAP::ArgException &e) {
    cerr << "error: " << e.error() << " for arg " << e.argId()
         << endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

