# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import logging
import textwrap
from dune.packagemetadata import getCMakeFlags
cmakeFlags = getCMakeFlags()
logger = logging.getLogger(__name__)

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
    withMPI = True
else:
    withMPI = False
