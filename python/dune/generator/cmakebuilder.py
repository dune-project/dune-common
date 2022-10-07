# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import importlib
import logging
import subprocess
import os
import sys
import shlex
import jinja2
import signal
import json
import copy

import dune

from dune.packagemetadata import (
    getDunePyDir, Description,
    getBuildMetaData, getCMakeFlags, envCMakeFlags, defaultCMakeFlags,
    getExternalPythonModules
)

from dune.common import comm
from dune.common.locking import Lock, LOCK_EX, LOCK_SH
from dune.common.utility import buffer_to_str, isString, reload_module

from dune.generator.exceptions import CompileError
from dune.generator.remove import removeGenerated

logger = logging.getLogger(__name__)
logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.INFO)

cxxFlags = None
noDepCheck = False

def deprecationMessage(dune_py_dir):
    print(f"Using a pre-existing old style dune-py with a newer version of dune-common. Remove dune-py folder `{dune_py_dir}` and re-run Python script!")
    print(f"It might also be required to execute 'cmake .' in the 'dune-common' build directory")
    print(f"It is possible to continue to use the old version by typing 'export DUNE_PY_USE_CMAKEBUILDER=1'")
    sys.exit(1)

def getCMakeCommand():
    try:
        return os.environ['DUNE_CMAKE']
    except KeyError:
        return 'cmake'


def getDefaultBuildArgs():
    try:
        return shlex.split(os.environ['DUNE_BUILD_FLAGS'])
    except KeyError:
        return None


class Builder:

    @staticmethod
    def generate_dunepy_from_template(dunepy_dir,force=False):
        # Extract the raw data dictionary

        metaData = getBuildMetaData()
        modules = metaData.combine_across_modules("MODULENAME")
        builddirs = metaData.zip_across_modules("DEPS", "DEPBUILDDIRS")

        # add dune modules which where available during the build of a
        # python module but don't provide their own python module
        for k,v in builddirs.items():
            if k not in modules and not v.endswith("NOTFOUND"):
                modules += [k]
        modules.sort()

        logger.debug("Found the following Dune module dependencies for dune-py: " + str(modules))

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

            # Gather and reorganize meta data context that is used to write dune-py
            context = {}
            context["modules"]        = modules
            context["builddirs"]      = builddirs
            context["install_prefix"] = metaData.unique_value_across_modules("INSTALL_PREFIX")
            context["cmake_flags"]    = getCMakeFlags()
            context["cxx_compiler"]   = context["cmake_flags"]["CMAKE_CXX_COMPILER"]
            context["cxx_flags"]      = context["cmake_flags"]["CMAKE_CXX_FLAGS"]

            # Find the correct template path
            path, _ = os.path.split(__file__)
            template_path = os.path.join(path, "template")

            # Run the template through Jinja2
            env = jinja2.Environment(
                loader=jinja2.FileSystemLoader(template_path),
                keep_trailing_newline=True,
            )
            for root, _, files in os.walk(template_path):
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

        # return force variable because this might be needed in the overloaded versions
        return force

    @staticmethod
    def build_dunepy_from_template(dunepy_dir,force=False):
        force = Builder.generate_dunepy_from_template(dunepy_dir,force)
        if force:
            # configure dune-py
            Builder.callCMake(["cmake"]+defaultCMakeFlags()+["."],
                              cwd=dunepy_dir,
                              infoTxt="Configuring dune-py with CMake",
                              active=True, # print details anyway
                              )
        return force

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
        self.dune_py_dir = getDunePyDir()
        self.build_args = getDefaultBuildArgs()
        self.generated_dir = os.path.join(self.dune_py_dir, 'python', 'dune', 'generated')
        self.initialized = False
        self.externalPythonModules = copy.deepcopy(getExternalPythonModules())

    def cacheExternalModules(self):
        """Store external modules in dune-py"""

        externalModulesPath = os.path.join(self.dune_py_dir, ".externalmodules.json")
        with open(externalModulesPath, "w") as externalModulesFile:
            json.dump(self.externalPythonModules, externalModulesFile)

    def initialize(self):
        self.externalPythonModules = copy.deepcopy(getExternalPythonModules())

        if comm.rank == 0:
            logger.debug("(Re-)Initializing JIT compilation module")
            os.makedirs(self.dune_py_dir, exist_ok=True)
            # need to lock here so that multiple processes don't try to
            # generate a new dune-py at the same time
            with Lock(os.path.join(self.dune_py_dir, '..', 'lock-module.lock'), flags=LOCK_EX):
                # check for existence of tag file - this is removed if a
                # new dune package is added
                tagfile = os.path.join(self.dune_py_dir, ".noconfigure")
                if not os.path.isfile(tagfile):
                    logger.info('Generating dune-py module in ' + self.dune_py_dir)
                    # create module cache for external modules that have been registered with dune-py
                    self.cacheExternalModules()
                    # create dune-py module
                    self.build_dunepy_from_template(self.dune_py_dir)
                    # create tag file so that dune-py is not rebuilt on the next build
                    open(tagfile, 'a').close()
                else:
                    logger.debug('Using existing dune-py module in ' + self.dune_py_dir)
                    self.compile("Rebuilding dune-py module")

                # Auto-clean up dune-py: Remove all modules that have not been used in the last 30 days.
                removeGenerated(['30'], date=True)

        comm.barrier()
        try:
            dune.__path__._path.insert(0,os.path.join(self.dune_py_dir, 'python', 'dune'))
        except:
            dune.__path__.insert(0,os.path.join(self.dune_py_dir, 'python', 'dune'))
        self.initialized = True


    @staticmethod
    def callCMake(cmake_args, cwd, infoTxt, verbose=False, active=False, logLevel=logging.DEBUG, env=None):
        # print initial info, if we are verbose
        # or we know that we have to build something
        stdout, stderr = None, None
        active = active or verbose
        if active:
            if infoTxt:
                logger.log(logLevel,infoTxt)
        # call the cmake process
        if verbose:
            print(cmake_args)

        with subprocess.Popen(cmake_args,
                              cwd=cwd,
                              env=env,
                              stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE) as cmake:
            try:
                stdout, stderr = cmake.communicate(timeout=2) # no message if delay is < 2sec
            except subprocess.TimeoutExpired:
                if infoTxt and not active:
                    logger.log(logLevel, infoTxt)
                # wait for cmd to finish
                stdout, stderr = cmake.communicate()

            # check return code
            if cmake.returncode > 0:
                # retrieve stderr output
                print(stderr,stdout)
                raise CompileError(buffer_to_str(stderr))

        return stdout, stderr

    def _compile(self, infoTxt, target='all', verbose=False):
        # if called for other builders don't do anything here
        assert isinstance(self, Builder)

        cmake_command = getCMakeCommand()
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
        stdout, stderr = Builder.callCMake(cmake_args,
                                           cwd=self.generated_dir,
                                           infoTxt=infoTxt,
                                           verbose=verbose,
                                           logLevel=logging.INFO
                                          )
        writeOutput = self.savedOutput is not None and target != 'all' if self.skipTargetAll else self.savedOutput is not None
        if writeOutput:
            if stdout is not None:
                out = buffer_to_str(stdout)
                nlines = out.count('\n')
                if nlines > 1:
                    self.savedOutput[0].write("###############################\n")
                    self.savedOutput[0].write("###" + " ".join(cmake_args)+"\n")
                if nlines > 0:
                    self.savedOutput[0].write(out)
                if nlines > 1:
                    self.savedOutput[0].write("\n###############################\n")

            if stderr is not None:
                err = buffer_to_str(stderr)
                nlines = err.count('\n')
                if nlines > 1:
                    self.savedOutput[1].write("###############################\n")
                    self.savedOutput[1].write("###" + " ".join(cmake_args)+"\n")
                if nlines > 0:
                    self.savedOutput[1].write(err)
                if nlines > 1:
                    self.savedOutput[1].write("\n###############################\n")

    def compile(self, infoTxt, target='all', verbose=False):
        return self._compile(infoTxt, target=target, verbose=verbose)

    def _maybeConfigureWithCMake(self, moduleName, source, pythonName, extraCMake):
        sourceFileName = os.path.join(self.generated_dir, moduleName + ".cc")
        line = "dune_add_pybind11_module(NAME " + moduleName + " EXCLUDE_FROM_ALL)"
        # first check if this line is already present in the CMakeLists file
        # (possible if a previous script was stopped by user before module was compiled)
        with open(os.path.join(self.generated_dir, "CMakeLists.txt"), 'r') as out:
            found = line in out.read()
        if not os.path.isfile(sourceFileName) or not found:
            compilationInfoMessage = f"Compiling {pythonName} (new)"
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
                try:
                    Builder.callCMake(["cmake","."],
                                      cwd=self.dune_py_dir,
                                      infoTxt="Configuring module {} ({}) with CMake".format(
                                          pythonName, moduleName
                                      ),
                                      )
                except: # all exceptions will cause a problem here
                    os.remove(os.path.join(sourceFileName))
                    # remove line from CMakeLists to avoid problems
                    with open(os.path.join(self.generated_dir, "CMakeLists.txt"), 'a') as out:
                        out.truncate(origPos)
                    raise
        elif isString(source) and not source == open(os.path.join(sourceFileName), 'r').read():
            compilationInfoMessage = f"Compiling {pythonName} (updated)"
            code = str(source)
            with open(os.path.join(sourceFileName), 'w') as out:
                out.write(code)
        else:
            # we can directly load the module - after checking if headers have changed
            compilationInfoMessage = f"Compiling {pythonName} (rebuilding)"
            # sanity check
            line = "dune_add_pybind11_module(NAME " + moduleName + " EXCLUDE_FROM_ALL)"
            # the CMakeLists file should already include this line
            with open(os.path.join(self.generated_dir, "CMakeLists.txt"), 'r') as out:
                found = line in out.read()
            assert found, "CMakeLists.txt file does not contain an entry to build"+moduleName

        return compilationInfoMessage

    def load(self, moduleName, source, pythonName, extraCMake=None):
        # check if we need to initialize dune-py either because
        # this is the first call to load or because an external module with metadata has been registered
        if not self.initialized or not self.externalPythonModules == getExternalPythonModules():
            self.initialize()

        # check whether module is already compiled and build it if necessary
        # (only try to build module on rank 0!)
        # TODO replace if rank with something better and remove barrier further down
        if comm.rank == 0:
            module = sys.modules.get("dune.generated." + moduleName)
            if module is None:
                self._buildModule( moduleName, source, pythonName, extraCMake )

        ## TODO remove barrier here
        comm.barrier()

        logger.debug("Loading " + moduleName)
        module = importlib.import_module("dune.generated." + moduleName)

        if self.force:
            logger.info("Reloading " + pythonName)
            module = reload_module(module)

        return module

    def _buildModule(self, moduleName, source, pythonName, extraCMake):
        logger.debug("Module {} not loaded".format(moduleName))
        # make sure nothing (compilation, generating and building) is taking place
        with Lock(os.path.join(self.dune_py_dir, '..', 'lock-module.lock'), flags=LOCK_EX):
            # module must be generated so lock the source file
            with Lock(os.path.join(self.dune_py_dir, 'lock-'+moduleName+'.lock'), flags=LOCK_EX):
                # the module might now be present, so check again
                # (see #295)
                module = sys.modules.get("dune.generated." + moduleName)
                if module is None:
                    compilationMessage = self._maybeConfigureWithCMake(
                        moduleName, source, pythonName, extraCMake
                    )
                else:
                    compilationMessage = f"Compiling {pythonName} (rebuilding after concurrent build)"
        # end of exclusive dune-py lock

        # we always compile even if the module is always compiled since it can happen
        # that dune-py was updated in the mean time.
        # This step is quite fast but there is room for optimization.

        # for compilation a shared lock is enough
        #
        # A side effect is that during the parallel make calls
        # for whatever reason cmake might be invoked due to
        # changes to the module (i.e. a new target
        # added). Parallel cmake calls are not allowed and as
        # a consequence the complete build may fail. We take
        # care of such parallel cmake calls by additional
        # locking in the dune-py CMakeLists.txt
        with Lock(os.path.join(self.dune_py_dir, '..', 'lock-module.lock'), flags=LOCK_SH):
            # lock generated module
            with Lock(os.path.join(self.dune_py_dir, 'lock-'+moduleName+'.lock'), flags=LOCK_EX):
                self._compile(infoTxt=compilationMessage, target=moduleName)
        return module

############################################################################
##
##  Builder based on Makefiles
##
############################################################################

class MakefileBuilder(Builder):

    # static vars holding command for bash and make
    bashCmd = 'bash' # default values
    makeCmd = 'make' # set in dunepy_from_template

    @staticmethod
    def build_dunepy_from_template(dunepy_dir, force=False):

        # call base class dunepy_from_template
        force = Builder.generate_dunepy_from_template(dunepy_dir, force=force)

        if force:
            # generate path for the buildScript and other filed to be created
            generatedDir = os.path.join(dunepy_dir,'python','dune','generated')
            buildScriptName = os.path.join(dunepy_dir,'python','dune','generated','buildScript.sh')

            # check whether ninja is available with cmake or not
            useNinja = False # to be revised in later versions
            if useNinja:
                try:
                    Builder.callCMake(["cmake", "-G", "Ninja"] + defaultCMakeFlags() + ["."],
                                       cwd=dunepy_dir,
                                       infoTxt="Configuring dune-py with CMake (ninja)",
                                       active=True, # print details anyway
                                     )
                    logger.debug("Using ninja as cmake generator!")
                except:
                    logger.debug("Check for ninja failed!")
                    useNinja = False
            if not useNinja:
                # call base class dunepy_from_template (re-initialize)
                force = Builder.generate_dunepy_from_template(dunepy_dir, force=True)

                Builder.callCMake(["cmake"] + defaultCMakeFlags() + ["."],
                                  cwd=dunepy_dir,
                                  infoTxt="Configuring dune-py with CMake (make)",
                                  active=True, # print details anyway
                                 )

            # obtain bash and make command generated by cmake
            # when ninja is used makeCmd is the ninja command
            try:
                commandSourceName = os.path.join(dunepy_dir,'unix_commands.json')
                with open(commandSourceName) as commandFile:
                    cmdList = json.load(commandFile)[0]
                    MakefileBuilder.bashCmd = cmdList["bash"] # shell command
                    MakefileBuilder.makeCmd = cmdList["make"] # make command
                    MakefileBuilder.generatorCmd = cmdList["generator"] # generator (make or ninja)
            except FileNotFoundError:
                pass

            if not useNinja:
                #########################################################################
                # Remark about ninja:
                # - the -B used below for the extractCompiler build needed for
                #   the second approach does not work with ninja
                # - the link.txt used in approach 1/3 is not available with ninja
                # - the CXXFLAGS overwrite seems not to work with ninja -
                #   the script is not generated so approach 2 will not work due to missing echo
                # We don't have to use ninja explicitly for dune-py but have
                # to make sure the user can't set up ninja for dune-py by # mistake
                #########################################################################
                try:
                    stdout, stderr = \
                      Builder.callCMake(["cmake"]+
                                         ['--build','.','--target',"extractCompiler"]+
                                         ['--','-B'],
                                         cwd=dunepy_dir,
                                         env={**os.environ,
                                              "CXXFLAGS":" ",
                                              },
                                         infoTxt="extract compiler command",
                                         active=True, # print details anyway
                                       )
                except CompileError:
                    deprecationMessage(dunepy_dir)

                # now also generate compiler command
                stdout, stderr = \
                  Builder.callCMake(["cmake","-DCMAKE_EXPORT_COMPILE_COMMANDS=ON","."],
                                     cwd=dunepy_dir,
                                     env={**os.environ,
                                          "CXXFLAGS":" ",
                                          },
                                     infoTxt="extract compiler command",
                                     active=True, # print details anyway
                                   )
                ########################################################################
                #   Write buildScript.sh
                ########################################################################

                # we already have link.txt from the previous cmake run (extractCompiler)
                buildDirBase = os.path.join(generatedDir,'CMakeFiles','extractCompiler.dir')
                linkerSourceName = os.path.join(buildDirBase,'link.txt')
                buildSourceName = os.path.join(buildDirBase,'build.make')

                commandSourceName = os.path.join(dunepy_dir,'compile_commands.json')
                # we do not need the buildScript template since the compiler command generated uses CXX_compiler.sh
                with open(buildScriptName, "w") as buildScript:
                    # write bash line
                    buildScript.write("#!" + MakefileBuilder.bashCmd + "\n")
                    # write compiler commands
                    with open(commandSourceName) as commandFile:
                        compilerCmd = json.load(commandFile)[0]["command"]
                    compilerCmd = compilerCmd.replace('extractCompiler', '$1')
                    # this line is extracted from build.make
                    usedBuildMake = False
                    #with open(buildSourceName, "r") as buildFile:
                    #    buildCmd = buildFile.readlines()
                    #    for idx,line in enumerate(buildCmd):
                    #        # compiler command is next line after "Building CXX object ..."
                    #        if "Building CXX object " in line and idx < len(buildCmd)-1:
                    #            cmd = buildCmd[idx+1].partition("(CXX_FLAGS)")[2]
                    #            cmd = cmd.partition(" -o ")[0].replace('extractCompiler', '$1')
                    #            compilerCmd += cmd
                    #            usedBuildMake = True
                    #            break
                    # fall back in case something went wrong with parsing build.make
                    if not usedBuildMake:
                        compilerCmd = compilerCmd + " -MD -MT CMakeFiles/$1.dir/$1.cc.o -MF CMakeFiles/$1.dir/$1.cc.o.d"

                    # forward errors so that compilation failure will be caught
                    buildScript.write('set -e\n')
                    buildScript.write(compilerCmd)
                    buildScript.write('\n')
                    # write linker commands
                    with open(linkerSourceName, "r") as linkerSource:
                        linkerCmd = linkerSource.read()
                    linkerCmd = linkerCmd.replace('extractCompiler','$1')
                    buildScript.write(linkerCmd)

            else: # useNinja
                ninja = MakefileBuilder.generatorCmd
                assert ninja.find("ninja")

                with subprocess.Popen([ninja, "-t","commands","extractCompiler"],
                                       cwd=dunepy_dir,
                                       stdout=subprocess.PIPE,
                                       stderr=subprocess.PIPE) as ninjaProcess:
                    stdout, stderr = ninjaProcess.communicate()
                    exit_code = ninjaProcess.returncode
                assert exit_code == 0

                ########################################################################
                #   Write buildScript.sh
                ########################################################################
                out = buffer_to_str(stdout).strip().split("\n")
                assert len(out) == 2
                with open(buildScriptName, "w") as buildScript:
                    # write bash line
                    buildScript.write("#!" + MakefileBuilder.bashCmd + "\n")
                    buildScript.write("set -e\n")
                    compilerCmd = out[0].replace('extractCompiler', '$1').\
                                         replace(' python/dune/generated/',' ') # better to move the script to the root of dune-py then this can be kept
                    compilerCmd = compilerCmd.split(' ',1)
                    compilerCmd = compilerCmd[0] + " " + compilerCmd[1]
                    buildScript.write(compilerCmd+"\n")
                    # this needs fixing: Issue is that at the linker line beginns with ': && '
                    linkerCmd = out[1].replace('extractCompiler','$1').\
                                       replace(' python/dune/generated/',' ') # better to move the script to the root of dune-py then this can be kept
                    linkerCmd = linkerCmd.split(' ',3)
                    linkerCmd = linkerCmd[2] + " " + linkerCmd[3].replace("&& :", "")
                    buildScript.write(linkerCmd+"\n")

    # constructor
    def __init__(self, force=False, saveOutput=False):
        # call __init__ of base class
        super().__init__(force=force, saveOutput=saveOutput)

    # just added to check for old versions of dune-py - can be removed once
    # this check is not needed anymore
    def initialize(self):
        super().initialize()
        # check that the compile script is available
        script = os.path.join(self.generated_dir,"buildScript.sh")
        if not os.path.exists(script):
            deprecationMessage(self.dune_py_dir)

    # nothing to be done in this class
    def compile(self, infoTxt, target='all', verbose=False):
        pass

    def _configureWithMake(self, moduleName, source, pythonName):
        sourceFileName = os.path.join(self.generated_dir, moduleName + ".cc")
        if not os.path.isfile(sourceFileName):
            compilationInfoMessage = f"Compiling {pythonName} (new)"
            code = str(source)
            with open(os.path.join(sourceFileName), 'w') as out:
                out.write(code)
        elif isString(source) and not source == open(os.path.join(sourceFileName), 'r').read():
            compilationInfoMessage = f"Compiling {pythonName} (updated)"
            code = str(source)
            with open(os.path.join(sourceFileName), 'w') as out:
                out.write(code)
        else:
            compilationInfoMessage = f"{pythonName} (loading)"
        os.makedirs(os.path.join(self.generated_dir,"CMakeFiles",moduleName+".dir"), exist_ok=True)
        return compilationInfoMessage

    def _buildModule(self, moduleName, source, pythonName, extraCMake ):
        logger.debug("Module {} not loaded".format(moduleName))
        # make sure nothing (compilation, generating and building) is taking place
        # module must be generated so lock the source file
        with Lock(os.path.join(self.dune_py_dir, '..', 'lock-module.lock'), flags=LOCK_SH):
            with Lock(os.path.join(self.dune_py_dir, 'lock-'+moduleName+'.lock'), flags=LOCK_EX):

                # the module might now be present, so check again
                # (see #295)
                module = sys.modules.get("dune.generated." + moduleName)
                if module is None:
                    compilationMessage = self._configureWithMake( moduleName, source, pythonName )
                else:
                    compilationMessage = f"Compiling {pythonName} (rebuilding after concurrent build)"

                # we always compile even if the module is already compiled since it can happen
                # that dune-py was updated in the mean time ?????
                # This step is quite fast but there is room for optimization.

                # we can always generate a new makefile - in case no
                # dependency file has already been generate leave
                # dependencies empty otherwise use existing depFile:
                depFileName  = os.path.join(self.generated_dir,"CMakeFiles",moduleName+'.dir',moduleName+'.cc.o.d')
                makeFileName = os.path.join(self.generated_dir,"CMakeFiles",moduleName+'.dir',moduleName+'.make')
                with open(makeFileName, "w") as makeFile:
                    makeFile.write('.SUFFIXES:\n')
                    try:
                        with open(depFileName, "r") as depFile:
                            makeFile.write(depFile.read())
                    except FileNotFoundError:
                        makeFile.write(os.path.join("CMakeFiles",moduleName+'.dir',moduleName+'.cc.o')+':\n')
                        pass
                    makeFile.write('\t'+MakefileBuilder.bashCmd+ ' buildScript.sh '+moduleName+"\n")
                    makeFile.write(moduleName+'.so: '+os.path.join("CMakeFiles",moduleName+'.dir',moduleName+'.cc.o')+'\n')


                # first just check if the makefile does not contain any error
                # An issue could be that a file in the dependency list has
                # moved (e.g. from installed to source build)
                with subprocess.Popen([MakefileBuilder.makeCmd, "-q", "-f",makeFileName, moduleName+'.so'],
                                      cwd=self.generated_dir,
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE) as make:
                    stdout, stderr = make.communicate()
                    exit_code = make.returncode

                # this means that there was a problem
                # with the makefile so we remove the dependencies and the
                # generated module so that it will be regenerated
                if exit_code == 2:
                    with open(makeFileName, "w") as makeFile:
                        makeFile.write('.SUFFIXES:\n')
                        makeFile.write(os.path.join("CMakeFiles",moduleName+'.dir',moduleName+'.cc.o')+':\n')
                        makeFile.write('\t'+MakefileBuilder.bashCmd+ ' buildScript.sh '+moduleName+"\n")
                        makeFile.write(moduleName+'.so: '+os.path.join("CMakeFiles",moduleName+'.dir',moduleName+'.cc.o')+'\n')

                if exit_code > 0:
                    # call make to build shared library
                    with subprocess.Popen([MakefileBuilder.makeCmd, "-f",makeFileName, moduleName+'.so'],
                                          cwd=self.generated_dir,
                                          stdout=subprocess.PIPE,
                                          stderr=subprocess.PIPE) as make:
                        try:
                            stdout, stderr = make.communicate(timeout=2) # no message if delay is <2sec
                        except subprocess.TimeoutExpired:
                            # compilation is taking place, replace loading with rebuilding
                            compilationMessage = compilationMessage.replace("loading", "rebuilding")
                            logger.log(logging.INFO,compilationMessage)
                            # wait for cmd to finish
                            stdout, stderr = make.communicate()
                        exit_code = make.returncode

                    if self.savedOutput is not None:
                        self.savedOutput[0].write('make return:' + str(exit_code) + "\n")
                        self.savedOutput[0].write(str(stdout.decode()) + "\n")
                        self.savedOutput[1].write(str(stderr.decode()) + "\n")

                    # check return code
                    if exit_code > 0:
                        # retrieve stderr output
                        raise CompileError(buffer_to_str(stderr))
