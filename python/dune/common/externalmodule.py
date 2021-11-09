
import logging
import os
import json

from dune.packagemetadata import get_dune_py_dir
import dune.generator

logger = logging.getLogger(__name__)


def _loadExternalModules():
    """Check which external modules are currently registered in dune-py"""

    externalModulesPath = os.path.join(get_dune_py_dir(), ".externalmodules.json")
    if os.path.exists(externalModulesPath):
        with open(externalModulesPath) as externalModulesFile:
            return json.load(externalModulesFile)
    else:
        return []


# a list of Python modules externally registered to dune-py
EXTERNAL_PYTHON_MODULES = _loadExternalModules()


def cacheExternalModules(dunePyDir):
    """Store external modules in dune-py"""

    externalModulesPath = os.path.join(dunePyDir, ".externalmodules.json")
    with open(externalModulesPath, "w") as externalModulesFile:
        json.dump(EXTERNAL_PYTHON_MODULES, externalModulesFile)


def registerExternalModule(module):
    """Register an external module into the dune-py machinery

        Required for modules outside the dune namespace package
        to be correctly identified as a dune module to be registered
        with the code generation module dune-py.
    """

    # check if this module is registered for the first time
    if module not in EXTERNAL_PYTHON_MODULES:
        EXTERNAL_PYTHON_MODULES.append(module)
        logger.info("Registered external module {}".format(module))

        # if dune-py has already been created
        # and we are registering a new module,
        # we need to make sure that dune-py is reconfigured
        dunePyDir = get_dune_py_dir()
        if os.path.isdir(dunePyDir):
            # force (re-)configuration
            tagfile = os.path.join(dunePyDir, ".noconfigure")
            if os.path.exists(tagfile):
                os.remove(tagfile)

            # reload cmake builder
            dune.generator.reloadBuilder()
