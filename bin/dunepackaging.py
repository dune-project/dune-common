#!/usr/bin/env python3

import sys, os, io, getopt, re
import importlib, subprocess
import email.utils
import pkg_resources
from datetime import date
import json

from dune.dunepackaging import metaData

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

    data, cmake_flags = metaData(version)

    # Generate setup.py
    print("Generate setup.py")
    f = open("setup.py", "w")
    f.write("from dune.dunepackaging import metaData\n")
    f.write("from skbuild import setup\n")
    f.write("setup(**metaData()[1])\n")
    f.close()

    # Generate pyproject.toml
    print("Generate pyproject.toml")
    f = open("pyproject.toml", "w")
    requires = ["setuptools", "wheel", "scikit-build", "cmake", "ninja"]
    requires += data.dune_modules
    f.write("[build-system]\n")
    f.write("requires = "+json.dumps(requires)+"\n")
    f.write("build-backend = \"setuptools.build_meta\"\n")
    f.close()


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
