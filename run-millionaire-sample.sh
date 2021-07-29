#!/bin/bash

# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set -e

PROG_NAME=$0
usage() {
  cat << EOF >&2
Usage: $PROG_NAME [-u]

-u: runs the millionaire sample using the ubuntu docker image (default)
EOF
  exit 1
}

IMAGE_PREFIX="ubuntu"
while getopts u o; do
  case $o in
    (u) IMAGE_PREFIX="ubuntu";;
    (*) usage
  esac
done
shift "$((OPTIND - 1))"

FBPCF_IMAGE=fbpcf/${IMAGE_PREFIX}:latest
if docker image inspect ${FBPCF_IMAGE} > /dev/null 2>&1; then
  printf "Found %s docker image... \n" ${FBPCF_IMAGE}
else
  printf "ERROR: Unable to find docker image %s. Please run build-docker.sh \n" ${FBPCF_IMAGE}
  exit 1
fi

docker run --rm \
    --network=host ${FBPCF_IMAGE} \
        millionaire \
        --role=1 \
        --port=5000 \
        2>&1 publisher & # Fork to background so "partner" can run below

if docker run --rm \
    --network=host ${FBPCF_IMAGE} \
        millionaire \
        --role=2 \
        --port=5000 \
        --server_ip=127.0.0.1; then
  printf "Millionaire ran successfully! "
else
  printf "Something went wrong. You may want to check the logs above."
  exit 1
fi
