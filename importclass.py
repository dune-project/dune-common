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
    if isString(includeFiles):
        if not os.path.dirname(includeFiles):
            with open(includeFiles, "r") as include:
                    source += include.read()
            source += "\n"
        else:
            source += "#include <"+includeFiles+">\n"
            includes += [includeFiles]
    elif isinstance(includeFiles, list):
        for includefile in includeFiles:
            if not os.path.dirname(includefile):
                with open(includefile, "r") as include:
                    source += include.read()
                source += "\n"
        else:
            source += "#include <"+includefile+">\n"
            includes += [includefile]

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

    source += "  auto cls = Dune::Python::insertClass< "+className+\
                 " >( module, \"cls\","+\
                 "Dune::Python::GenerateTypeName(\""+className+"\"),"+\
                 "Dune::Python::IncludeFiles{"+",".join(["\""+f+"\"" for f in includes])+"}).first;\n"
    ctorArgs = ", ".join([argTypes[i] + " arg" + str(i) for i in range(len(argTypes))])
    source += "cls.def( pybind11::init( [] ( "+ctorArgs+" ) {\n"
    source += "return new "+className+"( "+",".join(["arg"+str(i) for i in range(len(argTypes))]) +"); \n"
    source += " }));\n"
    source += "}"

    return builder.load(moduleName, source, signature).cls(*args)
