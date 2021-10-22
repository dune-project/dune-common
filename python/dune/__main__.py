import sys, os
from argparse import Action, ArgumentParser
from dune.packagemetadata import get_dune_py_dir, extract_metadata

def configure():
    # force a reconfiguration of dune-py by deleting tagfile
    tagfile = os.path.join(get_dune_py_dir(), ".noconfigure")
    if os.path.exists(tagfile):
        os.remove(tagfile)
def checkbuilddirs(args):
    # first arguments are the dune module name and last argument is a
    # string with builddirs separated by ';'
    modules   = args[:len(args)-1]
    builddirs = args[-1].split(';')

    # Extract the raw data dictionary
    data = extract_metadata(ignoreImportError=True)
    try:
        instbuilddirs = data.zip_across_modules("DEPS", "DEPBUILDDIRS")
    except ValueError as ex:
        print(ex)
        return 1

    for mod, bd in zip(modules, builddirs):
        instbd = instbuilddirs.get(mod, bd)
        if not instbd == bd:
            print("error in setup: module",mod,"installed from build directory",instbd,"but current build directory is based on module from",bd)
            return 1
    return 0

def run(arguments=None):
    parser = ArgumentParser(description='Execute DUNE commands', prog='dune')
    parser.add_argument('command', choices=['configure','checkbuilddirs'], help="Command to be executed")
    parser.add_argument('--args', nargs='+', default=[], help='command arguments')

    args = parser.parse_args(arguments)

    if args.command == 'configure':
        print('Set up dune-py module for reconfiguration')
        configure()
    elif args.command == 'checkbuilddirs':
        print('Comparing build directories of installed dune modules with given build directories')
        cmdArgs = args.args
        assert len(cmdArgs) > 0
        ret = checkbuilddirs(cmdArgs)
        sys.exit(ret)

if __name__ == '__main__':
    sys.exit( run() )
