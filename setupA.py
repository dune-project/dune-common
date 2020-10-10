import sys, os
from setuptools.command.install import install
import setuptools

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

def dunecontrol():
    CXXFLAGS = (' -Wl,-rpath,'+get_install_prefix()+'/lib/' if get_install_prefix() is not None else '')
    CMAKEFLAGS = ' -DBUILD_SHARED_LIBS=TRUE -DDUNE_ENABLE_PYTHONBINDINGS=TRUE' +\
                   (' -DCMAKE_INSTALL_PREFIX='+get_install_prefix() if get_install_prefix() is not None else '') +\
                 ' -DCMAKE_CXX_FLAGS="'+CXXFLAGS+'"'
    optsfile = open("config.opts", "w")
    optsfile.write('CMAKE_FLAGS=\'' + CMAKEFLAGS + '\'')
    optsfile.close()
    print(CMAKEFLAGS)

    configure = './bin/dunecontrol --opts=config.opts configure'
    status = os.system(configure)
    if status != 0:
        print("Could not execute script possibly the TMPDIR is mounted 'noexec'.")
        print("Try rerunning the install command after setting the environment variable `TMPDIR` to a folder which has execution privileges")
        raise RuntimeError(status)

    install = './bin/dunecontrol --opts=config.opts make install'
    status = os.system(install)
    if status != 0: raise RuntimeError(status)

    # remove existing dune-py module
    if get_install_prefix() is not None:
        os.system('rm -rf ' + os.path.join(get_install_prefix(), '.cache', 'dune-py'))
    else:
        os.system('rm -rf ' + os.path.join(os.path.expanduser('~'), '.cache', 'dune-py'))

class DuneControl(install):
    def run(self):
        dunecontrol()
        setuptools.command.install.install.run(self)

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
    classifiers=[
        "Programming Language :: C++",
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: GNU General Public License (GPL)",
    ],
    python_requires='>=3.4',
    install_requires=['numpy'],
    cmdclass={'install': DuneControl}
)
