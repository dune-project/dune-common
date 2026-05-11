# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import sys
import os
import importlib
import logging
import atexit
from pathlib import Path

logger = logging.getLogger(__name__)

# use citationsfile to output citations to a file
_bibFile = None

def _format_citations():
    coremodules = ["dune.common", "dune.geometry", "dune.grid", "dune.istl", "dune.localfunctions"]
    output = ""
    for modname in sys.modules:
        try:
            module = importlib.import_module(modname)
            if hasattr(module, "_cite_dune_module_as"):
                citations = module._cite_dune_module_as()
                if modname in coremodules and modname != "dune.common":
                    continue
                modulename = modname if modname != "dune.common" else str(", ".join(mod for mod in coremodules))
                output += f"\n% For {modulename} please cite:"
                output += citations
        except ImportError as e:
          logger.debug('failed to import ' + modname + ': ' + str(e) + '.')
          continue

    return output

# print citations to stdout
def _print_citations():
    from dune.common import comm

    citations = """%% In this program you have used DUNE modules for which
%% we kindly ask to cite the following articles:\n"""
    citations += _format_citations()
    if citations is not None and comm.rank == 0:
        if _bibFile is None:
            print(citations)
        else:
            with open(_bibFile,"w") as f:
                print(citations,file=f)

# prints all citations before the end of the program
def show():
    """
    Print citations of all DUNE modules included in the code so far.
    This function should be called at the end of the program.

    Note: In regular Python scripts the code
    ```
    import dune.citations
    ```
    will print all citations. This will not work in notebooks, which is why this
    function exists.
    """

    # avoid list being printed at exit (since it's printed now)
    atexit.unregister(_print_citations)

    _print_citations()

    ## end show

# when used with python -m dune.citations,
# references for all installed dune modules are printed
def collectAllCitations():
    import dune.common
    import pkgutil

    # avoid list being printed at exit
    atexit.unregister(_print_citations)

    package = dune
    prefix = package.__name__ + "."
    # first import all 'dune' subpackages and collect the 'registry' dicts
    dunesubmodules = set()
    for importer, modname, ispkg in pkgutil.iter_modules(package.__path__, prefix):
        if ispkg:
            try:
                # Note: modname.__init__ is imported so be aware of
                # possible side effects
                module = importlib.import_module(modname)
            except ImportError as e:
                logger.debug('failed to import ' + modname + ': ' + str(e) + '.')
                continue

    citations = "%% This is a list of references for all installed DUNE modules.\n"
    citations += _format_citations()
    return citations

# when the program that included citations.py end, the list of necessary
# citations is printed
atexit.register(_print_citations)
