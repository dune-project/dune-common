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
else:
    def isString(s):
        return isinstance(s, str)


if sys.version_info.major == 2:
    def reload_module(module):
        reload(module)
        return module
else:
    reload_module = importlib.reload
