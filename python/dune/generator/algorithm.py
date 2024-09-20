# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import numpy

from dune.common.hashit import hashIt
from dune.common.utility import isString

class CPPType:
    def __init__(self,typeName,includes=[]):
        self.cppTypeName = typeName
        self.cppIncludes = includes

def cppType(arg):
    """
    Determine the C++ type and header includes corresponding to a Python object.

    Args:
        arg: The Python object for which the C++ type needs to be determined.

    Returns:
        tuple: A tuple containing the determined C++ type and the necessary C++ include files as a list.

    Raises:
        Exception: If the C++ type for the given argument cannot be deduced.

    Notes:
        This function determines the corresponding C++ type for a given Python object.

        For numpy.ndarray objects, the function determines the corresponding C++ type based on the data type of the array elements.
        If the array contains elements of dtype other than int, long, std::size_t, or double, it's treated as a generic pybind11::array_t.

        For tuples/lists, if all elements have the same type, they are converted to a std::vector of that type. If the elements have different types, they are converted to a std::tuple.

    """
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
        elif isinstance(arg, float) or isinstance(arg,numpy.float64):
            t, i = "double", []
        elif isinstance(arg, numpy.ndarray):
            dtype = None
            if arg.dtype.type == numpy.intc:
                dtype="int"
            elif arg.dtype.type == numpy.int_:
                dtype="long"
            elif arg.dtype.type == numpy.intp:
                dtype="std::size_t"
            elif arg.dtype.type == numpy.float64:
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
            if len(arg) > 1 and cppType(arg[1])[0]!=t: # check if the second element has the same type, if not we create an std::tuple
                t = "std::tuple<" + t
                for a in arg[1:]:
                    tt, ii = cppType(a)
                    t += ", " + tt
                    i += ii
                i+=["tuple"]
                t += ">"
                return t, i
            t = "std::vector<"+t+">"
            i += ["vector"]
        else:
            raise Exception("Cannot deduce C++ type for the following argument: " + repr(arg))
    return t,i

def load(functionName, includes, *args, pythonName=None):
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

    The generated file will be compiled within the `dune-py` module and
    thus all include search paths and libraries are added to the command
    line. Currently we do not provide a direct way to add additional
    external packages not found during the build of the Dune modules. A way
    around this is to set the `CPATH` and `LIBRARY_PATH` environment
    variables before calling this function.

    Examples
    --------

      Assume that you want to use some `Eigen` linear algebra and would like
      to pass a `numpy` array to a function taking an `Eigen::VectorXd`. By
      default a `numpy` array will be export to C++ using `pybind11::array`
      although `pybind11` can also export this as `Eigen::VectorXd`.
      In addition `Dune` might not have been configured to find `Eigen` if
      it not in a default position in the system. In this case the
      following code will work
      ```
      code="""
      #include <dune/python/pybind11/eigen.h>
      auto test(ColVec_t& x_1) {...}
      """

      os.environ["CPATH"] = "PathToEigen"
      testAlgo = dune.generator.algorithm.load('test', io.StringIO(code),
                      dune.generator.algorithm.CPPType("Eigen::VectorXd") )

      ```

    Args:
        functionName:    name of the C++ function to provide bindings for
        includes:        single or list of files to add to the generated module
        *args:           list of arguments that will be passed to the generated module.
                         Entries here can be instances of standard Python types,
                         other Dune objects, or instances of the special class
                         `algorithm.CPPType` which is constructed by
                         providing a string with the required C++ type and
                         optionally a list of include files to add at the
                         top of the generated file.

                         Note: the type of the object passed to C++ must be
                               exportable.

    Kwargs:
        pythonName:      A readable name for the generated function that is used in
                         diagnostic messages. If this is not provided, the function
                         signature is used.

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

    # Use signature as python name if none was expicitely provided
    if not pythonName:
      pythonName = signature

    # make sure to reload the builder here in case it got updated
    from . import builder
    return builder.load(moduleName, source, pythonName).run


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
