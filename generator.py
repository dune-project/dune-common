""" Generator module:

    The module provides the main class for on the fly generation of boost
    python wrappers for implementations of a gives interface. The necessary
    details for each implementation (the C++ typedef and the includes) are
    provided by python dictonaries stored in files.
"""

from __future__ import absolute_import, division, print_function, unicode_literals

import importlib
import subprocess
import hashlib
import sys
import os
import re
import timeit

from .. import femmpi
from . import database

dataBasePath = os.path.join(os.path.dirname(__file__), "../../database")
compilePath = os.path.join(os.path.dirname(__file__), "../generated")
sys.path.append(compilePath)

class Generator(object):
    """ Generator class:

        The main class for on the fly generation of wrapper classes.
    """

    def __init__(self, typeName):
        """ Constructor

            Args:
                typeName (string): identifier for the interface classes to be
                                   generated (used for finding the right
                                   dictionary files)
        """
        self.typeName = typeName
        dbpath = os.path.join(dataBasePath, typeName.lower())
        self.dataBase =\
            database.DataBase(*[os.path.join(dbpath, dbfile)
                                for dbfile in os.listdir(dbpath)
                                if re.match(".*[.]db$", dbfile)])

    def getModule(self, myType, **parameters):
        """ generate and load the extension module for a given
            implementation

            Args:
                myType (string): identifier for the dictionary entry to use
                parameters (named arguments): values for free arguments
                    required for implementation's template arguments

            Returns:
                module: the name of the generated and imported extension module
        """
        selector = database.selector(myType, **{i: str(parameters[i])
                                                for i in parameters})
        self.dataBase.check_parameters(selector)
        myTypeName = self.modifyTypeName(self.dataBase.get_type(selector))
        myTypeHash = hashlib.md5(myTypeName.encode('utf-8')).hexdigest()
        moduleBase = self.typeName.lower()
        moduleName = moduleBase + "_" + myTypeHash

        includes = parameters[ "extra_includes" ] if "extra_includes" in parameters else ""
        for include in self.dataBase.get_includes(selector):
            includes = includes + "#include <" + include + ">\n"

        if femmpi.comm.rank == 0:
            if not os.path.isfile(os.path.join(compilePath, moduleName + ".so")):
                start_time = timeit.default_timer()
                out = open(os.path.join(compilePath, "generated_module.hh"), 'w')
                print(includes, file=out)
                print("#include <dune/fempy/py" + self.typeName.lower() + ".hh>", file=out)
                print(file=out)
                print("typedef " + myTypeName + " DuneType;", file=out)
                print(file=out)
                print("PYBIND11_PLUGIN( " + moduleName + " )", file=out)
                print("{", file=out)
                print("  pybind11::module module( \"" + moduleName + "\" );", file=out)
                print("  Dune::FemPy::register" + self.typeName + "< DuneType >( module );", file=out)
                print("  return module.ptr();", file=out)
                print("}", file=out)
                out.close()
                cmake = subprocess.Popen(["cmake", "--build", "../../..", "--target", "generated_module"], cwd=compilePath)
                cmake.wait()
                if cmake.returncode > 0:
                    print('FAILURE: could not build module -\
                           perhaps wrong choice of parameter or\
                           wrong configure options used during build?')
                    raise self.dataBase.CompileError(self.typeName.lower(), myTypeName)
                os.rename(os.path.join(compilePath, "generated_module.so"),
                          os.path.join(compilePath, moduleName + ".so"))
                os.rename(os.path.join(compilePath, "generated_module.hh"),
                          os.path.join(compilePath, moduleName + ".hh"))

        femmpi.comm.barrier()

        module = importlib.import_module("dune.generated."+moduleName)
        setattr(module, "_typeName", myTypeName)
        setattr(module, "_typeHash", myTypeHash)
        setattr(module, "_includes", includes)
        setattr(module, "_moduleBase", moduleBase)
        return module

    def modifyTypeName(self, typeName):
        return typeName
