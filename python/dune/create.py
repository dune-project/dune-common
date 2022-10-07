# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import importlib
import sys
import pkgutil
import importlib
import logging
import inspect

import dune.common.module
import dune.common
import dune.generator

logger = logging.getLogger(__name__)
_create_map = dict()

package = dune
prefix = package.__name__ + "."
subpackages = []
logMsg = "Importing create registries from [ "

# first import all 'dune' subpackages and collect the 'registry' dicts
for importer, modname, ispkg in pkgutil.iter_modules(package.__path__, prefix):
    if not ispkg:
        continue

    # can just use modname here if registry is part of __init__ file
    try:
        # Note: modname.__init__ is imported so be aware of
        # possible side effects
        module = importlib.import_module(modname)
    except ImportError as e:
        logger.debug('failed to import ' + modname + ': ' + str(e) + '.')
        continue

    # read the registry
    try:
        moduleRegistry = module.registry.items()
        logMsg = logMsg + modname + " "
    except AttributeError:
        logger.debug('Module ' + modname + ' does not provide a registry.')
        continue

    # combine all registries
    for obj, registry in moduleRegistry:
        objmap = dict()
        try:
            objmap = _create_map[obj]
        except KeyError:
            _create_map[obj] = objmap
        for key, value in registry.items():
            if key in objmap:
                raise RuntimeError('Key \'' + key + '\' registered twice for \'' + obj + '\'.')
            else:
                objmap[key.upper().lower()] = [value,modname]

# the grids registry also provide view -
# so we will add them to the 'view' entry
_create_map.setdefault("view",{}).update(_create_map["grid"])

logMsg = logMsg + "]"
logger.debug(logMsg)

############################################################################
def get(category=None,entry=None):
    entry_ = _create_map.get(category,None)
    if entry_ is None:
        if category is not None:
            print("category '",category,"' not valid,",end="")
        print("available categories are:\n",
                ','.join(k for k in sorted(_create_map)))
        return
    if entry is None:
        print("available entries for this category are:")
        entries = []
        colLength = [0,0,0]
        for k,e in entry_.items():
            n = e[0].__module__.split(".")
            if n[-1][0]=="_":
                del n[-1]
            entries += [ [k,e[0].__name__, '.'.join(m for m in n)] ]
            colLength[0] = max(colLength[0],len(entries[-1][0]))
            colLength[1] = max(colLength[1],len(entries[-1][1]))
            colLength[2] = max(colLength[2],len(entries[-1][2]))
        entries.sort()
        print("entry".ljust(colLength[0]),
              "function".ljust(colLength[1]),
              "module".ljust(colLength[2]))
        print("-"*sum(colLength))
        for e in entries:
            print(e[0].ljust(colLength[0]),
                  e[1].ljust(colLength[1]),
                  e[2].ljust(colLength[2]))
        print("-"*sum(colLength))
    else:
        entry__ = entry_.get(entry,None)
        if entry__ is None:
            print("available entries are:",
                    ','.join(k for k in entry_))
            return
        return entry__[0]

############################################################################
## the second part is for the 'load' method

# a helper class
class Empty:
    pass

def signatureDict(func):
    # get signature from func, we simply fill a dictionary with the name of
    # all non var argument of the function as key and containing a either
    # Empty or the default argument provided by the function signature

    ret = {}
    sig = inspect.signature(func)
    for p,v in sig.parameters.items():
        # we only extract positional or keyword argument (i.e. not  *args,**kwargs)
        if v.kind == v.POSITIONAL_OR_KEYWORD:
            name = v.name
            default = v.default if not v.default is v.empty else Empty
            ret.update({name:default})
    return ret

def _creatorCall(create, usedKeys, *args, **kwargs):
    # get signature of create function to call
    signature = signatureDict(create)
    # check if any of the parameter names correspond to some creator -
    # if a creator exists for that function name and the value passed in by
    # the user for that parameter is a string, call the creator otherwise
    # use the object provided. If no creator exists use the value
    # provided by the user or the default value.
    for name in signature:
        # special treatment of 'view'/'grid' parameter since a 'grid' is
        # also a view
        if name=='view' and not name in kwargs and 'grid' in kwargs:
            kwargs.update({"view":kwargs["grid"]})
            usedKeys.update(["grid"])
        creator = globals().get(name)
        if creator: # a creator for this parameter name exists
            assert signature[name] == Empty, "argument in create method corresponding to creatibles should not have default values"
            argument = kwargs.get(name, Empty)
            assert not argument == Empty, "required creatable argument " + argument + " not provided"
            if isinstance(argument,str):
                # recursion
                argument = argument.upper().lower()
                paramCreator = creator.registry[argument][0]
                signature[name] = _creatorCall(paramCreator, usedKeys, *args,**kwargs)
                kwargs[name] = signature[name] # replace the string with the actual object
            else:
                signature[name] = argument # store the object provided
            usedKeys.update([name])
        else: # no creator available
            argument = kwargs.get(name, Empty)
            if argument == Empty:
                assert not signature[name] == Empty, "no value for argument " + name + " provided"
                kwargs[name] = argument
            else:
                signature[name] = argument
                usedKeys.update([name])
    return create(**signature)

def creatorCall(self, key, *args, **kwargs):
    key = key.upper().lower()
    try:
        create = self.registry[key][0]
    except KeyError:
        raise RuntimeError('No ' + self.obj + ' implementation: ' + key +\
                '. Available: ' + ' '.join(r for r in self.registry) + '.' )
    # the complex creation mechanism is only allowed with named arguments
    # if positional arguments have been used, call the original function directly
    # without further checking the parameters
    if len(args)>0:
        return create(*args, **kwargs)
    else:
        usedKeys = set()
        # make a fix here for grids/views
        if 'grid' in kwargs and not self.obj == 'view' and not 'view' in kwargs:
            kwargs.update({'view':kwargs['grid']})
            usedKeys.update(['grid'])
        instance = _creatorCall(create,usedKeys,*args,**kwargs)
        assert set(kwargs) == usedKeys, "some provided named parameters where not used"
        return instance

##########################################################################
## for each 'registry' entry add a function to this module
for obj, registry in _create_map.items():
    # docs = "\n".join(k+" from "+v[1] for k,v in registry.items())
    docs_format = "{:<25}" * (2)
    docs = docs_format.format("key", "module") + "\n"
    docs = docs + docs_format.format("----------", "----------") + "\n"
    for k,v in registry.items():
        docs = docs + docs_format.format(k,v[1]) + "\n"

    attribs = {k: staticmethod(v[0]) for k, v in registry.items()}
    attribs.update({"__call__": creatorCall, "registry": registry, "obj": obj})
    C = type(str(obj), (object,), attribs)
    c = C()
    c.__doc__ = "Create a dune grid instance, available choices are:\n"+docs
    setattr(sys.modules[__name__], obj, c)
    logger.debug("added create."+obj+" with keys: \n"+\
            "\n".join("   "+k+" from subpackage "+v[1] for k,v in registry.items()))
