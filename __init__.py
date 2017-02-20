import os

import hashlib
from .builder import Builder

def hashIt(typeName):
    if hasattr(typeName, '__iter__'):
        return hashlib.md5("".join(t for t in typeName).encode('utf-8')).hexdigest()
    else:
        return hashlib.md5(typeName.encode('utf-8')).hexdigest()

try:
    env_force = os.environ['DUNE_FORCE_BUILD'].upper()
    if env_force == "1" or env_force == "TRUE":
        builder = Builder(True)
    else:
        builder = Builder(False)
except KeyError:
    builder = Builder(False)
