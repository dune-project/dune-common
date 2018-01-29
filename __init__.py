import logging
import os

logger = logging.getLogger(__name__)

loglevel = None
try:
    loglevel = getattr(logging, os.environ['DUNE_LOG_LEVEL'].upper())
except KeyError:
    pass
except AttributeError:
    logger.warn('Invalid log level in environment variable DUNE_LOG_LEVEL')

logformat = os.environ.get('DUNE_LOG_FORMAT')

logging.basicConfig(format=logformat, level=loglevel)

try:
    from mpi4py import MPI
    logger.info('MPI initialized successfully')
except ImportError:
    logger.info('mpi4py not found, MPI not initialized')

from ._common import *
from ..grid._grid import CommunicationDirection, DataType, InterfaceType, Marker, OutputType, PartitionType, reader

import numpy
def fvgetitem(self,index):
    try:
        return self._getitem(index)
    except TypeError:
        return numpy.array(self,copy=False)[index]
finished = False
nr = 1
while not finished:
    try:
        cls = globals()["FieldVector_"+str(nr)]
        setattr(cls, "_getitem", cls.__getitem__)
        setattr(cls, "__getitem__", fvgetitem)
        nr += 1
    except KeyError:
        finished = True

def FieldVector(values):
    fv = "FieldVector_" + str(len(values))
    return globals()[fv](values)
def FieldMatrix(values):
    fm = "FieldMatrix_" + str(len(values)) + "_" + str(len(values[0]))
    return globals()[fm](values)

def _raise(exception):
    raise exception
