#For now we just support appending Doxyfile and Doxylocal
FILE(READ ${DOXYSTYLE} file_contents)
FILE(WRITE Doxyfile.in ${file_contents})
FILE(READ ${DOXYLOCAL} file_contents)
FILE(APPEND Doxyfile.in ${file_contents})

# configure_file does not work as it insists an existing input file, which in our
# needs to be generated first.
# Therefore we read the Doxyfile.in and replace the variables using STRING(CONFIGURE)
# and the write the file.
FILE(READ Doxyfile.in file_contents)
STRING(CONFIGURE ${file_contents} output)
FILE(WRITE Doxyfile ${output})
