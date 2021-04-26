import importlib
import logging
import shlex
import subprocess
import os
import sys
import shutil
import pkgutil
import re
import jinja2
import dune

from dune.packagemetadata import get_dune_py_dir
from dune.common import comm
from dune.common.locking import Lock, LOCK_EX,LOCK_SH
from dune.common.utility import buffer_to_str, isString, reload_module
from dune.common.module import select_modules, is_installed
from dune.generator.exceptions import CompileError, ConfigurationError
import dune.common.module

logger = logging.getLogger(__name__)

cxxFlags = None
noDepCheck = False

class Builder:
    def extract_metadata():
        """ Extract meta data that was exported by CMake.

        This returns a dictionary that maps package names to the data associated
        with the given metadata key. Currently the following metadata keys are
        exported by Python packages created with the Dune CMake build system:
        * MODULENAME: The name of the Dune module
        * BUILDDIR: The build directory of the Dune module
        * DEPS: The name of all the dependencies of the module
        * DEPBUILDDIRS: The build directories of the dependencies
        """
        result = {}

        for package in pkgutil.iter_modules(dune.__path__, dune.__name__ + "."):
            # Avoid the dune.create module - it cannot be imported unconditionally!
            if package.name == "dune.create":
                continue

            # Avoid the dune.utility module - it import dune.create
            if package.name == "dune.utility":
                continue

            # Check for the existence of the metadata.cmake file in the package
            mod = importlib.import_module(package.name)
            path, filename = os.path.split(mod.__file__)

            # Only consider sub-packages, not modules
            if filename != "__init__.py":
                continue

            metadata_file = os.path.join(path, "metadata.cmake")
            if os.path.exists(metadata_file):
                # If it exists parse the line that defines the key that we are looking for
                for line in open(metadata_file, "r"):
                    ## todo: issue is that it will split at final '=' which
                    ## is a problem with CXXFLAGS
                    # match = re.match(f"(.*)=(.*)", line)
                    # if match:
                    #     result.setdefault(package.name, {})
                    #     key, value = match.groups()
                    #     result[package.name][key] = value
                    try:
                        key, value = line.split("=",1)
                        result.setdefault(package.name, {})
                        result[package.name][key] = value.strip()
                    except ValueError: # no '=' in line
                        pass
        return result

    def dunepy_from_template(dunepy_dir):
        # Remove any remnants of an old dune-py installation
        if os.path.exists(dunepy_dir):
            shutil.rmtree(dunepy_dir)

        # Extract the raw data dictionary
        data = Builder.extract_metadata()

        # Define some data processing patterns
        def combine_across_modules(key):
            return list(m[key] for m in data.values())

        def zip_across_modules(key, value):
            result = {}
            for moddata in data.values():
                # todo: space is bad separator for list of paths - needs
                # fixing in cmake module generating the metadata file
                for k, v in zip(moddata[key].split(" "), moddata[value].split(";")):
                    # we don't store paths for module that have not been found (suggested)
                    # and we also skip the path if it is empty (packaged module)
                    if v.endswith("NOTFOUND") or v == "": continue
                    # make sure build directory (if found) is unique across modules
                    if k in result and not result[k] == v:
                        raise ValueError(f"build dir {v} for module {k} is expected to be unique across the given metadata")
                    result[k] = v
            return result

        def unique_value_across_modules(key, default=""):
            values = set(m[key] for m in data.values())
            if len(values) > 1:
                raise ValueError(f"Key {key} is expected to be unique across the given metadata")
            if len(values) == 0:
                return default
            value, = values
            return value

        modules    = combine_across_modules("MODULENAME")
        builddirs  = zip_across_modules("DEPS", "DEPBUILDDIRS")

        # todo: improve on this - just getting the idea how this could work
        cmakeflags = combine_across_modules("CMAKE_FLAGS")
        cmakeFlags = {}
        for x in cmakeflags:
            for y in x.split(";"):
                try:
                    k,v = y.split(":=",1)
                    cmakeFlags[k] = v.strip()
                except ValueError: # no '=' in line
                    pass
        # print(cmakeFlags) # not used yet

        # add dune modules which where available during the build of a
        # python module but don't provide their own python module
        for k,v in builddirs.items():
            if k not in modules and not v.endswith("NOTFOUND"):
                modules += [k]

        # Gather and reorganize meta data context that is used to write dune-py
        context = {}
        context["modules"]        = modules
        context["builddirs"]      = builddirs
        context["install_prefix"] = unique_value_across_modules("INSTALL_PREFIX")
        context["cmake_flags"]    = cmakeFlags

        # Find the correct template path
        path, _ = os.path.split(__file__)
        template_path = os.path.join(path, "template")

        # Run the template through Jinja2
        env = jinja2.Environment(
            loader=jinja2.FileSystemLoader(template_path),
            keep_trailing_newline=True,
        )
        for root, dirs, files in os.walk(template_path):
            for template_file in files:
                full_template_file = os.path.join(root, template_file)
                relative_template_file = os.path.relpath(full_template_file, start=template_path)
                gen_file = os.path.join(dunepy_dir, relative_template_file.split(".template")[0])
                os.makedirs(os.path.split(gen_file)[0], exist_ok=True)
                with open(gen_file, "w") as outfile:
                    outfile.write(env.get_template(relative_template_file).render(**context))


    def __init__(self, force=False, saveOutput=False):
        self.force = force
        if saveOutput is True or saveOutput.lower() == "write":
            self.savedOutput = [open("generatorCompiler.out","w+"), open("generatorCompiler.err","w+")]
        elif saveOutput.lower() == "append":
            self.savedOutput = [open("generatorCompiler.out","a+"), open("generatorCompiler.err","a+")]
        elif saveOutput.lower() == "console" or saveOutput.lower() == "terminal":
            self.savedOutput = [sys.stdout, sys.stderr]
        else:
            self.savedOutput = None
        self.initialized = False

    def initialize(self):
        self.dune_py_dir = dune.common.module.get_dune_py_dir()
        os.makedirs(self.dune_py_dir, exist_ok=True)

        if comm.rank == 0:
            # Trigger the generation of dune-py
            from dune.common.locking import Lock, LOCK_EX,LOCK_SH
            dunepy = get_dune_py_dir()
            tagfile = os.path.join(dunepy, ".noconfigure")
            if os.path.exists(dunepy):
                # check for existence of .noconfigure tag file
                with Lock(os.path.join(dunepy, 'lock-module.lock'), flags=LOCK_EX):
                    if not os.path.isfile(tagfile):
                        generateDunePy = True
                    else:
                        generateDunePy = False
                        logger.debug('Using existing dune-py module in'+dunepy)
            else:
                generateDunePy = True
            if generateDunePy:
                Builder.dunepy_from_template(get_dune_py_dir())
                subprocess.check_call("cmake .".split(), cwd=get_dune_py_dir())
                open(tagfile, 'a').close()
        comm.barrier()

        self.build_args = dune.common.module.get_default_build_args()
        self.generated_dir = os.path.join(self.dune_py_dir, 'python', 'dune', 'generated')
        try:
            dune.__path__._path.insert(0,os.path.join(self.dune_py_dir, 'python', 'dune'))
        except:
            dune.__path__.insert(0,os.path.join(self.dune_py_dir, 'python', 'dune'))
        initialized = True

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
        ## TODO replace if rank with something better
        ## and remove barrier further down
        if not self.initialized:
            self.initialize()
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
                            assert os.path.isfile(sourceFileName), "Error in writing module .cc file"
                            if not found:
                                origPos = -1
                                with open(os.path.join(self.generated_dir, "CMakeLists.txt"), 'a') as out:
                                    # store original file size
                                    origPos = out.tell()
                                    out.write(line+"\n")
                                # update build system
                                logger.debug("Rebuilding module")
                                try:
                                    self.compile()
                                except: # all exceptions will cause a problem here
                                    os.remove(os.path.join(sourceFileName))
                                    # remove line from CMakeLists to avoid problems
                                    with open(os.path.join(self.generated_dir, "CMakeLists.txt"), 'a') as out:
                                        out.truncate(origPos)
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
