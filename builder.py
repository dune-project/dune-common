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
        #dune.__path__.append(os.path.join(self.dune_py_dir, 'python', 'dune'))
        dune.__path__.insert(0,os.path.join(self.dune_py_dir, 'python', 'dune'))

        if comm.rank == 0:
            logger.info('Started building dune-py module')
            foundModule = dune.common.module.make_dune_py_module(self.dune_py_dir)
            try:
                output = dune.common.module.build_dune_py_module(self.dune_py_dir)
                logger.info('Successfully built dune-py module')
            except:
                logger.exception('Failed to build dune-py module')
                if not foundModule:
                    raise
                else:
                    logger.warn('Could not reconfigure dune-py - using existing configuration')
        comm.barrier()


    def load(self, moduleName, source, classType=None):
        if comm.rank == 0:
            if not os.path.isfile(os.path.join(self.generated_dir, moduleName + ".so")) or self.force:
                with open(os.path.join(self.generated_dir, "generated_module.hh"), 'w') as out:
                    out.write(source)

                logger.info("Started compiling " + moduleName)

                cmake_args = ["cmake", "--build", self.dune_py_dir, "--target", "generated_module"]
                if self.build_args is not None:
                    cmake_args += ['--'] + self.build_args
                cmake = subprocess.Popen(cmake_args, cwd=self.generated_dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                stdout, stderr = cmake.communicate()
                logger.debug(buffer_to_str(stdout))
                if cmake.returncode > 0:
                    logger.error(buffer_to_str(stderr))
                    raise self.CompileError(buffer_to_str(stderr))

                os.rename(os.path.join(self.generated_dir, "generated_module.so"), os.path.join(self.generated_dir, moduleName + ".so"))
                os.rename(os.path.join(self.generated_dir, "generated_module.hh"), os.path.join(self.generated_dir, moduleName + ".hh"))

                logger.info('Successfully compiled ' + moduleName)

        comm.barrier()

        return importlib.import_module("dune.generated." + moduleName)
