#!/bin/bash
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# install googletest
cd /root || exit
wget https://github.com/google/googletest/archive/release-1.8.0.tar.gz && \
tar zxf release-1.8.0.tar.gz && \
rm -f release-1.8.0.tar.gz && \
cd googletest-release-1.8.0 || exit && \
cmake . && \
make && \
make install

# install packages
sudo apt-get install -y \
    g++ \
    cmake \
    libboost-all-dev \
    libevent-dev \
    libdouble-conversion-dev \
    libgoogle-glog-dev \
    libgflags-dev \
    libiberty-dev \
    liblz4-dev \
    liblzma-dev \
    libsnappy-dev \
    make \
    zlib1g-dev \
    binutils-dev \
    libssl-dev \
    pkg-config \
    libunwind-dev \
    libjemalloc-dev

sudo apt-get install -y \
    libunwind8-dev \
    libelf-dev \
    libdwarf-dev

# install fmt
cd /root || exit
# TODO: add version control
git clone https://github.com/fmtlib/fmt.git
cd fmt || exit
mkdir _build
cd _build || exit
cmake ..
make -j
sudo make install

# install folly
cd /root || exit
git clone https://github.com/facebook/folly.git
cd folly || exit
git checkout v2020.10.12.00
mkdir _build
cd _build || exit
cmake .. -DFOLLY_USE_JEMALLOC=0 -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-march=native"
make
sudo make install
