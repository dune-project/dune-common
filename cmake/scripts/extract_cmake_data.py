#!/usr/bin/env python
# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

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
        f.write(line[2:])
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
#         mod = open(modfile, 'w')
#         # Write the first block into the module rst file
#         mod.write(".. _" + modname + ":\n\n")
#         mod.write(modname + "\n")
#         mod.write("="*len(modname) + "\n\n")

#         listHeader = False
        o = None

        for l in i:
            if not l.startswith('#'):
                return
            if l.startswith('# .. cmake_function'):
                if o:
                    o.close()
                cmdpath = os.path.join(args['builddir'], 'commands')
                makedirs_if_not_exists(cmdpath)
                try:
                    cmd = re.findall(r'# .. cmake_function:: (.*)', l)[0]
                except IndexError as e:
                    print("CMake doc syntax error in {}: cannot parse function on line {}".format(args['module'], l))
                    raise e
                cmdfile = os.path.join(cmdpath, cmd + ".rst")
#                 if not listHeader:
#                     mod.write("\nThis module defines the following functions or macros:\n\n")
#                     listHeader = True
#                 mod.write("* :ref:`{}`\n".format(cmd))
                o = open(cmdfile, 'w')
                o.write(".. _" + cmd + ":\n\n")
                o.write(cmd + "\n")
                o.write("="*len(cmd) + "\n\n")
                write_line(o, l)
            elif l.startswith('# .. cmake_variable'):
                if o:
                    o.close()
                varpath = os.path.join(args['builddir'], 'variables')
                makedirs_if_not_exists(varpath)
                try:
                    var = re.findall(r'# .. cmake_variable:: (.*)', l)[0]
                except IndexError as e:
                    print("CMake doc syntax error in {}: cannot parse variable on line".format(args['module'], l))
                    raise e
                varfile = os.path.join(varpath, var + ".rst")
                o = open(varfile, 'w')
                o.write(".. _" + var + ":\n\n")
                o.write(var + "\n")
                o.write("="*len(var) + "\n\n")
                write_line(o, l)
            elif l.startswith('# .. cmake_module'):
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
