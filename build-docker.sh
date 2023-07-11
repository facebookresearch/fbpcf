#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set -e

UBUNTU_RELEASE="20.04"
EMP_TOOL_RELEASE="0.2.3"
EMP_RELEASE="0.2.2"
EMP_IMAGE_TAG="0.2.3-haswell"
AWS_RELEASE="1.9.379"
AWS_IMAGE_TAG="1.9.379-haswell"
GCP_RELEASE="v1.32.1"
FMT_RELEASE="7.1.3"
FOLLY_RELEASE="2021.06.28.00"
FOLLY_IMAGE_TAG="2021.06.28.00-haswell"
GITHUB_PACKAGES="ghcr.io/facebookresearch"

PROG_NAME=$0
usage() {
  cat <<EOF >&2
Usage: $PROG_NAME [-u] [-f] [-t TAG]

-u: builds the docker images against ubuntu (default)
-f: forces a rebuild of all docker image dependencies (even if they already exist)
-t TAG: Use the specified tag for the built image (default: latest)
EOF
  exit 1
}

IMAGE_PREFIX="ubuntu"
OS_RELEASE="${UBUNTU_RELEASE}"
DOCKER_EXTENSION=".ubuntu"
FORCE_REBUILD=false
TAG="latest"
while getopts "u,f,t:" o; do
  case $o in
  u)
    IMAGE_PREFIX="ubuntu"
    OS_RELEASE="${UBUNTU_RELEASE}"
    DOCKER_EXTENSION=".ubuntu"
    ;;
  f) FORCE_REBUILD=true ;;
  t) TAG=$OPTARG;;
  *) usage ;;
  esac
done
shift "$((OPTIND - 1))"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
# Docker build must run from this script build dir, so all the relative paths work inside the Dockerfile's
cd "$SCRIPT_DIR" || exit

build_dep_image() {
  local IMAGE=$1
  local DOCKER_DIR=$2
  local BUILD_ARGS=$3
  if [ "${FORCE_REBUILD}" == false ] && docker image inspect "${IMAGE}" >/dev/null 2>&1; then
    printf "%s docker image already exists. To force a rebuild please use '-f' flag...\n" "${IMAGE}"
  else
    printf "attempting to pull image %s from %s...\n" "${IMAGE}" "${GITHUB_PACKAGES}"
    if [ "${FORCE_REBUILD}" == false ] && docker pull "${GITHUB_PACKAGES}/${IMAGE}" 2>/dev/null; then
      # Use the external ghcr.io image (if a local image doesn't exist) instead of building locally...
      IMAGE="${GITHUB_PACKAGES}/${IMAGE}"
      printf "successfully pulled image %s\n\n" "${IMAGE}"
    else
      printf "%s/%s did not exist or '-f' flag was specifed\n" "${GITHUB_PACKAGES}" "${IMAGE}"
      printf "\nBuilding %s docker image...\n" "${IMAGE}"
      # shellcheck disable=SC2086
      docker build \
        ${BUILD_ARGS} \
        -t "${IMAGE}" -f "docker/${DOCKER_DIR}/Dockerfile${DOCKER_EXTENSION}" .
    fi
  fi
  RETURN="${IMAGE}"
}

EMP_IMAGE="fbpcf/${IMAGE_PREFIX}-emp:${EMP_IMAGE_TAG}"
build_dep_image "${EMP_IMAGE}" "emp" "--build-arg os_release=${OS_RELEASE} --build-arg emp_tool_release=${EMP_TOOL_RELEASE} --build-arg emp_release=${EMP_RELEASE}"
EMP_IMAGE="${RETURN}"

AWS_IMAGE="fbpcf/${IMAGE_PREFIX}-aws-s3-core:${AWS_IMAGE_TAG}"
build_dep_image "${AWS_IMAGE}" "aws-s3-core" "--build-arg os_release=${OS_RELEASE} --build-arg aws_release=${AWS_RELEASE}"
AWS_IMAGE="${RETURN}"

GCP_IMAGE="fbpcf/${IMAGE_PREFIX}-google-cloud-cpp:${GCP_RELEASE}"
build_dep_image "${GCP_IMAGE}" "google-cloud-cpp" "--build-arg os_release=${OS_RELEASE} --build-arg gcp_cpp_release=${GCP_RELEASE}"
GCP_IMAGE="${RETURN}"

FOLLY_IMAGE="fbpcf/${IMAGE_PREFIX}-folly:${FOLLY_IMAGE_TAG}"
build_dep_image "${FOLLY_IMAGE}" "folly" "--build-arg os_release=${OS_RELEASE} --build-arg folly_release=${FOLLY_RELEASE} --build-arg fmt_release=${FMT_RELEASE}"
FOLLY_IMAGE="${RETURN}"

printf "\nBuilding fbpcf/%s docker image...\n" ${IMAGE_PREFIX}
docker build \
  --build-arg os_release="${OS_RELEASE}" \
  --build-arg emp_image="${EMP_IMAGE}" \
  --build-arg aws_image="${AWS_IMAGE}" \
  --build-arg folly_image="${FOLLY_IMAGE}" \
  --build-arg gcp_image="${GCP_IMAGE}" \
  --compress \
  -t "fbpcf/${IMAGE_PREFIX}:${TAG}" -f "docker/Dockerfile${DOCKER_EXTENSION}" .
