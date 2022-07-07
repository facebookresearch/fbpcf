#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


class EditDistanceCalculator:
    @staticmethod
    def edit_distance(word: str, guess: str, delete_cost: int, insert_cost: int):
        word_len = len(word)
        guess_len = len(guess)

        # First index is position from end of word
        # second index is position from end of guess.
        sub_problems = [[0]]

        for i in range(1, word_len + 1):
            sub_problems.append([insert_cost + sub_problems[i - 1][0]])

        for i in range(1, guess_len + 1):
            sub_problems[0].append(delete_cost + sub_problems[0][i - 1])

        for i in range(1, word_len + 1):
            c_word = word[word_len - i]
            for j in range(1, guess_len + 1):
                c_guess = guess[guess_len - j]
                sub_problems[i].append(
                    min(
                        sub_problems[i - 1][j - 1]
                        + EditDistanceCalculator.dist(c_word, c_guess),
                        sub_problems[i - 1][j] + insert_cost,
                        sub_problems[i][j - 1] + delete_cost,
                    )
                )
        return sub_problems[word_len][guess_len]

    @staticmethod
    def dist(c_word: str, c_guess: str):
        return abs(ord(c_word) - ord(c_guess))
