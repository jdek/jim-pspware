#!/bin/sh

export EXTRA_CFLAGS="-D LUAPLAYER_USERMODE"
make clean
make

