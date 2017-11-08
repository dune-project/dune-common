from __future__ import absolute_import, division, print_function, unicode_literals

import numpy

from dune.common.hashit import hashIt
from . import builder

def load(functionName, includes, *args):
    source = '#include <config.h>\n\n'
    source += '#define USING_DUNE_PYTHON 1\n\n'
    if isinstance(includes, str):
        with open(includes, "r") as include:
            source += include.read()
        source += "\n"
        includes = []
    elif isinstance(includes, list):
        for includefile in includes:
            with open(includefile, "r") as include:
                source += include.read()
        source += "\n"
        includes = []

    argTypes = []
    for arg in args:
        try:
            t, i = arg._typeName + " &", arg._includes
        except AttributeError:
            if isinstance(arg, int) or isinstance(arg,numpy.int_):
                t, i = "int", []
            elif isinstance(arg, float) or isinstance(arg,numpy.float_):
                t, i = "double", []
            elif isinstance(arg, numpy.ndarray):
                dtype = None
                if arg.dtype.type == numpy.int_:
                    dtype="int"
                elif arg.dtype.type == numpy.float_:
                    dtype="double"
                if dtype is None:
                    t, i = "pybind11::array", ["dune/python/pybind11/numpy.h"]
                else:
                    t, i = "pybind11::array_t<"+dtype+">", ["dune/python/pybind11/numpy.h"]
            elif callable(arg):
                t, i = "pybind11::function", ["dune/python/pybind11/pybind11.h"]
            else:
                raise Exception("Cannot deduce C++ type for the following argument: " + repr(arg))
        argTypes.append(t)
        includes += i

    signature = functionName + "( " + ", ".join(argTypes) + " )"
    moduleName = "algorithm_" + hashIt(signature) + "_" + hashIt(source)

    source += "".join(["#include <" + i + ">\n" for i in includes])
    source += "\n"
    source += '#include <dune/python/common/typeregistry.hh>\n'
    source += '#include <dune/python/pybind11/pybind11.h>\n'
    source += '\n'

    source += "PYBIND11_MODULE( " + moduleName + ", module )\n"
    source += "{\n"

    source += "  module.def( \"run\", [] ( " + ", ".join([argTypes[i] + " arg" + str(i) for i in range(len(argTypes))]) + " ) {\n"
    source += "      return " + functionName + "( " + ", ".join(["arg" + str(i) for i in range(len(argTypes))]) + " );\n"
    source += "    } );\n"

    source += "}\n"

    return builder.load(moduleName, source, signature).run


def run(functionName, includes, *args):
    return load(functionName, includes, *args)(*args)
