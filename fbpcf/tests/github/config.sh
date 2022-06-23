#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# Usage: Github e2e configs

## Shared
export E2E_CLUSTER_NAME="onedocker-cluster-fbpcf-e2e-workflow"
export E2E_S3_BUCKET="fbpcf-e2e-github-workflow"

## Lift
# Lift result comparison
export LIFT_OUTPUT_PATH=s3://$E2E_S3_BUCKET/lift/outputs

## Attribution
# Attribution result comparison
export ATTRIBUTION_OUTPUT_PATH=s3://$E2E_S3_BUCKET/attribution/outputs
