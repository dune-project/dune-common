#!/bin/sh
# $Id$

# barf on errors
set -e

# may be used to force a certain automake-version e.g. 1.7
AMVERS=

DEFAULTCONFOPT=

usage () {
    echo "Usage: ./autogen.sh [options]"
    echo "  -i, --intel        use intel compiler"
    echo "  -g, --gnu          use gnu compiler (default)"
    echo "  -m, --mpi          use mpicc"
    echo "  --opts=FILE        use compiler-options from FILE"
    echo "  -d, --debug        switch debug-opts on"
    echo "  -n, --nodebug      switch debug-opts off"
    echo "  -o, --optim        switch optimization on"
    echo "  -h, --help         you already found this :)"
}

# no compiler set yet
COMPSET=0
for OPT in $* ; do

    set +e
    # stolen from configure...
    # when no option is set, this returns an error code
    arg=`expr "x$OPT" : 'x[^=]*=\(.*\)'`
    set -e

    case "$OPT" in
	-i|--intel)   . ./icc.opts ; COMPSET=1 ;;
	-g|--gnu)     . ./gcc.opts ; COMPSET=1 ;;
	-m|--mpi)     . ./mpi.opts ; COMPSET=1 ;;
	--opts=*)
	    if [ -r $arg ] ; then
	      echo "reading options from $arg..."
	      . ./$arg ;
	      COMPSET=1;
	    else
	      echo "Cannot open compiler options file $arg!" ;
	      exit 1;
	    fi ;;
	-d|--debug)   DEBUG=1 ;;
	-n|--nodebug) DEBUG=0 ;;
	-o|--optim)   OPTIM=1 ;;
	-h|--help) usage ; exit 0 ;;
	# pass unknown opts to ./configure
	*) CONFOPT="$CONFOPT $OPT" ;;
    esac
done

# use the free compiler as default
if [ "$COMPSET" != "1" ] ; then
    echo "No compiler set, using GNU compiler as default"
    . ./gcc.opts
fi

# create flags
COMPFLAGS="$FLAGS"

# maybe add debug flag
if [ "$DEBUG" = "1" ] ; then	
    COMPFLAGS="$COMPFLAGS $DEBUGFLAGS"
fi

# maybe add optimization flag
if [ "$OPTIM" = "1" ] ; then	
    COMPFLAGS="$COMPFLAGS $OPTIMFLAGS"
fi

# check if automake-version was set
if test "x$AMVERS" != x ; then
  echo Warning: explicitly using automake version $AMVERS
  # binaries are called automake-$AMVERS
  AMVERS="-$AMVERS"
fi

## run autotools

echo "--> libtoolize..."
# this script won't rewrite the files if they already exist. This is a
# PITA when you want to upgrade libtool, thus I'm setting --force
libtoolize --force

# prepare everything
echo "--> aclocal..."
aclocal$AMVERS -I m4

# applications should provide a config.h for now
echo "--> autoheader..."
autoheader

echo "--> automake..."
automake$AMVERS --add-missing

echo "--> autoconf..."
autoconf

#### start configure with special environment

export CC="$COMP"
export CXX="$CXXCOMP"
export CPP="$COMP -E"

export CFLAGS="$COMPFLAGS"
export CXXFLAGS="$COMPFLAGS"

./configure $DEFAULTCONFOPT $CONFOPT
