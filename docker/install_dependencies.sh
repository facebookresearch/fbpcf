#!/bin/bash
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# install gcc 8 and set up symlinks
cd /root || exit
apt-add-repository -y ppa:ubuntu-toolchain-r/test
apt-get update
apt-get install -y gcc-8 g++-8
ln -sf /usr/bin/gcc-8 /usr/bin/gcc
ln -sf /usr/bin/g++-8 /usr/bin/g++
ln -sf /usr/bin/gcc /usr/bin/cc
ln -sf /usr/bin/g++ /usr/bin/c++

# install latest cmake
cd /root || exit
wget https://cmake.org/files/v3.12/cmake-3.12.3-Linux-x86_64.sh
bash cmake-3.12.3-Linux-x86_64.sh --include-subdir --skip-license
cp cmake-3.12.3-Linux-x86_64/bin/* /usr/bin
cp -r cmake-3.12.3-Linux-x86_64/share/* /usr/share/
rm -r cmake*

# install gflags for private lift games
cd /root || exit
git clone https://github.com/gflags/gflags
cd gflags || exit
mkdir build && cd build || exit
cmake ..
make && make install

# install re2 for private lift games
cd /root || exit
# TODO: add version control
git clone https://code.googlesource.com/re2
cd re2 || exit
make
make test
sudo make install
sudo make testinstall

# install clang-12 for building private lift executables
cd /root || exit
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
add-apt-repository "deb http://apt.llvm.org/bionic/   llvm-toolchain-bionic-12  main"
apt-get update
apt-get install -y clang-12 lldb-12 lld-12 clangd-12
sudo ln -s /usr/bin/clang-12 /usr/bin/clang
sudo ln -s /usr/bin/clang++-12 /usr/bin/clang++
