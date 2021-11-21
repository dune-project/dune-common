import importlib
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
