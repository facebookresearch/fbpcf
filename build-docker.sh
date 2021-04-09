#!/bin/bash

# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
# Docker build must run from this script build dir, so all the relative paths work inside the Dockerfile's
cd "$SCRIPT_DIR" || exit

UBUNTU_RELEASE="20.04"
EMP_RELEASE="0.1"
AWS_RELEASE="1.8.177"
FMT_RELEASE="7.1.3"
FOLLY_RELEASE="2021.03.29.00"

printf "\nBuilding emp %s docker image...\n" ${EMP_RELEASE}
docker build \
    --build-arg ubuntu_release=${UBUNTU_RELEASE} \
    --build-arg emp_release=${EMP_RELEASE} \
    -t fbpcf/ubuntu-emp:${EMP_RELEASE} -f docker/emp/. .

printf "\nBuilding aws(s3/core) %s docker image...\n" ${AWS_RELEASE}
docker build  \
    --build-arg ubuntu_release=${UBUNTU_RELEASE} \
    --build-arg aws_release=${AWS_RELEASE} \
    -t fbpcf/ubuntu-aws-s3-core:${AWS_RELEASE} -f docker/aws-s3-core/. .

printf "\nBuilding folly %s docker image...\n" ${FOLLY_RELEASE}
docker build  \
    --build-arg ubuntu_release=${UBUNTU_RELEASE} \
    --build-arg folly_release=${FOLLY_RELEASE} \
    --build-arg fmt_release=${FMT_RELEASE} \
    -t fbpcf/ubuntu-folly:${FOLLY_RELEASE} -f docker/folly/. .

printf "\nBuilding fbpcf docker image...\n"
docker build  \
    --build-arg ubuntu_release=${UBUNTU_RELEASE} \
    --compress \
    -t fbpcf:latest -f docker/. .
# FYI: To create a "dev" build (with all source), comment out the docker build above
#     and uncommment the lines below
# docker build  \
#     --build-arg ubuntu_release=${UBUNTU_RELEASE} \
#     -t fbpcf:latest --target=dev -f docker/. .
