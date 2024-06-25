# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

""" Generator module:

    The module provides the main class for on the fly generation of pybind11
    Python wrappers for implementations of a gives interface. The necessary
    details for each implementation (the C++ typedef and the includes) are
    provided by python dictionaries stored in files.
"""

import logging

from dune.common.hashit import hashIt

logger = logging.getLogger(__name__)

class SimpleGenerator(object):
    def __init__(self, typeName, namespace, pythonname=None, filename=None):
        if not (isinstance(typeName,list) or isinstance(typeName,tuple)):
            self.single = True
            typeName = [typeName]
        else:
            self.single = False
        self.typeName = typeName
        if namespace:
            self.namespace = namespace+"::"
        else:
            self.namespace = ""
        if pythonname is None:
          self.pythonName = typeName
        else:
          self.pythonName = pythonname
        self.fileName = filename

    def pre(self, includes, duneType, moduleName, defines=None, preamble=None):
        if defines is None: defines = []
        source  = '#ifndef Guard_' + moduleName + '\n'
        source += '#define Guard_' + moduleName + '\n\n'
        source += '#include <config.h>\n\n'
        source += '#define USING_DUNE_PYTHON 1\n\n'
        source += ''.join(["#define " + d + "\n" for d in defines])
        source += ''.join(["#include <" + i + ">\n" for i in includes])
        source += '\n'
        source += '#include <dune/python/common/typeregistry.hh>\n'
        source += '#include <dune/python/pybind11/pybind11.h>\n'
        source += '#include <dune/python/pybind11/stl.h>\n'
        source += '\n'

        if self.fileName is not None:
            with open(self.fileName, "r") as include:
                source += include.read()
            source += "\n"
        if preamble is not None:
            source += preamble
            source += "\n"

        if self.namespace == "":
            source += "void register" + self.typeName[0] + "( ... ) {}\n"
        source += "PYBIND11_MODULE( " + moduleName + ", module )\n"
        source += "{\n"
        return source

    def main(self, nr, includes, duneType, *args,
            options=None, bufferProtocol=False, dynamicAttr=False,
            holder="default",
            baseClasses=None):
        if options is None: options=[]
        if baseClasses is None: baseClasses=[]
        source = "  using pybind11::operator\"\"_a;\n"
        if not bufferProtocol: # kwargs.get("bufferProtocol", False):
            clsParams = []
        else:
            clsParams = ['pybind11::buffer_protocol()']
        if dynamicAttr:
            clsParams += ['pybind11::dynamic_attr()']

        if nr == 0:
            source += '  pybind11::module cls0 = module;\n'

        source += '  {\n'
        source += "    using DuneType = " + duneType + ";\n"

        for i, bc in enumerate(baseClasses):
            if not holder == "default":
                baseHolder = "," + holder + "<" + bc + ">"
            else:
                baseHolder = ''
            source += '    Dune::Python::insertClass' +\
                           '< ' + bc + baseHolder + '>' +\
                           '( module, "cls' + str(i) + '"' +\
                           ', Dune::Python::GenerateTypeName("' + bc + '")' +\
                           ', Dune::Python::IncludeFiles{}' +\
                           ");\n"
            options.append(bc)

        if not holder == "default":
            options += [holder + "<DuneType>"]

        source += '    auto cls = Dune::Python::insertClass' +\
                       '< DuneType' +\
                       ', '.join(('',)+tuple(options)) + ' >' +\
                       '( cls0, "' + self.pythonName[nr] + '"' +\
                       ','.join(('',)+tuple(clsParams)) +\
                       ', Dune::Python::GenerateTypeName("' + duneType + '")' +\
                       ', Dune::Python::IncludeFiles{' + ','.join(['"' + i + '"' for i in includes]) + '}' +\
                       ").first;\n"
        source += "    " + self.namespace + "register" + self.typeName[nr] + "( cls0, cls );\n"

        for arg in args:
            if arg:
                source += "".join("    " + s + "\n" for s in str(arg).splitlines())
        source += '  }\n'
        return source

    def post(self, moduleName, source, postscript, extraCMake):
        if postscript:
            source += postscript
        source += "}\n"
        source += '#endif'

        # make sure to reload the builder here in case it got updated
        from . import builder
        module = builder.load(moduleName, source, self.typeName[0], extraCMake)

        return module

    def load(self, includes, typeName, moduleName, *args,
            extraCMake=None,
            defines=None, preamble=None, postscript=None,
            options=None, bufferProtocol=False, dynamicAttr=False,
            baseClasses=None, holder="default" ):
        if defines is None: defines = []
        if options is None: options = []
        if baseClasses is None: baseClasses = []
        if extraCMake is None: extraCMake = []
        if self.single:
            typeName = (typeName,)
            options = (options,)
            bufferProtocol = (bufferProtocol,)
            dynamicAttr = (dynamicAttr,)
            args = (args,)
            baseClasses = (baseClasses,)
            holder = (holder,)
        else:
            if len(args) == 0:
                args=((),)*2
            else:
                args = args[0]
            if holder == "default":
                holder = ("default",)*len(typeName)
        if len(options) == 0:
            options = ((),)*len(typeName)
        if len(baseClasses) == 0:
            baseClasses = ((),)*len(typeName)
        if not bufferProtocol:
            bufferProtocol = (False,)*len(typeName)
        if not dynamicAttr:
            dynamicAttr = (False,)*len(typeName)
        if isinstance(includes[0],tuple) or isinstance(includes[0],list):
            allIncludes = [item for sublist in includes for item in sublist]
            includes = includes[0]
        else:
            allIncludes = includes
        allIncludes = sorted(set(allIncludes))
        includes = sorted(set(includes))
        source  = self.pre(allIncludes, typeName[0], moduleName, defines, preamble)
        for nr, (tn, a, o, b, d, bc, h)  in enumerate( zip(typeName, args, options, bufferProtocol, dynamicAttr, baseClasses, holder) ):
            source += self.main(nr, includes, tn, *a, options=o,
                                bufferProtocol=b, dynamicAttr=d,
                                baseClasses=bc, holder=h)
        return self.post(moduleName, source, postscript, extraCMake)

def simpleGenerator(inc, baseType, namespace, pythonname=None, filename=None):
    generator = SimpleGenerator(baseType, namespace, pythonname, filename)
    def load(includes, typeName, *args):
        includes = includes + inc
        moduleName = namespace + "_" + baseType + "_" + hashIt(typeName)
        return generator.load(includes, typeName, moduleName, *args)
    return load
