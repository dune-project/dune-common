import sys, os
from setuptools import find_packages

try:
    from skbuild import setup
except ImportError:
    print('Please update pip, you need pip 10 or greater,\n'
          ' or you need to install the PEP 518 requirements in pyproject.toml yourself', file=sys.stderr)
    raise

with open("README.md", "r") as fh:
    long_description = fh.read()

setup(
    name="dune-common",
    version="2.8.20201123",
    author="The Dune Core developers",
    author_email="dune@lists.dune-project.org",
    description="""Basis infrastructure classes for all Dune modules""",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://gitlab.dune-project.org/core/dune-common",
    packages=find_packages(where="python"),
    package_dir={"": "python"},
    install_requires=['numpy', 'mpi4py'],
    cmake_args=['-DBUILD_SHARED_LIBS=TRUE',
                '-DDUNE_ENABLE_PYTHONBINDINGS=TRUE',
                '-DDUNE_PYTHON_INSTALL_LOCATION=none',
                '-DDUNE_GRID_GRIDTYPE_SELECTOR=ON',
                '-DALLOW_CXXFLAGS_OVERWRITE=ON',
                '-DUSE_PTHREADS=ON',
                '-DCMAKE_BUILD_TYPE=Release',
                '-DCMAKE_DISABLE_FIND_PACKAGE_LATEX=TRUE',
                '-DCMAKE_DISABLE_DOCUMENTATION=TRUE',
                '-DCMAKE_INSTALL_RPATH='+sys.prefix+'/lib/']
)
