# Try to find Blackchannel-ULFM library
#
#
#
#   BLACKCHANNEL_FOUND        - system has Blackchannel-ulfm
#   BLACKCHANNEL_INCLUDE_DIR  - include paths to use Blackchannel-ulfm
#   BLACKCHANNEL_LIBRARIES    - Link these to use Blackchannel-ulfm
#

set(BLACKCHANNEL_FOUND 0)

set(BLACKCHANNEL_INCLUDE_DIR "" CACHE STRING "Include path used by DUNE when compiling Blackchannel-ULFM programs")
set(BLACKCHANNEL_LIBRARIES "" CACHE STRING "Libraries used by DUNE when linking Blackchannel-ULFM programs")

find_path(BLACKCHANNEL_INCLUDE_DIR
  blackchannel-ulfm.h
  /usr/blackchannel-ulfm/include
  )

find_library(BLACKCHANNEL_LIBRARIES
  blackchannel-ulfm
  /usr/blackchannel-ulfm/lib
  )

# handle the QUIETLY and REQUIRED arguments and set BLACKCHANNEL_FOUND to TRUE if
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Blackchannel
  DEFAULT_MSG
  BLACKCHANNEL_LIBRARIES
  BLACKCHANNEL_INCLUDE_DIR)

include_directories(${BLACKCHANNEL_INCLUDE_DIR})
dune_register_package_flags(INCLUDE_DIRS "${BLACKCHANNEL_INCLUDE_DIR}"
    LIBRARIES "${BLACKCHANNEL_LIBRARIES}")

set(HAVE_BLACKCHANNEL ${BLACKCHANNEL_FOUND})

mark_as_advanced(
  BLACKCHANNEL_INCLUDE_DIR
  BLACKCHANNEL_LIBRARIES
)
