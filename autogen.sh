#!/bin/sh
# $Id$

# barf on errors
set -e

# prepare everything
echo "--> aclocal..."
aclocal
echo "--> autoheader..."
autoheader
echo "--> automake..."
automake --add-missing
echo "--> autoconf..."
autoconf
