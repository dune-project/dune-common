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

from . import database

from dune import comm
from dune import __path__ as basePaths

dataBasePaths = [os.path.join(p, "../database") for p in basePaths]
compilePath = os.path.join(basePaths[0],"generated")
sys.path.append(compilePath)

class Generator(object):
    """ Generator class:

        The main class for on the fly generation of wrapper classes.
    """
    force = False
    def __init__(self, typeName, pathToRegisterMethod, namespace, pythonname=None, filename=None):
        """ Constructor

            Args:
                typeName (string): identifier for the interface classes to be
                                   generated (used for finding the right
                                   dictionary files)
        """
        self.typeName = typeName
        self.pathToRegisterMethod = pathToRegisterMethod
        if namespace:
            self.namespace = namespace+"::"
        else:
            self.namespace = ""
        if pythonname == None:
          self.pythonName = typeName
        else:
          self.pythonName = pythonname
        self.fileName = filename
        if filename == None:
          dbpaths = [os.path.join(p, typeName.lower()) for p in dataBasePaths]
          dbfiles = []
          for p in dbpaths:
              if os.path.isdir(p):
                dbfiles += [os.path.join(p,dbfile)
                                    for dbfile in os.listdir(p)
                                    if re.match(".*[.]db$", dbfile)]
          self.dataBase = database.DataBase(*dbfiles,cppFile=False)
        else:
          self.dataBase = database.DataBase(filename,cppFile=True)

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
        selector = database.selector(myType, **{i: str(parameters[i]) for i in parameters})
        self.dataBase.check_parameters(selector)
        myTypeName = self.modifyTypeName(self.dataBase.get_type(selector))
        myTypeHash = hashlib.md5(myTypeName.encode('utf-8')).hexdigest()
        moduleBase = self.typeName.lower()
        moduleName = moduleBase + "_" + myTypeHash

        includes = parameters[ "extra_includes" ] if "extra_includes" in parameters else ""
        for include in self.dataBase.get_includes(selector):
            includes = includes + "#include <" + include + ">\n"
        includes = self.modifyIncludes(includes)
        # remove duplicate
        # includes = list(set( includes ))

        if comm.rank == 0:
            if not os.path.isfile(os.path.join(compilePath, moduleName + ".so")) or self.force:
                print("Compiling " + self.typeName + " module for " + myTypeName)
                start_time = timeit.default_timer()
                out = open(os.path.join(compilePath, "generated_module.hh"), 'w')
                print(includes, file=out)
                if self.dataBase.uses_extension(selector) == False:
                    if not self.pathToRegisterMethod == None:
                        print("#include <"+self.pathToRegisterMethod+"/" + self.typeName.lower() + ".hh>", file=out)
                        print(file=out)
                    if not self.fileName == None:
                        include = open(self.fileName, "r")
                        out.write( include.read() )
                        print(file=out)
                print("typedef " + myTypeName + " DuneType;", file=out)
                print(file=out)
                if self.namespace == "":
                    print("void register" + self.typeName + "( ... ) {}", file=out)
                print("PYBIND11_PLUGIN( " + moduleName + " )", file=out)
                print("{", file=out)
                print("  pybind11::module module( \"" + moduleName + "\" );", file=out)
                print("  typedef std::unique_ptr<DuneType> Holder;", file=out)
                print("  typedef DuneType TypeAlias;", file=out)
                print("  auto cls = pybind11::class_<DuneType,Holder,TypeAlias>(module,\"" + self.pythonName + "\");", file=out);
                print("  "+self.namespace+"register" + self.typeName + "( module, cls );", file=out)

                for c in self.dataBase.get_constructors(selector):
                    print(" cls.def(pybind11::init<"+c+">());", file=out)
                for m in self.dataBase.get_methods(selector):
                    print(" cls.def(\""+m[0]+"\",&"+m[1]+");", file=out)

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
                print("Compilation took: " , timeit.default_timer()-start_time, "seconds")

        comm.barrier()

        module = importlib.import_module("dune.generated."+moduleName)
        setattr(module, "_typeName", myTypeName)
        setattr(module, "_typeHash", myTypeHash)
        setattr(module, "_includes", includes)
        setattr(module, "_moduleBase", moduleBase)
        setattr(module, "_selector", selector)
        return module

    def modifyIncludes(self, includes):
        return includes

    def modifyTypeName(self, typeName):
        return typeName

def getModule(clsType, filename, **parameters):
    generator = Generator(clsType, None, None, None, filename)
    return generator.getModule(clsType, **parameters)
