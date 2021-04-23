# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

ARG os_release="latest"
FROM centos:centos${os_release} AS builder
ARG fmt_release="7.1.3"
ARG folly_release="2021.03.29.00"

# Required Packages for all builds
RUN yum -y install \
    ca-certificates \
    cmake \
    gcc \
    gcc-c++ \
    git \
    make \
    && yum clean all
RUN mkdir /root/build
WORKDIR /root/build

# fmt build and install
RUN git clone https://github.com/fmtlib/fmt.git
WORKDIR /root/build/fmt
RUN git checkout tags/${fmt_release} -b ${fmt_release}
RUN cmake .
RUN make
RUN make install

WORKDIR /root/build

# folly build and install
# install EPEL and PowerTools for CentOS (required for g* libs and doubleconv)
RUN echo "assumeyes=1" >> /etc/yum.conf
RUN yum -y install \
    epel-release \
    dnf-plugins-core \
    && yum config-manager --set-enabled powertools
 RUN yum -y install \
    boost-devel \
    # libboost-context-dev \
    # libboost-filesystem-dev \
    # libboost-program-options-dev \
    # libboost-regex-dev \
    # libboost-system-dev \
    double-conversion-devel \
    gflags-devel \
    glog-devel \
    gtest-devel \
    libevent-devel \
    openssl-devel \
    && yum clean all

RUN git clone https://github.com/facebook/folly.git
WORKDIR /root/build/folly
RUN git checkout tags/v${folly_release} -b v${folly_release}

RUN cmake DBUILD_SHARED_LIBS=OFF -DFOLLY_USE_JEMALLOC=0 -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-march=native" .
RUN make
RUN make install

FROM centos:centos${os_release}
COPY --from=builder /usr/local/include/. /usr/local/include/.
COPY --from=builder /usr/local/lib/. /usr/local/lib/.
ENTRYPOINT [ "sh" ]
