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

    def load(self, includes, typeName, moduleName, constructors=None, methods=None, metaclass=None, bufferProtocol=False, options=None):
        source = "".join(["#include <" + i + ">\n" for i in includes])
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
        source += "  using pybind11::operator\"\"_a;\n"
        source += "  pybind11::module module( \"" + moduleName + "\" );\n"
        source += '  auto entry = Dune::CorePy::typeRegistry().insert<DuneType>("' + typeName + '",{' +\
                  ",".join(['"' + i + '"' for i in includes]) + "});\n"

        classTArgs = ['DuneType']
        if options is not None:
            classTArgs += options

        classArgs = ['module', '"' + self.pythonName + '"']
        if metaclass is not None:
            classArgs.append('pybind11::metaclass( ' + metaclass + ' )')
        if bufferProtocol:
            classArgs.append('pybind11::buffer_protocol()')

        source += '  auto cls = pybind11::class_< ' + ', '.join(classTArgs) + ' >( ' + ', '.join(classArgs) + ' );\n'
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

        source += "  return module.ptr();\n"
        source += "}\n"

        module = builder.load(moduleName, source, self.pythonName)
        return module
