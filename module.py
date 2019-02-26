from __future__ import absolute_import, division, print_function, unicode_literals

import email.utils
import io
import logging
import os
import re
import shlex
import string
import subprocess
import sys

from os.path import expanduser

if __name__ == "dune.common.module":
    from dune.common.compatibility import buffer_to_str
    from dune.common import project
else:
    from compatibility import buffer_to_str
    import project

logger = logging.getLogger(__name__)

class Version:
    def __init__(self, s):
        if s is None:
            self.major = 0
            self.minor = 0
            self.revision = 0
        elif isinstance(s, Version):
            self.major = s.major
            self.minor = s.minor
            self.revision = s.revision
        else:
            match = re.match('(?P<major>[0-9]+)[.](?P<minor>[0-9]+)([.](?P<revision>[0-9]+))?', s)
            if not match:
                raise ValueError('Invalid version: \'' + s + '\'.')
            self.major = int(match.group('major'))
            self.minor = int(match.group('minor'))
            self.revision = int(match.group('revision')) if match.group( 'revision' ) else 0

    def __str__(self):
        return str(self.major) + '.' + str(self.minor) + '.' + str(self.revision)

    def as_tuple(self):
        return (self.major, self.minor, self.revision)

    def __eq__(self, other):
        return self.as_tuple() == other.as_tuple()

    def __ne__(self, other):
        return self.as_tuple() != other.as_tuple()

    def __lt__(self, other):
        return self.as_tuple() < other.as_tuple()

    def __le__(self, other):
        return self.as_tuple() <= other.as_tuple()

    def __gt__(self, other):
        return self.as_tuple() > other.as_tuple()

    def __ge__(self, other):
        return self.as_tuple() >= other.as_tuple()


class VersionRequirement:
    def __init__(self, s):
        if s:
            match = re.match('(?P<operator>(>|>=|==|<=|<))\s*(?P<version>[0-9.]+)', s)
            if not match:
                raise ValueError('Invalid version qualifier: \'' + s + '\'.')
            self.version = Version(match.group('version'))
            operator = match.group('operator')

            if operator == '>':
                self.operator = Version.__gt__
            elif operator == '>=':
                self.operator = Version.__ge__
            elif operator == '==':
                self.operator = Version.__eq__
            elif operator == '<=':
                self.operator = Version.__le__
            elif operator == '<':
                self.operator = Version.__lt__
            else:
                raise ValueError('Invalid comparison operator: \'' + operator + '\'.')
        else:
            self.operator = lambda a, b : True
            self.version = None

    def __bool__(self):
        return self.version is not None

    __nonzero__ = __bool__

    def __call__(self, version):
        return self.operator(version, self.version)

    def __repr__(self):
        return str(self)

    def __str__(self):
        if self.operator == Version.__gt__:
            return '(> ' + str(self.version) + ')'
        elif self.operator == Version.__ge__:
            return '(>= ' + str(self.version) + ')'
        elif self.operator == Version.__eq__:
            return '(== ' + str(self.version) + ')'
        elif self.operator == Version.__le__:
            return '(<= ' + str(self.version) + ')'
        elif self.operator == Version.__lt__:
            return '(< ' + str(self.version) + ')'
        else:
            return ''


class Description:
    def __init__(self, fileName=None, **kwargs):
        data = kwargs.copy()
        if fileName is not None:
            with io.open(fileName, 'r', encoding='utf-8') as file:
                for line in file:
                    line = line.strip()
                    if not line or line[ 0 ] == '#':
                        continue

                    pos = line.find(':')
                    if pos < 0:
                        raise ValueError('Invalid key:value pair (' + line + ').')
                    data[line[:pos].strip().lower()] = line[pos+1:].strip()

        try:
            self.name = data['module']
        except KeyError:
            raise KeyError('Module description does not contain module name.')

        self.version = Version(data.get('version'))

        try:
            self.maintainer = email.utils.parseaddr(data['maintainer'])
            if not self.maintainer[1]:
                raise ValueError('Module description contains invalid maintainer e-mail address.')
        except KeyError:
            self.maintainer = None

        try:
            wshook = data['whitespace-hook'].lower()
            if wshook == 'yes':
                self.whitespace_hook = True
            elif wshook == 'no':
                self.whitespace_hook = False
            else:
              raise ValueError('Invalid value for whitespace-hook: ' + wshook + '.')
        except KeyError:
            self.whitespace_hook = None

        def parse_deps(s):
            deps = []
            if isinstance(s, list):
                for m in s:
                    if isinstance(m, Description):
                        deps.append((m.name, VersionRequirement(None)))
                    else:
                        deps.append((m, VersionRequirement(None)))
            else:
                while s:
                    match = re.match('(?P<module>[a-zA-Z0-9_\-]+)(\s*\((?P<version>[^)]*)\))?', s)
                    if not match:
                        raise ValueError('Invalid dependency list.')
                    deps.append((match.group('module'), VersionRequirement(match.group('version'))))
                    s = s[match.end():].strip()
            return deps

        self.depends = parse_deps(data.get('depends'))
        self.suggests = parse_deps(data.get('suggests'))

    def __repr__(self):
        s = 'Module:          ' + self.name + '\n'
        s += 'Version:         ' + str(self.version) + '\n'
        if self.maintainer is not None:
            s += 'Maintainer:      ' + email.utils.formataddr(self.maintainer) + '\n'
        if self.whitespace_hook is not None:
            s += 'Whitespace-Hook: ' + ('Yes' if self.whitespace_hook else 'No') + '\n'

        def print_deps(deps):
            return ' '.join([m + (' ' + str(c) if c else '') for m, c in deps])

        if self.depends:
            s += 'Depends:         ' + print_deps(self.depends) + '\n'
        if self.suggests:
            s += 'Suggests:        ' + print_deps(self.suggests) + '\n'
        return s

    def __str__(self):
        return self.name + " (" + str(self.version) + ")"


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
            raise ValueError('Module \'' + module.name + '\' requires ' + desc.name + ' ' + req + '.')
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
       Ordred list of module names such that each module only depends on
       modules preceeding it.
    """
    order = []

    def resolve(m):
        if m not in order:
            for d in deps[m]:
                if d not in order:
                    resolve(d)
            order.append(m)

    for m in deps:
        resolve(m)
    return order


def pkg_config(pkg, var=None):
    args = ['pkg-config', pkg]
    if var is not None:
        args += ['--variable=' + var]
    pkgconfig = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    pkgconfig.wait()
    if pkgconfig.returncode != 0:
        raise KeyError('package ' + pkg + 'not found.')
    return buffer_to_str(pkgconfig.stdout.read())


def get_prefix(module):
    return pkg_config(module, 'prefix')


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
        return dir == os.path.join(get_prefix(module), 'lib', 'dunecontrol', module)
    except KeyError:
        return False

def get_cmake_command():
    try:
        return os.environ['DUNE_CMAKE']
    except KeyError:
        return 'cmake'

def get_module_path():
    try:
        path = [p for p in os.environ['DUNE_CONTROL_PATH'].split(':') if p and os.path.isdir(p)]
        logger.debug('Module path [DUNE_CONTROL_PATH]: ' + ':'.join(path))
        return path
    except KeyError:
        pass

    # try to guess module path using pkg-config
    try:
        prefix = pkg_config('dune-common', 'prefix').strip()
        path = [p for p in ['.', os.path.join(prefix, 'lib', 'dunecontrol')] if os.path.isdir(p)]
        logger.debug('Module path [pkg-config]: ' + ':'.join(path))
        return path
    except KeyError:
        pass

    # try to guess modules path for unix systems
    path = [p for p in ['.', '/usr/local/lib/dunecontrol', '/usr/lib/dunecontrol'] if os.path.isdir(p)]
    try:
        pkg_config_path = [p for p in os.environ['PKG_CONFIG_PATH'].split(':') if p and os.path.isdir(p)]
        pkg_config_path = [os.path.join(p, '..', 'dunecontrol') for p in pkg_config_path]
        path += [p for p in pkg_config_path if os.path.isdir(p)]
    except KeyError:
        pass

    logger.debug('Module path [guessed]: ' + ':'.join(path))
    return path


def select_modules(modules=None):
    """choose one version of each module from a list of modules

    Args:
        modules (optional): List of (description, dir) pairs
            If not given, the find_modules(get_module_path()) is used

    Returns:
        pair of dictionaries mapping module name to unique description and directory respectively
    """
    if modules is None:
        modules = find_modules(get_module_path())
    desc = {}
    dir = {}
    for d, p in modules:
        n = d.name
        if n in dir:
            if is_installed(dir[n], n):
                if is_installed(p, n):
                    raise KeyError('Multiple installed versions for module \'' + n + '\' found.')
                else:
                  desc[n], dir[n] = d, p
            else:
              if not is_installed(d, n):
                  raise KeyError('Multiple source versions for module \'' + n + '\' found.')
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


def configure_module(srcdir, builddir, prefix_dirs, definitions=None):
    """configure a given module by running CMake

    Args:
        srcdir:                  source directory of module
        builddir:                build directory for module (may equal srcdir for in-source builds)
        prefix_dirs:             dictionary mapping dependent modules to their prefix
        definitions (optional):  dictionary of additional CMake definitions

    Returns:
        Output of CMake command
    """
    args = [ get_cmake_command() ]
    if definitions is None:
        pass
    elif isinstance(definitions, dict):
        args += ['-D' + key + '=' + value + '' for key, value in definitions.items() if value]
        args += [key + '' for key, value in definitions.items() if not value]
    else:
        raise ValueError('definitions must be a dictionary.')
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


def get_dune_py_dir():
    try:
        basedir = os.path.realpath( os.environ['DUNE_PY_DIR'] )
        basedir = os.path.join(basedir,'dune-py')
        return basedir
    except KeyError:
        pass

    # test if in virtual env
    try:
        realPath = sys.real_prefix
        virtualEnvPath = sys.prefix
        return os.path.join(virtualEnvPath, '.cache', 'dune-py')
    except AttributeError:
        pass

    # generate in home directory
    try:
        home = expanduser("~")
        return os.path.join(home, '.cache', 'dune-py')
    except KeyError:
        pass

    raise RuntimeError('Unable to determine location for dune-py module. Please set the environment variable "DUNE_PY_DIR".')


def get_dune_py_version():
    # change this version on the following events:
    # - a release (major version numbers)
    # - any incompatible change to the dune-py module (revison number)
    return Version("2.6.0")


def get_cmake_definitions():
    definitions = {}
    try:
        for arg in shlex.split(os.environ['DUNE_CMAKE_FLAGS']):
            try:
                key, value = arg.split('=', 1)
                if key.startswith('-D'):
                    key = key[2:]
            except ValueError:
                key, value = arg, None
            definitions[key] = value
    except KeyError:
        pass
    return definitions


def make_dune_py_module(dune_py_dir=None):
    if dune_py_dir is None:
        dune_py_dir = get_dune_py_dir()
    descFile = os.path.join(dune_py_dir, 'dune.module')
    if not os.path.isfile(descFile):
        logger.info('Creating new dune-py module in ' + dune_py_dir)
        if not os.path.isdir(dune_py_dir):
            os.makedirs(dune_py_dir)

        # create python/dune/generated
        generated_dir_rel = os.path.join('python','dune', 'generated')
        generated_dir = os.path.join(dune_py_dir, generated_dir_rel)
        if not os.path.isdir(generated_dir):
            os.makedirs(generated_dir)

        cmake_content = ['add_executable(generated_test EXCLUDE_FROM_ALL generated_test.cc)',
                         'add_dune_mpi_flags(generated_test)',
                         'target_compile_definitions(generated_test PRIVATE USING_DUNE_PYTHON)',
                         'target_link_libraries(generated_test ${DUNE_LIBS})',
                         'file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/__init__.py")',
                         '',
                         '# The builder will append rules for dynamically generated modules, here']
        project.write_cmake_file(generated_dir, cmake_content)

        with open(os.path.join(generated_dir, 'generated_test.cc'), 'w') as file:
            file.write('#include <config.h>\n\n')
            file.write('#define USING_DUNE_PYTHON 1\n\n')
            file.write('\n#include "generated_module.hh"\n')

        modules, _ = select_modules()
        description = Description(module='dune-py', version=get_dune_py_version(),  maintainer='dune@lists.dune-project.org', depends=list(modules.values()))
        logger.debug('dune-py will depend on ' + ' '.join([m + (' ' + str(c) if c else '') for m, c in description.depends]))
        project.make_project(dune_py_dir, description, subdirs=[generated_dir])
    else:
        description = Description(descFile)
        if description.name != 'dune-py':
            raise RuntimeError('"' + dune_py_dir + '" already contains a different dune module.')
        if description.version != get_dune_py_version():
            logger.error('"' + dune_py_dir + '" contains version ' + str(description.version) + ' of the dune-py module, ' + str(get_dune_py_version()) + ' required.')
            logger.error('If you upgraded dune-python, you can safely remove "' + dune_py_dir + '" and retry.')
            raise RuntimeError('"' + dune_py_dir + '" contains a different version of the dune-py module.')
        logger.info('Using existing dune-py module in ' + dune_py_dir)


def build_dune_py_module(dune_py_dir=None, definitions=None, build_args=None, builddir=None):
    if dune_py_dir is None:
        dune_py_dir = get_dune_py_dir()
    if definitions is None:
        definitions = get_cmake_definitions()

    desc = Description(os.path.join(dune_py_dir, 'dune.module'))

    modules, dirs = select_modules()
    deps = resolve_dependencies(modules, desc)

    prefix = {}
    for name, dir in dirs.items():
        if is_installed(dir, name):
            prefix[name] = get_prefix(name)
        else:
            prefix[name] = default_build_dir(dir, name, builddir)

    output = configure_module(dune_py_dir, dune_py_dir, {d: prefix[d] for d in deps}, definitions)
    output += build_module(dune_py_dir, build_args)
    return output
