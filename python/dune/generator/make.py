# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import glob, os, sys
from concurrent.futures import ThreadPoolExecutor
import logging
import dune.common.module

from . import builder
from .exceptions import CompileError

logger = logging.getLogger(__name__)

dune_py_dir = dune.common.module.getDunePyDir()
generated_dir = os.path.join(dune_py_dir, 'python', 'dune', 'generated')

def makeGenerated(threads, modules = []):
    if len(modules) == 0:
        return

    moduleFiles = set()

    def rmJit(fileBase):
        print(f"make {fileBase}")
        try:
            builder.makeModule( fileBase )
        except CompileError:
            print(f"FAILED TO COMPILE {moduleBase}!")
        moduleFiles.update( [fileBase] )

    bases = set()
    if 'all' in modules:
        modules = ['']
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
        for i, base in enumerate(bases):
            executor.submit(rmJit, base)
