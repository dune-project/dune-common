import os

from .builder import Builder

env_force = os.environ.get('DUNE_FORCE_BUILD', 'FALSE').upper()
if env_force in ('1', 'TRUE'):
    builder = Builder(True)
else:
    builder = Builder(False)
