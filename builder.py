from __future__ import absolute_import, division, print_function, unicode_literals

import importlib
import subprocess
import sys
import os
import timeit

from dune import comm
from dune import __path__ as basePaths

class Builder:
    class CompileError(Exception):
        '''raise this when there's a problem compiling an extension module'''
        def __init__(self, error):
            Exception.__init__(self,error)

    def __init__(self, force=False, verbose=False):
        self.force = force
        self.verbose = verbose

        self.path = os.path.join(basePaths[0], "generated")
        sys.path.append(self.path)

    def load(self, moduleName, source):
        if comm.rank == 0:
            if not os.path.isfile(os.path.join(self.path, moduleName + ".so")) or self.force:
                with open(os.path.join(self.path, "generated_module.hh"), 'w') as out:
                    out.write(source)

                if self.verbose:
                    print("Compiling " + moduleName + "...")
                    start_time = timeit.default_timer()

                cmake = subprocess.Popen(["cmake", "--build", "../../..", "--target", "generated_module"], cwd=self.path, stderr=subprocess.PIPE)
                cmake.wait()
                if cmake.returncode > 0:
                    raise self.CompileError(cmake.stderr.read())

                os.rename(os.path.join(self.path, "generated_module.so"), os.path.join(self.path, moduleName + ".so"))
                os.rename(os.path.join(self.path, "generated_module.hh"), os.path.join(self.path, moduleName + ".hh"))

                if self.verbose:
                    print("Compilation took", (timeit.default_timer() - start_time), "seconds")

        comm.barrier()

        return importlib.import_module("dune.generated." + moduleName)
