#!/bin/bash
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# get emp readme scripts
cd /root || exit
git clone https://github.com/emp-toolkit/emp-readme.git
cd emp-readme || exit
git checkout d31ffad00ee86f470dcb12ff50b3d88567577d1f

# install emp dependencies
cd /root || exit
bash ./emp-readme/scripts/install_packages.sh
bash ./emp-readme/scripts/install_relic.sh
#EC STRING SIZE
sed -i "s/FB_POLYN:STRING=283/FB_POLYN:STRING=251/" ~/relic/CMakeCache.txt

# get and install emp-tool
git clone https://github.com/emp-toolkit/emp-tool.git
cd emp-tool || exit
git checkout 508db1726c3c040fd12ad1f4d870169b29dbda13
cd /root/emp-tool || exit
cmake . -DBUILD_SHARED_LIBS=OFF -DTHREADING=ON
make
make install

# get and install emp-ot
cd /root || exit
git clone https://github.com/emp-toolkit/emp-ot.git
cd emp-ot || exit
git checkout 7a3ff4b567631ef441ba6a85aadd395bfe925839
cmake . -DBUILD_SHARED_LIBS=OFF -DTHREADING=ON
make
make install

# get and install emp-sh2pc
cd /root || exit
git clone https://github.com/emp-toolkit/emp-sh2pc.git
cd emp-sh2pc || exit
git checkout 07271059d99312cfc0c6589f43fc2d9ddfe6788b
cd /root/emp-sh2pc || exit
mkdir build
cd build || exit
cmake .. -DBUILD_SHARED_LIBS=OFF -DTHREADING=ON
make
make install
