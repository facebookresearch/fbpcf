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
Usage: $PROG_NAME [-u | -c] [-f]

-c: builds the docker images aginsts centos
-u: builds the docker images against ubuntu (default)
-f: forces a rebuild of all docker image dependencies (even if they already exist)
EOF
  exit 1
}

IMAGE_PREFIX="ubuntu"
OS_RELEASE=${UBUNTU_RELEASE}
DOCKER_EXTENSION=".ubuntu"
FORCE_REBUILD=false
while getopts u,c,f o; do
  case $o in
    (u) IMAGE_PREFIX="ubuntu"
        OS_RELEASE=${UBUNTU_RELEASE}
        DOCKER_EXTENSION=".ubuntu";;
    (c) IMAGE_PREFIX="centos"
        OS_RELEASE=${CENTOS_RELEASE}
        DOCKER_EXTENSION=".centos";;
    (f) FORCE_REBUILD=true;;
    (*) usage
  esac
done
shift "$((OPTIND - 1))"

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
# Docker build must run from this script build dir, so all the relative paths work inside the Dockerfile's
cd "$SCRIPT_DIR" || exit

EMP_IMAGE="fbpcf/${IMAGE_PREFIX}-emp:${EMP_RELEASE}"
if [ $FORCE_REBUILD == false ] && docker image inspect ${EMP_IMAGE} > /dev/null 2>&1; then
  printf "%s docker image already exists. To force a rebuild please use '-f' flag...\n" ${EMP_IMAGE}
else
  printf "\nBuilding %s docker image...\n" ${EMP_IMAGE}
  docker build \
      --build-arg os_release=${OS_RELEASE} \
      --build-arg emp_release=${EMP_RELEASE} \
      -t ${EMP_IMAGE} -f docker/emp/Dockerfile${DOCKER_EXTENSION} .
fi

AWS_IMAGE="fbpcf/${IMAGE_PREFIX}-aws-s3-core:${AWS_RELEASE}"
if [ $FORCE_REBUILD == false ] && docker image inspect ${AWS_IMAGE} > /dev/null 2>&1; then
  printf "%s docker image already exists. To force a rebuild please use '-f' flag...\n" ${AWS_IMAGE}
else
  printf "\nBuilding %s docker image...\n" ${AWS_IMAGE}
  docker build  \
      --build-arg os_release=${OS_RELEASE} \
      --build-arg aws_release=${AWS_RELEASE} \
      -t ${AWS_IMAGE} -f docker/aws-s3-core/Dockerfile${DOCKER_EXTENSION} .
fi

FOLLY_IMAGE="fbpcf/${IMAGE_PREFIX}-folly:${FOLLY_RELEASE}"
if [ $FORCE_REBUILD == false ] && docker image inspect ${FOLLY_IMAGE} > /dev/null 2>&1; then
  printf "%s docker image already exists. To force a rebuild please use '-f' flag...\n" ${FOLLY_IMAGE}
else
  printf "\nBuilding %s docker image...\n" ${FOLLY_IMAGE}
  docker build  \
      --build-arg os_release=${OS_RELEASE} \
      --build-arg folly_release=${FOLLY_RELEASE} \
      --build-arg fmt_release=${FMT_RELEASE} \
      -t fbpcf/${IMAGE_PREFIX}-folly:${FOLLY_RELEASE} -f docker/folly/Dockerfile${DOCKER_EXTENSION} .
fi

printf "\nBuilding fbpcf/%s docker image...\n" ${IMAGE_PREFIX}
docker build  \
    --build-arg os_release=${OS_RELEASE} \
    --build-arg emp_release=${EMP_RELEASE} \
    --build-arg aws_release=${AWS_RELEASE} \
    --build-arg folly_release=${FOLLY_RELEASE} \
    --compress \
    -t fbpcf/${IMAGE_PREFIX}:latest -f docker/Dockerfile${DOCKER_EXTENSION} .
