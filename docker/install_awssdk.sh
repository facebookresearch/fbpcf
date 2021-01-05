#!/bin/bash
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cd /root || exit
sudo apt-get install -y libcurl4-openssl-dev libssl-dev uuid-dev zlib1g-dev libpulse-dev
# TODO: add version control
git clone https://github.com/aws/aws-sdk-cpp.git
cd aws-sdk-cpp || exit
mkdir build
cd build || exit
# -DCUSTOM_MEMORY_MANAGEMENT=0 is added to avoid Aws::String and std::string issue
# ref: https://github.com/aws/aws-sdk-cpp/issues/416
cmake .. -DBUILD_ONLY="s3;core" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_SHARED_LIBS=OFF -DCUSTOM_MEMORY_MANAGEMENT=0
make
sudo make install
