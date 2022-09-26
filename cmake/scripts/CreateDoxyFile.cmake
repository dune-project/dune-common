# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# For now we just support appending Doxyfile and Doxylocal
file(READ ${DOXYSTYLE} file_contents)
file(WRITE Doxyfile.in ${file_contents})
# Write the list of predefined C preprocessor macros
file(READ ${DOXYGENMACROS} file_contents)
file(APPEND Doxyfile.in ${file_contents})
if(DOXYLOCAL)
  file(READ ${DOXYLOCAL} file_contents)
endif()
file(APPEND Doxyfile.in ${file_contents})

# configure_file does not work as it insists an existing input file, which in our
# needs to be generated first.
# Therefore we read the Doxyfile.in and replace the variables using string(CONFIGURE)
# and then write the file.
file(READ Doxyfile.in file_contents)
string(CONFIGURE ${file_contents} output)
file(WRITE Doxyfile ${output})
