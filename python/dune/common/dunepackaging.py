#!/usr/bin/env python3

from setuptools import find_packages
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
                (dep[0]+str(dep[1])).replace("("," ").replace(")","")+".dev0"
                for dep in description.depends
             ]
        self.install_requires = [
                (dep[0]+str(dep[1])).replace("("," ").replace(")","")
                for dep in description.python_requires
             ]

def metaData(version=None):
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

    cmake_flags= [
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

    requires=data.install_requires+data.dune_dependencies

    with open("README.md", "r") as fh:
        long_description = fh.read()

    setupParams = {
        "name":data.name,
        "version":data.version,
        "author":data.author,
        "author_email":data.author_email,
        "description":data.description,
        "long_description":long_description,
        "long_description_content_type":"text/markdown",
        "url":data.url if data.url is not None else '',
        "packages":find_packages(where="python"),
        "package_dir":{"": "python"},
        "install_requires":requires,
        "classifiers":[
            "Programming Language :: C++",
            "Programming Language :: Python :: 3",
            "License :: OSI Approved :: GNU General Public License (GPL)",
        ],
        "python_requires":'>=3.4',
        "cmake_args":cmake_flags
    }

    return data, setupParams
