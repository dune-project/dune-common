set(INSTALL_FILES
  DuneAddLibrary.cmake
  DuneAddTest.cmake
  DuneCheckCXXSourceCompiles.cmake
  DuneDeclareTestLabel.cmake
  DuneFeatureSummary.cmake
  DuneInstallLibrary.cmake
  DuneMacros.cmake
  DuneModuleName.cmake
  DuneTesting.cmake
  DuneTargetOptionalCompileFeatures.cmake
  FindBLAS.cmake
  FindHYPRE.cmake
  FindLAPACK.cmake
  FindMProtect.cmake
  TargetSourcesLocal.cmake
)

install(
  FILES ${INSTALL_FILES}
  DESTINATION
    ${CMAKE_INSTALL_DATADIR}/cmake
)

foreach (file ${INSTALL_FILES})
  configure_file(${file} ${PROJECT_BINARY_DIR}/${file} COPYONLY)
endforeach (file)

add_subdirectory(legacy)
