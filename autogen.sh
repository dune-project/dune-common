#!/bin/sh
# $Id$

# barf on errors
set -e

echo "--> libtoolize..."
# this script won't rewrite the files if they already exist. This is a
# PITA when you want to upgrade libtool, thus I'm setting --force
libtoolize --force

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