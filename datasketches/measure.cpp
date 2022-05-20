#include "../hyperlogloglog/measure.hpp"
#include "../hyperlogloglog/common.hpp"
#include <tclap/CmdLine.h>
#include <hll.hpp>
#include <cpc_sketch.hpp>
#include <cpc_union.hpp> // needed for merging
#include <iostream>
#include <cstdint>
#include <chrono>

using datasketches::target_hll_type;
using datasketches::cpc_sketch;
using datasketches::hll_sketch;
using datasketches::hll_union;
using datasketches::cpc_union;
using datasketches::HLL_4;
using datasketches::HLL_6;
using datasketches::HLL_8;
using TCLAP::CmdLine;
using TCLAP::SwitchArg;
using TCLAP::UnlabeledValueArg;
using TCLAP::ValuesConstraint;
using TCLAP::ValueArg;
using std::ostringstream;
using std::cout;
using std::cerr;
using std::endl;
using std::ostream;
using std::string;
using std::vector;
using std::unique_ptr;
using std::make_unique;
using std::chrono::duration_cast;
using std::chrono::nanoseconds;
using std::chrono::steady_clock;
using std::cin;
using hyperlogloglog::readData;


template<typename T>
void serialize(const T& S, ostream& os);

template<>
void serialize(const hll_sketch& S, ostream& os) {
  S.serialize_updatable(os);
}

template<>
void serialize(const cpc_sketch& S, ostream& os) {
  S.serialize(os);
}

template<typename T>
size_t getBitSize(const T& S) {
  ostringstream oss;
  serialize(S,oss);
  return oss.tellp() * CHAR_BIT;
}



template<typename DataType>
void add(vector<DataType>& data, const string& line);

template<>
void add(vector<string>& data, const string& line) {
  data.push_back(line);
}

template<>
void add(vector<uint64_t>& data, const string& line) {
#ifdef __clang__
  static_assert(std::is_same<uint64_t,unsigned long long>::value);
  static_assert(sizeof(unsigned long) == sizeof(unsigned long long));
#elif __GNUC__
  static_assert(std::is_same<uint64_t,unsigned long>::value);
#endif // __GNUC__
  data.push_back(std::stoul(line));
}


template<typename SketchType> 
static unique_ptr<SketchType> constructSketch(int logM,
                                              target_hll_type hllType);

template<>
unique_ptr<hll_sketch> constructSketch(int logM,
                                              target_hll_type hllType) {
  return make_unique<hll_sketch>(logM, hllType);
}

template<>
unique_ptr<cpc_sketch> constructSketch(int logM,
                                              target_hll_type) {
  return make_unique<cpc_sketch>(logM);
}

template<typename DataType, typename SketchType>
void adds(SketchType& S, const vector<DataType>& data) {
  for (const auto& d : data)
    S.update(d);
}

template<typename ConstIterator, typename SketchType>
void adds(SketchType& S, ConstIterator begin,
          ConstIterator end) {
  for (auto it = begin; it != end; ++it)
    S.update(*it);
}



template<typename T>
void report(double seconds, T& S) {
  double estimate = S.get_estimate();
  size_t bitsize = getBitSize(S);
  int compressCount = -1;
  int rebaseCount = -1;
  
  fprintf(stdout, "time %g\n", seconds);
  fprintf(stdout, "estimate %f\n", estimate);
  fprintf(stdout, "bitsize %zu\n", bitsize);
  fprintf(stdout, "compressCount %d\n", compressCount);
  fprintf(stdout, "rebaseCount %d\n", rebaseCount);
}


template<typename DataType, typename SketchType> 
static void measureQuery(const vector<DataType>& data,
                         int logM,
                         target_hll_type hllType) {
  unique_ptr<SketchType> S = constructSketch<SketchType>(logM,
                                                         hllType);
  auto start = steady_clock::now();
  adds(*S, data);
  auto end = steady_clock::now();
  auto diff = end - start;
  double seconds = duration_cast<nanoseconds>(diff).count()/1e9;
  report(seconds, *S);
}



template<typename SketchType>
static SketchType merge(const SketchType& S1,
                        const SketchType& S2,
                        int logM);

template<>
hll_sketch merge(const hll_sketch& S1,
                 const hll_sketch& S2,
                 int logM) {
  hll_union U(logM);
  U.update(S1);
  U.update(S2);
  return U.get_result();
}



template<>
cpc_sketch merge(const cpc_sketch& S1,
                 const cpc_sketch& S2,
                 int logM) {
  cpc_union U(logM);
  U.update(S1);
  U.update(S2);
  return U.get_result();
}



template<typename DataType, typename SketchType> 
static void measureMerge(const vector<DataType>& data,
                         int logM,
                         target_hll_type hllType) {
  size_t n1 = data.size() / 2;
  unique_ptr<SketchType> S1 = constructSketch<SketchType>(logM,
                                                          hllType);
  unique_ptr<SketchType> S2 = constructSketch<SketchType>(logM,
                                                          hllType);
  adds(*S1, data.cbegin(), data.cbegin() + n1);
  adds(*S2, data.cbegin() + n1, data.cend());
  auto start = steady_clock::now();
  auto S = merge(*S1, *S2, logM);
  auto end = steady_clock::now();
  auto diff = end - start;
  double seconds = duration_cast<nanoseconds>(diff).count()/1e9;
  report(seconds, S);
}



template<typename DataType, typename SketchType> 
static void measure(const string& mode,
                    const vector<DataType>& data,
                    int logM,
                    target_hll_type hllType) {
  if (mode == "query")
    measureQuery<DataType,SketchType>(data, logM, hllType);
  else if (mode == "merge")
    measureMerge<DataType,SketchType>(data, logM, hllType);
}



template<typename DataType> 
static void measure(const string& mode,
                    const string& algo,
                    int logM,
                    target_hll_type hllType,
                    size_t n, size_t len) {
  vector<DataType> data = readData<DataType>(n, len);
  if (algo == "hll")
    measure<DataType,hll_sketch>(mode, data, logM, hllType);
  else if (algo == "cpc")
    measure<DataType,cpc_sketch>(mode, data, logM, hllType);
}



static void measure(const string& mode,
                    const string& algo,
                    const string& dt,
                    int logM,
                    target_hll_type hllType,
                    size_t n,
                    size_t len) {
  if (dt == "uint64")
    measure<uint64_t>(mode, algo, logM, hllType, n, len);
  else if (dt == "str")
    measure<string>(mode, algo, logM, hllType, n, len);
}



int main(int argc, char* argv[]) {
  try {
    CmdLine cmd("Make measurements of apache datasketches.", ' ', "", false);
    SwitchArg helpSwitch("h", "help", "Print this message", cmd, false);
    vector<string> modeValues { "query", "merge" };
    ValuesConstraint<string> modeValuesConstraint(modeValues);
    UnlabeledValueArg<string> modeArg("mode", "measurement mode", true, "query",
                                      &modeValuesConstraint, cmd);
    vector<string> algorithmValues { "hll", "cpc" };
    ValuesConstraint<string> algorithmValuesConstraint(algorithmValues);
    UnlabeledValueArg<string> algorithmArg("algorithm", "algorithm to measure",
                                           true, "hyperloglog",
                                           &algorithmValuesConstraint, cmd);
    vector<string> datatypeValues { "uint64", "str" };
    ValuesConstraint<string> datatypeValuesConstraint(datatypeValues);
    UnlabeledValueArg<string> datatypeArg("datatype", "type of input data",
                                          true, "uint64",
                                          &datatypeValuesConstraint, cmd);
    UnlabeledValueArg<int> mArg("m", "number of registers", true, 1024, "int power of two", cmd);
    UnlabeledValueArg<size_t> nArg("n", "number of values to read from stdin", true, 0,
                                "int", cmd);

    vector<int> hllBitValues { 4, 6, 8 };
    ValuesConstraint<int> hllBitValuesConstraint(hllBitValues);
    ValueArg<int> hllBitArg("", "hll-bits",
                            "number of bits per register for hll",
                            false, 8, &hllBitValuesConstraint, cmd);
    ValueArg<size_t> lenArg("", "len", "length of strings to read", false, 0, "int", cmd);
    cmd.parse(argc, argv);
    
    if (helpSwitch.getValue()) {
      TCLAP::StdOutput().usage(cmd);
      return EXIT_SUCCESS;
    }

    int m = mArg.getValue();
    int logM = std::log2(m);
    if (m != (1 << logM)) {
      cerr << "the number of registers must be a power of two!" << endl;
      return EXIT_FAILURE;
    }

    string mode = modeArg.getValue();

    string dt = datatypeArg.getValue();

    string algo = algorithmArg.getValue();
    if (algo == "cpc" && hllBitArg.isSet()) {
      cerr << "--hll-bits is only supported for hll!" << endl;
      return EXIT_FAILURE;
    }

    if (algo == "hll" && !hllBitArg.isSet()) {
      cerr << "--hll-bits must be set for hll" << endl;
      return EXIT_FAILURE;
    }
    
    int hllBits = hllBitArg.getValue();
    target_hll_type hllType =
      hllBits == 4 ? HLL_4 :
      hllBits == 6 ? HLL_6 :
      HLL_8;

    size_t n = nArg.getValue();
    size_t len = lenArg.getValue();

    if (n < 0) {
      cerr << "n must be non-negative" << endl;
      return EXIT_FAILURE;
    }

    if (dt == "str" && !lenArg.isSet()) {
      cerr << "len must be set if datatype is string" << endl;
      return EXIT_FAILURE;
    }
    if (dt != "str" && lenArg.isSet()) {
      cerr << "len must not be set if datatype is not string" << endl;
      return EXIT_FAILURE;
    }

    measure(mode, algo, dt, logM, hllType, n, len);
  }
  catch (TCLAP::ArgException &e) {
    cerr << "error: " << e.error() << " for arg " << e.argId()
         << endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
