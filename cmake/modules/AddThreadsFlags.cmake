# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
AddThreadsFlags
---------------

Global thread-library integration for DUNE modules.

If the CMake package :cmake:command:`find_package(Threads)` succeeds, this
module:

``HAVE_THREADS``
  Sets the config-header variable to reflect whether the thread library was
  found.

``Threads::Threads``
  Links the imported thread library target globally via
  :cmake:command:`link_libraries()`. As a result, targets created afterwards in
  the current directory scope inherit the thread library automatically.

#]=======================================================================]

include_guard(GLOBAL)

# text for feature summary
set_package_properties("Threads" PROPERTIES
  DESCRIPTION "Multi-threading library")

# set HAVE_THREADS for config.h
set(HAVE_THREADS ${Threads_FOUND})

# register the Threads imported target globally
if(Threads_FOUND)
  link_libraries(Threads::Threads)
endif()
