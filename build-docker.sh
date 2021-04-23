#!/bin/bash

# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.
set -e

CENTOS_RELEASE="8.3.2011"
UBUNTU_RELEASE="20.04"
EMP_RELEASE="0.1"
AWS_RELEASE="1.8.177"
FMT_RELEASE="7.1.3"
FOLLY_RELEASE="2021.03.29.00"

PROG_NAME=$0
usage() {
  cat << EOF >&2
Usage: $PROG_NAME [-u | --ubuntu]

-c: builds the docker images aginsts centos (default)
-u: builds the docker images against ubuntu
EOF
  exit 1
}

IMAGE_PREFIX="centos"
OS_RELEASE=${CENTOS_RELEASE}
DOCKER_EXTENSION=""
while getopts u o; do
  case $o in
    (u) IMAGE_PREFIX="ubuntu"
        OS_RELEASE=${UBUNTU_RELEASE}
        DOCKER_EXTENSION=".ubuntu";;
    (*) usage
  esac
done
shift "$((OPTIND - 1))"

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
# Docker build must run from this script build dir, so all the relative paths work inside the Dockerfile's
cd "$SCRIPT_DIR" || exit

printf "\nBuilding %s-emp %s docker image...\n" ${IMAGE_PREFIX} ${EMP_RELEASE}
docker build \
    --build-arg os_release=${OS_RELEASE} \
    --build-arg emp_release=${EMP_RELEASE} \
    -t fbpcf/${IMAGE_PREFIX}-emp:${EMP_RELEASE} -f docker/emp/Dockerfile${DOCKER_EXTENSION} .

printf "\nBuilding %s-aws(s3/core) %s docker image...\n" ${IMAGE_PREFIX} ${AWS_RELEASE}
docker build  \
    --build-arg os_release=${OS_RELEASE} \
    --build-arg aws_release=${AWS_RELEASE} \
    -t fbpcf/${IMAGE_PREFIX}-aws-s3-core:${AWS_RELEASE} -f docker/aws-s3-core/Dockerfile${DOCKER_EXTENSION} .

printf "\nBuilding %s-folly %s docker image...\n" ${IMAGE_PREFIX} ${FOLLY_RELEASE}
docker build  \
    --build-arg os_release=${OS_RELEASE} \
    --build-arg folly_release=${FOLLY_RELEASE} \
    --build-arg fmt_release=${FMT_RELEASE} \
    -t fbpcf/${IMAGE_PREFIX}-folly:${FOLLY_RELEASE} -f docker/folly/Dockerfile${DOCKER_EXTENSION} .

printf "\nBuilding %s-fbpcf docker image...\n" ${IMAGE_PREFIX}
docker build  \
    --build-arg os_release=${OS_RELEASE} \
    --compress \
    -t fbpcf:latest -f docker/Dockerfile${DOCKER_EXTENSION} .
# FYI: To create a "dev" build (with all source), comment out the docker build above
#     and uncommment the lines below
# docker build  \
#     --build-arg ubuntu_release=${UBUNTU_RELEASE} \
#     -t fbpcf:latest --target=dev -f docker/. .
