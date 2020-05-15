try:
    from dune.generated._typeregistry import *
except ImportError:
    from dune.utility import buildAndImportModule
    buildAndImportModule(__file__,"_typeregistry",globals())
