import os, sys
here = os.path.dirname(os.path.abspath(__file__))
mods = os.path.join(here, "python")
sys.path.append(mods)

from dune.dunepackaging import metaData
from skbuild import setup
setup(**metaData()[1])
