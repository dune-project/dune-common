from __future__ import absolute_import, division, print_function, unicode_literals

import importlib
import logging
import shlex
import subprocess
import sys
import os

from dune.common import comm
import dune.common.module


logger = logging.getLogger(__name__)

def buffer_to_str(b):
    return b if sys.version_info.major == 2 else b.decode('utf-8')


class Builder:
    class CompileError(Exception):
        '''raise this when there's a problem compiling an extension module'''
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
                logger.info('Started building dune-py module')
                dune.common.module.make_dune_py_module(self.dune_py_dir)
                output = dune.common.module.build_dune_py_module(self.dune_py_dir)
                logger.info('Successfully built dune-py module')
            comm.barrier()
        else:
            logger.info('using pre configured dune-py module')


    def compile(self, source, target="generated_module" ):
        with open(os.path.join(self.generated_dir, "generated_module.hh"), 'w') as out:
            out.write(source)

        cmake_args = ["cmake", "--build", self.dune_py_dir, "--target", target]
        if self.build_args is not None:
            cmake_args += ['--'] + self.build_args
        cmake = subprocess.Popen(cmake_args, cwd=self.generated_dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = cmake.communicate()
        logger.debug(buffer_to_str(stdout))
        return cmake.returncode, stdout, stderr

    def load(self, moduleName, source, pythonName):
        if comm.rank == 0:
            if not os.path.isfile(os.path.join(self.generated_dir, moduleName + ".so")) or self.force:
                logger.info("Started compiling " + pythonName)
                returncode, stdout, stderr = self.compile(source)
                logger.debug(buffer_to_str(stdout))
                if returncode > 0:
                    logger.error(buffer_to_str(stderr))
                    raise self.CompileError(buffer_to_str(stderr))

                os.rename(os.path.join(self.generated_dir, "generated_module.so"), os.path.join(self.generated_dir, moduleName + ".so"))
                os.rename(os.path.join(self.generated_dir, "generated_module.hh"), os.path.join(self.generated_dir, moduleName + ".hh"))

                logger.info('Successfully compiled ' + pythonName)

        comm.barrier()

        module = importlib.import_module("dune.generated." + moduleName)
        if self.force:
            module = importlib.reload(module)
        return module
