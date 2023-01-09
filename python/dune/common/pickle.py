# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import json as _json
import pickle as _pickle
import pickletools
import importlib, sys, os
def dump(objects, f, protocol=None, includeJITSources=True):
    if not (isinstance(objects,list) or isinstance(objects,tuple)):
        raise TypeError("only pass in tuples/lists to pickle")
    obj = objects
    if includeJITSources:
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
    else:
        _pickle.dump([],f,protocol)
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

class SeriesPickler:
    def __init__(self, fileBase, objects=None, *, append=False, protocol=None):
        self.fileBase = fileBase
        self.objects = objects
        self.protocol = protocol
        self.seriesFileName = fileBase+".series."+"dbf"
        if append and os.path.exists(self.seriesFileName):
            with open(self.seriesFileName,"r+") as f:
                self.data = _json.load(f)
            self.count = max(self.data.keys())+1
        else:
            self.data = {}
            self.count = 0

    def _dumpFileName(self,count):
        return self.fileBase+"."+str(count).zfill(6)+".dbf"

    def dump(self,data):
        assert self.objects is not None, "if used to dump data the 'objects' to dump need to be provided in the ctor"
        dumpFileName = self._dumpFileName(self.count)
        if not type(data) is dict:
            raise TypeError("data passed to 'dump' should be a dictionary")
        data["dumpFileName"] = dumpFileName
        self.data[self.count] = data
        with open(self.seriesFileName,"w") as f:
            _json.dump(self.data,f,indent=2)
        with open(dumpFileName,"wb") as f:
            dump(self.objects,f,protocol=self.protocol,includeJITSources=(self.count==0))
        self.count += 1
    def load(self,count=None):
        if not self.data:
            with open(self.seriesFileName,"r+") as f:
                self.data = _json.load(f)
        if count is None:
            count = self.count
            self.count += 1
        try:
            data = self.data[str(count)]
        except KeyError:
            raise FileNotFoundError
        with open(data["dumpFileName"],"rb") as f:
            obj = load(f)
        return data,obj
