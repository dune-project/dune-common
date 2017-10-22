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

class Constructor(object):
    def __init__(self, args, body=None, extra=None):
        self.args = args
        self.body = body
        self.extra = [] if extra is None else extra

    def register(self, cls="cls"):
        if self.body is None:
            return cls + ".def( pybind11::init( " + args + " )" + "".join(", " + e for e in self.extra) + " );\n"
        if self.args:
            source = cls + ".def( pybind11::init( [] ( " + ", ".join(self.args) + " ) {"
        else:
            source = cls + ".def( pybind11::init( [] () {"
        source += "\n    ".join(self.body)
        source += "\n  } )" + "".join(", " + e for e in self.extra) + " );\n"
        return source

    def __str__(self):
        return self.register()


class Method(object):
    def __init__(self, name, args, body=None, extra=None):
        self.name = name
        self.args = args
        self.body = body
        self.extra = extra

    def register(self, cls="cls"):
        if self.body is None:
            return cls + ".def( " + self.name + ", " + args + " )" + "".join(", " + e for e in self.extra) + " );\n"
        if self.args:
            source = cls + ".def( " + self.name + ", [] ( " + ", ".join(self.args) + " ) {"
        else:
            source = cls + ".def( " + self.name + ", [] () {"
        source += "\n    ".join(self.body)
        source += "\n  } )" + "".join(", " + e for e in self.extra) + " );\n"
        return source

    def __str__(self):
        return self.register()


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
