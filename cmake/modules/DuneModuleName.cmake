macro (string_capitalize STR STR_CAP)
  string(SUBSTRING "${STR}" 0 1 FIRST_LETTER)
  string(SUBSTRING "${STR}" 1 -1 REMAINING_LETTERS)

  string(TOUPPER "${FIRST_LETTER}" FIRST_LETTER)
  string(TOLOWER "${REMAINING_LETTERS}" REMAINING_LETTERS)

  set(${STR_CAP} "${FIRST_LETTER}${REMAINING_LETTERS}")
endmacro (string_capitalize)


macro(dune_module_name NAME)
  # ARGV1 = MODULE_NAME
  # ARGV2 = CONFIG_NAME (optional)

  string(SUBSTRING "${NAME}" 0 5 DUNE_PREFIX)
  if (DUNE_PREFIX STREQUAL "dune-")
    # extract modulename postfix from NAME
    string(SUBSTRING "${NAME}" 5 -1 ${ARGV1})
  else ()
    set(${ARGV1} "${NAME}")
  endif ()

  # optional 3rd argument for config-file-name
  if (${ARGC} GREATER 2)
    string(REPLACE "-" ";" NAME_LIST "${NAME}")
    set(${ARGV2} "")
    foreach (PART ${NAME_LIST})
      string_capitalize(${PART} PART_CAP)
      string(APPEND ${ARGV2} ${PART_CAP})
    endforeach (PART)
  endif ()

endmacro(dune_module_name)