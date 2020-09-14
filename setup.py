import setuptools

class get_pybind_include(object):
    """Helper class to determine the pybind11 include path
    The purpose of this class is to postpone importing pybind11
    until it is actually installed, so that the ``get_include()``
    method can be invoked. """

    def __str__(self):
        import pybind11
        return pybind11.get_include()

ext_modules = [
    setuptools.Extension(
        'dune.common_',
        # Sort input source files to ensure bit-for-bit reproducible builds
        # (https://github.com/pybind/python_example/pull/53)
        sorted(['python/dune/common/_common.cc']),
        include_dirs=[
            # Path to pybind11 headers
            get_pybind_include(),
        ],
        language='c++'
    ),
]

with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="dune-common", # Replace with your own username
    version="2.8",
    author="The Dune Core developers",
    author_email="dune@lists.dune-project.org",
    description="Basis infrastructure classes for all Dune modules",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://gitlab.dune-project.org/core/dune-common",
    packages=['dune'],
    package_dir={'dune': 'python/dune'},
    classifiers=[
        "Programming Language :: C++",
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: GNU General Public License (GPL)",
    ],
    python_requires='>=3.4',
    setup_requires=['pybind11>=2.5.0'],
    ext_modules=ext_modules,
)
