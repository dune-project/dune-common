print("""WARNING:
Importing deprecated `dune.common.compatibility'. The Python 2.7 versions
of the functions defined here were removed and the Python 3+ versions moved
so use
    import dune.common.utility
instead
""")
from .utility import *
