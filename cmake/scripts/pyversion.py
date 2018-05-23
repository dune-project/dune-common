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

# NOTE: pip itself implements pip.__version__, so we never get here, when
# checking the version of pip itself

# Alternative implementation: use pkg_resources
import pkg_resources

# Generate a dict of distribution information with project names as keys
dist_info = {d.project_name: d for d in pkg_resources.working_set}

# Check if package is available at all
if modstr in dist_info:
    # Check if there is version information and, if yes, write it to stdout
    pkg_info = dist_info[modstr]

    if pkg_info.has_version():
        sys.stdout.write(pkg_info.version)
        sys.exit(0)

# If this point is reached, no version information could be extracted
sys.exit(1)
