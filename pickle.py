import sys

if sys.version_info.major == 2:
    from cPickle import *
else:
    from pickle import *
