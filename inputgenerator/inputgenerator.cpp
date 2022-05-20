#include "../hyperlogloglog/common.hpp"
#include <tclap/CmdLine.h>
#include <random>
#include <cstdint>
#include <cstring>
#include <chrono>

using TCLAP::CmdLine;
using TCLAP::SwitchArg;
using TCLAP::UnlabeledValueArg;
using TCLAP::ValuesConstraint;
using TCLAP::ValueArg;
using std::string;
using std::vector;
using std::cerr;
using std::cout;
using std::endl;
using std::mt19937;
using std::uniform_int_distribution;
using std::uniform_real_distribution;
using std::chrono::duration_cast;
using std::chrono::nanoseconds;
using std::chrono::steady_clock;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#ifndef htonll // MacOS X defines this as a macro
using hyperlogloglog::htonll;
#endif // htonll
#endif // byte order

static void generateUint64(mt19937& rng, uint64_t n) {
  auto start = steady_clock::now();
  uniform_int_distribution<uint64_t> dist(0,~static_cast<uint64_t>(0));
  vector<uint64_t> xs(n);
  uint64_t x;
  for (size_t i = 0; i < n; ++i) {
    x = dist(rng);
    xs[i] = htonll(x);
  }
  auto end = steady_clock::now();
  auto diff = end - start;
  double seconds = duration_cast<nanoseconds>(diff).count()/1e9;
  cerr << "data generation took " << seconds << endl;
  start = steady_clock::now();
  cout.write(reinterpret_cast<char*>(&xs[0]), n*sizeof(uint64_t));
  end = steady_clock::now();
  diff = end - start;
  seconds = duration_cast<nanoseconds>(diff).count()/1e9;
  cerr << "data writing took " << seconds << endl;
}



static void generateStr(mt19937& rng, uint64_t n, int len) {
  auto start = steady_clock::now();
  static const char* ALPHANUMBERS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  uniform_int_distribution<uint64_t> dist(0,strlen(ALPHANUMBERS)-1);
  vector<char> temp(len*n);
  for (auto it = temp.begin(); it != temp.end(); ++it)
    *it = ALPHANUMBERS[dist(rng)];
  auto end = steady_clock::now();
  auto diff = end - start;
  double seconds = duration_cast<nanoseconds>(diff).count()/1e9;
  cerr << "data generation took " << seconds << endl;

  start = steady_clock::now();
  cout.write(&temp[0], temp.size());
  end = steady_clock::now();
  diff = end - start;
  seconds = duration_cast<nanoseconds>(diff).count()/1e9;
  cerr << "data writing took " << seconds << endl;
}



static void generateJr(mt19937& rng, uint64_t n, int m) {
  auto start = steady_clock::now();
  uniform_int_distribution<uint32_t> jdist(0,m-1);
  uniform_real_distribution<double> cdist;
  vector<uint32_t> temp(2*n);
  uint32_t j, r;
  for (uint64_t i = 0; i < n; ++i) {
    j = jdist(rng);
    r = static_cast<uint32_t>(ceil(-log2(1-cdist(rng))));
    temp[2*i] = htonl(j);
    temp[2*i+1] = htonl(r);
  }
  auto end = steady_clock::now();
  auto diff = end - start;
  double seconds = duration_cast<nanoseconds>(diff).count()/1e9;
  cerr << "data generation took " << seconds << endl;

  start = steady_clock::now();
  cout.write(reinterpret_cast<char*>(&temp[0]), 2*n*sizeof(uint32_t));
  end = steady_clock::now();
  diff = end - start;
  seconds = duration_cast<nanoseconds>(diff).count()/1e9;
  cerr << "data writing took " << seconds << endl;
}



int main(int argc, char* argv[]) {
  try {
    CmdLine cmd("generate input", ' ', "", false);
    SwitchArg helpSwitch("h", "help", "Print this message", cmd, false);
    UnlabeledValueArg<uint64_t> nArg("n", "number of elements to create", true, 0,
                                "int", cmd);
    vector<string> dtValues { "uint64", "str", "jr" };
    ValuesConstraint<string> dtValuesConstraint(dtValues);
    UnlabeledValueArg<string> dtArg("dt", "datatype", true, "uint64",
                                    &dtValuesConstraint, cmd);
    UnlabeledValueArg<uint32_t> seedArg("seed", "random number generator seed",
                                        true, 0, "int", cmd);
    ValueArg<int> mArg("m", "m-registers", "number of registers (for jr input)",
                       false, 0, "int power of two", cmd);
    ValueArg<int> lenArg("", "len",
                         "length of strings to create (for str input)",
                         false, 0, "int", cmd);
    cmd.parse(argc,argv);

    if (helpSwitch.getValue()) {
      TCLAP::StdOutput().usage(cmd);
      return EXIT_SUCCESS;
    }

    string dt = dtArg.getValue();

    if (mArg.isSet() && dt != "jr") {
      cerr << "-m can be used only in conjunction with datatype jr" << endl;
      return EXIT_FAILURE;
    }

    if (!mArg.isSet() && dt == "jr") {
      cerr << "datatype jr requires -m" << endl;
      return EXIT_FAILURE;
    }

    if (lenArg.isSet() && dt != "str") {
      cerr << "--len can be used only in conjunction with datatype str" << endl;
      return EXIT_FAILURE;
    }
    
    if (!lenArg.isSet() && dt == "str") {
      cerr << "datatype str requires --len" << endl;
      return EXIT_FAILURE;
    }

    mt19937 rng(seedArg.getValue());

    if (dt == "uint64")
      generateUint64(rng, nArg.getValue());
    else if (dt == "str")
      generateStr(rng, nArg.getValue(), lenArg.getValue());
    else if (dt == "jr")
      generateJr(rng, nArg.getValue(), mArg.getValue());
  }
  catch (TCLAP::ArgException &e) {
    cerr << "error: " << e.error() << " for arg " << e.argId()
         << endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

