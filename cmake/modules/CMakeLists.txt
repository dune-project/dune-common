set(modules
  AddGMPFlags.cmake
  AddMETISFlags.cmake
  AddParMETISFlags.cmake
  AddPTScotchFlags.cmake
  AddUMFPackFlags.cmake
  CheckCXX11Features.cmake
  DuneBoost.cmake
  DuneCMakePackageConfigHelpers.cmake
  DuneCommonMacros.cmake
  DuneCxaDemangle.cmake
  DuneDoc.cmake
  DuneDoxygen.cmake
  DuneEnableAllPackages.cmake
  DuneMacros.cmake
  DuneMPI.cmake
  DunePkgConfig.cmake
  DuneStreams.cmake
  DuneSymlinkOrCopy.cmake
  DuneTestMacros.cmake
  DuneTests.cmake
  FindBoostFusion.cmake
  FindGMP.cmake
  FindInkscape.cmake
  FindMETIS.cmake
  FindMProtect.cmake
  FindParMETIS.cmake
  FindPTScotch.cmake
  FindThreadsCMake31.cmake
  FindUMFPack.cmake
  Headercheck.cmake
  LanguageSupport.cmake
  UseInkscape.cmake
  UseLATEX.cmake)
set(test_programs CheckForPthreads.c)
install(FILES ${modules}
              ${test_programs}
        DESTINATION ${DUNE_INSTALL_MODULEDIR})
