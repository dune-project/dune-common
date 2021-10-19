import importlib
import logging
import shlex
import subprocess
import os
import sys
import shutil
import re
import jinja2
import dune

from dune.packagemetadata import get_dune_py_dir, extract_metadata, Description
from dune.common import comm
from dune.common.locking import Lock, LOCK_EX,LOCK_SH
from dune.common.utility import buffer_to_str, isString, reload_module
from dune.common.module import select_modules, is_installed
from dune.generator.exceptions import CompileError, ConfigurationError
import dune.common.module

logger = logging.getLogger(__name__)

import logging
import threading
import os
import subprocess

logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.INFO)

cxxFlags = None
noDepCheck = False

class Builder:
    def dunepy_from_template(dunepy_dir,force=False):
        # Extract the raw data dictionary
        data = extract_metadata()

        modules    = data.combine_across_modules("MODULENAME")
        builddirs  = data.zip_across_modules("DEPS", "DEPBUILDDIRS")

        # todo: improve on this - just getting the idea how this could work
        cmakeflags = data.combine_across_modules("CMAKE_FLAGS")

        duneOptsFile = None

        cmakeFlags = {}
        for x in cmakeflags:
            for y in x.split(";"):
                try:
                    k,v = y.split(":=",1)
                    if k=="DUNE_OPTS_FILE": duneOptsFile=v
                    else: cmakeFlags[k] = v.strip()
                except ValueError: # no '=' in line
                    pass

        # add dune modules which where available during the build of a
        # python module but don't provide their own python module
        for k,v in builddirs.items():
            if k not in modules and not v.endswith("NOTFOUND"):
                modules += [k]
        modules.sort()

        # for an existing dune-py compare module dependencies to check if rebuild required
        if os.path.exists(os.path.join(dunepy_dir,"dune.module")):
            description = Description(os.path.join(dunepy_dir,"dune.module"))
            deps = description.depends
            force = not modules == [d[0] for d in deps] # ignore version number in dependency for now
            # todo also check if location of modules have changed to find
            # modules moved from build to install or vice versa - or tag that some other way
            # For now we simply always force a rebuild of the dune-py
            # module which might lead to more work than required
            force = True
        else:
            force = True

        if force:
            # Remove any remnants of an old dune-py installation
            # if os.path.exists(dunepy_dir):
            #     shutil.rmtree(dunepy_dir)
            # only remove cache
            try:
                os.remove(os.path.join(dunepy_dir, 'CMakeCache.txt'))
            except FileNotFoundError:
                pass

            # add flags from some opts file
            duneOptsFile = os.environ.get('DUNE_OPTS_FILE', duneOptsFile)
            if duneOptsFile:
                # TODO: check here if the duneOptsFile exists and warn if not
                #       Should be possible by checking return code of the subprocess
                #       so that other bash errors are also caught and warned about
                command = ['bash', '-c', 'source ' + duneOptsFile + ' && echo "$CMAKE_FLAGS"']
                proc = subprocess.Popen(command, stdout = subprocess.PIPE)
                stdout, _ = proc.communicate()
                cmake_args = shlex.split(buffer_to_str(stdout))
            else:
                cmake_args = []

            # check environment variable
            cmake_args += shlex.split( os.environ.get('CMAKE_FLAGS','') )

            for y in cmake_args:
                try:
                    k,v = y.split("=",1)
                    if k.startswith('-D'): k = k[2:]
                    cmakeFlags[k] = v.strip()
                except ValueError: # no '=' in line
                    pass

            # Gather and reorganize meta data context that is used to write dune-py
            context = {}
            context["modules"]        = modules
            context["builddirs"]      = builddirs
            context["install_prefix"] = data.unique_value_across_modules("INSTALL_PREFIX")
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
                # fix issues due to template taken from build dir
                if root.endswith("CMakeFiles") or root.endswith("src_dir"): continue

                for template_file in files:
                    full_template_file = os.path.join(root, template_file)
                    relative_template_file = os.path.relpath(full_template_file, start=template_path)
                    gen_file = os.path.join(dunepy_dir, relative_template_file.split(".template")[0])

                    # we don't want to remove CMakeLists.txt in the generated folder if it already exists
                    if root.endswith('generated') and os.path.exists(gen_file): continue
                    if gen_file.endswith(".cmake") or gen_file.endswith("Makefile"): continue # issues due to template taken from build dir

                    os.makedirs(os.path.split(gen_file)[0], exist_ok=True)
                    with open(gen_file, "w") as outfile:
                        outfile.write(env.get_template(relative_template_file).render(**context))


            # configure dune-py
            cmake = subprocess.Popen( "cmake .".split(),
                                     cwd=dunepy_dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            stdout, stderr = cmake.communicate()
            if cmake.returncode > 0:
                raise CompileError(buffer_to_str(stderr))
            else:
                logger.debug("Build output: "+buffer_to_str(stdout))

    def __init__(self, force=False, saveOutput=False):
        self.force = force
        self.skipTargetAll = False
        if saveOutput is True or saveOutput.lower() == "write":
            self.savedOutput = [open("generatorCompiler.out","w+"), open("generatorCompiler.err","w+")]
        elif saveOutput.lower() == "append":
            self.savedOutput = [open("generatorCompiler.out","a+"), open("generatorCompiler.err","a+")]
        elif saveOutput.lower() == "console" or \
             saveOutput.lower() == "terminal" or \
             saveOutput.lower() == "console_nocmake":
            self.savedOutput = [sys.stdout, sys.stderr]
            if saveOutput.lower() == "console_nocmake":
                self.skipTargetAll = True
        else:
            self.savedOutput = None
        self.dune_py_dir = dune.common.module.get_dune_py_dir()
        self.build_args = dune.common.module.get_default_build_args()
        self.generated_dir = os.path.join(self.dune_py_dir, 'python', 'dune', 'generated')
        self.initialized = False

    def initialize(self):
        if comm.rank == 0:
            os.makedirs(self.dune_py_dir, exist_ok=True)
            # need to lock here so that multiple procersses don't try to
            # generate a new dune-py at the same time
            with Lock(os.path.join(self.dune_py_dir, '..', 'lock-module.lock'), flags=LOCK_EX):
                # check for existence of tag file - this is removed if a
                # new dune package is added
                tagfile = os.path.join(self.dune_py_dir, ".noconfigure")
                if not os.path.isfile(tagfile):
                    logger.info('Generating dune-py module in '+self.dune_py_dir)
                    Builder.dunepy_from_template(get_dune_py_dir())
                    open(tagfile, 'a').close()
                else:
                    logger.debug('Using existing dune-py module in '+self.dune_py_dir)
                    self.compile(verbose=True)

        comm.barrier()
        try:
            dune.__path__._path.insert(0,os.path.join(self.dune_py_dir, 'python', 'dune'))
        except:
            dune.__path__.insert(0,os.path.join(self.dune_py_dir, 'python', 'dune'))
        self.initialized = True

    def compile(self, target='all', verbose=False):
        cmake_command = dune.common.module.get_cmake_command()
        cmake_args = [cmake_command, "--build", self.dune_py_dir,
                      "--target", target, "--parallel"]
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
        if cmake.returncode > 0:
            raise CompileError(buffer_to_str(stderr))
        elif verbose:
            logger.debug("Compiler output: "+buffer_to_str(stdout))
        writeOutput = self.savedOutput is not None and target != 'all' if self.skipTargetAll else self.savedOutput is not None
        if writeOutput:
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

    def load(self, moduleName, source, pythonName, extraCMake=None):
        ## TODO replace if rank with something better
        ## and remove barrier further down
        if not self.initialized:
            self.initialize()
        if comm.rank == 0:
            module = sys.modules.get("dune.generated." + moduleName)
            if module is None:
                # make sure nothing (compilation, generating and building) is # taking place
                with Lock(os.path.join(self.dune_py_dir, '..', 'lock-module.lock'), flags=LOCK_EX):
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
                                    if extraCMake is not None:
                                        for x in extraCMake:
                                            out.write(x.replace("TARGET",moduleName)+"\n")
                                # update build system
                                logger.debug("Rebuilding module")
                                try:
                                    # self.compile()
                                    cmake = subprocess.Popen( "cmake .".split(),
                                                       cwd=self.dune_py_dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                                    stdout, stderr = cmake.communicate()
                                    # logger.debug("Build output: "+buffer_to_str(stdout))
                                    if cmake.returncode > 0:
                                        raise CompileError(buffer_to_str(stderr))
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
                with Lock(os.path.join(self.dune_py_dir, '..', 'lock-module.lock'), flags=LOCK_SH):
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
