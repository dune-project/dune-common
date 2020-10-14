import importlib
import logging
import shlex
import subprocess
import os
import sys

try:
    from portalocker import Lock as _Lock
    from portalocker.constants import LOCK_EX, LOCK_SH
    class Lock(_Lock):
        def __init__(self, path, flags, *args, **kwargs):
            _Lock.__init__(self,path,*args,flags=flags,timeout=None,**kwargs)
except ModuleNotFoundError:
    import fcntl
    from fcntl import LOCK_EX, LOCK_SH
    # file locking from fcntl
    def lock_file(f, cmd=fcntl.LOCK_EX):
        fcntl.flock(f, cmd)
        return f
    def unlock_file(f):
        fcntl.flock(f, fcntl.LOCK_UN)
        return f

    # This file opener *must* be used in a "with" block.
    class Lock:
        # Open the file with arguments provided by user. Then acquire
        # a lock on that file object (WARNING: Advisory locking).
        def __init__(self, path, flags, *args, **kwargs):
            # Open the file and acquire a lock on the file before operating
            self.file = open(path, mode='w+', *args, **kwargs)
            # Lock the opened file
            self.file = lock_file(self.file, flags) # flags are either LOCK_EX or LOCK_SH

        # Return the opened file object (knowing a lock has been obtained).
        def __enter__(self, *args, **kwargs): return self.file

        # Unlock the file and close the file object.
        def __exit__(self, exc_type=None, exc_value=None, traceback=None):
            # Flush to make sure all buffered contents are written to file.
            self.file.flush()
            os.fsync(self.file.fileno())
            # Release the lock on the file.
            self.file = unlock_file(self.file)
            self.file.close()
            # Handle exceptions that may have come up during execution, by
            # default any exceptions are raised to the user.
            return exc_type == None

from dune.common import comm
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
        logger.debug(buffer_to_str(stdout))
        if cmake.returncode > 0:
            logger.error(buffer_to_str(stderr))
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
        if comm.rank == 0:
            module = sys.modules.get("dune.generated." + moduleName)
            if module is None:
                # make sure nothing (compilation, generating and building) is # taking place
                with Lock(os.path.join(self.dune_py_dir, 'lock-module.lock'), flags=LOCK_EX):
                    # module must be generated so lock the source file
                    with Lock(os.path.join(self.dune_py_dir, 'lock-'+moduleName+'.lock'), flags=LOCK_EX):
                        sourceFileName = os.path.join(self.generated_dir, moduleName + ".cc")
                        if not os.path.isfile(sourceFileName):
                            logger.info("Loading " + pythonName + " (new)")
                            code = str(source)
                            # the CMakeLists.txt needs changing and cmake rerun - lock folder
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
                # end of exclusive dune-py lock
            # for compilation a shared lock is enough
            with Lock(os.path.join(self.dune_py_dir, 'lock-module.lock'), flags=LOCK_SH):
                # lock generated module
                with Lock(os.path.join(self.dune_py_dir, 'lock-'+moduleName+'.lock'), flags=LOCK_EX):
                    self.compile(moduleName)

        comm.barrier()
        module = importlib.import_module("dune.generated." + moduleName)

        if self.force:
            logger.info("Reloading " + pythonName)
            module = reload_module(module)

        return module
