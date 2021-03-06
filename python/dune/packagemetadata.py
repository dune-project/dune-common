#!/usr/bin/env python3

from setuptools import find_packages
import sys, os, io, getopt, re, ast
import shlex
import importlib, subprocess
import email.utils
import pkg_resources
from datetime import date

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

        valid_entries = ['Module','Maintainer','Version','Maintainer',
                         'Depends','Suggests','Python-Requires',
                         'Whitespace-Hook',
                         'Author','Description','URL']

        if fileName is not None:
            with io.open(fileName, 'r', encoding='utf-8') as file:
                import re
                for line in file:
                    line = line.strip()
                    if not line or line[ 0 ] == '#':
                        continue
                    m = re.search(r'^([a-zA-Z0-9-_]+):(.*)', line)
                    if m:
                        key = m.group(1)
                        val = m.group(2)
                        if not key in valid_entries:
                            raise ValueError('Invalid dune.module entry %s (%s).' % (key,fileName))
                        data[key.lower()] = val.strip()
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
                    match = re.match('(?P<module>[a-zA-Z0-9_\-]+)(\s*\((?P<version>[^)]*)\))?((?P<pyversion>[^\s]*))?', s)
                    if not match:
                        raise ValueError('Invalid dependency list.')
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
            self.depends = [(dep[0], '(<= '+self.version+')') for dep in self.depends]
            self.python_requires = [((pr[0], '(<= '+self.version+')') if pr[0].startswith('dune-') else pr) for pr in self.python_requires]

    def asPythonRequirementString(self, requirements):
        return [(r[0]+str(r[1])).replace("("," ").replace(")","").replace(" ","") for r in requirements]

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

def cmakeFlags():
    # defaults
    flags = cmakeArguments(dict([
        ('CMAKE_BUILD_TYPE','Release'),
        ('CMAKE_INSTALL_RPATH_USE_LINK_PATH','TRUE'),
        ('DUNE_ENABLE_PYTHONBINDINGS','TRUE'),
        ('DUNE_PYTHON_INSTALL_LOCATION','none'),
        ('ALLOW_CXXFLAGS_OVERWRITE','ON'),
        ('CMAKE_DISABLE_FIND_PACKAGE_LATEX','TRUE'),
        ('CMAKE_DISABLE_FIND_PACKAGE_Doxygen','TRUE'),
        ('INKSCAPE','FALSE')
    ]))
    # test environment for additional flags
    cmakeFlags = os.environ.get('DUNE_CMAKE_FLAGS')
    # split cmakeFlags and add them to flags
    if cmakeFlags is not None:
        flags += shlex.split(cmakeFlags)
    cmakeFlags = os.environ.get('CMAKE_FLAGS')
    if cmakeFlags is not None:
        flags += shlex.split(cmakeFlags)
    return flags

def inVEnv():
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

def get_dune_py_dir():
    try:
        basedir = os.path.realpath( os.environ['DUNE_PY_DIR'] )
        basedir = os.path.join(basedir,'dune-py')
        return basedir
    except KeyError:
        pass

    # test if in virtual env
    if inVEnv():
        virtualEnvPath = sys.prefix
        return os.path.join(virtualEnvPath, '.cache', 'dune-py')

    # generate in home directory
    try:
        home = os.path.expanduser("~")
        return os.path.join(home, '.cache', 'dune-py')
    except KeyError:
        pass

    raise RuntimeError('Unable to determine location for dune-py module. Please set the environment variable "DUNE_PY_DIR".')

def metaData(version=None, dependencyCheck=True):
    data = Data(version)

    cmake_flags = cmakeFlags()

    # check if all dependencies are listed in pyproject.toml
    if dependencyCheck:
        try:
            with io.open('pyproject.toml', 'r', encoding='utf-8') as f:
                for line in f:
                    if 'requires' in line:
                        line = line.split('=',maxsplit=1)[1].strip()
                        modules = ast.literal_eval(line)
                        modules = [x for x in modules
                                      if x not in ["setuptools", "wheel", "scikit-build", "cmake", "ninja", "requests"]
                                  ]
                        for dep in data.depends:
                            if not any([mod.startswith(dep[0]) for mod in modules]):
                                raise RuntimeError("""
    pyproject.toml file does not contain all required dune projects defined in the
    dune.module file: """ + dep[0])

        except IOError:
            pass

    install_requires = data.asPythonRequirementString(data.python_requires + data.depends)

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
        "name":data.name,
        "version":data.version,
        "author":data.author,
        "author_email":data.author_email,
        "description":data.description,
        "long_description":long_description,
        "long_description_content_type":"text/markdown",
        "url":data.url if data.url is not None else '',
        "classifiers":[
            "Programming Language :: C++",
            "Programming Language :: Python :: 3",
            "License :: OSI Approved :: GNU General Public License (GPL)",
        ],
        "cmake_args":cmake_flags
      }
    if os.path.isdir('python'):
      setupParams.update({
            "packages":find_packages(where="python"),
            "package_dir":{"": "python"},
            "install_requires":install_requires,
            "python_requires":'>=3.4',
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
