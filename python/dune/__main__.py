import sys, os
from argparse import Action, ArgumentParser
from .packagemetadata import get_dune_py_dir

def configure():
    # force a reconfiguration of dune-py by deleting tagfile
    tagfile = os.path.join(get_dune_py_dir(), ".noconfigure")
    if os.path.exists(tagfile):
        os.remove(tagfile)


if __name__ == '__main__':
    parser = ArgumentParser(description='Execute DUNE commands', prog='dune')
    parser.add_argument('command', choices=['configure'], help="Command to be executed")

    args = parser.parse_args()

    if args.command == 'configure':
        print('Configure dune-py module')
        configure()
