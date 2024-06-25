# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import os

from dune.common.hashit import hashIt
from dune.common.utility import isString
from dune.generator.algorithm import cppType

def load(className, includeFiles, *args,
         options=None, bufferProtocol=False, dynamicAttr=False,
         holder="default",
         baseClasses=None ):
    if options is None: options=[]
    if baseClasses is None: baseClasses=[]
    if not bufferProtocol: # kwargs.get("bufferProtocol", False):
        clsParams = []
    else:
        clsParams = ['pybind11::buffer_protocol()']
    if dynamicAttr:
        clsParams += ['pybind11::dynamic_attr()']

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

    includes += ["python/dune/generated/"+moduleName+".cc"]

    source += "".join(["#include <" + i + ">\n" for i in includes])
    source += "\n"
    source += '#include <dune/python/common/typeregistry.hh>\n'
    source += '#include <dune/python/pybind11/pybind11.h>\n'
    source += '\n'

    source += "PYBIND11_MODULE( " + moduleName + ", module )\n"
    source += "{\n"


    for i, bc in enumerate(baseClasses):
        if not holder == "default":
            baseHolder = ", " + holder + "<" + bc + ">"
        else:
            baseHolder = ''
        source += 'Dune::Python::insertClass' +\
                       '< ' + bc + baseHolder + '>' +\
                       '( module, "cls' + str(i) + '"' +\
                       ', Dune::Python::GenerateTypeName("' + bc + '")' +\
                       ', Dune::Python::IncludeFiles{}' +\
                       ");\n"
        options.append(bc)

    if not holder == "default":
        options += [holder + "<" + className + ">"]

    source += "auto cls = Dune::Python::insertClass< "+className+\
                   ', '.join(('',)+tuple(options)) + ' >('+\
                 "module, \"cls\","+','.join(('',)+tuple(clsParams))+\
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

    # make sure to reload the builder here in case it got updated
    from . import builder
    return builder.load(moduleName, source, signature).cls(*args)
