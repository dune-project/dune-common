#!/bin/sh
# $Id$

# barf on errors
set -e

# prepare everything
echo "--> aclocal..."
aclocal

# applications should provide a config.h for now
#echo "--> autoheader..."
#autoheader

echo "--> automake..."
automake --add-missing

echo "--> autoconf..."
autoconf

echo "files successfully created, please run ./configure now"