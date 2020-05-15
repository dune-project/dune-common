# from dune.create import get as components

import hashlib
import importlib
import sys
from inspect import signature

def buffer_to_str(b):
    return b.decode('utf-8')

def isString(s):
    return isinstance(s, str)

def isInteger(i):
    return isinstance(i, int)

reload_module = importlib.reload

def getNumberOfParameters(func):
    return len( signature(func).parameters )

def hashIt(typeName):
    if hasattr(typeName, '__iter__'):
        return hashlib.md5("".join(t for t in typeName).encode('utf-8')).hexdigest()
    else:
        return hashlib.md5(typeName.encode('utf-8')).hexdigest()
