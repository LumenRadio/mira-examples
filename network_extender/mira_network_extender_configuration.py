#!/usr/bin/env python3

# A tool to setup MiraUSB Network Extender
#
#
# MIT License
#
# Copyright (c) 2023 LumenRadio AB
#
# SPDX-License-Identifier: MIT
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
#

import argparse
import sys

import serial


# Custom action, to parse hex values for address/length instead of plain int
def arg_net_pan_id(value):
    if not value.startswith("0x"):
        value = "0x" + value
    pan_id = int(value, 0)
    if (pan_id & 0xFFFFFFFF) != pan_id:
        raise argparse.ArgumentTypeError("Invalid PAN ID")
    return pan_id


def arg_net_key(value):
    value_arr = bytearray.fromhex(value)
    if len(value_arr) != 16:
        raise argparse.ArgumentTypeError("Invalid length")
    return value


def arg_net_rate(value):
    rate = int(value, 10)
    if rate < 0 or rate > 15:
        raise argparse.ArgumentTypeError("Invalid rate")
    return rate


def arg_antenna(value):
    antenna = int(value, 10)
    if antenna < 0 or antenna > 255:
        raise argparse.ArgumentTypeError("Invalid antenna")
    return antenna


def arg_build_parser():
    parser = argparse.ArgumentParser(
        description="MiraUSB Network Extender configuration tool"
    )

    hw_group = parser.add_argument_group("Hardware interface:")
    hw_group.add_argument(
        "-d",
        "--hw-dev",
        dest="hw_dev",
        metavar="DEV",
        help="Device for sniffer",
        required=True,
    )

    net_group = parser.add_argument_group("Network parameters")
    net_group.add_argument(
        "-p",
        "--net-pan-id",
        dest="net_pan_id",
        type=arg_net_pan_id,
        metavar="PANID",
        help="PAN ID to listen to",
        required=True,
    )

    net_group.add_argument(
        "-k",
        "--net-key",
        dest="net_key",
        type=arg_net_key,
        metavar="KEY",
        help="AES key of string",
        required=True,
    )

    net_group.add_argument(
        "-r",
        "--net-rate",
        dest="net_rate",
        type=arg_net_rate,
        metavar="RATE",
        help="Rate (0..15)",
        required=True,
    )

    net_group.add_argument(
        "-a",
        "--antenna",
        dest="antenna",
        type=arg_antenna,
        metavar="ANTENNA",
        help="Antenna to use. Specifies antenna index in module config. Default: 0.",
        required=True,
    )

    return parser


if __name__ == "__main__":
    import argparse

    parser = arg_build_parser()
    args = parser.parse_args()

    try:
        sdev = serial.Serial(args.hw_dev, 115200)
    except:
        print("Could not open device: " + args.hw_dev)
        sys.exit(-1)

    sdev.timeout = 3
    sdev.writeTimeout = 3
    sdev.write(("\n").encode())
    sdev.write(("version\n").encode())
    try:
        version = sdev.read_until("\n")
    except serial.SerialTimeoutException:
        print("Timeout while waiting for device")
        sys.exit(-1)

    if len(version) == 0 or str(version).startswith("MiraUSB Network Extender"):
        print(version)
        print("Not a MiraUSB Network Extender device")
        sys.exit(-1)

    print("MiraUSB Network Extender found!")
    sdev.write(("config set_pan_id " + hex(args.net_pan_id)[2:] + "\n").encode())
    sdev.write(("config set_key " + str(args.net_key) + "\n").encode())
    sdev.write(("config set_rate " + str(args.net_rate) + "\n").encode())
    sdev.write(("config set_antenna " + str(args.antenna) + "\n").encode())
    print("MiraUSB Network Extender configured!")

    sdev.close()
