import os, sys
here = os.path.dirname(os.path.abspath(__file__))
mods = os.path.join(here, "python", "dune")
sys.path.append(mods)

try:
    from dune.packagemetadata import metaData
except ImportError:
    from packagemetadata import metaData
from skbuild import setup
setup(**metaData()[1])
