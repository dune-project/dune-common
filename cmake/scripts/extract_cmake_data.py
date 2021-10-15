#!/usr/bin/env python

""" This script will parse a cmake module and extract some
    rst documentation from it. This might not be as elegant as
    writing a Sphinx domain or using a custom extension with
    cmake related directives, but it provides a straightforward
    working way.

    This is used by dune-common to generate the build system documentation.
    Users do not want to use this!!!
"""
from __future__ import print_function

import argparse
import errno
import os
import re

def get_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('-b', '--builddir', help='The directory where to place the produced output', required=True)
    parser.add_argument('-m', '--module', help='The module to parse', required=True)
    return vars(parser.parse_args())

def write_line(f, line):
    if len(line) > 2:
        if line.startswith('#'):
            f.write(line[2:])
        else:
            f.write(line)
    else:
        f.write('\n')

def makedirs_if_not_exists(path):
    # Python3's os.makedirs has exist_ok=True, but this is still Python2...
    try:
        os.makedirs(path)
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

def read_module(args=get_args()):
    modname = os.path.splitext(os.path.basename(args['module']))[0]
    modpath = os.path.join(args['builddir'], 'modules')
    makedirs_if_not_exists(modpath)
    modfile = os.path.join(modpath, modname + '.rst')
    with open(args['module'], 'r') as i:
        o = None
        ml_block = 0

        for l in i:
            if ml_block > 0 and l.startswith('#]=='):
                ml_end = re.match(r'^#](=+)]', l)
                if len(ml_end.group(0)) == ml_block:
                    # end of multiline comment
                    return
            elif ml_block == 0 and l.startswith('#[==') and l.strip().endswith('.rst:'):
                # start of multiline comments
                ml_begin = re.match(r'^#\[(=+)\[', l)
                ml_block = len(ml_begin.group(0))

                # multiline comments always start the documentation of a module
                if o:
                    o.close()
                modpath = os.path.join(args['builddir'], 'modules')
                makedirs_if_not_exists(modpath)
                modfile = os.path.join(modpath, modname + ".rst")
                o = open(modfile, 'w')
                continue
            elif ml_block == 0 and not l.startswith('#'):
                # a line after a contiguous sequence of comments
                return

            comment = re.sub(r'^#?[ \t]*', '', l) # strip comment sign an spaces

            if comment.startswith('.. cmake_function'):
                if o:
                    o.close()
                cmdpath = os.path.join(args['builddir'], 'commands')
                makedirs_if_not_exists(cmdpath)
                try:
                    cmd = re.findall(r'\.\. cmake_function:: (.*)', l)[0]
                except IndexError as e:
                    print("CMake doc syntax error in {}: cannot parse function on line {}".format(args['module'], l))
                    raise e
                cmdfile = os.path.join(cmdpath, cmd + ".rst")
                o = open(cmdfile, 'w')
                o.write(".. _" + cmd + ":\n\n")
                o.write(cmd + "\n")
                o.write("="*len(cmd) + "\n\n")
                write_line(o, l)
            elif comment.startswith('.. cmake_variable'):
                if o:
                    o.close()
                varpath = os.path.join(args['builddir'], 'variables')
                makedirs_if_not_exists(varpath)
                try:
                    var = re.findall(r'\.\. cmake_variable:: (.*)', l)[0]
                except IndexError as e:
                    print("CMake doc syntax error in {}: cannot parse variable on line".format(args['module'], l))
                    raise e
                varfile = os.path.join(varpath, var + ".rst")
                o = open(varfile, 'w')
                o.write(".. _" + var + ":\n\n")
                o.write(var + "\n")
                o.write("="*len(var) + "\n\n")
                write_line(o, l)
            elif comment.startswith('.. cmake_module'):
                if o:
                    o.close()
                modpath = os.path.join(args['builddir'], 'modules')
                makedirs_if_not_exists(modpath)
                modfile = os.path.join(modpath, modname + ".rst")
                o = open(modfile, 'w')
                o.write(".. _" + modname + ":\n\n")
                o.write(modname + "\n")
                o.write("="*len(modname) + "\n\n")
                write_line(o, l)
            else:
                if o:
                    write_line(o, l)

# Parse the given arguments
read_module()
