# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
FixtureModernDocs
-----------------

Minimal documentation fixture for the DUNE CMake documentation generator.

This file is intentionally small, but it demonstrates the modern bracketed
module documentation block, modern CMake domain commands, version directives,
cross references, variables, and an internal marker aligned with the Doxygen
concept of internal documentation.

.. cmake:command:: dune_fixture_public

  Public command used to validate extraction of modern documentation.

  .. code-block:: cmake

    dune_fixture_public(TARGET mytarget)

  See :cmake:command:`dune_fixture_helper()` for the internal helper that is
  not expected to appear in a public command TOC.

  .. versionchanged:: 2.9

    The ``TARGET`` argument replaces the legacy ``NAME`` argument.

.. cmake:command:: dune_fixture_helper

  .. dune:internal::

  Internal helper command used to validate public/internal filtering once the
  command TOC generator is implemented.

  .. code-block:: cmake

    dune_fixture_helper(TARGET mytarget)

.. cmake:variable:: DUNE_FIXTURE_VARIABLE

  Example cache/configuration variable used to validate modern variable page
  generation.

#]=======================================================================]

function(dune_fixture_public)
endfunction()

function(dune_fixture_helper)
endfunction()
