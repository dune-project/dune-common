# This python script tries to figure out the version of a given python
# package. This is only intended to be used from DunePythonFindPackage.cmake
#
# There is no unified way of specifying the version of a python package. This
# script implements some methods. For discussion on the implemented methods see
# http://stackoverflow.com/questions/20180543
#

import sys

# Load the module passed as argument (this avoids the need for a template
# to be configured to put the package name inhere)
modstr = sys.argv[1]
module = __import__(modstr)

# The most common mechanism is module.__version__
if hasattr(module, '__version__'):
    sys.stdout.write(module.__version__)
    sys.exit(0)

# Alternative implementation: through pip (pip itself implement pip.__version__,
# so we never get here, when checking the version of pip itself), only works if
# package name and distribution name are the same
import pip
for package in pip.get_installed_distributions():
    if package.project_name == modstr and package.has_version():
        sys.stdout.write(package.version)
        sys.exit(0)

# Give up on this one
sys.exit(1)