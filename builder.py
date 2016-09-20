from __future__ import absolute_import, division, print_function, unicode_literals

import importlib
import subprocess
import sys
import os
import timeit

from dune import comm, pathToGenerated

class Builder:
    class CompileError(Exception):
        '''raise this when there's a problem compiling an extension module'''
        def __init__(self, error):
            Exception.__init__(self,error)

    def __init__(self, force=False, verbose=False):
        self.force = force
        self.verbose = verbose

        self.path = pathToGenerated

    def load(self, moduleName, source):
        if comm.rank == 0:
            if not os.path.isfile(os.path.join(self.path, moduleName + ".so")) or self.force:
                with open(os.path.join(self.path, "generated_module.hh"), 'w') as out:
                    out.write(source)

                if self.verbose:
                    print("Compiling " + moduleName + "...")
                    start_time = timeit.default_timer()

                cmake = subprocess.Popen(["cmake", "--build", pathToGenerated + "/../../..", "--target", "generated_module"], cwd=self.path, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                cmake.wait()
                if cmake.returncode > 0:
                    if sys.version_info.major == 2:
                        raise self.CompileError(cmake.stdout.read() + cmake.stderr.read())
                    else:
                        raise self.CompileError(cmake.stdout.read().decode('utf-8') + cmake.stderr.read().decode("utf-8"))

                os.rename(os.path.join(self.path, "generated_module.so"), os.path.join(self.path, moduleName + ".so"))
                os.rename(os.path.join(self.path, "generated_module.hh"), os.path.join(self.path, moduleName + ".hh"))

                if self.verbose:
                    print("Compilation took", (timeit.default_timer() - start_time), "seconds")

        comm.barrier()

        return importlib.import_module("dune.generated." + moduleName)
