#!/bin/bash
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# install clang-12
cd /root || exit
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 12
sudo ln -s /usr/bin/clang-12 /usr/bin/clang
sudo ln -s /usr/bin/clang++-12 /usr/bin/clang++

# compile calculator and aggregator
cd /root || exit
cmake . -DTHREADING=ON -DEMP_USE_RANDOM_DEVICE=ON
make
