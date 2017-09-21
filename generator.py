""" Generator module:

    The module provides the main class for on the fly generation of pybind11
    Python wrappers for implementations of a gives interface. The necessary
    details for each implementation (the C++ typedef and the includes) are
    provided by python dictonaries stored in files.
"""

from __future__ import absolute_import, division, print_function, unicode_literals

from . import builder

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

    def load(self, includes, typeName, moduleName, constructors=None, methods=None, bufferProtocol=False, options=None):
        source = '#include <config.h>\n\n'
        source += '#define USING_COREPY 1\n\n'
        source += ''.join(["#include <" + i + ">\n" for i in includes])
        source += '\n'
        source += '#include <dune/corepy/common/typeregistry.hh>\n'
        source += '#include <dune/corepy/pybind11/pybind11.h>\n'
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
        source += '  auto entry = Dune::CorePy::typeRegistry().insert<DuneType>("' + typeName + '",{' +\
                  ",".join(['"' + i + '"' for i in includes]) + "});\n"
        if options is None:
            options = ""
        else:
            options = ", " + options
        if not bufferProtocol:
            source += "  auto cls = pybind11::class_< DuneType " + options + " >( module, \"" + self.pythonName + "\" );\n"
        else:
            source += "  auto cls = pybind11::class_< DuneType " + options + " >( module, \"" + self.pythonName + "\", pybind11::buffer_protocol() );\n"
        source += "  Dune::CorePy::typeRegistry().exportToPython(cls,entry.first->second);\n"
        source += "  " + self.namespace + "register" + self.typeName + "( module, cls );\n"

        if constructors is not None:
            for constructor in constructors:
                if isinstance(constructor, list):
                    source += "  cls.def( \"__init__\", "
                    source += "\n    ".join(constructor)
                    source += " );\n"
                else:
                    source += "  cls.def( pybind11::init< " + constructor + " >() );\n"
        if methods is not None:
            source += "".join(["  cls.def( \"" + m[0] + "\", &" + m[1] + ");\n" for m in methods])

        source += "}\n"

        module = builder.load(moduleName, source, self.pythonName)
        return module
