# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import sys
import os
import importlib
import logging
import atexit
from pathlib import Path

logger = logging.getLogger(__name__)

def _format_citations(print_if_statement = False):
    outprefix = "%% In this program you have used modules for which we kindly ask to cite the following articles:\n"
    if print_if_statement:
        outprefix = "%% These are the articles we kindly ask to cite for the installed DUNE modules.\n"
    coremodules = ["dune.common", "dune.geometry", "dune.grid", "dune.istl", "dune.localfunctions"]
    output = ""
    for modname in sys.modules:
        try:
            module = importlib.import_module(modname)
            if hasattr(module, "__cite_dune_module_as__"):
                citations = module.__cite_dune_module_as__
                if modname in coremodules and modname != "dune.common":
                    continue
                if print_if_statement:
                    modulename = modname if modname != "dune.common" else str(", ".join(mod for mod in coremodules))
                    output += f"\n% If you are using {modulename}, please cite:"
                output += citations
        except ImportError as e:
          logger.debug('failed to import ' + modname + ': ' + str(e) + '.')
          continue

    return outprefix + output

# print citations to stdout
def _print_citations(print_if_statement = False):
    from dune.common import comm

    citations = _format_citations(print_if_statement)
    if citations is not None and comm.rank == 0:
        if bibFile is None:
            print(citations)
        else:
            with open(bibFile,"w") as f:
                print(citations,file=f)

def _print_citations_if_statement():
    _print_citations(True)

# when used with python -m dune.citations,
# references for all installed dune modules are printed
def collectAllCitations(bibtexfile = False):
    import dune.common
    import pkgutil

    # avoid list being printed at exit
    atexit.unregister(_print_citations)

    package = dune
    prefix = package.__name__ + "."
    # first import all 'dune' subpackages and collect the 'registry' dicts
    dunesubmodules = set()
    _print_if_statement = True
    for importer, modname, ispkg in pkgutil.iter_modules(package.__path__, prefix):
        if ispkg:
            try:
                # Note: modname.__init__ is imported so be aware of
                # possible side effects
                module = importlib.import_module(modname)
            except ImportError as e:
                logger.debug('failed to import ' + modname + ': ' + str(e) + '.')
                continue

    citations = _format_citations(print_if_statement=bibtexfile is None)
    return citations

# set 'bibFile' to a file name to get the output printed into that file instead of stdout
# The following line could be used to set this to 'scriptname.py.bib' - turned off for now
# bibFile = Path(sys.argv[0]).resolve().name + ".bib"
bibFile = None

# when the program that included citations.py end, the list of necessary
# citations is printed
atexit.register(_print_citations)
