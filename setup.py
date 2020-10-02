import sys, os
from setuptools.command.build_ext import build_ext
import setuptools

# module libaries to be build
modules = ['common', 'typeregistry']

builddir = os.path.abspath(os.getcwd())

def inVEnv():
    # if sys.real_prefix exists, this is a virtualenv set up with the virtualenv package
    if hasattr(sys, 'real_prefix'):
        return 1
    # if a virtualenv is set up with pyvenv, we check for equality of base_prefix and prefix
    if hasattr(sys, 'base_prefix'):
        return (sys.prefix != sys.base_prefix)
    # if none of the above conditions triggered, this is probably no virtualenv interpreter
    return 0
def get_install_prefix():
    # test if in virtual env
    if inVEnv():
        return sys.prefix
    # generate in home directory
    try:
        home = os.path.expanduser("~")
        return os.path.join(home, '.local')
    except KeyError:
        pass
    raise RuntimeError('unable to determine location for dune-py module. please set the environment variable "dune_py_dir".')

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
            os.path.join(get_install_prefix(), 'lib')
        ],
        libraries=['dunecommon'],
        runtime_library_dirs=[
            os.path.join(get_install_prefix(), 'lib')
        ],
        language='c++'
    ) for ext in modules
]

def dunecontrol():
    optsfile = open("config.opts", "w")
    optsfile.write('CMAKE_FLAGS=\"-DCMAKE_INSTALL_PREFIX='+get_install_prefix()+
                      ' -DBUILD_SHARED_LIBS=TRUE -DDUNE_ENABLE_PYTHONBINDINGS=TRUE\"')
    optsfile.close()

    configure = './bin/dunecontrol --opts=config.opts configure'
    status = os.system(configure)
    if status != 0: raise RuntimeError(status)

    install = './bin/dunecontrol --opts=config.opts make install'
    status = os.system(install)
    if status != 0: raise RuntimeError(status)

    dunepy = os.path.join(get_install_prefix(), 'bin', 'setup-dunepy.py --opts=config.opts')
    status = os.system(dunepy)
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
    version="2.7.200001",
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
    setup_requires=['wheel', 'pybind11>=2.5.0'],
    ext_modules=ext_modules,
    cmdclass={'build_ext': BuildExt}
)
