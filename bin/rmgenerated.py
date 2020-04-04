#!/usr/bin/env python

import glob, os, sys, re, fileinput

import dune.common.module
dune_py_dir = dune.common.module.get_dune_py_dir()
generated_dir = os.path.join(dune_py_dir, 'python', 'dune', 'generated')

from argparse import ArgumentParser
parser = ArgumentParser(description='Removing generated module from dune-py')
parser.add_argument('-a', '--all', help='remove all modules', action='store_true', default=False)
parser.add_argument('modules', metavar='M', nargs='*',
                    help='base of the modules to remove')

try:
    args = parser.parse_args()
except:
    sys.exit(0)

moduleFiles = set()

if args.all:
    base = os.path.join(generated_dir, '*.so')
    for filename in glob.iglob( base ):
        os.remove( filename )
        os.remove( os.path.splitext(filename)[0]+'.cc' )
        moduleFiles.update( [os.path.splitext(os.path.basename(filename))[0]] )
elif len(args.modules)>0:
    for m in args.modules:
        base = os.path.join(generated_dir, m+'*')
        for filename in glob.iglob( base ):
            os.remove( filename )
            moduleFiles.update( [os.path.splitext(os.path.basename(filename))[0]] )
else:
    parser.print_help()
    sys.exit(0)

for line in fileinput.input( os.path.join(generated_dir, 'CMakeLists.txt'), inplace = True):
    if not any( [m in line for m in moduleFiles] ):
         print(line, end="")
