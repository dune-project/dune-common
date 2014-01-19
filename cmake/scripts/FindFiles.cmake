#Do not follow symlinks during FILE GLOB_RECURSE
cmake_policy(SET CMP0009 NEW)
file(GLOB_RECURSE makefiles RELATIVE ${RELPATH} "CMakeLists.txt")
foreach(_file ${makefiles})
  # path contains "/test"
  string(REGEX MATCH ".*/test" _testdir ${_file})
  # path end with "/test" and not "/testFoo" or "/test/foo"
  string(REGEX MATCH ".*/test/CMakeLists.txt$" _testdirensured ${_file})
  if(_testdir AND _testdirensured)
    list(APPEND _makefiles ${_testdir})
  endif(_testdir AND _testdirensured)
endforeach(_file ${makefiles})
if(_makefiles)
  message("${_makefiles}")
endif(_makefiles)
