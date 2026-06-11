# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

if (NOT DEFINED DOXYFILE)
  set(DOXYFILE "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
endif()

# For now we just support appending Doxyfile and Doxylocal
file(READ ${DOXYSTYLE} file_contents)
file(WRITE ${DOXYFILE}.in ${file_contents})
# Write the list of predefined C preprocessor macros
file(READ ${DOXYGENMACROS} file_contents)
file(APPEND ${DOXYFILE}.in ${file_contents})
if(DOXYLOCAL)
  file(READ ${DOXYLOCAL} file_contents)
endif()
file(APPEND ${DOXYFILE}.in ${file_contents})

# configure_file does not work as it insists an existing input file, which in our
# needs to be generated first.
# Therefore we read the ${DOXYFILE}.in and replace the variables using string(CONFIGURE)
# and then write the file.

# Handle tagfiles: we only want to include existing tagfiles,
# otherwise doxygen will fail with an error about missing tagfiles.
if(DEFINED DOXYGEN_TAGFILES AND NOT DOXYGEN_TAGFILES STREQUAL "")
  separate_arguments(_doxygen_tagfiles NATIVE_COMMAND "${DOXYGEN_TAGFILES}")
  set(_doxygen_existing_tagfiles "")
  foreach(_doxygen_tagfile IN LISTS _doxygen_tagfiles)
    if(_doxygen_tagfile MATCHES "^([^=]+)=(.*)$")
      set(_doxygen_tagfile_path "${CMAKE_MATCH_1}")
      if(EXISTS "${_doxygen_tagfile_path}")
        message(VERBOSE "Found Doxygen tagfile: ${_doxygen_tagfile_path}")
        list(APPEND _doxygen_existing_tagfiles "${_doxygen_tagfile}")
      else()
        message(STATUS "Skipping missing Doxygen tagfile: ${_doxygen_tagfile_path}")
      endif()
    else()
      list(APPEND _doxygen_existing_tagfiles "${_doxygen_tagfile}")
    endif()
  endforeach()
  string(JOIN " " DOXYGEN_TAGFILES ${_doxygen_existing_tagfiles})
endif()

file(READ ${DOXYFILE}.in file_contents)
string(CONFIGURE ${file_contents} output)
file(WRITE ${DOXYFILE} ${output})
