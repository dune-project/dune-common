import sys
from argparse import ArgumentParser
from dune.commands import configure, checkbuilddirs, rmgenerated, listdunetype, listgenerated

# NOTE: do not import from dune.common (and consequently from dune.generator)
#       at top level to avoid failure due to missing mpi4py.


def run(arguments=None):
    parser = ArgumentParser(description='Execute DUNE commands', prog='dune')
    subparsers = parser.add_subparsers(dest='command')

    # Configure
    parserConfigure = subparsers.add_parser('configure',
              help='Tag dune-py to be reconfigured before next use')

    # List
    parserList = subparsers.add_parser('list', help='List all generated modules')
    parserList.add_argument('--alphabetical', dest='sort', action='store_const', const='alphabetical', default='bydate',
              help='List modules in alphabetical order (default: by date)')

    # Remove
    parserRemove = subparsers.add_parser('remove', help='Remove generated modules')
    parserRemove.add_argument('--beforedate', dest='date', action='store_const', const=True, default=False,
              help='Instead of a pattern provide a date to remove all modules not having been loaded after that date')
    parserRemove.add_argument('modules', nargs='*',  default=[],
              help='Patterns of modules ("*.cc" and dune-py path is added to each argument) or "all"')

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
    if args.command == 'configure':
        ret = configure()

    elif args.command == 'checkbuilddirs':
        ret = checkbuilddirs(args.args)

    elif args.command == 'remove':
        if args.modules == []:
            parserRemove.print_help()
        else:
            ret = rmgenerated(args.modules, args.date)

    elif args.command == 'dunetype':
        if args.modules == []:
            parserDunetype.print_help()
        else:
            ret = listdunetype(args.modules)

    elif args.command == 'list':
        ret = listgenerated(args.sort)

    else:
        parser.print_help()

    sys.exit(ret)


if __name__ == '__main__':
    sys.exit( run() )
