set(INSTALL_FILES
  AddMETISFlags.cmake
  AddParMETISFlags.cmake
  AddPTScotchFlags.cmake
  AddSuiteSparseFlags.cmake
  AddUMFPackFlags.cmake
  AddVcFlags.cmake
  CheckCXXFeatures.cmake
  DuneCxaDemangle.cmake
  DuneDoc.cmake
  DuneDoxygen.cmake
  DuneEnableAllPackages.cmake
  DuneExecuteProcess.cmake
  DuneLegacyAddTest.cmake
  DuneLegacyMacros.cmake
  DuneMPI.cmake
  DunePathHelper.cmake
  DunePkgConfig.cmake
  DuneSphinxCMakeDoc.cmake
  DuneSphinxDoc.cmake
  DuneSymlinkOrCopy.cmake
  Headercheck.cmake
  OverloadCompilerFlags.cmake
  UseLatexMk.cmake
  UseInkscape.cmake
)

install(
  FILES ${INSTALL_FILES}
  DESTINATION
    ${CMAKE_INSTALL_DATADIR}/cmake/legacy
)

foreach (file ${INSTALL_FILES})
  configure_file(${file} ${PROJECT_BINARY_DIR}/legacy/${file} COPYONLY)
endforeach (file)
