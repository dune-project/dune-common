# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import sys
from argparse import ArgumentParser
from dune.commands import printinfo, configure, listgenerated, rmgenerated, fixdunepy, listdunetype, checkbuilddirs

# NOTE: do not import from dune.common (and consequently from dune.generator)
#       at top level to avoid failure due to missing mpi4py.


def run(arguments=None):
    parser = ArgumentParser(description='Execute DUNE commands', prog='dune')
    subparsers = parser.add_subparsers(dest='command')

    # Info
    parserConfigure = subparsers.add_parser('info',
              help='Print information about dune-py')

    # Configure
    parserConfigure = subparsers.add_parser('configure',
              help='Tag dune-py to be reconfigured before next use')

    # List
    parserList = subparsers.add_parser('list', help='List all generated modules')
    parserList.add_argument('--alphabetical', dest='sort', action='store_const', const='alphabetical', default='bydate',
              help='List modules in alphabetical order (default: by date)')
    parserList.add_argument('--ccfiles', dest='ccfiles', action='store_const', const=True, default=False,
              help='List modules by .cc ending instead of .so')

    # Remove
    parserRemove = subparsers.add_parser('remove', help='Remove generated modules')
    parserRemove.add_argument('--beforedate', dest='date', action='store_const', const=True, default=False,
              help='Instead of a pattern provide a date to remove all modules not having been loaded after that date')
    parserRemove.add_argument('modules', nargs='*',  default=[],
              help='Patterns of modules ("*.cc" and dune-py path is added to each argument) or "all"')

    # Fix dune-py
    parserFix = subparsers.add_parser('fix-dunepy',
              help='Find inconsistencies in dune-py and try to fix automatically. This will potentially delete all generated modules.')
    parserFix.add_argument('--force', dest='force', action='store_const', const=True, default=False,
              help='force complete removal of dune-py without checking for inconsistencies')

    # Dunetype
    parserDunetype = subparsers.add_parser('dunetype', help='Show dune types for given modules')
    parserDunetype.add_argument('modules', nargs='*', default=[],
              help='Patterns of modules ("*.cc" and dune-py path is added to each argument) or "all"')

    # CheckBuildDirs
    parserCheckBuildDirs = subparsers.add_parser('checkbuilddirs', help='(internal use) Check build directories')
    parserCheckBuildDirs.add_argument('args', nargs='+', default=[],
              help='Dune module name and string with builddirs separated by ";"')


    ret = 0
    args = parser.parse_args(arguments)

    if args.command == 'info':
        ret = printinfo()

    elif args.command == 'configure':
        ret = configure()

    elif args.command == 'list':
        ret = listgenerated(args.sort, args.ccfiles)

    elif args.command == 'remove':
        if args.modules == []:
            parserRemove.print_help()
        else:
            ret = rmgenerated(args.modules, args.date)

    elif args.command == 'fix-dunepy':
        ret = fixdunepy(args.force)

    elif args.command == 'dunetype':
        if args.modules == []:
            parserDunetype.print_help()
        else:
            ret = listdunetype(args.modules)

    elif args.command == 'checkbuilddirs':
        ret = checkbuilddirs(args.args)

    else:
        parser.print_help()

    sys.exit(ret)


if __name__ == '__main__':
    sys.exit( run() )
