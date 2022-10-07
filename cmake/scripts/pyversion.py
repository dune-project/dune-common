# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#
# This python script tries to figure out the version of a given python
# package. This is only intended to be used from DunePythonFindPackage.cmake
#
# There is no unified way of specifying the version of a python package. This
# script implements some methods. For discussion on the implemented methods see
# http://stackoverflow.com/questions/20180543
#

import sys
import warnings

# Load the module name passed as argument (this avoids the need for a template
# to be configured to put the package name inhere)
modstr = sys.argv[1]

with warnings.catch_warnings():
    # suppress any warnings which may be raised on import (e.g. virtualenv 20.2.2)
    warnings.simplefilter("ignore")

    module = __import__(modstr)
    # The most common mechanism is module.__version__
    if hasattr(module, '__version__'):
        sys.stdout.write(module.__version__)
        sys.exit(0)

# Alternative implementation: through pip (pip itself implement pip.__version__,
# so we never get here, when checking the version of pip itself), only works if
# package name and distribution name are the same
import pkg_resources
for package in pkg_resources.working_set:
    if package.project_name == modstr and package.has_version():
        sys.stdout.write(package.version)
        sys.exit(0)

# Give up on this one
sys.exit(1)
