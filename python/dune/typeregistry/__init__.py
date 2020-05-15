try:
    from dune.generated._typeregistry import *
except ImportError:
    from dune.generator import builder, path
    from . import path
    builder.load("_typeregistry",
                 "#include \""+path(__file__)+'_typeregistry.cc'+"\"",
                 "_typeregistry")
    from dune.generated._typeregistry import *
