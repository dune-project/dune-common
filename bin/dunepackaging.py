#!/usr/bin/env python3

import sys, os, io, getopt, re
import importlib, subprocess
import email.utils
import pkg_resources
from datetime import date

from dune.common.module import Version, VersionRequirement, Description
class Data:
    def __init__(self):
        description = Description('dune.module')
        self.name = description.name
        self.version = str(description.version)
        self.author_email = description.maintainer[1]
        self.author = description.author or self.author_email
        self.description = description.description
        self.url = description.url
        self.dune_dependencies = [
                (dep[0]+str(dep[1])).replace("("," ").replace(")","")
                for dep in description.depends
             ]

        self.install_requires = []
        try:
            with open('python/setup.py.in', 'r') as setuppyinfile:
                content = setuppyinfile.read()
                if content.find('install_requires'):
                    bracket = content.split('install_requires')[1].split('[')[1].split(']')[0]
                    self.install_requires = [r.strip('\'"') for r in bracket.split(',')]
        except FileNotFoundError:
            pass

def main(argv):

    repositories = ["gitlab", "testpypi", "pypi"]
    def usage():
        return 'usage: dunepackaging.py [--upload <'+"|".join(repositories)+'> | -c | --clean | --version 1.0.DATE>]'

    try:
        opts, args = getopt.getopt(argv, "hc", ["upload=", "clean", "version="])
    except getopt.GetoptError:
        print(usage())
        sys.exit(2)

    sdist = True
    upload = False
    repository = "gitlab"
    clean = False
    version = None
    for opt, arg in opts:
        if opt == '-h':
            print(usage())
            sys.exit(2)
        elif opt in ("--upload"):
            upload = True
            if arg != '':
                repository = arg
                if repository not in repositories:
                    print("Specified repository must be one of: " + " ".join(repositories))
                    sys.exit(2)
        elif opt in ("-c", "--clean"):
            clean = True
        elif opt in ("--version"):
            version = arg

    # Remove generated files
    def removeFiles():
        import glob
        files = ['setup.py', 'MANIFEST', 'pyproject.toml', 'dist', '_skbuild', '__pycache__']
        print("Remove generated files: " + ", ".join(files))
        remove = ['rm', '-rf'] + files
        subprocess.call(remove)

    if clean:
        removeFiles()
        if not upload:
            sys.exit(2)

    data = Data()

    # defaults
    if not hasattr(data, 'dune_dependencies'):
        data.dune_dependencies = []

    if not hasattr(data, 'install_requires'):
        data.install_requires = []

    # if no version parameter specified, append DATE to version number in package.py
    if version is None:
        if not hasattr(data, 'version'):
            print("No version number specified!")
            sys.exit(2)
        version = data.version + '.devDATE'

    # version - replacing "DATE" with yearmonthday string
    t = date.today()
    today = t.strftime('%Y%m%d')
    data.version = version.replace("DATE",today)

    # Generate setup.py
    print("Generate setup.py")
    setuppy = '''\
import sys, os
from setuptools import find_packages
from skbuild import setup

with open("README.md", "r") as fh:
    long_description = fh.read()
'''
    setuppy += '''
setup(
'''
    setuppy += '    name="'+data.name+'",\n'
    setuppy += '    version="'+data.version+'",\n'
    setuppy += '    author="'+data.author+'",\n'
    setuppy += '    author_email="'+data.author_email+'",\n'
    setuppy += '    description="'+data.description+'",\n'
    setuppy += '    long_description=long_description,\n'
    setuppy += '    long_description_content_type="text/markdown",\n'
    if data.url is not None:
        setuppy += '    url="'+data.url+'",\n'
    setuppy += '    packages=find_packages(where="python"),\n'
    setuppy += '    package_dir={"": "python"},\n'
    setuppy += '    install_requires='+(data.install_requires+data.dune_dependencies).__str__()+','
    setuppy += '''
    classifiers=[
        "Programming Language :: C++",
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: GNU General Public License (GPL)",
    ],
    python_requires='>=3.4',
    cmake_args=[
        '-DBUILD_SHARED_LIBS=TRUE',
        '-DDUNE_ENABLE_PYTHONBINDINGS=TRUE',
        '-DDUNE_PYTHON_INSTALL_LOCATION=none',
        '-DDUNE_GRID_GRIDTYPE_SELECTOR=ON',
        '-DALLOW_CXXFLAGS_OVERWRITE=ON',
        '-DUSE_PTHREADS=ON',
        '-DCMAKE_BUILD_TYPE=Release',
        '-DCMAKE_DISABLE_FIND_PACKAGE_LATEX=TRUE',
        '-DCMAKE_DISABLE_DOCUMENTATION=TRUE',
        '-DINKSCAPE=FALSE',
        '-DCMAKE_INSTALL_RPATH='+sys.prefix+'/lib/',
        '-DCMAKE_MACOSX_RPATH=TRUE',
    ]
)
'''
    f = open("setup.py", "w")
    f.write(setuppy)
    f.close()

    # Generate pyproject.toml
    print("Generate pyproject.toml")
    f = open("pyproject.toml", "w")
    requires = ["setuptools", "wheel", "scikit-build", "cmake", "ninja"]
    requires += data.dune_dependencies
    f.write("[build-system]\n")
    f.write("requires = "+requires.__str__()+"\n")
    f.write("build-backend = 'setuptools.build_meta'\n")
    f.close()

    # Generate MANIFEST
    with open('MANIFEST', 'wb') as manifest_file:
        manifest_file.write("setup.py\n".encode())
        manifest_file.write("pyproject.toml\n".encode())
        manifest_file.write(
             subprocess.check_output(['git', 'ls-files'])
        )

    # Create source distribution
    python = sys.executable
    if sdist:
        print("Remove dist")
        remove = ['rm', '-rf', 'dist']
        subprocess.call(remove)

        # check if we have scikit-build
        import pkg_resources
        installed = {pkg.key for pkg in pkg_resources.working_set}
        if not 'scikit-build' in installed:
            print("Please install the pip package 'scikit-build' to build the source distribution.")
            sys.exit(2)

        print("Create source distribution")
        build = [python, 'setup.py', 'sdist']
        subprocess.call(build, stdout=subprocess.DEVNULL)

    # Upload to repository
    if upload:
        # check if we have twine
        import pkg_resources
        installed = {pkg.key for pkg in pkg_resources.working_set}
        if not 'twine' in installed:
            print("Please install the pip package 'twine' to upload the source distribution.")
            sys.exit(2)

        twine = [python, '-m', 'twine', 'upload']
        twine += ['--repository', repository]
        twine += ['dist/*']
        subprocess.call(twine)

        removeFiles()


if __name__ == "__main__":
    main(sys.argv[1:])
