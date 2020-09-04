import os, logging

from dune.common.module import getCXXFlags
from .exceptions import CompileError, ConfigurationError
from .builder import Builder
from . import builder as builderModule

logger = logging.getLogger(__name__)

env_force = os.environ.get('DUNE_FORCE_BUILD', 'FALSE').upper()
env_save  = os.environ.get('DUNE_SAVE_BUILD' , 'FALSE').upper()
builder = Builder( env_force in ('1', 'TRUE'), env_save )

def setNoDependencyCheck():
    logger.info("Switching off dependency check - modules will always be compiled")
    builderModule.noDepCheck = True
def setDependencyCheck():
    logger.info("Switching on dependency check")
    builderModule.noDepCheck = False
def setFlags(flags="-g",noChecks=None):
    logger.info("Using compile flags '"+flags+"'")
    builderModule.cxxFlags = flags
    if noChecks is True:
        setNoDependencyCheck()
    elif noChecks is False:
        setDependencyCheck()
def addToFlags(pre="",post="",noChecks=None):
    setFlags(pre+" "+getCXXFlags()+" "+post,noChecks)

def unsetFlags(noChecks=None):
    logger.info("Using compile flags from configuration of dune-py")
    builderModule.cxxFlags = None
    if noChecks is True:
        setNoDependencyCheck()
    elif noChecks is False:
        setDependencyCheck()
def reset():
    unsetFlags()
    setDependencyCheck()

def path(f):
    return os.path.dirname(os.path.realpath(f))+"/"

class Constructor(object):
    def __init__(self, args, body=None, extra=None):
        self.args = args
        self.body = body
        self.extra = [] if extra is None else extra

    def register(self, cls="cls"):
        if self.body is None:
            return cls + ".def( pybind11::init( " + self.args + " )" + "".join(", " + e for e in self.extra) + " );\n"
        if self.args:
            source = cls + ".def( pybind11::init( [] ( " + ", ".join(self.args) + " ) {"
        else:
            source = cls + ".def( pybind11::init( [] () {"
        source += "\n    ".join(self.body)
        source += "\n  } )" + "".join(", " + e for e in self.extra) + " );\n"
        return source

    def __str__(self):
        return self.register()


class Method(object):
    def __init__(self, name, args, body=None, extra=None):
        self.name = name
        self.args = args
        self.body = body
        if extra is None:
            self.extra = []
        else:
            self.extra = extra

    def register(self, cls="cls"):
        if self.body is None:
            return cls + ".def( \"" + self.name + "\", " + self.args + "".join(", " + e for e in self.extra) + " );\n"
        if self.args:
            source = cls + ".def(\"" + self.name + "\", [] ( " + ", ".join(self.args) + " ) {"
        else:
            source = cls + ".def( \"" + self.name + "\", [] () {"
        source += "\n    ".join(self.body)
        source += "\n  } " + "".join(", " + e for e in self.extra) + " );\n"
        return source

    def __str__(self):
        return self.register()
