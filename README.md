# hyperlogloglog
HyperLogLogLog: Cardinality Estimation With One Log More

## Introduction

This is an implementation of the HyperLogLogLog algorithm, as
described in [1]. We also include heuristic variants, called
HyperLogLogLog* in the paper, the vanilla HyperLogLog, and an
entropy-compressed version of the HyperLogLog, compressed using
Facebook's Zstd library.

In addition to the implementations of the algorithms, we also provide
the experimental framework to run the experiments described in the
paper in a controlled Docker environment.

## Requirements

You will need at least the following to compile the code:
* A sufficiently recent C++ compiler with at least C++17 support
(tested: Apple CLang 13.1.6, GCC 10.3.0)

The following are recommended:
* A sufficiently recent version of Python to run the experiments
  (tested: CPython 3.8.10)
* A sufficiently recent version of Docker (tested: 20.10.11)
* OpenJDK 11 for building the ZetaSketch tool if Docker is not used 

## Building with Docker (Recommended)
The following instructions will walk you through building a Docker
image for a controlled environment where you can run the experiments.

Run the following command to build the image with the name `hyperlogloglog`:
```
$ docker build -t hyperlogloglog .
```

If everything goes smoothly, you should be done! You can run unit
tests with the following command:
```
$ docker run hyperlogloglog /bin/bash -c 'cd hyperlogloglog && make test && ./test'
```

## Running experiments on Docker

Experiments can be run with the `experiments.py` file provided for
convenience. By default, the script will run a full set of
experiments. The results will be stored under a subdirectory called
`results`.

Please run `python3 experiments.py --help` for instructions on how to
run individual experiments or filter which experiments to run.

Note that the script assumes the Docker environment has been set
up. It does not work without the Docker image.

## Building the code manually

Follow these instructions if you want to compile the code manually,
for example, for embedding the code in your own projects. Please also have
a look at `Dockerfile` as it describes how the experimental pipeline
is set up. 

The `hyperlogloglog` library depends on Zstd, so start by compiling
it.
```
cd external/zstd
make ZSTD_LEGACY_SUPPORT=0
```
This should produce a file called `libzstd.so` or `libzstd.dylib` or
such (depending on your OS) in the directory.

Now you can proceed to building the actual `hyperlogloglog` library.
```
cd hyperlogloglog
make
```
This will compile the `measure` program that can be used to measure
various algorithms with various input (as in the experiments). See
`measure --help` for more information.

Note: On MacOS, loading the dynamic library might not work as
expected. A workaround is to run `measure` as follows:
```
$ DYLD_LIBRARY_PATH=../external/zstd ./measure
```

If you want to use the input generator, compile it as follows:
```
cd inputgenerator
make
```
The `inputgenerator` works similarly to `measure`, see `inputgenerator
--help` for more information.

For a comparable measurement tool for the Apache Data Sketches
implementation, compile as follows:
```
cd datasketches
make
```
This also creates an executable called `measure`. As before, see
`measure --help` for more information.

Finally, for compiling the ZetaSketch measurement tool, use gradle:
```
cd zetasketch
./gradlew build
```
This creates a self-contained file called `build/libs/zetasketch.jar`
that can be run as any usual Java archive. Run it using `java -jar
build/libs/zetasketch.jar` for more information.

## External libraries
The following external libraries are provided:
* [Catch2](https://github.com/catchorg/Catch2) v2.13.7
* [Apache DataSketches](https://datasketches.apache.org/) 3.2.0
* [Google FarmHash](https://github.com/google/farmhash) version 1.1
* [TCLAP](http://tclap.sourceforge.net/) 1.4.0
* [Facebook ZStandard](https://github.com/facebook/zstd) commit 64205b7832fa0b4433214e26c294545b4c962834

## License
All HyperLogLogLog code and the experimental framework has been
licensed under the MIT license. For external libraries, see the
respective subdirectories for license information.

## How to cite?
If you use this work as part of your research endeavors, we kindly ask
you to cite the KDD paper [1].

## References
[1] Matti Karppa and Rasmus Pagh. 2022. HyperLogLogLog:
Cardinality Estimation With One Log More. In Proceedings of the 28th
ACM SIGKDD Conference on Knowledge Discovery & Data Mining (KDD '22).
Association for Computing Machinery, New York, NY, USA.
