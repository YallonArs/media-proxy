#!/bin/bash

if [ "$(id -u)" -ne 0 ]; then
    echo "Error: This script must be run as root." >&2
    exit 1
fi


if [ ! -f ".venv" ]; then
	python3 -m venv .venv
fi

source .venv/bin/activate
pip3 install keyboard
make -j

.venv/bin/python hotkey.py &
sleep 2
./main /dev/video0 &

wait
