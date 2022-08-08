#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set -e

PROG_NAME=$0
usage() {
  cat << EOF >&2
Usage: $PROG_NAME [-u]

-u: runs the sanity check sample game using the ubuntu docker image (default)
EOF
  exit 1
}

IMAGE_PREFIX="ubuntu"
while getopts "u" o; do
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

DATA_DIRECTORY=https://fbpcf-e2e-github-workflow.s3.us-west-2.amazonaws.com/edit_distance
GLOBAL_PARAMS=edit_distance_200_params.csv
PLAYER_0_INPUT=edit_distance_200_player_1_input.csv
PLAYER_0_OUTPUT=edit_distance_200_output_1.json
PLAYER_1_INPUT=edit_distance_200_player_2_input
PLAYER_1_OUTPUT=edit_distance_200_output_2.json
EXPECTED_RESULTS=edit_distance_200_results.csv

TEMP_DIR=$(echo $RANDOM | md5sum | head -c 20; echo;)

docker run --rm \
  --network=host ${FBPCF_IMAGE} \
  edit_distance_runner \
    --party=0 \
    --server_ip=127.0.0.1 \
    --port=5001 \
    --input_path=${DATA_DIRECTORY}/inputs/${PLAYER_0_INPUT} \
    --input_params=${DATA_DIRECTORY}/inputs/${GLOBAL_PARAMS} \
    --output_file_path="${DATA_DIRECTORY}/outputs/${TEMP_DIR}/${PLAYER_0_OUTPUT}" \
    2>&1 player0 & # Fork to background so other party can run below

if docker run --rm \
  --network=host ${FBPCF_IMAGE} \
  edit_distance_runner \
    --party=1 \
    --server_ip=127.0.0.1 \
    --port=5001 \
    --input_path=${DATA_DIRECTORY}/inputs/${PLAYER_1_INPUT}.csv \
    --input_params=${DATA_DIRECTORY}/inputs/${GLOBAL_PARAMS} \
    --output_file_path="${DATA_DIRECTORY}/outputs/${TEMP_DIR}/${PLAYER_1_OUTPUT}"; then
  printf "Edit Distance Ran Successfully\n"
  printf "Outputs saved to %s/outputs/%s\n" "$DATA_DIRECTORY" "$TEMP_DIR"
else
  printf "Something went wrong running edit distance. You may want to check the logs above."
  exit 1
fi

if docker run --rm \
  --network=host ${FBPCF_IMAGE} \
  edit_distance_validator \
    --shares_filepath_0="${DATA_DIRECTORY}/outputs/${TEMP_DIR}/${PLAYER_0_OUTPUT}" \
    --shares_filepath_1="${DATA_DIRECTORY}/outputs/${TEMP_DIR}/${PLAYER_1_OUTPUT}" \
    --results_path=${DATA_DIRECTORY}/inputs/${EXPECTED_RESULTS}; then
  printf "Successfully validated edit distance results\n"
else
  printf "Validation failed. You may want to check the logs above.\n"
  exit 1
fi
