from __future__ import absolute_import, division, print_function, unicode_literals

import os

from dune.common.hashit import hashIt
from . import builder
from dune.common.compatibility import isString
from dune.generator.algorithm import cppType

def load(className, includeFiles, *args):
    source = '#include <config.h>\n\n'
    source += '#define USING_DUNE_PYTHON 1\n\n'
    includes = []
    if isString(includeFiles) or hasattr(includeFiles,"readable"):
        includeFiles = [includeFiles]
    for includefile in includeFiles:
        if isString(includefile):
            if not os.path.dirname(includefile):
                with open(includefile, "r") as include:
                    source += include.read()
                source += "\n"
            else:
                source += "#include <"+includefile+">\n"
                includes += [includefile]
        elif hasattr(includefile,"readable"): # for IOString
            with includefile as include:
                source += include.read()
            source += "\n"

    argTypes = []
    for arg in args:
        t,i = cppType(arg)
        argTypes.append(t)
        includes += i

    signature = className + "( " + ", ".join(argTypes) + " )"
    moduleName = "class_" + hashIt(signature) + "_" + hashIt(source)

    includes = sorted(set(includes))
    source += "".join(["#include <" + i + ">\n" for i in includes])
    source += "\n"
    source += '#include <dune/python/common/typeregistry.hh>\n'
    source += '#include <dune/python/pybind11/pybind11.h>\n'
    source += '\n'

    source += "PYBIND11_MODULE( " + moduleName + ", module )\n"
    source += "{\n"

    includes += ["python/dune/generated/"+moduleName+".cc"]
    source += "  auto cls = Dune::Python::insertClass< "+className+\
                 " >( module, \"cls\","+\
                 "Dune::Python::GenerateTypeName(\""+className+"\"),"+\
                 "Dune::Python::IncludeFiles{"+",".join(["\""+f+"\"" for f in includes])+"}).first;\n"
    ctorArgs = ", ".join([argTypes[i] + " arg" + str(i) for i in range(len(argTypes))])
    source += "cls.def( pybind11::init( [] ( "+ctorArgs+" ) {\n"
    source += "return new "+className+"( "+",".join(["arg"+str(i) for i in range(len(argTypes))]) +"); \n"
    source += "})"+" ".join([", pybind11::keep_alive< 1, {} >()".format(i+2) for i in range(len(argTypes))]) + "\n"
    source += ");\n"
    source += "}"

    source = "#ifndef def_"+moduleName+\
             "\n#define def_"+moduleName+"\n"+\
             source+\
             "\n#endif\n"

    return builder.load(moduleName, source, signature).cls(*args)
