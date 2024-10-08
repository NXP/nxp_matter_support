# Copyright 2022 The Pigweed Authors
# Copyright 2022 NXP
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

import argparse
import os
import sys

import pw_tokenizer
import serial


def parse_args():
    """Parse input arguments

    Return:
        parsed arguments struncture
    """

    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest='type')
    subparsers.required = True

    parser_file = subparsers.add_parser('file')
    parser_file.add_argument(
        "-i", "--input", help="Input file name.", required=True)
    parser_file.add_argument(
        "-d", "--database", help="Token database.", required=True)
    parser_file.add_argument(
        "-o", "--output", help="Output file name.", required=True)

    parser_file = subparsers.add_parser('serial')
    parser_file.add_argument(
        "-i", "--input", help="Input serial port name.", required=True)
    parser_file.add_argument(
        "-d", "--database", help="Token database.", required=True)
    parser_file.add_argument(
        "-o", "--output", help="Output file name. Write to stdout and to file.")

    return parser.parse_args()


def decode_string(tstr, detok):
    """Decodes a single token.

    Args:
        tstr        - encoded input string
        detok       - detokenizer

    Return:
        decoded string or None
    """
    try:
        t = bytes.fromhex(tstr)
        s = str(detok.detokenize(t))

        if s.find('$') == 0:
            return None
        return s
    except ValueError:
        return None


def decode_serial(serialport, outfile, database):
    """Decodes logs from serial port.

    Args:
        infile      - path to input file
        outfile     - path to output file
        database    - path to token database
    """

    detokenizer = pw_tokenizer.Detokenizer(database)
    input = serial.Serial(serialport, 115200, timeout=None)

    output = None
    if outfile:
        output = open(outfile, 'w')

    if input:

        try:
            while (True):
                # read line from serial port and ascii decode
                line = input.readline().decode('ascii').strip()
                # find token start and detokenize
                idx = line.rfind(']')
                dstr = decode_string(line[idx + 1:], detokenizer)
                if dstr:
                    line = line[:idx+1] + dstr
                print(line, file=sys.stdout)
                if output:
                    print(line, file=output)
        except Exception:
            print("Serial error or program closed", file=sys.stderr)

        if output:
            input.close()
            output.close()

    else:
        print("Invalid or closed serial port.", file=sys.stderr)


def decode_file(infile, outfile, database):
    """Decodes logs from input file.

    Args:
        infile      - path to input file
        outfile     - path to output file
        database    - path to token database
    """

    if os.path.isfile(infile):

        detokenizer = pw_tokenizer.Detokenizer(database)

        output = open(outfile, 'w')

        with open(infile, 'rb') as file:
            for line in file:
                try:
                    # ascii decode line
                    # serial terminals may include non ascii characters
                    line = line.decode('ascii').strip()
                except Exception:
                    continue
                # find token start and detokenize
                idx = line.rfind(']')
                dstr = decode_string(line[idx + 1:], detokenizer)
                if dstr:
                    line = line[:idx+1] + dstr
                print(line, file=output)
        output.close()

    else:
        print("File does not exist or is not a file.", file=sys.stderr)


def detokenize_input():

    args = parse_args()

    if args.type == 'file':
        decode_file(args.input, args.output, args.database)
    if args.type == 'serial':
        decode_serial(args.input, args.output, args.database)


if __name__ == '__main__':
    detokenize_input()
    sys.exit(0)
