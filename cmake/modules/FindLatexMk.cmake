#[=======================================================================[.rst:
FindLatexMk
===========

Find LatexMk, the Perl-based LaTeX build tool.

This find-module locates the ``latexmk`` executable and extracts version
information from ``latexmk --version`` output.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

.. cmake:variable:: LATEXMK_FOUND

  Boolean result indicating whether the ``latexmk`` executable was found.

.. cmake:variable:: LATEXMK_EXECUTABLE

  Full path to the discovered ``latexmk`` executable.

.. cmake:variable:: LATEXMK_VERSION_STRING

  Version string reported by the detected ``latexmk`` executable.

.. cmake:variable:: LATEXMK_VERSION_MAJOR

  Major version component extracted from ``LATEXMK_VERSION_STRING``.

.. cmake:variable:: LATEXMK_VERSION_MINOR

  Minor version component extracted from ``LATEXMK_VERSION_STRING``.

Hints
^^^^^

The following variables may be set to influence this module's behavior:

.. cmake:variable:: LATEXMK_ROOT

  Preferred search prefix for the ``latexmk`` executable.

.. cmake:variable:: LATEXMK_DIR

  Alternative search prefix for the ``latexmk`` executable. This is treated
  like ``LATEXMK_ROOT``.

#]=======================================================================]

# Copyright (c) 2017, Dominic Kempf, Steffen Müthing
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice, this
#   list of conditions and the following disclaimer in the documentation and/or
#   other materials provided with the distribution.
#
# * Neither the name of the Universität Heidelberg nor the names of its
#   contributors may be used to endorse or promote products derived from this
#   software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

# Find the actual program
find_program(LATEXMK_EXECUTABLE
             latexmk
             PATHS ${LATEXMK_ROOT}
                   ${LATEXMK_DIR}
             )

# If found, figure out a version
if(LATEXMK_EXECUTABLE)
  execute_process(COMMAND ${LATEXMK_EXECUTABLE} --version
                  OUTPUT_VARIABLE LATEXMK_VERSION_LINE
                  OUTPUT_STRIP_TRAILING_WHITESPACE
                  )
  string(REGEX MATCH "Version.*$" LATEXMK_VERSION_STRING "${LATEXMK_VERSION_LINE}")
  string(REGEX REPLACE "([0-9]+)\\." "\\1" LATEXMK_VERSION_MINOR "${LATEXMK_VERSION_STRING}")
  string(REGEX REPLACE "[0-9]+\\.([0-9a-z]+)" "\\1" LATEXMK_VERSION_MAJOR "${LATEXMK_VERSION_STRING}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LatexMk
                                  FOUND_VAR LATEXMK_FOUND
                                  REQUIRED_VARS LATEXMK_EXECUTABLE
                                  VERSION_VAR LATEXMK_VERSION_STRING)
