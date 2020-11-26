from skbuild import setup
import os, sys

here = os.path.dirname(os.path.abspath(__file__))
mods = os.path.join(here, "python", "dune")
sys.path.append(mods)
from dunepackaging import metaData

setup(**metaData()[1])
