#!/bin/sh
#
aclocal -I m4
automake --include-deps --add-missing --copy
autoconf

#./configure $*
echo "Now you are ready to run ./configure"
