from __future__ import absolute_import, division, print_function, unicode_literals

import importlib
import logging
import shlex
import subprocess
import os

from dune.common import comm
from dune.common.compatibility import buffer_to_str, reload_module
import dune.common.module

logger = logging.getLogger(__name__)

class Builder:
    class CompileError(Exception):
        '''raise this when there's a problem compiling an extension module'''
        def __init__(self, error):
            Exception.__init__(self,error)
    class ConfigurationError(Exception):
        '''raise this when there's a problem with the configuration of dune-py'''
        def __init__(self, error):
            Exception.__init__(self,error)

    def __init__(self, force=False):
        self.force = force

        self.build_args = dune.common.module.get_default_build_args()
        self.dune_py_dir = dune.common.module.get_dune_py_dir()
        self.generated_dir = os.path.join(self.dune_py_dir, 'python', 'dune', 'generated')
        try:
            dune.__path__._path.insert(0,os.path.join(self.dune_py_dir, 'python', 'dune'))
        except:
            dune.__path__.insert(0,os.path.join(self.dune_py_dir, 'python', 'dune'))

        tagfile = os.path.join(self.dune_py_dir, ".noconfigure")
        if not os.path.isfile(tagfile):
            if comm.rank == 0:
                dune.common.module.make_dune_py_module(self.dune_py_dir)
                output = dune.common.module.build_dune_py_module(self.dune_py_dir)
            comm.barrier()
        else:
            logger.info('using pre configured dune-py module')

    def compile(self, target='all'):
        cmake_args = ["cmake", "--build", self.dune_py_dir, "--target", target]
        if self.build_args is not None:
            cmake_args += ['--'] + self.build_args
        cmake = subprocess.Popen(cmake_args, cwd=self.generated_dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = cmake.communicate()
        logger.debug(buffer_to_str(stdout))
        if cmake.returncode > 0:
            logger.error(buffer_to_str(stderr))
            raise self.CompileError(buffer_to_str(stderr))

    def load(self, moduleName, source, pythonName):
        if comm.rank == 0:
            logger.info("Loading " + pythonName)
            sourceFileName = os.path.join(self.generated_dir, moduleName + ".cc")
            if not os.path.isfile(sourceFileName):
                with open(os.path.join(sourceFileName), 'w') as out:
                    out.write(str(source))
                with open(os.path.join(self.generated_dir, "CMakeLists.txt"), 'a') as out:
                    out.write("dune_add_pybind11_module(NAME " + moduleName + " EXCLUDE_FROM_ALL)\n")
                # update build system
                self.compile()

            self.compile(moduleName)

        comm.barrier()

        module = importlib.import_module("dune.generated." + moduleName)
        if self.force:
            module = reload_module(module)
        return module
