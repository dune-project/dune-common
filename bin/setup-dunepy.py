#!/usr/bin/env python3
import getopt
import os
import shlex
import subprocess
import sys
import shutil
import logging

logger = logging.getLogger(__name__)

try:
    from dune.common.module import build_dune_py_module, get_dune_py_dir, make_dune_py_module, select_modules, resolve_dependencies, resolve_order
    from dune.common.locking import Lock, LOCK_EX
except ImportError:
    import os
    here = os.path.dirname(os.path.abspath(__file__))
    modsA = os.path.join(os.path.dirname(here), "python", "dune")
    modsB = os.path.join(modsA,"common")
    sys.path.append(modsB)
    sys.path.append(modsA)
    if os.path.exists(os.path.join(modsB, "module.py")):
        from module import build_dune_py_module, get_dune_py_dir, make_dune_py_module, select_modules, resolve_dependencies, resolve_order
        from locking import Lock, LOCK_EX
    else:
        raise

def buffer_to_str(b):
    return b if sys.version_info.major == 2 else b.decode('utf-8')

def toBuildDir(builddir, moddir, module):
    if os.path.isabs(builddir):
        return os.path.join(builddir ,module)
    else:
        return os.path.join(moddir, builddir)

def main(argv):
    try:
        opts, args = getopt.getopt(argv,"ho",["opts=","builddir=","module="])
    except getopt.GetoptError:
        print('usage: setup-dunepy.py [-o config.opts | --opts=config.opts | --builddir] [--module=mod] [install]')
        sys.exit(2)

    optsfile = None
    builddir = None
    masterModule = None
    for opt, arg in opts:
        if opt == '-h':
            print('usage: setup-dunepy.py [-o config.opts | --opts=config.opts] [install]')
            sys.exit(2)
        elif opt in ("-o", "--opts"):
            optsfile = arg
        elif opt in ("--builddir",):
            builddir = arg
        elif opt in ("--module",):
            masterModule = arg
    if len(args) > 0:
        execute = args[0]
    else:
        execute = ""

    if optsfile is not None:
        definitions = {}
        command = ['bash', '-c', 'source ' + optsfile + ' && echo "$CMAKE_FLAGS"']
        proc = subprocess.Popen(command, stdout = subprocess.PIPE)
        stdout, _ = proc.communicate()
        cmake_args = shlex.split(buffer_to_str(stdout))
        if builddir is None:
            # get the build dir (check for BUILDDIR, DUNE_BUILDDIR in opts file
            # and then DUNE_BUILDDIR in environment variable
            command = ['bash', '-c', 'source ' + optsfile + ' && echo "$BUILDDIR"']
            proc = subprocess.Popen(command, stdout = subprocess.PIPE)
            stdout, _ = proc.communicate()
            builddir = buffer_to_str(stdout).strip()
            if not builddir:
                command = ['bash', '-c', 'source ' + optsfile + ' && echo "$DUNE_BUILDDIR"']
                proc = subprocess.Popen(command, stdout = subprocess.PIPE)
                stdout, _ = proc.communicate()
                builddir = buffer_to_str(stdout).strip()
                if not builddir:
                    builddir = os.environ.get('DUNE_BUILDDIR', 'build-cmake')
    else:
        cmake_args = None
        if builddir is None:
            builddir = os.environ.get('DUNE_BUILDDIR', 'build-cmake')

    # Generate list of all modules
    duneModules = select_modules()

    # Generate list of dependencies for dune-py. If --module=mod is passed,
    # use mod and all its dependencies only. Otherwise use all found modules
    # as dependencies.
    if masterModule is None:
        deps = resolve_order(duneModules[0])
    else:
        depsList = resolve_dependencies(duneModules[0], masterModule)
        deps = {k:v for k,v in duneModules[0].items() if k in depsList}
        deps = resolve_order(deps)
        deps += [masterModule]

    if execute == "install":
        for m in deps:
            moddir = duneModules[1][m]
            pythonModule = toBuildDir(builddir,moddir,m)
            print("calling install_python for %s (%s)" % (m,pythonModule))
            try:
                command = ['cmake', '--build', '.', '--target', 'install_python']
                proc = subprocess.Popen(command, cwd=pythonModule, stdout = subprocess.PIPE)
                stdout, stderr = proc.communicate()
                logger.debug(buffer_to_str(stdout))
            except FileNotFoundError:
                print("Warning: build dir not found possibly module is installed then python bindings should be already available")

    dunepy = get_dune_py_dir()
    dunepyBase = os.path.realpath( os.path.join(dunepy,"..") )
    if not os.path.exists(dunepyBase):
        os.makedirs(dunepyBase)
    with Lock(os.path.join(dunepyBase, 'lock-module.lock'), flags=LOCK_EX):
        if os.path.exists(dunepy):
            shutil.rmtree(dunepy)
        os.makedirs(dunepy)
        foundModule = make_dune_py_module(dunepy, deps)
        output = build_dune_py_module(dunepy, cmake_args, None, builddir, deps, writetagfile=True)

    print("CMake output")
    print(output)

if __name__ == "__main__":
    main(sys.argv[1:])
