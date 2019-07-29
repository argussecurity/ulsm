# User mode linux security module example
# Copyright (C) 2019  Argus Cyber Security Ltd, Tel Aviv.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import os
import socket
import struct
import sys

import capnp

NL_MSG_HDR_FORMAT = "=LHHLL"
NL_MSG_HDR_FORMAT_SIZE = struct.calcsize(NL_MSG_HDR_FORMAT)
VERDICTS = {0:"Allow", 1:"Block"}
COLOR_START_DICT = {0:"\x1b[32m", 1:"\x1b[31m"}
COLOR_END = "\x1b[0m"

_, COLS = os.popen('stty size', 'r').read().split()
COLS = int(COLS)

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
PROC_SCHEME_PATH = os.path.join(SCRIPT_DIR, "process.capnp")


def send_initializaiton_message(sock):
    """
    send an initialization message to let kernel module know new user client is connected.
    :param sock: socket to send on
    :return:
    """
    send_messsage(sock, b"\x00\x00\x00\x00")


def send_messsage(sock, data):
    """
    send a data buffer to the kernel security module using netlink
    :param sock: socket to send on
    :param data: the data to send
    :return:
    """
    length = NL_MSG_HDR_FORMAT_SIZE + len(data)
    flags = 0
    seq = 0
    type_ = 0
    nlmsg = struct.pack(NL_MSG_HDR_FORMAT, length, type_, flags, seq, os.getpid())
    sock.send(nlmsg + data)


def recv_messsage(sock):
    """
    Receive a data buffer from the kernel security module using netlink
    :param sock: socket to send on
    :return: data buffer received
    """
    data = sock.recv(100)
    return data[NL_MSG_HDR_FORMAT_SIZE:]


def security_logic(proc):
    """
    Dummy function that performs functionality on messages arrived from kernel security module
    hooks in middle of operation.
    :param proc: the proc received from the kernel
    :return: 0 - to allow operation to continue, 1 - to block operation from proceeding.
    """
    if proc.name == "/bin/ls":
        return 1
    return 0


def main():
    """
    User security module example implementation.
    :return: -1 upon failure.
    """
    try:
        sock = socket.socket(socket.AF_NETLINK, socket.SOCK_DGRAM, 31)
        sock.bind((0, 0))
        send_initializaiton_message(sock)
    except Exception as e:
        print("Failure message: " + str(e))
        exit(-1)

    print("Listening to messages from kernel")
    print("_" * COLS)
    capnp_scheme = capnp.load(PROC_SCHEME_PATH)

    while True:
        kernel_message = recv_messsage(sock)
        proc = capnp_scheme.Process.from_bytes(kernel_message)
        verdict = security_logic(proc)
        print()
        print("Message ID: {} |\t Executable path: {} |\t {}Verdict: {}{}".format(
            proc.id, proc.name, COLOR_START_DICT[verdict], VERDICTS[verdict], COLOR_END))
        print("_" * COLS)
        send_messsage(sock, struct.pack("=II", proc.id, verdict))


if __name__ == '__main__':
    print("Starting")
    main()
    sys.exit(0)
