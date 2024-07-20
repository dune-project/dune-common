# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import logging
import numpy as np
import os
import textwrap
from dune.packagemetadata import getCMakeFlags

logger = logging.getLogger(__name__)

# default log level is INFO
loglevel = logging.INFO
try:
    loglevel = getattr(logging, os.environ['DUNE_LOG_LEVEL'].upper())
except KeyError:
    pass
except AttributeError:
    logger.warn('Invalid log level in environment variable DUNE_LOG_LEVEL.  Valid are {debug,info,warning,error}')

logformat = os.environ.get('DUNE_LOG_FORMAT', 'DUNE-%(levelname)s: %(message)s')

logging.basicConfig(format=logformat, level=loglevel)

cmakeFlags = getCMakeFlags()

try:
    from mpi4py import MPI
    if MPI.COMM_WORLD.Get_rank() >= 0:
        logger.debug('MPI initialized successfully using mpi4py')
except ImportError:
    if 'HAVE_MPI' in cmakeFlags and cmakeFlags['HAVE_MPI']:
        logger.debug('mpi4py not found but MPI used during configuration of dune-common')
        raise RuntimeError(textwrap.dedent("""
            The Dune modules were configured using MPI. For the Python bindings to work,
            the Python package 'mpi4py' is required.
            Please run
                pip install mpi4py
            before rerunning your Dune script.
        """))
    else:
        logger.debug('dune not configured with mpi')

from ._common import *
from .deprecated import DeprecatedObject

def _fieldVectorGetItem(self,index):
    try:
        return self._getitem(index)
    except TypeError:
        return np.array(self, copy=False)[index]


def _initializeFieldVector():
    finished = False
    nr = 1
    while not finished:
        try:
            cls = globals()["FieldVector_" + str(nr)]
            setattr(cls, "_getitem", cls.__getitem__)
            setattr(cls, "__getitem__", _fieldVectorGetItem)
            nr += 1
        except KeyError:
            finished = True

_initializeFieldVector()


def _loadVec(includes ,typeName ,constructors=None, methods=None):
    from dune.generator.generator import SimpleGenerator
    from dune.common.hashit import hashIt
    generator = SimpleGenerator("FieldVector", "Dune::Python")
    includes = includes + ["dune/python/common/fvector.hh"]
    typeHash = "fieldvector_" + hashIt(typeName)
    return generator.load(
        includes, typeName, typeHash,
        constructors, methods, bufferProtocol=True
    )


def FieldVector(values):
    """Construct a new FieldVector"""

    values = list(values)
    fv = "FieldVector_" + str(len(values))
    try:
        return globals()[fv](values)
    except KeyError:
        try:
            # try to import pre-compiled version from _common
            from importlib import import_module
            cls = getattr(import_module('dune.common'), 'FieldVector_double_'+str(len(values)))
        except (ImportError,AttributeError):
            # otherwise create module
            typeName = "Dune::FieldVector< double ," + str(len(values)) + " >"
            includes = []
            cls = _loadVec(includes, typeName).FieldVector

        setattr(cls, "_getitem", cls.__getitem__)
        setattr(cls, "__getitem__", _fieldVectorGetItem)
        globals().update({fv: cls})
    return globals()[fv](values)

# implementation needs to be completed similar to the FV above
# def FieldMatrix(values):
#     fm = "FieldMatrix_" + str(len(values)) + "_" + str(len(values[0]))
#     return globals()[fm](values)


def _cppTypesFromTuple(tup,tupleNameWrapper,allowByReference):
    from dune.generator.algorithm import cppType
    """
    Converts Python types into C++ types for TupleVector.

    Args:
        tup (tuple): A tuple representing the Python types.
        allowByReference (bool): Indicates whether the deduced type of C++ objects should be a (mutable) reference.

    Returns:
        str: The C++ type string representing the TupleVector.
        list: A list of includes needed for the C++ types.
    """
    typeName= tupleNameWrapper+"<"
    includes=[]
    for arg in tup:
        ti,i = cppType(arg)
        includes+= i
        if not allowByReference:
             ti=ti.rstrip("&")  #  remove trailing reference from c++ type
        typeName+= ti+ ","
    typeName = typeName.rstrip(",") + " >"
    return typeName,includes


def TupleVector(*args, allowByReference=False):
    from dune.common.hashit import hashIt
    """
    Creates a TupleVector object.

    This function creates a TupleVector object based on the provided arguments.

    Args:
        *args: Variable-length argument list representing the types of TupleVector.
            You can pass several objects or pass in a single tuple of objects.
        allowByReference (bool, optional): Indicates whether to allow storing reference to C++ objects inside the TupleVector.

    Returns:
        TupleVector: A TupleVector object.
    """
    includes = []
    typeName= ""

    if len(args)==1 and isinstance(args,tuple):
        typeName,includes=  _cppTypesFromTuple(*args,"Dune::TupleVector",allowByReference)
        stdTupleType,_=  _cppTypesFromTuple(*args,"std::tuple",allowByReference)
    else:
        typeName,includes=  _cppTypesFromTuple(args,"Dune::TupleVector",allowByReference)
        stdTupleType,_=  _cppTypesFromTuple(args,"std::tuple",allowByReference)

    includes+= ["dune/python/common/tuplevector.hh"]
    typeHash = "tuplevector_" + hashIt(typeName)
    from dune.generator.generator import SimpleGenerator
    generatorMTBV =SimpleGenerator("TupleVector","Dune::Python")
    if len(args)==1 and isinstance(args,tuple):
        return generatorMTBV.load(includes ,typeName ,typeHash, baseClasses=[stdTupleType] ).TupleVector(*args)
    else:
        return generatorMTBV.load(includes ,typeName ,typeHash, baseClasses=[stdTupleType] ).TupleVector(args)
