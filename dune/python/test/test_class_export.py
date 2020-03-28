from dune.generator.importclass import load
from dune.generator.algorithm   import run
from dune.generator import path
from dune.typeregistry import generateTypeName
cls = load("MyClassA",path(__file__)+"myclass.hh",10,20)
print( run("run","myclass.hh",cls) )
clsName,includes = generateTypeName("MyClassB",cls)
cls = load(clsName,[path(__file__)+"myclass.hh"]+includes,cls,2)
print( run("run","myclass.hh",cls) )
