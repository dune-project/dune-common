#!/bin/sh
# $Id$

# barf on errors
set -e

usage () {
    echo "Usage: ./autogen.sh [options]"
    echo "  --ac=, --acversion=VERSION   use a specific VERSION of autoconf"
    echo "  --am=, --amversion=VERSION   use a specific VERSION of automake"
    echo "  -h,    --help                you already found this :-)"
}

for OPT in "$@"; do
    set +e
    # stolen from configure...
    # when no option is set, this returns an error code
    arg=`expr "x$OPT" : 'x[^=]*=\(.*\)'`
    set -e

    case "$OPT" in
	--ac=*|--acversion=*)
			test "x$arg" == "x" || (usage; exit 1;)
			ACVERSION=$arg
			;;
	--am=*|--amversion=*)
			test "x$arg" == "x" || (usage; exit 1;)
			AMVERSION=$arg
			;;
	-h|--help) usage ; exit 0 ;;
    esac
done

## report parameters
if test "x$ACVERSION" != "x"; then
	echo "Forcing autoconf version $ACVERSION"
	ACVERSION=-$ACVERSION
	if ! which autoconf$ACVERSION > /dev/null; then
		echo
		echo "Error: Could not find autoconf$ACVERSION"
		echo "       Did you specify a wrong version?"
		exit 1
	fi
fi
if test "x$AMVERSION" != "x"; then
	echo "Forcing automake version $ACVERSION"
	AMVERSION=-$AMVERSION
	if ! which automake$AMVERSION > /dev/null; then
		echo
		echo "Error: Could not find automake$AMVERSION"
		echo "       Did you specify a wrong version?"
		exit 1
	fi
fi


## run autotools

echo "--> libtoolize..."
# this script won't rewrite the files if they already exist. This is a
# PITA when you want to upgrade libtool, thus I'm setting --force
libtoolize --force

# prepare everything
echo "--> aclocal..."
aclocal$AMVERSION -I m4

# applications should provide a config.h for now
echo "--> autoheader..."
autoheader$ACVERSION

echo "--> automake..."
automake$AMVERSION --add-missing

echo "--> autoconf..."
autoconf$ACVERSION

## tell the user what to do next
echo "Now run ./configure to setup your Dune"
