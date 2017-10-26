import importlib
import sys

if sys.version_info.major == 2:
    def buffer_to_str(b):
        return b
else:
    def buffer_to_str(b):
        return b.decode('utf-8')


if sys.version_info.major == 2:
    def isString(s):
        return isinstance(s, (str, unicode))

    def isInteger(i):
        return isinstance(i, (int, long))
else:
    def isString(s):
        return isinstance(s, str)

    def isInteger(i):
        return isinstance(i, int)


if sys.version_info.major == 2:
    def reload_module(module):
        reload(module)
        return module
else:
    reload_module = importlib.reload

if sys.version_info.major == 2:
    from inspect import getargspec
    def getNumberOfParameters(func):
        return len( getargspec(func).args )
else:
    from inspect import signature
    def getNumberOfParameters(func):
        return len( signature(func).parameters )
