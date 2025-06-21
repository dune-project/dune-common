# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

try:
    from mpi4py import MPI
except ImportError:
    MPI = None

import glob, os
from concurrent.futures import ThreadPoolExecutor
import logging
import dune.common.module

from . import builder
from .exceptions import CompileError

logger = logging.getLogger(__name__)

dune_py_dir = dune.common.module.getDunePyDir()
generated_dir = os.path.join(dune_py_dir, 'python', 'dune', 'generated')

def makeGenerated(modules, fileName=None, threads=4, force=False, verbose=False):
    if MPI is not None:
        comm = MPI.COMM_WORLD
        rank = comm.Get_rank()
    else:
        rank == 0

    if rank > 0:
        comm.barrier()
        return

    if len(modules) == 0 and fileName is None:
        return

    moduleFiles = set()

    def makeJit(fileBase):
        try:
            if verbose:
                    print(f"building {fileBase}")
            builder.makeModule( fileBase, force=force )
        except CompileError as e:
            print(f"Failed to compile {fileBase} - ignoring!",flush=True)
            if verbose:
                raise e
        moduleFiles.update( [fileBase] )

    bases = set()
    if 'all' in modules:
        modules = ['']
    if fileName is not None and not fileName == '':
        try:
            with open(fileName,'r') as f:
                for line in f:
                    modules += [line.rstrip()]
        except FileNotFoundError:
            print(f"file {fileName} not found - continuing")
    for m in modules:
        files = []
        for ext in ('.so', '.cc'):
            pattern = os.path.join(generated_dir, m+'*'+ext)
            files += glob.glob(pattern)
        if len(files) == 0:
            bases.add(m)
        else:
            bases.update( [os.path.splitext(os.path.basename(f))[0] for f in files] )

    with ThreadPoolExecutor(max_workers=threads) as executor:
        # using 'map' leads to exceptions being shown (submit does not)
        # but only if we try to access the results but
        # we don't need to do anything with the result
        for result in executor.map(makeJit, bases):
            pass
    comm.barrier()
