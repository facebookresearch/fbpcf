#!/bin/bash

# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# This is a temporary fix that installs the previous cmake 3.11.4
# until Centos 8 upgrades from 3.18.2 to 3.20.3
set -e

yum -y install \
    emacs-filesystem \
    wget \
    libuv \
    && yum clean all
wget https://rpmfind.net/linux/centos/8.3.2011/AppStream/x86_64/os/Packages/cmake-3.11.4-7.el8.x86_64.rpm
wget https://rpmfind.net/linux/centos/8.3.2011/AppStream/x86_64/os/Packages/cmake-data-3.11.4-7.el8.noarch.rpm
wget https://rpmfind.net/linux/centos/8.3.2011/AppStream/x86_64/os/Packages/cmake-filesystem-3.11.4-7.el8.x86_64.rpm
wget https://rpmfind.net/linux/centos/8.3.2011/AppStream/x86_64/os/Packages/cmake-rpm-macros-3.11.4-7.el8.noarch.rpm
rpm -i ./*.rpm
rm ./*.rpm
rm ./centos-cmake-fix.sh
