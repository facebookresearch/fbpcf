#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import unittest

from fbpcf.public_tld.example.edit_distance.data_gen.edit_distance_calculator import (
    EditDistanceCalculator,
)

# import fbpcf.public_tld.example.edit_distance.data_gen.generate_edit_distance_data as edit_distance
# from edit_distance_calculator import EditDistanceCalculator


class TestGenerateEditDistanceData(unittest.TestCase):
    def assert_single(
        self,
        word: str,
        guess: str,
        delete_cost: str,
        insert_cost: str,
        correct_distance: int,
    ):
        cost = EditDistanceCalculator.edit_distance(
            word, guess, delete_cost, insert_cost
        )
        self.assertEqual(
            cost,
            correct_distance,
            f"Word: {word}, Guess: {guess}, Expected Cost {correct_distance}, Actual Cost {cost}",
        )

    def test_edit_distance(self):
        test_cases = [
            ("aaaaaaaaaa", "", 35, 30, 300),
            ("", "aaaaaaaaaa", 35, 30, 350),
            ("", "", 35, 30, 0),
            ("abc", "abc", 35, 30, 0),
            ("trail", "toil", 35, 30, 33),
            ("thank", "think", 35, 30, 8),
            ("thank", "think", 1, 1, 2),
        ]

        for case in test_cases:
            self.assert_single(case[0], case[1], case[2], case[3], case[4])
