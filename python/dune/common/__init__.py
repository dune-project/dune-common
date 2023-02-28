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
    logger.warn('Invalid log level in environment variable DUNE_LOG_LEVEL. Valid are {debug,info,warn,error}')

logformat = os.environ.get('DUNE_LOG_FORMAT', 'DUNE-%(levelname)s: %(message)s')

logging.basicConfig(format=logformat, level=loglevel)

cmakeFlags = getCMakeFlags()

if 'HAVE_MPI' in cmakeFlags and cmakeFlags['HAVE_MPI']:
    try:
        from mpi4py import MPI
        if MPI.COMM_WORLD.Get_rank() == 0:
            logger.debug('MPI initialized successfully')
    except ImportError:
        logger.debug('mpi4py not found but MPI used during configuration of dune-common')
        raise RuntimeError(textwrap.dedent("""
            The Dune modules were configured using MPI. For the Python bindings to work,
            the Python package 'mpi4py' is required.
            Please run
                pip install mpi4py
            before rerunning your Dune script.
        """))

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
