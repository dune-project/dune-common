prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@CMAKE_INSTALL_PREFIX@/bin
libdir=@CMAKE_INSTALL_PREFIX@/lib
includedir=@CMAKE_INSTALL_PREFIX@/include
CXX=@CMAKE_CXX_COMPILER@
CC=@CMAKE_C_COMPILER@
DEPENDENCIES=@REQUIRES@

Name: @DUNE_MODULE_NAME@
Version: @DUNE_VERSION_STRING@
Description: Dune (Distributed and Unified Numerics Environment) common module
URL: http://dune-project.org/
Requires: ${DEPENDENCIES}
Libs: -L${libdir} -ldunecommon
Cflags: -I${includedir}
