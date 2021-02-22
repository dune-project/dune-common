import importlib
import logging
import shlex
import subprocess
import os
import sys

from dune.common import comm
from dune.common.locking import Lock, LOCK_EX,LOCK_SH
from dune.common.utility import buffer_to_str, isString, reload_module
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
            # lock the whole dune-py module exclusively to possibly
            # generate and then build the module
            with Lock(os.path.join(self.dune_py_dir, 'lock-module.lock'), flags=LOCK_EX):
                dune.common.module.make_dune_py_module(self.dune_py_dir)
                tagfile = os.path.join(self.dune_py_dir, ".noconfigure")
                if not os.path.isfile(tagfile):
                    dune.common.module.build_dune_py_module(self.dune_py_dir)
                    # create .noconfigure to disable configuration for future calls
                    open(tagfile, 'a').close()
                else:
                    logger.debug('Using pre configured dune-py module')
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
        elif saveOutput.lower() == "console" or saveOutput.lower() == "terminal":
            self.savedOutput = [sys.stdout, sys.stderr]
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
        logger.debug("Compiler output: "+buffer_to_str(stdout))
        if cmake.returncode > 0:
            raise CompileError(buffer_to_str(stderr))
        if self.savedOutput is not None:
            out = buffer_to_str(stdout)
            nlines = out.count('\n')
            if nlines > 1:
                self.savedOutput[0].write("###############################\n")
                self.savedOutput[0].write("###" + " ".join(cmake_args)+"\n")
            if nlines > 0:
                self.savedOutput[0].write(out)
            if nlines > 1:
                self.savedOutput[0].write("\n###############################\n")

            err = buffer_to_str(stderr)
            nlines = err.count('\n')
            if nlines > 1:
                self.savedOutput[1].write("###############################\n")
                self.savedOutput[1].write("###" + " ".join(cmake_args)+"\n")
            if nlines > 0:
                self.savedOutput[1].write(err)
            if nlines > 1:
                self.savedOutput[1].write("\n###############################\n")

    def load(self, moduleName, source, pythonName):

        ## TODO replace if rank if something better
        ## and remove barrier further down
        if comm.rank == 0:
            module = sys.modules.get("dune.generated." + moduleName)
            if module is None:
                # make sure nothing (compilation, generating and building) is # taking place
                with Lock(os.path.join(self.dune_py_dir, 'lock-module.lock'), flags=LOCK_EX):
                    # module must be generated so lock the source file
                    with Lock(os.path.join(self.dune_py_dir, 'lock-'+moduleName+'.lock'), flags=LOCK_EX):
                        sourceFileName = os.path.join(self.generated_dir, moduleName + ".cc")
                        line = "dune_add_pybind11_module(NAME " + moduleName + " EXCLUDE_FROM_ALL)"
                        # first check if this line is already present in the CMakeLists file
                        # (possible if a previous script was stopped by user before module was compiled)
                        with open(os.path.join(self.generated_dir, "CMakeLists.txt"), 'r') as out:
                            found = line in out.read()
                        if not os.path.isfile(sourceFileName) or not found:
                            logger.info("Compiling " + pythonName)
                            code = str(source)
                            with open(os.path.join(sourceFileName), 'w') as out:
                                out.write(code)
                            if not found:
                                with open(os.path.join(self.generated_dir, "CMakeLists.txt"), 'a') as out:
                                    out.write(line+"\n")
                                # update build system
                                logger.debug("Rebuilding module")
                                try:
                                    self.compile()
                                except KeyboardInterrupt:
                                    os.remove(os.path.join(sourceFileName))
                                    raise
                        elif isString(source) and not source == open(os.path.join(sourceFileName), 'r').read():
                            logger.info("Compiling " + pythonName + " (updated)")
                            code = str(source)
                            with open(os.path.join(sourceFileName), 'w') as out:
                                out.write(code)
                        else:
                            logger.debug("Loading " + pythonName)
                            line = "dune_add_pybind11_module(NAME " + moduleName + " EXCLUDE_FROM_ALL)"
                            # the CMakeLists file should already include this line
                            with open(os.path.join(self.generated_dir, "CMakeLists.txt"), 'r') as out:
                                found = line in out.read()
                            assert found, "CMakeLists file does not contain an entry to build"+moduleName
                # end of exclusive dune-py lock

                # for compilation a shared lock is enough
                with Lock(os.path.join(self.dune_py_dir, 'lock-module.lock'), flags=LOCK_SH):
                    # lock generated module
                    with Lock(os.path.join(self.dune_py_dir, 'lock-'+moduleName+'.lock'), flags=LOCK_EX):
                        logger.debug("Now compiling "+moduleName)
                        self.compile(moduleName)
            ## end if module is not None

        ## TODO remove barrier here
        comm.barrier()
        module = importlib.import_module("dune.generated." + moduleName)

        if self.force:
            logger.info("Reloading " + pythonName)
            module = reload_module(module)

        return module
