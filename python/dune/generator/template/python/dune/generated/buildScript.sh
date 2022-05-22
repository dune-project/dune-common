#!/bin/bash
SED=/bin/sed
GREP=/bin/grep
CUT=/usr/bin/cut
ENV=/usr/bin/env
ECHO=/bin/echo
# store flags
FLAGS="$@"
MAKE_EXECUTABLE_NEW=0

if [ "$CXXFLAGS" == "" ]; then
  # default CXX flags
  CXXFLAGS="{{ cxx_flags }}"
fi

if [ "$EXTRA_CXXFLAGS" != "" ]; then
  # extra CXX flags
  CXXFLAGS="$CXXFLAGS $EXTRA_CXXFLAGS"
fi

GRIDS=
CONFIG_H=/tmp/dune-env/.cache/dune-py/config.h
if [ "$GRIDTYPE" != "" ]; then
  GRIDS=`$GREP "defined USED_[A-Z_]*_GRIDTYPE" $CONFIG_H | $SED 's/\(.*defined USED\_\)\(.*\)\(\_GRIDTYPE*\)/\2/g'`
fi

OLDFLAGS=$FLAGS
FLAGS=
for FLAG in $OLDFLAGS; do
  NEWFLAG=$FLAG
  VARNAME=`$ECHO $FLAG | $GREP "\-D" | $SED 's/-D//g'`
  for GRID in $GRIDS; do
    if [ "$VARNAME" == "$GRID" ]; then
      NEWFLAG="-D$GRIDTYPE"
      break
    fi
  done
  VARNAME=`$ECHO $VARNAME | $GREP "=" | $CUT -d "=" -f 1`
  if [ "$VARNAME" != "" ]; then
    VAR=`$ENV | $GREP $VARNAME`
    if [ "$VAR" != "" ]; then
      # add variable from environment to flags list
      NEWFLAG="-D$VARNAME=${!VARNAME}"
    fi
  fi
  FLAGS="$FLAGS $NEWFLAG"
done





# Version 2 using link.txt and flags.make
# echo Building
# . flags.make
# {{ cxx_compiler }} $CXX_DEFINES $CXX_INCLUDES $CXX_FLAGS -MD -MT $1.dir/$1.cc.o -MF $1.dir/$1.cc.o.d -o ./$1.dir/$1.cc.o -c $1.cc
# bash linker.sh $1
# echo Done
