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
    from dune.common.module import build_dune_py_module, get_dune_py_dir, make_dune_py_module, select_modules
except ImportError:
    import os
    here = os.path.dirname(os.path.abspath(__file__))
    modsA = os.path.join(os.path.dirname(here), "python", "dune")
    modsB = os.path.join(modsA,"common")
    sys.path.append(modsB)
    sys.path.append(modsA)

    if os.path.exists(os.path.join(modsB, "module.py")):
        from module import build_dune_py_module, get_dune_py_dir, make_dune_py_module, select_modules
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
        opts, args = getopt.getopt(argv,"ho",["opts=","builddir="])
    except getopt.GetoptError:
        print('usage: setup-dunepy.py [-o config.opts | --opts=config.opts | --builddir] [install]')
        sys.exit(2)

    optsfile = None
    builddir = None
    for opt, arg in opts:
        if opt == '-h':
            print('usage: setup-dunepy.py [-o config.opts | --opts=config.opts] [install]')
            sys.exit(2)
        elif opt in ("-o", "--opts"):
            optsfile = arg
        elif opt in ("--builddir"):
            builddir = arg
    if len(args) > 0:
        execute = args[0]
    else:
        execute = ""

    if optsfile is not None:
        definitions = {}
        command = ['bash', '-c', 'source ' + optsfile + ' && echo "$CMAKE_FLAGS"']
        proc = subprocess.Popen(command, stdout = subprocess.PIPE)
        stdout, _ = proc.communicate()
        for arg in shlex.split(buffer_to_str(stdout)):
            key, value = arg.split('=', 1)
            if key.startswith('-D'):
                key = key[2:]
            definitions[key] = value
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
        definitions = None
        if builddir is None:
            builddir = os.environ.get('DUNE_BUILDDIR', 'build-cmake')

    dunepy = get_dune_py_dir()

    if os.path.exists(dunepy):
        shutil.rmtree(dunepy)
    foundModule = make_dune_py_module(dunepy)

    output = build_dune_py_module(dunepy, definitions, None, builddir)

    print("CMake output")
    print(output)

    # set a tag file to avoid automatic reconfiguration in builder
    tagfile = os.path.join(dunepy, ".noconfigure")
    f = open(tagfile, 'w')
    f.close()

    if execute == "install":
        duneModules = select_modules()
        # moddir = duneModules[1]["dune-python"]
        for m,depends in duneModules[0].items():
            moddir = duneModules[1][m]
            pythonModule = toBuildDir(builddir,moddir,m)
            print("calling install_python in",moddir)
            command = ['cmake', '--build', '.', '--target', 'install_python']
            proc = subprocess.Popen(command, cwd=pythonModule, stdout = subprocess.PIPE)
            stdout, stderr = proc.communicate()
            logger.debug(buffer_to_str(stdout))

if __name__ == "__main__":
    main(sys.argv[1:])
