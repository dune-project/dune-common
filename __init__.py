import os

from .builder import Builder

try:
    env_force = os.environ['DUNE_FORCE_BUILD'].upper()
    if env_force == "1" or env_force == "TRUE":
        builder = Builder(True)
    else:
        builder = Builder(False)
except KeyError:
    builder = Builder(False)
