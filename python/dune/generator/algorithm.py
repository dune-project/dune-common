# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import numpy

from dune.common.hashit import hashIt
from dune.common.utility import isString

def cppType(arg):
    try:
        t, i = arg.cppTypeName + " &", arg.cppIncludes
    except AttributeError:
        if isinstance(arg, bool):
            t, i = "bool", []
        elif isinstance(arg, int) or isinstance(arg,numpy.intc):
            t, i = "int", []
        elif isinstance(arg,numpy.int_):
            t, i = "long", []
        elif isinstance(arg,numpy.intp):
            t, i = "std::size_t", []
        elif isinstance(arg, float) or isinstance(arg,numpy.float_):
            t, i = "double", []
        elif isinstance(arg, numpy.ndarray):
            dtype = None
            if arg.dtype.type == numpy.intc:
                dtype="int"
            elif arg.dtype.type == numpy.int_:
                dtype="long"
            elif arg.dtype.type == numpy.intp:
                dtype="std::size_t"
            elif arg.dtype.type == numpy.float_:
                dtype="double"
            if dtype is None:
                t, i = "pybind11::array", ["dune/python/pybind11/numpy.h"]
            else:
                t, i = "pybind11::array_t<"+dtype+">", ["dune/python/pybind11/numpy.h"]
        elif isinstance(arg, str):
            t, i = "std::string", ["string"]
        elif callable(arg):
            t, i = "pybind11::function", ["dune/python/pybind11/pybind11.h"]
        elif isinstance(arg,tuple) or isinstance(arg,list):
            t, i = cppType(arg[0])
            t = "std::vector<"+t+">"
            i += ["vector"]
        else:
            raise Exception("Cannot deduce C++ type for the following argument: " + repr(arg))
    return t,i

def load(functionName, includes, *args):
    '''Just in time compile an algorithm.

    Generates binding for a single (template) function. The name of the
    function and the C++ types of the arguments passed to this function are
    used to generate a static type used in the bindings. The file(s)
    containing the code for the function are passed in either as single
    string or as a list of strings. Note that these files will be copied
    into the generated module. The file name can include a path name. So in
    the simples case `includes="header.hh" will include the file from the
    current working directory. To include a file from the directory
    containing the calling script use
    `includes=dune.generator.path(__file__)+"header.hh"`.

    Args:
        functionName:    name of the C++ function to provide bindings for
        includes:        single or list of files to add to the generated module
        *args:           list of arguments that will be passed to the generated module

    Returns:
        Callalble object
    '''

    # header guard is added further down
    source  = '#include <config.h>\n\n'
    source += '#define USING_DUNE_PYTHON 1\n\n'
    if isString(includes):
        with open(includes, "r") as include:
            source += include.read()
        source += "\n"
        includes = []
    elif hasattr(includes,"readable"): # for IOString
        with includes as include:
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
        t,i = cppType(arg)
        argTypes.append(t)
        includes += i

    signature = functionName + "( " + ", ".join(argTypes) + " )"
    moduleName = "algorithm_" + hashIt(signature) + "_" + hashIt(source)

    # add unique header guard with moduleName
    source = '#ifndef Guard_'+moduleName+'\n' + \
             '#define Guard_'+moduleName+'\n\n' + \
             source

    includes = sorted(set(includes))
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
    source += "#endif\n"

    # make sure to reload the builder here in case it got updated
    from . import builder
    return builder.load(moduleName, source, signature).run


def run(functionName, includes, *args):
    '''Just in time compile and run an algorithm.

    For details see the help for `dune.algorithm.load`.

    Args:
        functionName:    name of the C++ function to provide bindings for
        includes:        single or list of files to add to the generated module
        *args:           list of arguments that will be passed to the generated module

    Returns:
        return value of `functionName(*args)`
    '''
    return load(functionName, includes, *args)(*args)
