#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""
CLI for generating data files for the edit distance example application that is used to benchmark PCF

Usage:
    generate_edit_distance_data --threshold=<threshold> --dictionary_path=<dictionary_path> --output_size=<size> --output_prefix=<output_prefix>
        [--delete_cost=<cost> --insert_cost=<cost>]

Options:
    --threshold=<threshold>             Desc
    --dictionary_path=<dictionary_path> Desc
    --output_size=<size>                Desc
    --output_prefix=<output_prefix>     Desc
    --delete_cost=<cost>                Desc
    --insert_cost=<cost>                Desc

Example:
    buck run //fbpcf/public_tld/example/edit_distance/data_gen:generate_edit_distance_data -- --threshold=100 \
        --dictionary_path="$HOME/fbsource/fbcode/fbpcf/public_tld/example/edit_distance/data_gen/dictionary.txt" --output_size=200 --output_prefix='edit_distance_200'
"""

import io
import os
import random

import schema
from docopt import docopt
from fbpcf.public_tld.example.edit_distance.data_gen.edit_distance_calculator import (
    EditDistanceCalculator,
)


DEFAULT_INSERT_COST = 30
DEFAULT_DELETE_COST = 35


def main():
    s = schema.Schema(
        {
            "--threshold": schema.Use(int),
            "--dictionary_path": str,
            "--output_size": schema.Use(int),
            "--output_prefix": str,
            "--delete_cost": schema.Or(None, schema.Use(int)),
            "--insert_cost": schema.Or(None, schema.Use(int)),
        }
    )

    arguments = s.validate(docopt(__doc__))

    dictionary = read_dictionary(arguments["--dictionary_path"])
    dict_size = len(dictionary)

    threshold = arguments["--threshold"]
    size = arguments["--output_size"]
    insert_cost = arguments["--insert_cost"] or DEFAULT_INSERT_COST
    delete_cost = arguments["--delete_cost"] or DEFAULT_DELETE_COST

    random.seed()

    output_prefix = arguments["--output_prefix"]

    player_1_input_fd = io.BufferedWriter(
        open(f"{output_prefix}_player_1_input.csv", "wb")
    )
    player_2_input_fd = io.BufferedWriter(
        open(f"{output_prefix}_player_2_input.csv", "wb")
    )
    results_fd = io.BufferedWriter(open(f"{output_prefix}_results.csv", "wb"))

    player_1_input_fd.write(b"word,sender_message\n")
    player_2_input_fd.write(b"guess\n")
    results_fd.write(b"distance,receiver_message\n")

    for _ in range(size):
        word = dictionary[random.randrange(0, dict_size)]
        guess = dictionary[random.randrange(0, dict_size)]
        sender_message = dictionary[random.randrange(0, dict_size)]
        distance = EditDistanceCalculator.edit_distance(
            word, guess, delete_cost, insert_cost
        )

        if distance < threshold:
            receiver_message = sender_message
        elif distance <= threshold * 2:
            receiver_message = sender_message[0 : len(sender_message) // 2]
        else:
            receiver_message = ""

        player_1_input_fd.write(f"{word},{sender_message}\n".encode("UTF-8"))
        player_2_input_fd.write(f"{guess}\n".encode("UTF-8"))
        results_fd.write(f"{distance},{receiver_message}\n".encode("UTF-8"))

    player_1_input_fd.close()
    print(f"Wrote player 1 input to {os.path.realpath(player_1_input_fd.name)}")

    player_2_input_fd.close()
    print(f"Wrote player 2 input to {os.path.realpath(player_2_input_fd.name)}")

    results_fd.close()
    print(f"Wrote results to {os.path.realpath(results_fd.name)}")

    write_game_params(arguments["--output_prefix"], threshold, delete_cost, insert_cost)


def read_dictionary(dictionary_path: str):
    with open(dictionary_path) as f:
        lines = f.readlines()
        dictionary = list(map(lambda line: line.lower().strip(), lines))
    return dictionary


def write_game_params(
    output_prefix: str, threshold: int, delete_cost: int, insert_cost: int
):
    file_name = f"{output_prefix}_params.csv"
    with open(file_name, "w") as f:
        f.write("threshold,delete_cost,insert_cost\n")
        f.write(f"{threshold},{delete_cost},{insert_cost}\n")
        print(f"Wrote game parameters to {os.path.realpath(f.name)}")


if __name__ == "__main__":
    main()
