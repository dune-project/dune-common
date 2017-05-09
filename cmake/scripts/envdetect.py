# A python script that determines whether the current interpreter is
# running inside a virtual environment. For discussion of the implemented
# methods, see http://stackoverflow.com/questions/1871549
#
# Meant to be run from DunePythonCommonMacros.cmake. For that reason, it
# exits with either 1 or 0, where 1 indicates that the interpreter
# runs inside a virtualenv
#

import sys

# If sys.real_prefix exists, this is a virtualenv set up with the virtualenv package
real_prefix = hasattr(sys, 'real_prefix')
if real_prefix:
    sys.exit(1)

# If a virtualenv is set up with pyvenv, we check for equality of base_prefix and prefix
if hasattr(sys, 'base_prefix'):
    sys.exit(sys.prefix != sys.base_prefix)

# If none of the above conditions triggered, this is probably no virtualenv interpreter
sys.exit(0)