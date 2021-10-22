#!/usr/bin/env python3

import glob, os, sys, re, fileinput, shutil

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

def rmJit(filename):
    fileBase = os.path.splitext(os.path.basename(filename))[0]
    filePath, fileName = os.path.split(filename)
    os.remove( os.path.join(filePath,filename) )
    os.remove( os.path.join(filePath,fileBase+'.cc') )
    try:
        shutil.rmtree( os.path.join(filePath,"CMakeFiles",fileBase+".dir") )
    except:
        pass
    moduleFiles.update( [fileBase] )

if args.all:
    base = os.path.join(generated_dir, '*.so')
    for filename in glob.iglob( base ):
        rmJit(filename)
elif len(args.modules)>0:
    for m in args.modules:
        base = os.path.join(generated_dir, m+'*.so')
        for filename in glob.iglob( base ):
            rmJit(filename)
else:
    parser.print_help()
    sys.exit(0)

for line in fileinput.input( os.path.join(generated_dir, 'CMakeLists.txt'), inplace = True):
    if not any( [m in line for m in moduleFiles] ):
         print(line, end="")
