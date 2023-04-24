#!/bin/bash
# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
####################################################################################
# Usage: Use this script as compiler launcher within cmake, i.e.
#
# -DCMAKE_CXX_COMPILER_LAUNCHER=path_to_dune/dune-common/python/dune/generator/template/dune-compiler_launcher.sh
#
# In addition set -DCMAKE_CXX_FLAGS="" to only get compiler flags added by the
# cmake configuration. Set you own compiler flags by setting
#
# export DEFAULT_CXXFLAGS="-O3 -DNDEBUG -Wall"
#
# or whatever the default should be.
#
# Overload the CXXFLAGS when calling make by
#
# make target CXXFLAGS="-g"
#
# or
#
# make target GRIDTYPE=" .. "
#
#####################################################################################

# set as environment variable or change here
if [ "$DEFAULT_CXXFLAGS" == "" ]; then
  # default CXX flags provided here
  DEFAULT_CXXFLAGS="-O3 -DNDEBUG -Wall"
fi

SED=sed
GREP=grep
CUT=cut
ENV=env
ECHO=echo

# store compiler, provided as first argument to script
COMPILER=$1

# store compiler flags
FLAGS="${@:2}"

if [ "$CXXFLAGS" == "" ]; then
  # default CXX flags provided by user
  CXXFLAGS="$DEFAULT_CXXFLAGS"
fi

if [ "$EXTRA_CXXFLAGS" != "" ]; then
  # extra CXX flags
  CXXFLAGS="$CXXFLAGS $EXTRA_CXXFLAGS"
fi

# check for available grid implementations
GRIDS=
if [ "$GRIDTYPE" != "" ]; then
  CONFIG_H=config.h
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
# call compiler
$ECHO "$COMPILER $CXXFLAGS $FLAGS"
exec $COMPILER $CXXFLAGS $FLAGS
