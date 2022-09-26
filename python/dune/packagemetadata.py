#!/usr/bin/env python3
# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception


"""Module that helps with meta data for Dune Python packages

This module implements helpers for two classes of meta data

  - Classical package meta data such as maintainer email, description, ...
  - Build meta data that is needed for the Dune generator module
    and the just-in-time compilation of C++/Python bindings

"""

# NOTE: do not import from dune.common (and consequently from dune.generator)
#       at top level to avoid failure due to missing mpi4py.

from setuptools import find_namespace_packages
import sys
import os
import io
import re
import ast
import json
import shlex
import subprocess
import email.utils
import glob
import logging
from datetime import date

logger = logging.getLogger(__name__)


class Version:
    def __init__(self, s):
        if s is None:
            self.major = 0
            self.minor = ''
            self.revision = ''
            self.build = ''
        elif isinstance(s, Version):
            self.major = s.major
            self.minor = s.minor
            self.revision = s.revision
            self.build = s.build
        else:
            match = re.match('(?P<major>[0-9]+)([.](?P<minor>[0-9*]+))?([.](?P<revision>[0-9*]+))?([.](?P<build>[a-z0-9*]+))?', s)
            if not match:
                raise ValueError('Invalid version: \'' + s + '\'.')
            self.major = int(match.group('major'))
            self.minor = match.group('minor') if match.group('minor') else ''
            self.revision = match.group('revision') if match.group('revision') else ''
            self.build = match.group('build') if match.group('build') else ''

    def __str__(self):
        s = str(self.major)
        if self.minor != '':
            s += '.' + str(self.minor)
            if self.revision != '':
                s += '.' + str(self.revision)
                if self.build != '':
                    s += '.' + str(self.build)
        return s

    def as_tuple(self):
        return (self.major, self.minor, self.revision, self.build)

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
            match = re.match(r'(?P<operator>(>|>=|==|<=|<))\s*(?P<version>[a-z0-9.*]+)', s)
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
            self.operator = lambda a, b: True
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

        validEntries = [
            'Module', 'Maintainer', 'Version', 'Maintainer',
            'Depends', 'Suggests', 'Python-Requires',
            'Whitespace-Hook',
            'Author', 'Description', 'URL',
        ]

        if fileName is not None:
            with io.open(fileName, 'r', encoding='utf-8') as file:
                import re
                for line in file:
                    line = line.strip()
                    if not line or line[0] == '#':
                        continue
                    m = re.search(r'^([a-zA-Z0-9-_]+):(.*)', line)
                    if m:
                        key = m.group(1)
                        val = m.group(2)
                        if key in validEntries:
                            data[key.lower()] = val.strip()
                        else:
                            logger.debug(f"Discarded non-standard dune.module entry {key}")
        try:
            self.name = data['module']
        except KeyError:
            raise KeyError('Module description does not contain module name.')

        self.versionstring = data.get('version')
        self.version = Version(data.get('version'))

        try:
            self.maintainer = email.utils.parseaddr(data['maintainer'])
            if not self.maintainer[1]:
                raise ValueError('Module description contains invalid maintainer e-mail address.')
        except KeyError:
            self.maintainer = None

        try:
            self.author = data['author']
        except KeyError:
            self.author = None

        try:
            self.description = data['description']
        except KeyError:
            self.description = ''

        try:
            self.url = data['url']
        except KeyError:
            self.url = None

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
                    match = re.match(r'(?P<module>[a-zA-Z0-9_\-]+)(\s*\((?P<version>[^)]*)\))?((?P<pyversion>[^\s]*))?', s)
                    if not match:
                        raise ValueError('Invalid dependency list: ' + s)
                    deps.append((match.group('module'), VersionRequirement(match.group('version') or match.group('pyversion'))))
                    s = s[match.end():].strip()
            return deps

        self.depends = parse_deps(data.get('depends'))
        self.suggests = parse_deps(data.get('suggests'))
        self.python_requires = parse_deps(data.get('python-requires'))

    def __repr__(self):
        s = 'Module:          ' + self.name + '\n'
        s += 'Version:         ' + str(self.version) + '\n'
        if self.maintainer is not None:
            s += 'Maintainer:      ' + email.utils.formataddr(self.maintainer) + '\n'
        if self.author is not None:
            s += 'Author:          ' + self.author + '\n'
        if self.description != '':
            s += 'Description:     ' + self.description + '\n'
        if self.url is not None:
            s += 'URL:             ' + self.url + '\n'
        if self.whitespace_hook is not None:
            s += 'Whitespace-Hook: ' + ('Yes' if self.whitespace_hook else 'No') + '\n'

        def print_deps(deps):
            return ' '.join([m + (' ' + str(c) if c else '') for m, c in deps])

        if self.depends:
            s += 'Depends:         ' + print_deps(self.depends) + '\n'
        if self.suggests:
            s += 'Suggests:        ' + print_deps(self.suggests) + '\n'
        if self.python_requires:
            s += 'Python-Requires: ' + print_deps(self.python_requires) + '\n'
        return s

    def __str__(self):
        return self.name + " (" + str(self.version) + ")"


class Data:
    def __init__(self, version=None):
        description = Description('dune.module')
        self.name = description.name
        self.version = version or description.versionstring
        self.author_email = description.maintainer[1]
        self.author = description.author or self.author_email
        self.description = description.description
        self.url = description.url
        self.depends = description.depends
        self.suggests = description.suggests
        self.python_requires = description.python_requires

        # if -git version parameter specified, append devDATE to version number for all DUNE modules
        if self.version.find('git') or version is not None:
            if version is None:
                major = self.version.split('-')[0]
                self.version = Version(major).__str__() + '.dev' + date.today().strftime('%Y%m%d')


            # append self.version to any dune python requirement that has no specified version number
            new_python_requires = []
            for req in self.python_requires:
                name, versionReq = req
                if (name.startswith('dune-') and not versionReq.version):
                    req = (name, '(<= '+self.version+')')
                new_python_requires += [req]
            self.python_requires = new_python_requires

            # append self.version to any dune dependency if not python-requires specifies a version number
            new_depends = []
            for dep in self.depends:
                name, versionReq = dep
                pyVersionList = [pyVersion for pyName, pyVersion in self.python_requires if pyName == name]
                if len(pyVersionList) > 0:
                    assert len(pyVersionList) == 1
                    pyVersion = pyVersionList[0]
                    print("Note:", name, "will get version requirement", pyVersion, "in pyproject.toml as stated in Python-Requires.")
                    dep = (name, str(pyVersion))
                else:
                    dep = (name, '(<= '+self.version+')')
                new_depends += [dep]
            self.depends = new_depends

    def asPythonRequirementString(self, requirements):
        return [(r[0]+str(r[1])).replace("(", " ").replace(")", "").replace(" ", "") for r in requirements]


def cmakeArguments(cmakeArgs):
    if cmakeArgs is None:
        return []
    elif isinstance(cmakeArgs, list):
        return cmakeArgs
    elif isinstance(cmakeArgs, dict):
        args = ['-D' + key + '=' + value + '' for key, value in cmakeArgs.items() if value]
        args += [key + '' for key, value in cmakeArgs.items() if not value]
        return args
    else:
        raise ValueError('definitions must be a list or a dictionary.')

def envCMakeFlags(flags=[]):
    cmakeFlags = os.environ.get('DUNE_CMAKE_FLAGS')
    # split cmakeFlags and add them to flags
    if cmakeFlags is not None:
        flags += shlex.split(cmakeFlags)
    cmakeFlags = os.environ.get('CMAKE_FLAGS')
    if cmakeFlags is not None:
        flags += shlex.split(cmakeFlags)
    return flags

def defaultCMakeFlags():
    # defaults
    flags = dict([
        ('CMAKE_CXX_FLAGS', '-O3 -DNDEBUG'),              # same as release
        ('CMAKE_INSTALL_RPATH_USE_LINK_PATH', 'TRUE'),
        ('DUNE_ENABLE_PYTHONBINDINGS', 'TRUE'),
        ('ALLOW_CXXFLAGS_OVERWRITE', 'ON'),
        ('CMAKE_DISABLE_FIND_PACKAGE_LATEX', 'TRUE'),
        ('CMAKE_DISABLE_FIND_PACKAGE_Doxygen', 'TRUE'),
        ('INKSCAPE', 'FALSE'),
    ])
    # if inVEnv():
    #     flags['DUNE_PYTHON_VIRTUALENV_PATH'] = sys.prefix
    flags = cmakeArguments(flags)  # make cmake command line out of dict
    # test environment for additional flags
    flags = envCMakeFlags(flags)
    return flags


def inVirtualEnvironment():
    # check whether we are in a anaconda environment
    # were the checks based on prefix and base_prefix
    # seem to fail
    if "CONDA_DEFAULT_ENV" in os.environ:
        return 1

    # If sys.real_prefix exists, this is a virtualenv set up with the virtualenv package
    real_prefix = hasattr(sys, 'real_prefix')
    if real_prefix:
        return 1
    # If a virtualenv is set up with pyvenv, we check for equality of base_prefix and prefix
    if hasattr(sys, 'base_prefix'):
        return (sys.prefix != sys.base_prefix)
    # If none of the above conditions triggered, this is probably no virtualenv interpreter
    return 0


def getDunePyDir():
    try:
        basedir = os.path.realpath(os.environ['DUNE_PY_DIR'])
        basedir = os.path.join(basedir, 'dune-py')
        return basedir
    except KeyError:
        pass

    if inVirtualEnvironment():
        virtualEnvPath = sys.prefix
        return os.path.join(virtualEnvPath, '.cache', 'dune-py')

    # generate in home directory
    try:
        home = os.path.expanduser("~")
        return os.path.join(home, '.cache', 'dune-py')
    except KeyError:
        pass

    raise RuntimeError('Unable to determine location for dune-py module. Please set the environment variable "DUNE_PY_DIR".')


def forceConfigure():
    # force a reconfiguration of dune-py by deleting tagfile
    tagfile = os.path.join(getDunePyDir(), ".noconfigure")
    if os.path.exists(tagfile):
        os.remove(tagfile)


def metaData(version=None, dependencyCheck=True):
    data = Data(version)

    # check if all dependencies are listed in pyproject.toml
    if dependencyCheck:
        try:
            with io.open('pyproject.toml', 'r', encoding='utf-8') as f:
                for line in f:
                    if not line.startswith("#") and 'requires' in line:
                        line = line.split('=', maxsplit=1)[1].strip()
                        modules = ast.literal_eval(line)
                        modules = [
                            x for x in modules
                            if x not in [
                                "setuptools", "wheel", "scikit-build", "cmake", "ninja", "requests"
                            ]
                        ]
                        for dep in data.depends:
                            if not any([mod.startswith(dep[0]) for mod in modules]):
                                raise RuntimeError("""
    pyproject.toml file does not contain all required dune projects defined in the
    dune.module file: """ + dep[0])

        except IOError:
            pass

    install_requires = data.asPythonRequirementString(data.python_requires + data.depends)
    install_requires = list(set(install_requires))

    try:
        with open("README.md", "r") as fh:
            long_description = fh.read()
    except FileNotFoundError:
        try:
            with open("README", "r") as fh:
                long_description = fh.read()
        except FileNotFoundError:
            long_description = 'No long description available for this package'
            print("Warning: no README[.md] file found so providing a default 'long_description' for this package")

    setupParams = {
        "name": data.name,
        "version": data.version,
        "author": data.author,
        "author_email": data.author_email,
        "description": data.description,
        "long_description": long_description,
        "long_description_content_type": "text/markdown",
        "url": data.url if data.url is not None else '',
        "classifiers": [
            "Programming Language :: C++",
            "Programming Language :: Python :: 3",
            "License :: OSI Approved :: GNU General Public License (GPL)",
        ],
        "cmake_args": defaultCMakeFlags(),
    }
    if os.path.isdir('python'):
        setupParams.update({
            "packages": find_namespace_packages(where="python"),
            "package_dir": {"": "python"},
            "install_requires": install_requires,
            "python_requires": ">=3.4",
        })

    from skbuild.command.build_py import build_py

    class DunepyConfigure(build_py):
        def run(self):
            build_py.run(self)
            subprocess.call([sys.executable, '-m', 'dune', 'configure'])

    setupParams['cmdclass'] = {
        'build_py': DunepyConfigure,
    }

    return data, setupParams


class BuildMetaData(dict):
    """Dictionary with some data processing patterns"""

    def combine_across_modules(self, key):
        return list(m[key] for m in self.values())

    def zip_across_modules(self, key, value):
        result = {}
        for moddata in self.values():
            # todo: space is bad separator for list of paths - needs
            # fixing in cmake module generating the metadata file
            for k, v in zip(moddata[key].split(" "), moddata[value].split(";")):
                # we don't store paths for module that have not been found (suggested)
                # and we also skip the path if it is empty (packaged module)
                if v.endswith("NOTFOUND") or v == "":
                    continue
                # make sure build directory (if found) is unique across modules
                if k in result and not result[k] == v:
                    raise ValueError(f"build dir {v} for module {k} is expected to be unique across the given metadata - found {result[k]}")
                result[k] = v
        return result

    def unique_value_across_modules(self, key, default=""):
        values = set(m[key] for m in self.values() if not m[key] == "")
        if len(values) > 1:
            raise ValueError(f"Key {key} is expected to be unique across the given metadata. Got {values}")
        if len(values) == 0:
            return default
        value, = values
        return value


def _loadExternalModules():
    """Check which external modules are currently registered in dune-py"""

    externalModulesPath = os.path.join(getDunePyDir(), ".externalmodules.json")
    if os.path.exists(externalModulesPath):
        with open(externalModulesPath) as externalModulesFile:
            return json.load(externalModulesFile)

    return {}


# registered external modules and their path are internally cached
_externalPythonModules = _loadExternalModules()


def getExternalPythonModules():
    """Get information on external modules

    This returns a dictionary that maps from
    the name of the external module to the module path
    """
    return _externalPythonModules


def _extractBuildMetaData():
    """ Extract meta data that was exported by CMake."""
    result = BuildMetaData()

    # add meta data of packages from the dune namespace
    def addPackageMetaData(package, metaDataFile):
        result.setdefault(package, {})
        for line in open(metaDataFile, "r"):
            try:
                key, value = line.split("=", 1)
                result[package][key] = value.strip()
            except ValueError:  # no '=' in line
                pass

    try:
        import dune.data
        for metadataPath in dune.data.__path__:
            for metaDataFile in glob.glob(os.path.join(metadataPath, "*.cmake")):
                package = os.path.splitext(os.path.basename(metaDataFile))[0]
                addPackageMetaData(package, metaDataFile)
    except ImportError:  # no dune module was installed which can happen during packaging
        pass

    # possible add meta data from externally registered modules
    for module, metadataPath in _externalPythonModules.items():
        for metaDataFile in glob.glob(os.path.join(metadataPath, "data", "*.cmake")):
            addPackageMetaData(module, metaDataFile)

    return result


# the current meta data is internally cached
_buildMetaData = _extractBuildMetaData()


def getBuildMetaData():
    """Return the current meta data object with information on all registered modules
    This returns a dictionary that maps package names to the data associated
    with the given metadata key. Currently the following metadata keys are
    exported by Python packages created with the Dune CMake build system:
    * MODULENAME: The name of the Dune module
    * BUILDDIR: The build directory of the Dune module
    * DEPS: The name of all the dependencies of the module
    * DEPBUILDDIRS: The build directories of the dependencies
    """
    return _buildMetaData


def _extractCMakeFlags():
    duneOptsFile = None
    cmakeFlags = {}
    for x in _buildMetaData.combine_across_modules("CMAKE_FLAGS"):
        for y in x.split("<SEP>"):
            try:
                k, v = y.split(":=", 1)
                v = v.strip(' "')
                if k == "DUNE_OPTS_FILE":
                    duneOptsFile = v
                else:
                    cmakeFlags[k] = v
            except ValueError:  # no '=' in line
                pass

    # add flags from some opts file
    duneOptsFile = os.environ.get('DUNE_OPTS_FILE', duneOptsFile)
    cmakeArgs = []
    if duneOptsFile:
        # TODO: check here if the duneOptsFile exists and warn if not
        #       Should be possible by checking return code of the subprocess
        #       so that other bash errors are also caught and warned about
        command = ['bash', '-c', 'source ' + duneOptsFile + ' && echo "$CMAKE_FLAGS"']
        proc = subprocess.Popen(command, stdout=subprocess.PIPE)
        stdout, _ = proc.communicate()
        cmakeArgs = shlex.split(stdout.decode('utf-8'))

    """
    # check environment variable
    cmakeArgs += shlex.split(os.environ.get('CMAKE_FLAGS', ''))

    for y in cmakeArgs:
        try:
            k, v = y.split("=", 1)
            if k.startswith('-D'):
                k = k[2:]
            cmakeFlags[k] = v.strip()
        except ValueError:  # no '=' in line
            pass
    """

    # try to unify 'ON' and 'OFF' values
    for k, v in cmakeFlags.items():
        if v.upper() in ['ON', 'TRUE', 'T', '1', 'YES', 'Y']:
            cmakeFlags[k] = True
        elif v.upper() in ['OFF', 'FALSE', 'F', '0', 'NO', 'N']:
            cmakeFlags[k] = False

    return cmakeFlags


# the CMake flags are internally cached
_cmakeFlags = _extractCMakeFlags()


def getCMakeFlags():
    """Return the currently registered CMake flags"""
    return _cmakeFlags


def registerExternalModule(moduleName, modulePath):
    """Register an external module into the dune-py machinery

        Required for modules outside the dune namespace package
        to be correctly identified as a dune module to be registered
        with the code generation module dune-py.

        Parameters are
        - the module name which is used as key in the external module dictionary
        - a path name, metadata files are searched for using 'path/data/*.cmake'
    """

    global _cmakeFlags, _buildMetaData, _externalPythonModules

    # check if this module is being registered for the first time or if the location of its metafile has changed
    if moduleName not in _externalPythonModules or modulePath != _externalPythonModules[moduleName]:

        _externalPythonModules[moduleName] = modulePath
        logger.info("Registered external module {}".format(moduleName))

        # if dune-py has already been created
        # and we are registering a new module,
        # we need to make sure that dune-py is reconfigured
        forceConfigure()

        # update metadata structures
        _buildMetaData = _extractBuildMetaData()
        _cmakeFlags = _extractCMakeFlags()
