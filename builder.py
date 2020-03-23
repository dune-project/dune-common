from __future__ import absolute_import, division, print_function, unicode_literals

import importlib
import logging
import shlex
import subprocess
import os
import sys
from portalocker import Lock
from portalocker.constants import LOCK_EX, LOCK_SH

from dune.common import comm
from dune.common.compatibility import buffer_to_str, isString, reload_module
from dune.generator.exceptions import CompileError, ConfigurationError
import dune.common.module

logger = logging.getLogger(__name__)
cxxFlags = None
noDepCheck = False

class Builder:
    def __init__(self, force=False, saveOutput=False):
        self.force = force

        self.dune_py_dir = dune.common.module.get_dune_py_dir()
        os.makedirs(self.dune_py_dir, exist_ok=True)

        if comm.rank == 0:
            with Lock(os.path.join(self.dune_py_dir, 'lock-module.lock'), flags=LOCK_EX):
                dune.common.module.make_dune_py_module(self.dune_py_dir)
                tagfile = os.path.join(self.dune_py_dir, ".noconfigure")
                if not os.path.isfile(tagfile):
                    dune.common.module.build_dune_py_module(self.dune_py_dir)
                else:
                    logger.info('using pre configured dune-py module')
        comm.barrier()

        self.build_args = dune.common.module.get_default_build_args()
        self.generated_dir = os.path.join(self.dune_py_dir, 'python', 'dune', 'generated')
        try:
            dune.__path__._path.insert(0,os.path.join(self.dune_py_dir, 'python', 'dune'))
        except:
            dune.__path__.insert(0,os.path.join(self.dune_py_dir, 'python', 'dune'))

        if saveOutput is True or saveOutput.lower() == "write":
            self.savedOutput = [open("generatorCompiler.out","w+"), open("generatorCompiler.err","w+")]
        elif saveOutput.lower() == "append":
            self.savedOutput = [open("generatorCompiler.out","a+"), open("generatorCompiler.err","a+")]
        else:
            self.savedOutput = None

    def compile(self, target='all'):
        cmake_command = dune.common.module.get_cmake_command()
        cmake_args = [cmake_command, "--build", self.dune_py_dir, "--target", target]
        make_args = []
        if self.build_args is not None:
            make_args += self.build_args
        if cxxFlags is not None:
            make_args += ['CXXFLAGS='+cxxFlags]
        if noDepCheck:
            make_args += ["-B"]

        if cmake_args != []:
            cmake_args += ["--"] + make_args
        cmake = subprocess.Popen(cmake_args, cwd=self.generated_dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = cmake.communicate()
        logger.debug(buffer_to_str(stdout))
        if cmake.returncode > 0:
            logger.error(buffer_to_str(stderr))
            raise CompileError(buffer_to_str(stderr))
        if self.savedOutput is not None:
            self.savedOutput[0].write("###############################\n")
            self.savedOutput[0].write("###" + " ".join(cmake_args)+"\n")
            self.savedOutput[0].write(buffer_to_str(stdout))
            self.savedOutput[0].write("\n###############################\n")
            err = buffer_to_str(stderr)
            if err is "":
                self.savedOutput[1].write("###############################\n")
                self.savedOutput[1].write("###" + " ".join(cmake_args)+"\n")
                self.savedOutput[1].write(err)
                self.savedOutput[1].write("\n###############################\n")

    def load(self, moduleName, source, pythonName):
        module = sys.modules.get("dune.generated." + moduleName)
        if module is None:
            if comm.rank == 0:
                # lock the source file
                with Lock(os.path.join(self.dune_py_dir, 'lock-'+moduleName+'-source.lock'), flags=LOCK_EX):
                    sourceFileName = os.path.join(self.generated_dir, moduleName + ".cc")
                    if not os.path.isfile(sourceFileName):
                        logger.info("Loading " + pythonName + " (new)")
                        code = str(source)
                        # the CMakeLists.txt needs changing and cmake rerun - lock folder
                        with Lock(os.path.join(self.dune_py_dir, 'lock-all.lock'), flags=LOCK_EX):
                            with open(os.path.join(sourceFileName), 'w') as out:
                                out.write(code)
                            line = "dune_add_pybind11_module(NAME " + moduleName + " EXCLUDE_FROM_ALL)"
                            # first check if trhis line is already present in the CMakeLists file
                            # (possible if a previous script was stopped by user before module was compiled)
                            with open(os.path.join(self.generated_dir, "CMakeLists.txt"), 'r') as out:
                                found = line in out.read()
                            if not found:
                                with open(os.path.join(self.generated_dir, "CMakeLists.txt"), 'a') as out:
                                    out.write(line+"\n")
                                # update build system
                                self.compile()
                    elif isString(source) and not source == open(os.path.join(sourceFileName), 'r').read():
                        logger.info("Loading " + pythonName + " (updated)")
                        code = str(source)
                        with open(os.path.join(sourceFileName), 'w') as out:
                            out.write(code)
                    else:
                        logger.info("Loading " + pythonName)

                # lock generated module and make sure the folder isn't
                # locked due to CMakeLists.txt changed being made
                with Lock(os.path.join(self.dune_py_dir, 'lock-'+moduleName+'.lock'), flags=LOCK_EX):
                    with Lock(os.path.join(self.dune_py_dir, 'lock-all.lock'), flags=LOCK_SH):
                        self.compile(moduleName)

            comm.barrier()
            module = importlib.import_module("dune.generated." + moduleName)

        if self.force:
            logger.info("Reloading " + pythonName)
            module = reload_module(module)
        return module
