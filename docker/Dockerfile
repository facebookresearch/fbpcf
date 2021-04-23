# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

ARG os_release="latest"
FROM centos:centos${os_release} as dev

# Required Packages for pcf
# install EPEL and PowerTools for CentOS (required for g* libs and doubleconv)
RUN echo "assumeyes=1" >> /etc/yum.conf
RUN yum -y install \
    epel-release \
    dnf-plugins-core \
    && yum config-manager --set-enabled powertools
 RUN yum -y install \
    boost-devel \
    ca-certificates \
    cmake \
    double-conversion-devel \
    gcc \
    gcc-c++ \
    gflags-devel \
    git \
    glog-devel \
    gmp-devel \
    gtest-devel \
    libcurl-devel \
    libevent-devel \
    make \
    openssl-devel \
    re2-devel \
    zlib-devel \
    && yum clean all
RUN mkdir /root/build
WORKDIR /root/build

# Copy build dependencies from other docker images
COPY --from=fbpcf/centos-emp:0.1 /usr/local/. /usr/local/.
COPY --from=fbpcf/centos-aws-s3-core:1.8.177 /usr/local/. /usr/local/.
COPY --from=fbpcf/centos-folly:2021.03.29.00 /usr/local/. /usr/local/.

# fbpcf build and install
COPY docker/CMakeLists.txt .
COPY lift/ ./lift
COPY pcf/ ./pcf
COPY example/ ./example

RUN cmake . -DTHREADING=ON -DEMP_USE_RANDOM_DEVICE=ON
RUN make

# This section is only needed to make a dev image run properly...
RUN cp /root/build/bin/* /usr/local/bin/.
WORKDIR /usr/local/bin
CMD ["/bin/sh"]

# Create a minified prod build with only required libraries (no source)
FROM centos:centos${os_release} as prod
RUN yum -y install \
    epel-release \
    dnf-plugins-core \
    && yum config-manager --set-enabled powertools
RUN yum -y install \
    boost-atomic \
    boost-context \
    boost-chrono \
    boost-date-time \
    boost-filesystem \
    boost-program-options \
    boost-regex \
    boost-system \
    boost-thread \
    double-conversion \
    gflags \
    glog \
    libevent \
    openssl \
    re2 \
    && yum clean all \
    && rm -rf /var/cache/yum

COPY --from=dev /root/build/bin/. /usr/local/bin/.
WORKDIR /usr/local/bin
CMD ["/bin/sh"]
