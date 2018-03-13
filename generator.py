""" Generator module:

    The module provides the main class for on the fly generation of pybind11
    Python wrappers for implementations of a gives interface. The necessary
    details for each implementation (the C++ typedef and the includes) are
    provided by python dictonaries stored in files.
"""

from __future__ import absolute_import, division, print_function, unicode_literals

import logging

from . import builder

logger = logging.getLogger(__name__)

class SimpleGenerator(object):
    def __init__(self, typeName, namespace, pythonname=None, filename=None):
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

    def load(self, includes, typeName, moduleName, *args, **kwargs):
        source = '#include <config.h>\n\n'
        source += '#define USING_DUNE_PYTHON 1\n\n'
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

        source += "typedef " + typeName + " DuneType;\n"
        source += "\n"
        if self.namespace == "":
            source += "void register" + self.typeName + "( ... ) {}\n"
        source += "PYBIND11_MODULE( " + moduleName + ", module )\n"
        source += "{\n"
        source += "  using pybind11::operator\"\"_a;\n"
        options = kwargs.get("options", [])
        if not kwargs.get("bufferProtocol", False):
            clsParams = []
        else:
            clsParams = ['pybind11::buffer_protocol()']
        if kwargs.get("dynamicAttr", False):
            clsParams += ['pybind11::dynamic_attr()']
        source += '  auto cls = Dune::Python::insertClass' +\
                     '< DuneType' + ', '.join([""]+options) + ' >' +\
                     '( module, "' + self.pythonName + '"' +\
                     ','.join(['']+clsParams) +\
                     ', Dune::Python::GenerateTypeName("' + typeName + '")' +\
                     ', Dune::Python::IncludeFiles{' + ','.join(['"' + i + '"' for i in includes]) + '}' +\
                     ").first;\n"
        source += "  " + self.namespace + "register" + self.typeName + "( module, cls );\n"

        for arg in args:
            if arg:
                source += "".join("  " + s + "\n" for s in str(arg).splitlines())

        source += "}\n"

        module = builder.load(moduleName, source, self.pythonName)
        # setattr(getattr(module,self.pythonName),"_module",module)
        return module

from dune.common.hashit import hashIt
def simpleGenerator(inc, baseType, namespace, pythonname=None, filename=None):
    generator = SimpleGenerator(baseType, namespace, pythonname, filename)
    def load(includes, typeName, *args):
        includes = includes + inc
        moduleName = namespace + "_" + baseType + "_" + hashIt(typeName)
        return generator.load(includes, typeName, moduleName, *args)
    return load

from . import Method as Method_
from . import Constructor as Constructor_
from dune.deprecate import deprecated
@deprecated("import from dune.generator directly")
class Method(Method_):
    pass
@deprecated("import from dune.generator directly")
class Constructor(Constructor_):
    pass
