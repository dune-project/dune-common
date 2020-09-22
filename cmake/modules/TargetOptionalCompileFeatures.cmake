#[=======================================================================[.rst:
TargetOptionalCompileFeatures
-----------------------------

Check if given feature exists, by either inspecting cxx symbols, checking for
includes, or trying to compile a piece of code. The features are listed below.

  target_optional_compile_features(<target> <PRIVATE|PUBLIC|INTERFACE> <feature>...)

#]=======================================================================]

include_guard(GLOBAL)

function (target_optional_compile_features TARGET SCOPE)
  # <target> must be a real cmake target
  if (NOT TARGET ${TARGET})
    message(FATAL_ERROR "target_optional_compile_features: <target> must be a cmake target.")
  endif ()

  # <scope> must be one of PRIVATE, PUBLIC, or INTERFACE
  if (NOT (${SCOPE} STREQUAL "PRIVATE" OR ${SCOPE} STREQUAL "PUBLIC" OR ${SCOPE} STREQUAL "INTERFACE"))
    message(FATAL_ERROR "target_optional_compile_features: <scope> must be one of PRIVATE, PUBLIC, or INTERFACE.")
  endif ()

  # traverse all the features and call the feature-test function
  foreach (FEATURE ${ARGN})
    string(TOLOWER ${FEATURE} feature)
    set(feature_impl ${PROJECT_SOURCE_DIR}/cmake/feature_tests/${feature}.cmake)
    if (NOT EXISTS ${feature_impl})
      message(WARNING "target_optional_compile_features: Feature ${FEATURE} unknown. Will be skipped.")
      continue()
    endif ()

    target_optional_compile_features_impl(${TARGET} ${SCOPE} ${feature_impl})
  endforeach (FEATURE)
endfunction (target_optional_compile_features)


# --- Implementation details ---
function (target_optional_compile_features_impl TARGET SCOPE FILE)
  include(${FILE})
  add_feature(${TARGET} ${SCOPE})
endfunction (target_optional_compile_features_impl)
