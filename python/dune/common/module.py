# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import logging
import os
import re
import shlex
import subprocess
import sys

from os.path import expanduser

if __name__ == "dune.common.module":
    from dune.common.utility import buffer_to_str
    from dune.common import project
    from dune.packagemetadata import (
        Version, Description,
        defaultCMakeFlags, cmakeArguments,
        inVirtualEnvironment, getDunePyDir,
    )
    from dune.deprecate import deprecated
# this can also be used as a stand-alone script
else:
    from utility import buffer_to_str
    import project, deprecate
    from packagemetadata import (
        Version, Description,
        defaultCMakeFlags, cmakeArguments,
        inVirtualEnvironment, getDunePyDir,
    )
    from dune.deprecate import deprecated

@deprecated(name="dune.common.module.get_dune_py_dir", msg="Use 'dune.packagemetadata.getDunePyDir' instead")
def get_dune_py_dir():
    return getDunePyDir()

logger = logging.getLogger(__name__)

def find_modules(path):
    """find DUNE modules in given path

    Args:
        path: Iterable containing directories to search modules in

    Returns:
        List of (description, dir) pairs of found modules.
    """
    modules = []
    for dir in path:
        for root, dirs, files in os.walk(dir):
            if 'dune.module' in files:
                description = Description(os.path.join(root, 'dune.module'))
                if not description.name == "dune-py":
                    modules.append((description,os.path.abspath(root)))
                # do not traverse subdirectories
                # del dirs[:]
    return modules


def resolve_dependencies(modules, module=None, deps=None):
    """resolve module dependencies

    Args:
        modules:             dictionary mapping module name to description
        module (optional):   name or description of module to resolve dependencies for
        deps (optional):     dictionary mapping module name to an unordered set of its
            dependency names. This dictionary is extedend.

    Return:
        If module is given, an unordered set of its dependency names is returned.
        Otherwise a dictionary mapping module name to such a set is returned (i.e., deps).
    """
    if deps is None:
        deps = dict()

    if module is None:
        for m in modules:
            if m not in deps:
                resolve_dependencies(modules, m, deps)
        return deps

    if not isinstance(module, Description):
        module = modules[module]

    def resolve(desc, req):
        if not req(desc.version):
            raise ValueError('Module \'' + module.name + '\' requires ' + desc.name + ' ' + str(req) + '.')
        try:
            d = deps[desc.name]
            if d is None:
                raise ValueError('Module \'' + module.name + '\' has circular dependency on ' + desc.name + '.')
            return d | {desc.name}
        except KeyError:
            return resolve_dependencies(modules, desc, deps) | {desc.name}

    deps[module.name] = None
    mod_deps = set()
    for m, r in module.depends:
        try:
            mod_deps |= resolve(modules[m], r)
        except KeyError:
            raise ValueError('Module \'' + module.name + '\' has missing dependency \'' + m + '\'.')
    for m, r in module.suggests:
        try:
            mod_deps |= resolve(modules[m], r)
        except KeyError:
            pass
    deps[module.name] = mod_deps
    return mod_deps


def resolve_order(deps):
    """resolve module dependencies

    Args:
       deps:     dictionary mapping module name to its dependency names

    Return:
       Ordered list of module names such that each module only depends on
       modules preceding it.
    """
    order = []

    def resolve(m):
        if m not in order:
            for d, r in deps[m].depends:
                if d not in order:
                    resolve(d)
            order.append(m)

    for m in deps:
        resolve(m)
    return order


def pkg_config(pkg, var=None, paths=[]):
    args = ['pkg-config', pkg]
    if var is not None:
        args += ['--variable=' + var]
    env = dict(os.environ)
    env.update({'PKG_CONFIG_PATH': ':'.join(paths)})
    pkgconfig = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=env)
    pkgconfig.wait()
    prefix = pkgconfig.stdout.read()
    if pkgconfig.returncode != 0:
        raise KeyError('package ' + pkg + ' not found.')
    return buffer_to_str(prefix).strip()


def get_prefix(module):
    paths = get_module_path("pkgconfig")
    return pkg_config(module, var='prefix', paths=paths)


def is_installed(dir, module=None):
    """check whether a path contains an installed or a source version of a DUNE module

    Args:
        dir:                directory containing the module description file (dune.module)
        module (optional):  name of the module (either str or Description)
            If omitted, the module description file is parsed for it.

    Returns:
        True, if the module is installed, false otherwise
    """
    if module is None:
        module = Description(os.path.join(dir, 'dune.module'))
    if isinstance(module, Description):
        module = module.name
    try:
        prefix = get_prefix(module)
    except KeyError:
        return False
    for l in ['lib','lib32','lib64']:
        if os.path.realpath(dir) == os.path.realpath(os.path.join(prefix, l, 'dunecontrol', module)):
            return True
    return False

def get_cmake_command():
    try:
        return os.environ['DUNE_CMAKE']
    except KeyError:
        return 'cmake'

def get_local():
    if inVirtualEnvironment():
        return sys.prefix
    try:
        home = expanduser("~")
        return os.path.join(home, '.local')
    except KeyError:
        pass
    return ''

def get_module_path(post="dunecontrol"):
    path = ['.']
    # try to guess modules path for unix systems
    for l in ['lib','lib32','lib64']:
        path = path + [p for p in [
                       os.path.join('usr','local',l,post),
                       os.path.join('usr',l,post),
                       os.path.join(get_local(),l,post),
                     ]
                  if os.path.isdir(p)]
    try:
        path = path + [p for p in os.environ['DUNE_CONTROL_PATH'].split(':') if p and os.path.isdir(p)]
        if post == 'dunecontrol':
            logger.debug('Module path [DUNE_CONTROL_PATH]: ' + ':'.join(path))
    except KeyError:
        pass

    try:
        pkg_config_path = [p for p in os.environ['PKG_CONFIG_PATH'].split(':') if p and os.path.isdir(p)]
        if post == 'dunecontrol':
            pkg_config_path = [os.path.join(p, '..', post) for p in pkg_config_path]
        path = path + [p for p in pkg_config_path if os.path.isdir(p)]
    except KeyError:
        pass
    # try to guess module path using pkg-config
    try:
        prefix = pkg_config('dune-common', 'prefix').strip()
        path = path + [p for p in [ os.path.join(prefix, 'lib', post)] if os.path.isdir(p)]
        path = path + [p for p in [ os.path.join(prefix, 'lib32', post)] if os.path.isdir(p)]
        path = path + [p for p in [ os.path.join(prefix, 'lib64', post)] if os.path.isdir(p)]
        if post == 'dunecontrol':
            logger.debug('Module path [pkg-config]: ' + ':'.join(path))
    except KeyError:
        pass
    if post == 'dunecontrol':
        logger.debug('Module path [guessed]: ' + ':'.join(path))
    return path


def select_modules(modules=None, module=None):
    """choose one version of each module from a list of modules

    Args:
        modules (optional): List of (description, dir) pairs
            If not given, the find_modules(get_module_path()) is used
        module (optional):

    Returns:
        pair of dictionaries mapping module name to unique description and directory respectively
    """
    if modules is None:
        modules = find_modules(get_module_path())
    desc = {}
    dir = {}
    for d, p in modules:
        p = os.path.realpath(p)
        n = d.name
        if n in dir:
            if p == dir[n]: continue
            if is_installed(dir[n], n):
                if is_installed(p, n):
                    foundVersions = " In " + p + " and in " + dir[n]
                    raise KeyError('Multiple installed versions for module \'' + n + '\' found.'+foundVersions)
                else:
                    desc[n], dir[n] = d, p
            else:
              if not is_installed(p, n):
                  foundVersions = " In " + p + " and in " + dir[n]
                  raise KeyError('Multiple source versions for module \'' + n + '\' found.'+foundVersions)
        else:
            desc[n], dir[n] = d, p
    return (desc, dir)


def default_build_dir(srcdir, module=None, builddir=None):
    if builddir is None:
        builddir = os.environ.get('DUNE_BUILDDIR', 'build-cmake')

    if os.path.isabs(builddir):
        if module is None:
            module = Description(os.path.join(srcdir, 'dune.module'))
        if isinstance(module, Description):
            module = module.name
        return os.path.join(builddir, module)
    else:
        return os.path.join(srcdir, builddir)


def configure_module(srcdir, builddir, prefix_dirs, cmake_args=None):
    """configure a given module by running CMake

    Args:
        srcdir:                  source directory of module
        builddir:                build directory for module (may equal srcdir for in-source builds)
        prefix_dirs:             dictionary mapping dependent modules to their prefix
        cmake_args (optional):   list of additional CMake flags

    Returns:
        Output of CMake command
    """
    args = [ get_cmake_command() ]
    args += cmakeArguments(cmake_args)
    args += ['-D' + module + '_DIR=' + dir for module, dir in prefix_dirs.items()]
    args.append(srcdir)
    if not os.path.isdir(builddir):
        os.makedirs(builddir)
    logger.debug('Calling "' + ' '.join(args) + '"')
    cmake = subprocess.Popen(args, cwd=builddir, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = cmake.communicate()
    logging.debug(buffer_to_str(stdout))
    if cmake.returncode != 0:
        raise RuntimeError(buffer_to_str(stderr))
    return buffer_to_str(stdout)


def get_default_build_args():
    try:
        return shlex.split(os.environ['DUNE_BUILD_FLAGS'])
    except KeyError:
        pass

    return None

def build_module(builddir, build_args=None):
    if build_args is None:
        build_args = get_default_build_args()

    cmake_args = [get_cmake_command(), '--build', '.']
    if build_args is not None:
        cmake_args += ['--'] + build_args

    cmake = subprocess.Popen(cmake_args, cwd=builddir, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = cmake.communicate()
    if cmake.returncode != 0:
        raise RuntimeError(buffer_to_str(stderr))
    return buffer_to_str(stdout)

def get_dune_py_version():
    # change this version on the following events:
    # - a release (major version numbers)
    # - any incompatible change to the dune-py module (revision number)
    return Version("2.8.0")


def make_dune_py_module(dune_py_dir=None, deps=None):
    if dune_py_dir is None:
        dune_py_dir = getDunePyDir()
    os.makedirs(dune_py_dir, exist_ok=True)

    descFile = os.path.join(dune_py_dir, 'dune.module')
    if not os.path.isfile(descFile):
        logger.info('Creating new dune-py module in ' + dune_py_dir)
        # create python/dune/generated
        generated_dir_rel = os.path.join('python','dune', 'generated')
        generated_dir = os.path.join(dune_py_dir, generated_dir_rel)
        if not os.path.isdir(generated_dir):
            os.makedirs(generated_dir)

        cmake_content = ['add_executable(generated_test EXCLUDE_FROM_ALL generated_test.cc)',
                         'add_dune_mpi_flags(generated_test)',
                         'target_compile_definitions(generated_test PRIVATE USING_DUNE_PYTHON)',
                         'target_link_libraries(generated_test PUBLIC ${DUNE_LIBS})',
                         'file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/__init__.py")',
                         '',
                         '# The builder will append rules for dynamically generated modules, here']
        project.write_cmake_file(generated_dir, cmake_content)

        with open(os.path.join(generated_dir, 'generated_test.cc'), 'w') as file:
            file.write('#include <config.h>\n\n')
            file.write('#define USING_DUNE_PYTHON 1\n\n')
            file.write('\n#include "generated_module.hh"\n')

        if deps is None:
            modules, _ = select_modules()
            deps = modules.keys()

        description = Description(module='dune-py', version=get_dune_py_version(),  maintainer='dune@lists.dune-project.org', depends=list(deps))
        logger.debug('dune-py will depend on ' + ' '.join([m + (' ' + str(c) if c else '') for m, c in description.depends]))
        project.make_project(dune_py_dir, description,
                subdirs=[generated_dir_rel], is_dunepy=True)
    else:
        description = Description(descFile)
        if description.name != 'dune-py':
            raise RuntimeError('"' + dune_py_dir + '" already contains a different dune module.')
        if description.version != get_dune_py_version():
            logger.error('"' + dune_py_dir + '" contains version ' + str(description.version) + ' of the dune-py module, ' + str(get_dune_py_version()) + ' required.')
            logger.error('If you upgraded dune-python, you can safely remove "' + dune_py_dir + '" and retry.')
            raise RuntimeError('"' + dune_py_dir + '" contains a different version of the dune-py module.')
        logger.debug('Using dune-py module in ' + dune_py_dir)

def build_dune_py_module(dune_py_dir=None, cmake_args=None, build_args=None, builddir=None, deps=None, writetagfile=False):
    if dune_py_dir is None:
        dune_py_dir = getDunePyDir()
    if cmake_args is None:
        cmake_args = defaultCMakeFlags()

    modules, dirs = select_modules()
    if deps is None:
        deps = resolve_dependencies(modules)

    desc = Description(module='dune-py', version=get_dune_py_version(),  maintainer='dune@lists.dune-project.org', depends=list(deps))

    with open(os.path.join(dune_py_dir, 'dune.module'), 'w') as file:
        file.write(repr(desc))

    # remove cache
    try:
        os.remove(os.path.join(dune_py_dir, 'CMakeCache.txt'))
    except FileNotFoundError:
        pass

    prefix = {}
    for name, dir in dirs.items():
        if is_installed(dir, name):
            found = False
            # switch prefix to location of name-config.cmake
            for l in ['lib','lib32','lib64']:
                substr = l + '/cmake'
                newpath = dir.replace('lib/dunecontrol', substr)
                for _, _, files in os.walk(newpath):
                    # if name-config.cmake is found
                    # then this is the correct folder
                    if name+'-config.cmake' in files:
                        found = True
                        prefix[name] = newpath
                        break
                if found: break
            assert found
            # store new module path
        else:
            prefix[name] = default_build_dir(dir, name, builddir)

    logger.info('Configuring dune-py module in ' + dune_py_dir)
    output = configure_module(dune_py_dir, dune_py_dir, {d: prefix[d] for d in deps}, cmake_args)
    output += build_module(dune_py_dir, build_args)

    if writetagfile:
        # set a tag file to avoid automatic reconfiguration in builder
        tagfile = os.path.join(dune_py_dir, ".noconfigure")
        f = open(tagfile, 'w')
        f.close()
    return output

def getCXXFlags():
    '''Return the CXXFLAGS used during configuration of dune-py.
       These are extracted from the CMackeCache.txt file.
    '''
    cache = os.path.join(getDunePyDir(), "CMakeCache.txt")
    matches = [match for match in [re.match('DEFAULT_CXXFLAGS:STRING=', line) for line in open(cache)] if match is not None]
    if not matches:
        return ''
    if matches.__len__() > 1:
        # TODO move this function to the generator
        import generator
        raise generator.ConfigurationError("found multiple entries for CXXFLAGS in CMakeCache.txt")
    return matches[0].string.partition('=')[2].rstrip()
