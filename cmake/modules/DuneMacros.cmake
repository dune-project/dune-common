include(GNUInstallDirs)
include(DuneAddLibrary)
include(DuneCheckCXXSourceCompiles)
include(DuneFeatureSummary)
include(DuneInstallLibrary)
include(DuneTargetOptionalCompileFeatures)
include(DuneTesting)
include(TargetSourcesLocal)

# add legacy build-system functions for compatibility with old dune modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/legacy)
include(DuneLegacyMacros)
