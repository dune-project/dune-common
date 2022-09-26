# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import os, glob, time, fileinput, subprocess, shutil
from dune.packagemetadata import getBuildMetaData, forceConfigure

# NOTE: do not import from dune.common (and consequently from dune.generator)
#       at top level to avoid failure due to missing mpi4py.

def printinfo():
    from dune.common.module import getDunePyDir
    dune_py_dir = getDunePyDir()
    print("Location of dune-py:", dune_py_dir)

    generated_dir = os.path.join(dune_py_dir, 'python', 'dune', 'generated')
    files = glob.glob(os.path.join(generated_dir, '*.so'))
    print(len(files), "generated modules")
    return 0


def configure():
    print('Set up dune-py module for reconfiguration')
    forceConfigure()
    return 0


def checkbuilddirs(args):
    print('Comparing build directories of installed dune modules with given build directories')
    assert len(args) > 0

    # first arguments are the dune module name and last argument is a
    # string with builddirs separated by ';'
    modules   = args[:len(args)-1]
    builddirs = args[-1].split(';')

    # Extract the raw data dictionary
    try:
        metaData = getBuildMetaData()
        instbuilddirs = metaData.zip_across_modules("DEPS", "DEPBUILDDIRS")
    except ValueError as ex:
        print(ex)
        return 1

    for mod, bd in zip(modules, builddirs):
        instbd = instbuilddirs.get(mod, bd)
        if not instbd == bd:
            print("error in setup: module",mod,"installed from build directory",instbd,"but current build directory is based on module from",bd)
            return 1
    return 0


def rmgenerated(args, date):
    from dune.generator.remove import removeGenerated
    removeGenerated(args, date)
    return 0


def fixdunepy(force):
    from dune.common.module import getDunePyDir
    from dune.generator.remove import removeGenerated
    dune_py_dir = getDunePyDir()
    generated_dir = os.path.join(dune_py_dir, 'python', 'dune', 'generated')

    def filebases(list):
        return [os.path.splitext(os.path.basename(l))[0] for l in list]

    if force:
        if os.path.isdir(dune_py_dir):
            shutil.rmtree(dune_py_dir)
    else:
        ccfiles = filebases( glob.glob(os.path.join(generated_dir, '*.cc')) )
        sofiles = filebases( glob.glob(os.path.join(generated_dir, '*.so')) )
        cmakedirs = filebases( glob.glob(os.path.join(generated_dir, "CMakeFiles", "*.dir") ) )

        # Search CMakeLists.txt for pattern: dune_add_pybind11_module(NAME module_123 EXCLUDE_FROM_ALL)
        bracket = ['dune_add_pybind11_module(NAME ', ' EXCLUDE_FROM_ALL']
        cmakeentries = []
        for line in fileinput.input( os.path.join(generated_dir, 'CMakeLists.txt') ):
            start = line.find(bracket[0])
            if start != -1:
              start = start + len(bracket[0])
              end = line.find(bracket[1], start)
              cmakeentries += [line[start:end]]

        occurrences = {}
        for i in ccfiles + sofiles + cmakedirs + cmakeentries:
            occurrences[i] = occurrences.get(i, 0) + 1

        inconsistent = []
        for o in occurrences:
            if occurrences[o] != 4:
                inconsistent += [o]

        if len(inconsistent) > 0:
            print("Fix inconsistencies:")
            for i in inconsistent:
                print(" ", i)
            removeGenerated(inconsistent)

        # Call 'cmake .' and check output
        if os.path.isdir(dune_py_dir):
            p = subprocess.run(["cmake", "."], cwd=dune_py_dir, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            if p.returncode != 0:
                print("CMake failed! Remove whole dune-py.")
                shutil.rmtree(dune_py_dir)

    return 0

def listgenerated(sort, ccfiles):
    from dune.common.module import getDunePyDir
    dune_py_dir = getDunePyDir()
    generated_dir = os.path.join(dune_py_dir, 'python', 'dune', 'generated')

    if ccfiles:
        files = glob.glob(os.path.join(generated_dir, '*.cc'))
    else:
        files = glob.glob(os.path.join(generated_dir, '*.so'))

    if sort == 'bydate':
        files.sort(key=os.path.getatime)
    elif sort == 'alphabetical':
        files.sort()

    for filename in files:
        fileBase = os.path.splitext(os.path.basename(filename))[0]
        t = time.ctime(os.path.getatime(filename))
        print(t, ' ', fileBase)

    return 0


def listdunetype(args):
    from dune.common.module import getDunePyDir
    dune_py_dir = getDunePyDir()
    generated_dir = os.path.join(dune_py_dir, 'python', 'dune', 'generated')

    if args == ['all']: args = ['']
    for file in args:
        files = glob.glob(os.path.join(generated_dir, file+'*.cc'))
        files.sort(key=os.path.getmtime)
        for filename in files:
            mod = os.path.splitext(os.path.basename(filename))[0]
            t = time.ctime(os.path.getmtime(filename))
            print(t, mod+":", flush=True)
            with open(filename, 'rt') as f:
                for line in f:
                    if "using DuneType" in line:
                        print("   ", line.strip())
            print("")
    return 0
