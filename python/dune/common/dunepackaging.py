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
    return data, cmake_flags
