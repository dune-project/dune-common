# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import pickle as _pickle
import pickletools
import importlib, sys
def dump(objects, f, protocol=None):
    if not (isinstance(objects,list) or isinstance(objects,tuple)):
        raise TypeError("only pass in tuples/lists to pickle")
    obj = objects
    objDump = _pickle.dumps(obj,protocol)
    # add source code for all required jit modules - extracct all modules from the pickle stream and
    # store the ones relating to jit modules in 'dune.generated'
    mods = []
    for opcode,arg,pos in pickletools.genops(objDump):
        try:
            if "dune.generated" in arg:
                mods += [arg]
        except:
            pass
    modsCode = []
    for m in mods:
        fname = importlib.import_module(m).__file__
        fname = fname.replace(".so",".cc")
        with open(fname,"r") as fmod:
            modsCode += [[m, fmod.read()]]
    _pickle.dump(modsCode,f,protocol)
    _pickle.dump(obj,f,protocol)

def load(f):
    # make sure dune.generated in dune-py is added to dune package path
    from dune.generator import builder
    builder.initialize()

    # extract and compile module code
    mods = _pickle.load(f)
    while len(mods)>0:
        remainingMods = []
        cond = False
        for m in mods:
            try:
                module = builder.load(m[0].split(".")[-1], m[1], None)
                # print("loading",m[0])
                cont = True
            except ImportError as ex:
                # print("skipping",m[0],"due to",ex)
                remainingMods.append(m)
                pass
        assert cont, "no module could be loaded"
        mods = remainingMods

    obj = _pickle.load(f)
    return obj
