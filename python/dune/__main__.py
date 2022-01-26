import sys
from argparse import ArgumentParser
from dune.commands import * # configure, checkbuilddirs, rmgenerated, listgenerated

def run(arguments=None):
    parser = ArgumentParser(description='Execute DUNE commands', prog='dune')
    subparsers = parser.add_subparsers(dest='command')

    # Configure
    parserConfigure = subparsers.add_parser('configure', help='Tag dune-py to be reconfigured before next use')

    # Remove
    parserRemove = subparsers.add_parser('remove', help='Remove generated modules')
    parserRemove.add_argument('modules', nargs='+', default=[], help='Patterns of modules to remove or "all"')

    # List
    parserList = subparsers.add_parser('list', help='List all generated modules')
    parserList.add_argument('--date', dest='sort',
                            action='store_const', const='date',
                            default='alpha', help='sort by creation date (default it alphabetical)')

    # dunetype
    parserDunetype = subparsers.add_parser('dunetype', help='Show dune types for given modules')
    parserDunetype.add_argument('modules', nargs='+', default=[],
              help='Patterns of modules - "*.cc" and dune-py path is added to each argument')

    # CheckBuildDirs
    parserCheckBuildDirs = subparsers.add_parser('checkbuilddirs', help='(internal use) Check build directories')
    parserCheckBuildDirs.add_argument('args', nargs='+', default=[], help='Dune module name and string with builddirs separated by ";"')


    ret = 0
    args = parser.parse_args(arguments)
    if args.command == 'configure':
        ret = configure()

    elif args.command == 'checkbuilddirs':
        ret = checkbuilddirs(args.args)

    elif args.command == 'remove':
        ret = rmgenerated(args.modules)

    elif args.command == 'dunetype':
        ret = listdunetype(args.modules)

    elif args.command == 'list':
        print(args.sort)
        ret = listgenerated(args.sort)

    else:
        ret = 1

    sys.exit(ret)


if __name__ == '__main__':
    sys.exit( run() )
