import os, glob, time
from dune.packagemetadata import getBuildMetaData, forceConfigure

# NOTE: do not import from dune.common (and consequently from dune.generator)
#       at top level to avoid failure due to missing mpi4py.

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


def listgenerated(args):
    from dune.common.module import getDunePyDir
    dune_py_dir = getDunePyDir()
    generated_dir = os.path.join(dune_py_dir, 'python', 'dune', 'generated')

    files = glob.glob(os.path.join(generated_dir, '*.so'))
    if args == 'bydate':
        files.sort(key=os.path.getatime)
    elif args == 'alphabetical':
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
