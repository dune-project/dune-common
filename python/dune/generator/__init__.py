import os
import logging
import shutil
import pkgutil
import dune
import importlib
import re
import jinja2
import subprocess

from dune.common.module import getCXXFlags
from .exceptions import CompileError, ConfigurationError
from .builder import Builder
from . import builder as builderModule
from dune.packagemetadata import get_dune_py_dir

logger = logging.getLogger(__name__)


def extract_metadata():
    """ Extract meta data that was exported by CMake.

    This returns a dictionary that maps package names to the data associated
    with the given metadata key. Currently the following metadata keys are
    exported by Python packages created with the Dune CMake build system:
    * MODULENAME: The name of the Dune module
    * BUILDDIR: The build directory of the Dune module
    * DEPS: The name of all the dependencies of the module
    * DEPBUILDDIRS: The build directories of the dependencies
    """
    result = {}

    for package in pkgutil.iter_modules(dune.__path__, dune.__name__ + "."):
        # Avoid infinite recursion
        if package.name == "dune.generator":
            continue

        # Avoid the dune.create module - it cannot be imported unconditionally!
        if package.name == "dune.create":
            continue

        # Avoid the dune.utility module - it import dune.create
        if package.name == "dune.utility":
            continue

        # Avoid the dune.plotting module - it depends on matplotlib without making this explicit
        if package.name == "dune.plotting":
            continue

        # Check for the existence of the metadata.cmake file in the package
        mod = importlib.import_module(package.name)
        path, filename = os.path.split(mod.__file__)

        # Only consider sub-packages, not modules
        if filename != "__init__.py":
            continue

        metadata_file = os.path.join(path, "metadata.cmake")
        if os.path.exists(metadata_file):
            # If it exists parse the line that defines the key that we are looking for
            for line in open(metadata_file, "r"):
                match = re.match(f"(.*)=(.*)", line)
                if match:
                    result.setdefault(package.name, {})
                    key, value = match.groups()
                    result[package.name][key] = value

    return result


def dunepy_from_template(dunepy_dir):
    # Remove any remnants of an old dune-py installation
    if os.path.exists(dunepy_dir):
        shutil.rmtree(dunepy_dir)

    # Extract the raw data dictionary
    data = extract_metadata()

    # Define some data processing patterns
    def combine_across_modules(key):
        return list(m[key] for m in data.values())

    def zip_across_modules(key, value):
        result = {}
        for moddata in data.values():
            for k, v in zip(moddata[key].split(" "), moddata[value].split(";")):
                result[k] = v
        return result

    def unique_value_across_modules(key, default=""):
        values = set(m[key] for m in data.values())
        if len(values) > 1:
            raise ValueError(f"Key {key} is expected to be unique across the given metadata")
        if len(values) == 0:
            return default
        value, = values
        return value

    # Gather and reorganize meta data context that is used to write dune-py
    context = {}
    context["modules"] = combine_across_modules("MODULENAME")
    context["builddirs"] = zip_across_modules("DEPS", "DEPBUILDDIRS")
    context["install_prefix"] = unique_value_across_modules("INSTALL_PREFIX")

    # Find the correct template path
    path, _ = os.path.split(__file__)
    template_path = os.path.join(path, "template")

    # Run the template through Jinja2
    env = jinja2.Environment(
        loader=jinja2.FileSystemLoader(template_path),
        keep_trailing_newline=True,
    )
    for root, dirs, files in os.walk(template_path):
        for template_file in files:
            full_template_file = os.path.join(root, template_file)
            relative_template_file = os.path.relpath(full_template_file, start=template_path)
            gen_file = os.path.join(dunepy_dir, relative_template_file)
            os.makedirs(os.path.split(gen_file)[0], exist_ok=True)
            with open(gen_file, "w") as outfile:
                outfile.write(env.get_template(relative_template_file).render(**context))


# Trigger the generation of dune-py
dunepy = get_dune_py_dir()
if not os.path.exists(dunepy):
    dunepy_from_template(get_dune_py_dir())
    subprocess.check_call("cmake .".split(), cwd=get_dune_py_dir())

env_force = os.environ.get('DUNE_FORCE_BUILD', 'FALSE').upper()
env_save  = os.environ.get('DUNE_SAVE_BUILD' , 'FALSE').upper()
builder = Builder( env_force in ('1', 'TRUE'), env_save )

def setNoDependencyCheck():
    logger.debug("Switching off dependency check - modules will always be compiled")
    builderModule.noDepCheck = True
def setDependencyCheck():
    logger.debug("Switching on dependency check")
    builderModule.noDepCheck = False
def setFlags(flags="-g",noChecks=None):
    logger.debug("Using compile flags '"+flags+"'")
    builderModule.cxxFlags = flags
    if noChecks is True:
        setNoDependencyCheck()
    elif noChecks is False:
        setDependencyCheck()
def addToFlags(pre="",post="",noChecks=None):
    setFlags(pre+" "+getCXXFlags()+" "+post,noChecks)

def unsetFlags(noChecks=None):
    logger.debug("Using compile flags from configuration of dune-py")
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
