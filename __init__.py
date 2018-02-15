import os

from .builder import Builder

env_force = os.environ.get('DUNE_FORCE_BUILD', 'FALSE').upper()
if env_force in ('1', 'TRUE'):
    builder = Builder(True)
else:
    builder = Builder(False)

class Constructor(object):
    def __init__(self, args, body=None, extra=None):
        self.args = args
        self.body = body
        self.extra = [] if extra is None else extra

    def register(self, cls="cls"):
        if self.body is None:
            return cls + ".def( pybind11::init( " + args + " )" + "".join(", " + e for e in self.extra) + " );\n"
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
            source = cls + ".def( " + self.name + ", [] ( " + ", ".join(self.args) + " ) {"
        else:
            source = cls + ".def( " + self.name + ", [] () {"
        source += "\n    ".join(self.body)
        source += "\n  } )" + "".join(", " + e for e in self.extra) + " );\n"
        return source

    def __str__(self):
        return self.register()
