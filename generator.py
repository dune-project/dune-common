""" Generator module:

    The module provides the main class for on the fly generation of boost
    python wrappers for implementations of a gives interface. The necessary
    details for each implementation (the C++ typedef and the includes) are
    provided by python dictonaries stored in files.
"""

from __future__ import absolute_import, division, print_function, unicode_literals

import hashlib
import os
import re

from . import builder
from . import database

from dune import __path__ as basePaths

dataBasePaths = [os.path.join(p, "../database") for p in basePaths]

class Generator(object):
    """ Generator class:

        The main class for on the fly generation of wrapper classes.
    """
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
        self.builder = builder.Builder(verbose=True)

    def load(self, includes, typeName, moduleName, constructors=None, methods=None):
        source = includes
        if self.pathToRegisterMethod is not None:
            source += "#include <" + self.pathToRegisterMethod + "/" + self.typeName.lower() + ".hh>\n"
            source += "\n"

        if self.fileName is not None:
            with open(self.fileName, "r") as include:
                source += include.read()
            source += "\n"

        source += "typedef " + typeName + " DuneType;\n"
        source += "\n"
        if self.namespace == "":
            source += "void register" + self.typeName + "( ... ) {}\n"
        source += "PYBIND11_PLUGIN( " + moduleName + " )\n"
        source += "{\n"
        source += "  pybind11::module module( \"" + moduleName + "\" );\n"
        source += "  typedef std::unique_ptr< DuneType > Holder;\n"
        source += "  typedef DuneType TypeAlias;\n"
        source += "  auto cls = pybind11::class_< DuneType, Holder, TypeAlias >( module, \"" + self.pythonName + "\" );\n"
        source += "  " + self.namespace + "register" + self.typeName + "( module, cls );\n"

        if constructors is not None:
            source += "".join(["  cls.def( pybind11::init< " + c + " >() );\n" for c in constructors])
        if methods is not None:
            source += "".join(["  cls.def( \"" + m[0] + "\", &" + m[1] + ");\n" for m in methods])

        source += "  return module.ptr();\n"
        source += "}\n"

        module = self.builder.load(moduleName, source)
        setattr(module, "_typeName", typeName)
        setattr(module, "_includes", includes)
        return module

    def getModule(self, myType, myTypeHash=None, **parameters):
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
        if not myTypeHash:
            myTypeHash = hashlib.md5(myTypeName.encode('utf-8')).hexdigest()
        moduleBase = self.typeName.lower()
        moduleName = moduleBase + "_" + myTypeHash

        includes = parameters[ "extra_includes" ] if "extra_includes" in parameters else ""
        for include in self.dataBase.get_includes(selector):
            includes = includes + "#include <" + include + ">\n"
        includes = self.modifyIncludes(includes)
        # remove duplicate
        # includes = list(set( includes ))

        constructors = self.dataBase.get_constructors(selector)
        methods = self.dataBase.get_methods(selector)
        module = self.load(includes, myTypeName, moduleName, constructors, methods)
        setattr(module, "_moduleBase", moduleBase)
        setattr(module, "_selector", selector)
        return module

    def modifyIncludes(self, includes):
        return includes

    def modifyTypeName(self, typeName):
        return typeName

def getModule(filename, **parameters):
    extra_includes=""
    for k,v in parameters.items():
        try:
            module = v._module
            parameters[k] = module._typeName
            extra_includes += module._includes
        except:
            pass
    with open(filename, 'r') as myfile:
        fileHash = hashlib.md5(myfile.read().encode('utf-8')).hexdigest()
    className = parameters.get("class","Object")
    generator = Generator("Object", None, None, None, filename)
    return generator.getModule("Object", myTypeHash=fileHash, extra_includes=extra_includes,**parameters).Object
