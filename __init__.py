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

import numpy
def fvgetitem(self,index):
    return numpy.array(self,copy=False)[index]
finished = False
nr = 1
while not finished:
    try:
        setattr(globals()["FieldVector_"+str(nr)], "__getitem__", fvgetitem)
        nr += 1
    except KeyError:
        finished = True

def FieldVector(values):
    fv = "FieldVector_" + str(len(values))
    return globals()[fv](values)

def _raise(exception):
    raise exception
