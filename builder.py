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


def get_dune_py_dir():
    try:
        return os.environ['DUNE_PY_DIR']
    except KeyError:
        pass

    try:
        return os.path.join(os.environ['HOME'], '.cache', 'dune-py')
    except KeyError:
        pass

    raise RuntimeError('Unable to determine location for dune-py module. Please set the environment variable "DUNE_PY_DIR".')


def get_cmake_definitions():
    definitions = {}
    try:
        for arg in shlex.split(os.environ['DUNE_CMAKE_FLAGS']):
            key, value = arg.split('=', 1)
            definitions[key] = value
    except KeyError:
        pass
    return definitions


def make_dune_py_module(dune_py_dir=None):
    if dune_py_dir is None:
        dune_py_dir = get_dune_py_dir()
    descFile = os.path.join(dune_py_dir, 'dune.module')
    if not os.path.isfile(descFile):
        if not os.path.isdir(dune_py_dir):
            os.makedirs(dune_py_dir)

        # create python/dune/generated
        generated_dir_rel = os.path.join('python','dune', 'generated')
        generated_dir = os.path.join(dune_py_dir, generated_dir_rel)
        if not os.path.isdir(generated_dir):
            os.makedirs(generated_dir)

        cmake_content = ['add_library(generated_module SHARED EXCLUDE_FROM_ALL generated_module.cc)',
                         'target_include_directories(generated_module PRIVATE ${CMAKE_CURRENT_BINARY_DIR})',
                         'add_dune_mpi_flags(generated_module)',
                         'set_target_properties(generated_module PROPERTIES PREFIX "")',
                         'target_compile_definitions(generated_module PRIVATE USING_COREPY)',
                         'file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/__init__.py")']
        dune.project.write_cmake_file(generated_dir, cmake_content)

        with open(os.path.join(generated_dir, 'generated_module.cc'), 'w') as file:
            file.write('#include <config.h>\n\n')
            file.write('#define USING_COREPY 1\n\n')
            file.write('#include <dune/corepy/common/typeregistry.hh>\n')
            file.write('#include <dune/corepy/pybind11/pybind11.h>\n')
            file.write('\n#include "generated_module.hh"\n')

        modules, _ = dune.module.select_modules()
        description = dune.module.Description(module='dune-py', maintainer='dune@dune-project.org', depends=list(modules.values()))

        dune.project.make_project(dune_py_dir, description, subdirs=[generated_dir])
    else:
        if dune.module.Description(descFile).name != 'dune-py':
            raise RunetimeError('"' + dune_py_dir + '" already contains a different dune module.')


def build_dune_py_module(dune_py_dir=None):
    if dune_py_dir is None:
        dune_py_dir = get_dune_py_dir()
    definitions = get_cmake_definitions()

    desc = dune.module.Description(os.path.join(dune_py_dir, 'dune.module'))

    modules, dirs = dune.module.select_modules()
    deps = dune.module.resolve_dependencies(modules, desc)

    prefix = {}
    for name, dir in dirs.items():
        if dune.module.is_installed(dir, name):
            prefix[name] = dune.module.get_prefix(name)
        else:
            prefix[name] = dune.module.default_build_dir(dir, name)

    output = dune.module.configure_module(dune_py_dir, dune_py_dir, {d: prefix[d] for d in deps}, definitions)
    output += dune.module.build_module(dune_py_dir)
    return output


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

        self.dune_py_dir = get_dune_py_dir()
        self.generated_dir = os.path.join(self.dune_py_dir, 'python', 'dune', 'generated')
        #dune.__path__.append(os.path.join(self.dune_py_dir, 'python', 'dune'))
        dune.__path__.insert(0,os.path.join(self.dune_py_dir, 'python', 'dune'))

        if comm.rank == 0:
            if self.verbose:
                print("Building dune-py module...")
                start_time = timeit.default_timer()
            make_dune_py_module(self.dune_py_dir)
            output = build_dune_py_module(self.dune_py_dir)
            if self.verbose:
                print(output)
                print("Building dune-py module took", (timeit.default_timer() - start_time), "seconds")
        comm.barrier()


    def load(self, moduleName, source):
        if comm.rank == 0:
            if not os.path.isfile(os.path.join(self.generated_dir, moduleName + ".so")) or self.force:
                with open(os.path.join(self.generated_dir, "generated_module.hh"), 'w') as out:
                    out.write(source)

                if self.verbose:
                    print("Compiling " + moduleName + "...")
                    start_time = timeit.default_timer()

                cmake = subprocess.Popen(["cmake", "--build", self.dune_py_dir, "--target", "generated_module"], cwd=self.generated_dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
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
