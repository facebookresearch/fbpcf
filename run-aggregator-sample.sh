#!/bin/bash

# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
# Run from the root dir of fbpcf so the sample paths exist
cd "$SCRIPT_DIR" || exit
mkdir -p sample

docker run --rm \
    -v "$SCRIPT_DIR/lift/aggregator/test:/input" \
    -v "$SCRIPT_DIR/sample:/output" \
    --network=host fbpcf:latest \
        aggregator \
        --role=1 \
        --visibility=0 \
        --input_path=/input/aggregator_alice \
        --output_path=/output/output_alice \
        --num_shards=3 \
        --threshold=100 \
        & # Fork to background so "Bob" can run below

docker run --rm \
    -v "$SCRIPT_DIR/lift/aggregator/test:/input" \
    -v "$SCRIPT_DIR/sample:/output" \
    --network=host fbpcf:latest \
        aggregator \
        --server_ip=127.0.0.1 \
        --role=2 \
        --visibility=0 \
        --input_path=/input/aggregator_bob \
        --output_path=/output/output_bob \
        --num_shards=3 \
        --threshold=100
