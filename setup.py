import sys, os
from setuptools.command.build_ext import build_ext
import setuptools

# module libaries to be build
modules = ['common', 'typeregistry']

builddir = os.path.abspath(os.getcwd())

class get_pybind_include(object):
    def __str__(self):
        import pybind11
        return pybind11.get_include()

ext_modules = [
    setuptools.Extension(
        'dune.'+ext+'._'+ext,
        sorted(['python/dune/'+ext+'/_'+ext+'.cc']),
        include_dirs=[
            get_pybind_include(),
            os.path.join(builddir, 'build-cmake'),
            '.',
        ],
        library_dirs=[
            os.path.join(builddir, 'build-cmake', 'lib'),
        ],
        libraries=['dunecommon'],
        language='c++'
    ) for ext in modules
]

def dunecontrol():
    options = ['--all-opts=\'CMAKE_FLAGS=\"-DBUILD_SHARED_LIBS=TRUE -DDUNE_ENABLE_PYTHONBINDINGS=ON\"\'']
    command = ['./bin/dunecontrol'] + options + ['all']
    status = os.system(" ".join(command))
    if status != 0: raise RuntimeError(status)


class BuildExt(build_ext):
    def build_extensions(self):
        dunecontrol()
        for ext in self.extensions:
            ext.extra_compile_args = ['-std=c++17', '-fvisibility=hidden']
        build_ext.build_extensions(self)

with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="dune-common",
    version="2.8",
    author="The Dune Core developers",
    author_email="dune@lists.dune-project.org",
    description="Basis infrastructure classes for all Dune modules",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://gitlab.dune-project.org/core/dune-common",
    packages=[
        'dune.common',
        'dune.generator',
        'dune.typeregistry'
    ],
    package_dir={
        'dune.common':       'python/dune/common',
        'dune.generator':    'python/dune/generator',
        'dune.typeregistry': 'python/dune/typeregistry'
    },
    classifiers=[
        "Programming Language :: C++",
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: GNU General Public License (GPL)",
    ],
    python_requires='>=3.4',
    setup_requires=['pybind11>=2.5.0'],
    ext_modules=ext_modules,
    scripts=['bin/dunecontrol', 'bin/dune-git-whitespace-hook'],
    libs=['lib/dunemodules.lib'],
    cmdclass={'build_ext': BuildExt},
)
