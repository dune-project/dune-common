file(
  WRITE ${metadatafile}
  "DEPBUILDDIRS=${DEPBUILDDIRS}\nDEPS=${DEPS}\nMODULENAME=${MODULENAME}\nINSTALL_PREFIX=${INSTALL_PREFIX}\nCMAKE_FLAGS=${CMAKE_FLAGS}"
)