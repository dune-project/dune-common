MODULES = \
  AddGMPFlags.cmake        \
  AddMETISFlags.cmake      \
  AddParMETISFlags.cmake   \
  AddPTScotchFlags.cmake   \
  AddUMFPackFlags.cmake    \
  CheckCXXFeatures.cmake   \
  DuneCMakePackageConfigHelpers.cmake \
  DuneCommonMacros.cmake  \
  DuneCxaDemangle.cmake   \
  DuneDoc.cmake           \
  DuneDoxygen.cmake       \
  DuneEnableAllPackages.cmake \
  DuneMacros.cmake        \
  DuneMPI.cmake           \
  DunePkgConfig.cmake     \
  DuneStreams.cmake       \
  DuneSymlinkOrCopy.cmake \
  DuneTestMacros.cmake    \
  DuneTests.cmake         \
  FindGMP.cmake           \
  FindInkscape.cmake      \
  FindMETIS.cmake         \
  FindMProtect.cmake      \
  FindParMETIS.cmake      \
  FindPTScotch.cmake      \
  FindThreadsCMake31.cmake \
  FindUMFPack.cmake       \
  Headercheck.cmake       \
  LanguageSupport.cmake   \
  OverloadCompilerFlags.cmake \
  UseInkscape.cmake       \
  UseLATEX.cmake

TESTPROGRAMS = CheckForPthreads.c

modulesdir = $(datadir)/dune/cmake/modules
dist_modules_DATA = ${MODULES} ${TESTPROGRAMS}

include $(top_srcdir)/am/global-rules
EXTRA_DIST = CMakeLists.txt
