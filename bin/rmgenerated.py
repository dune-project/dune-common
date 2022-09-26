#!/usr/bin/env python3
# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import sys
from dune.generator.remove import removeGenerated
from argparse import ArgumentParser
parser = ArgumentParser(description='Removing generated module from dune-py')
parser.add_argument('-a', '--all', help='remove all modules', action='store_true', default=False)
parser.add_argument('modules', metavar='M', nargs='*',
                    help='base of the modules to remove')

print(
"""
Deprecation warning:
This script is deprecated and will be removed in the next release.
Call `python -m dune remove` with the same arguments instead.
""")

try:
    args = parser.parse_args()
except:
    sys.exit(0)

modules = []
if args.all:
    modules = ['all']
elif len(args.modules) > 0:
    modules = args.modules
else:
    parser.print_help()
    sys.exit(0)

removeGenerated(modules)
