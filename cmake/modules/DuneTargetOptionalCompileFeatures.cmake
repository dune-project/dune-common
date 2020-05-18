#[=======================================================================[.rst:
DuneTargetOptionalCompileFeatures
---------------------------------

Check if given feature exists, by either inspecting cxx symbols, checking for
includes, or trying to compile a piece of code. The features are listed below.

.. command:: dune_target_optional_compile_features

  .. code-block:: cmake

    dune_target_optional_compile_features(<target> <PRIVATE|PUBLIC|INTERFACE> <feature>...)

#]=======================================================================]

include_guard(GLOBAL)

function (dune_target_optional_compile_features TARGET SCOPE)
  # <target> must be a real cmake target
  if (NOT TARGET ${TARGET})
    message(FATAL_ERROR "dune_target_optional_compile_features: <target> must be a cmake target.")
  endif ()

  # <scope> must be one of PRIVATE, PUBLIC, or INTERFACE
  if (NOT (${SCOPE} STREQUAL "PRIVATE" OR ${SCOPE} STREQUAL "PUBLIC" OR ${SCOPE} STREQUAL "INTERFACE"))
    message(FATAL_ERROR "dune_target_optional_compile_features: <scope> must be one of PRIVATE, PUBLIC, or INTERFACE.")
  endif ()

  # traverse all the features and call the feature-test function
  foreach (FEATURE ${ARGN})
    string(TOLOWER ${FEATURE} feature)
    set(feature_impl ${PROJECT_SOURCE_DIR}/cmake/cxx_features/${feature}.cmake)
    if (NOT EXISTS ${feature_impl})
      message(WARNING "dune_target_optional_compile_features: Feature ${FEATURE} unknown. Will be skipped.")
      continue()
    endif ()

    dune_target_optional_compile_features_impl(${TARGET} ${SCOPE} ${feature_impl})
  endforeach (FEATURE)
endfunction (dune_target_optional_compile_features)


# --- Implementation details ---
function (dune_target_optional_compile_features_impl TARGET SCOPE FILE)
  include(${FILE})
  add_feature(${TARGET} ${SCOPE})
endfunction (dune_target_optional_compile_features_impl)
