from __future__ import absolute_import, division, print_function, unicode_literals

import importlib
import shlex
import subprocess
import sys
import os
import timeit

try:
    from mpi4py import MPI
except:
    pass

from dune.common import comm
import dune.module
import dune.project


class Builder:
    class CompileError(Exception):
        '''raise this when there's a problem compiling an extension module'''
        def __init__(self, error):
            Exception.__init__(self,error)

    def __init__(self, force=False, verbose=None):
        self.force = force

        if verbose is None:
            try:
                verbose = os.environ['DUNE_PY_BUILDER_VERBOSE'].lower()
                if verbose in ['yes', 'true', '1']:
                    self.verbose = True
                elif verbose in ['no', 'false', '0']:
                    self.verbose = False
                else:
                    raise RuntineError('Invalid value for environment variable DUNE_PY_BUILDER_VERBOSE: "' + verbose + '".')
            except KeyError:
                self.verbose = False
        else:
            self.verbose = verbose

        self.build_args = dune.module.get_default_build_args()
        self.dune_py_dir = dune.module.get_dune_py_dir()
        self.generated_dir = os.path.join(self.dune_py_dir, 'python', 'dune', 'generated')
        #dune.__path__.append(os.path.join(self.dune_py_dir, 'python', 'dune'))
        dune.__path__.insert(0,os.path.join(self.dune_py_dir, 'python', 'dune'))

        if comm.rank == 0:
            if self.verbose:
                print("Building dune-py module...")
                start_time = timeit.default_timer()
            foundModule = dune.module.make_dune_py_module(self.dune_py_dir)
            try:
                output = dune.module.build_dune_py_module(self.dune_py_dir)
                if self.verbose:
                    print(output)
                    print("Building dune-py module took", (timeit.default_timer() - start_time), "seconds")
            except:
                if not foundModule:
                    raise
                else:
                    if self.verbose:
                        print("couldn't reconfigure - using existing configuration")
        comm.barrier()


    def load(self, moduleName, source, classType=None):
        if comm.rank == 0:
            if not os.path.isfile(os.path.join(self.generated_dir, moduleName + ".so")) or self.force:
                with open(os.path.join(self.generated_dir, "generated_module.hh"), 'w') as out:
                    out.write(source)

                if classType:
                    print('generating: ', classType)
                if self.verbose:
                    print("Compiling " + moduleName + "...")
                    start_time = timeit.default_timer()

                cmake_args = ["cmake", "--build", self.dune_py_dir, "--target", "generated_module"]
                if self.build_args is not None:
                    cmake_args += ['--'] + self.build_args
                cmake = subprocess.Popen([cmake_args, cwd=self.generated_dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                cmake.wait()
                if cmake.returncode > 0:
                    if sys.version_info.major == 2:
                        raise self.CompileError(cmake.stdout.read() + cmake.stderr.read())
                    else:
                        raise self.CompileError(cmake.stdout.read().decode('utf-8') + cmake.stderr.read().decode("utf-8"))

                os.rename(os.path.join(self.generated_dir, "generated_module.so"), os.path.join(self.generated_dir, moduleName + ".so"))
                os.rename(os.path.join(self.generated_dir, "generated_module.hh"), os.path.join(self.generated_dir, moduleName + ".hh"))

                if self.verbose:
                    print("Compilation took", (timeit.default_timer() - start_time), "seconds")

        comm.barrier()

        return importlib.import_module("dune.generated." + moduleName)
