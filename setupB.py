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
    # use system default
    return None

ext_modules = [
    setuptools.Extension(
        'dune.'+ext+'._'+ext,
        sorted(['python/dune/'+ext+'/_'+ext+'.cc']),
        include_dirs=[
            os.path.join(builddir, 'build-cmake'),
            '.',
        ],
        library_dirs=[os.path.join(builddir, 'build-cmake', 'lib')]
          + [os.path.join(get_install_prefix(), 'lib')] if get_install_prefix() is not None else [],
        libraries=['dunecommon'],
        # none of these works
        extra_compile_args = [' --test -Wl,-rpath,' + get_install_prefix()+'/lib/'] \
                          if get_install_prefix() is not None else ['--test'],
        extra_link_args = [' -Wl,-rpath,' + get_install_prefix()+'/lib/'] \
                          if get_install_prefix() is not None else [],
        rpath=[]
          + [os.path.join(get_install_prefix(), 'lib')] if get_install_prefix() is not None else [],
        runtime_library_dirs=[]
          + [os.path.join(get_install_prefix(), 'lib')] if get_install_prefix() is not None else [],
        language='c++'
    ) for ext in modules
]

def dunecontrol():
    optsfile = open("config.opts", "w")
    optsfile.write('CMAKE_FLAGS=\"' + ('-DCMAKE_INSTALL_PREFIX='+get_install_prefix() if get_install_prefix() is not None else '') +
                   ' -DBUILD_SHARED_LIBS=TRUE -DDUNE_ENABLE_PYTHONBINDINGS=TRUE'+\
                   ' -DDUNE_PYTHON_INSTALL_LOCATION="none"' +\
                  '\"')
    optsfile.close()

    configure = './bin/dunecontrol --opts=config.opts configure'
    status = os.system(configure)
    if status != 0: raise RuntimeError(status)

    install = './bin/dunecontrol --opts=config.opts make install'
    status = os.system(install)
    if status != 0: raise RuntimeError(status)

    # remove existing dune-py module
    if get_install_prefix() is not None:
        os.system('rm -rf ' + os.path.join(get_install_prefix(), '.cache', 'dune-py'))
    else:
        os.system('rm -rf ' + os.path.join(os.path.expanduser('~'), '.cache', 'dune-py'))

class BuildExt(build_ext):
    def build_extensions(self):
        dunecontrol()
        for ext in self.extensions:
            ext.extra_compile_args += ['-std=c++17', '-fvisibility=hidden']
        build_ext.build_extensions(self)

with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="dune-common",
    version="2.8.200911",
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
    install_requires=['numpy'],
    ext_modules=ext_modules,
    cmdclass={'build_ext': BuildExt}
)
