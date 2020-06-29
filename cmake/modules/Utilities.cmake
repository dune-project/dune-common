#[=======================================================================[.rst:
DuneModuleToUppercase
---------------------

Utility functions

Generate uppercase module names
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Converts a module name given by ``<lower>`` into an uppercase string
where all dashes (-) are replaced by underscores (_)

  dune_module_to_uppercase(<var> <lower>)

Example: dune-common -> DUNE_COMMON


Processing version strings
^^^^^^^^^^^^^^^^^^^^^^^^^^
extracts major, minor, and revision from version string:

  extract_major_minor_version(<version_string> <var>)

Split list of modules, potentially with version information
into list of modules and list of versions:

  split_module_version(<string> <modules> <versions>)


More utility functions
^^^^^^^^^^^^^^^^^^^^^^
Convert a string with spaces in a list which is a string with semicolon:

  convert_deps_to_list(<var>)

  extract_line(<header> <output> <filename>)

#]=======================================================================]

include_guard(GLOBAL)

#####
macro(dune_module_to_uppercase _upper _module)
  string(TOUPPER "${_module}" ${_upper})
  string(REPLACE "-" "_" ${_upper} "${${_upper}}")
endmacro(dune_module_to_uppercase _upper _module)


#####
function(extract_major_minor_version version_string varname)
    string(REGEX REPLACE "([0-9]+).*" "\\1" ${varname}_MAJOR "${version_string}")
  string(REGEX REPLACE "[0-9]+\\.([0-9]+).*" "\\1" ${varname}_MINOR "${version_string}")
  string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" ${varname}_REVISION "${version_string}")
  set(${varname}_MAJOR "${${varname}_MAJOR}" PARENT_SCOPE) # make variable accessible in parent scope

  # remove false matches in version string and export to parent scop
  string(REGEX MATCH "[^0-9]" NON_NUMBER_CHARACTER "${${varname}_MINOR}")
  if(NON_NUMBER_CHARACTER)
    set(${varname}_MINOR "0" PARENT_SCOPE)
  else()
    set(${varname}_MINOR ${${varname}_MINOR} PARENT_SCOPE)
  endif()
  string(REGEX MATCH "[^0-9]" NON_NUMBER_CHARACTER "${${varname}_REVISION}")
  if(NON_NUMBER_CHARACTER)
    set(${varname}_REVISION "0"  PARENT_SCOPE)
  else()
    set(${varname}_REVISION ${${varname}_REVISION} PARENT_SCOPE)
  endif()
endfunction(extract_major_minor_version version_string varname)

#
# split list of modules, potentially with version information
# into list of modules and list of versions
#
macro(split_module_version STRING MODULES VERSIONS)
  set(REGEX "[a-zA-Z0-9-]+[ ]*(\\([ ]*([^ ]+)?[ ]*[^ ]+[ ]*\\))?")
  string(REGEX MATCHALL "${REGEX}" matches "${STRING}")
  set(${MODULES} "")
  set(${VERSIONS} "")
  foreach(i ${matches})
    string(REGEX REPLACE "^([a-zA-Z0-9-]+).*$" "\\1" mod ${i})
    string(REGEX MATCH "\\([ ]*(([^ ]+)?[ ]*[^ ]+)[ ]*\\)" have_version
      ${i})
    if(have_version)
      string(REGEX REPLACE "^\\([ ]*([^ ]*[ ]*[^ ]+)[ ]*\\)$" "\\1"
        version ${have_version})
      else()
        set(version " ") # Mark as no version requested.
        # Having a space is mandatory as we will append it to a list
        # and an empty string will not be treated as entry we append to it.
      endif()
    list(APPEND ${MODULES} ${mod})
    list(APPEND ${VERSIONS} ${version})
  endforeach()
endmacro(split_module_version)

#####
function(convert_deps_to_list var)
  string(REGEX REPLACE "([a-zA-Z0-9\\)]) ([a-zA-Z0-9])" "\\1;\\2" ${var} ${${var}})
  set(${var} ${${var}} PARENT_SCOPE)
endfunction(convert_deps_to_list var)


#####
macro(extract_line HEADER  OUTPUT FILE_NAME)
  set(REGEX "^${HEADER}[ ]*[^\\n]+")
  file(STRINGS "${FILE_NAME}" OUTPUT1 REGEX "${REGEX}")
  if(OUTPUT1)
    set(REGEX "^[ ]*${HEADER}[ ]*(.+)[ ]*$")
    string(REGEX REPLACE ${REGEX} "\\1" ${OUTPUT} "${OUTPUT1}")
  else(OUTPUT1)
    set(OUTPUT OUTPUT-NOTFOUND)
  endif()
endmacro(extract_line)
