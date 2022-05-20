FROM ubuntu:20.04
RUN apt update && apt -y upgrade
RUN apt install -y build-essential
RUN apt install -y gcc-10 g++-10 cpp-10
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 100 --slave /usr/bin/g++ g++ /usr/bin/g++-10 --slave /usr/bin/gcov gcov /usr/bin/gcov-10
RUN DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get -y install tzdata
RUN apt install -y openjdk-11-jdk
RUN apt install -y python3 python3-numpy
COPY external/ /app/external/
RUN cd /app/external/zstd && make ZSTD_LEGACY_SUPPORT=0
COPY zetasketch /app/zetasketch
RUN cd /app/zetasketch && ./gradlew build
RUN cp -v /app/zetasketch/build/libs/zetasketch.jar /app/zetasketch/measure.jar
COPY hyperlogloglog /app/hyperlogloglog
RUN cd /app/hyperlogloglog && make
COPY inputgenerator /app/inputgenerator
RUN cd /app/inputgenerator && make
COPY datasketches /app/datasketches
RUN cd /app/datasketches && make
WORKDIR /app/
