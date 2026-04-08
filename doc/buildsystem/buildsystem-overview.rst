.. _buildsystem:

---------------------
The DUNE Build System
---------------------

.. overview-intro-start

DUNE provides a set of utilities to aid the setup of a project. Most of the
utilities are CMake scripts that automate common use cases or required steps
needed to use a DUNE project. Among other things, these utilities provide:

* Automatic intra-module project configuration
* Find modules for software commonly used in the DUNE context
* Testing utilities
* Configuration-file generation
* Setup of DUNE Python bindings

.. overview-intro-end

.. _duneproject:

Dune Project
============

In this documentation, a *dune project* is a project that calls the CMake
functions ``dune_project()`` and ``finalize_dune_project()``. A typical dune
project looks like this:

.. code-block:: cmake
   :caption: CMakeLists.txt

   cmake_minimum_required(VERSION 3.16)
   project("dune-foo" CXX)

   find_package(dune-common)
   list(APPEND CMAKE_MODULE_PATH "${dune-common_MODULE_PATH}")
   include(DuneMacros)

   dune_project()

   # ...

   finalize_dune_project()

The minimum required CMake version for the DUNE core modules is 3.16.

.. _configfile:

Configuration File Header
=========================

DUNE projects may provide a configuration template ``config.h.cmake``. This
template is parsed by the build system and used to generate C++ configuration
headers.

.. code-block:: c
   :caption: config.h.cmake

   /* begin private */

   /* Everything within begin/end private will be exclusively generated to the current project */

   #define DUNE_FOO_BUILD_OPTION 1

   /* end private */

   /* Everything outside of begin/end private will be generated to all downstream projects using the module */

   #define DUNE_FOO_HEADER_OPTION 0

In addition to the definitions from ``config.h.cmake``, the build system also
generates a ``HAVE_${module_upper}`` preprocessor definition for each found
DUNE module, where ``module_upper`` is the uppercase module name:

.. code-block:: c

   /* Define to 1 if you have module ${module} available */
   #ifndef HAVE_${module_upper}
   #cmakedefine01 HAVE_${module_upper}
   #endif

The following files are generated from ``config.h.cmake``:

* ``config.h.cmake``                    [configuration template - installed]
* ``${ProjectName}-config.hh``          [public generated header - installed]
* ``${ProjectName}-config-private.hh``  [private generated header - not installed]
* ``config.h``                          [collected configuration header - not installed]

The public and private ``${ProjectName}`` headers are generated at configure
time of the module itself. The collected ``config.h`` is generated for the
consumer project and incorporates non-private configuration fragments from
upstream modules.

Example
^^^^^^^

Suppose that project ``dune-bar`` depends on ``dune-foo`` and they have the
following configuration template:

.. code-block:: c
   :caption: dune-foo/config.h.cmake

   #cmakedefine OPTION_FOO 1

The resulting generated headers depend on which options are enabled when the
owning module and the downstream module are configured:

* ``OPTION_FOO=1`` in dune-foo and ``OPTION_FOO=1`` in dune-bar:
  ``dune-foo-config.hh`` contains ``#define OPTION_FOO 1`` and ``config.h``
  contains ``#define OPTION_FOO 1``.
* ``OPTION_FOO=1`` in dune-foo and ``OPTION_FOO=0`` in dune-bar:
  ``dune-foo-config.hh`` contains ``#define OPTION_FOO 1`` and ``config.h``
  leaves the option undefined.
* ``OPTION_FOO=0`` in dune-foo and ``OPTION_FOO=0`` in dune-bar:
  both generated headers leave the option undefined.
* ``OPTION_FOO=0`` in dune-foo and ``OPTION_FOO=1`` in dune-bar:
  ``dune-foo-config.hh`` leaves the option undefined and ``config.h``
  contains ``#define OPTION_FOO 1``.

Whether these combinations are valid is the responsibility of the module
author. In practice, configurations where the downstream project enables a
feature that the upstream module was not built with are often invalid.

If both ``dune-foo-config.hh`` and ``config.h`` are available, the first one
included by the translation unit determines which configuration is active. For
example:

.. code-block:: c
   :caption: dune-bar.cc

   #ifdef HAVE_CONFIG_H
   #include "config.h"
   #endif

   #include <dune/foo/feature.hh> /* already includes dune-foo-config.hh */

   /* ... */

.. _policies:

Dune Build System Policies
==========================

Similar to the CMake `policy system <https://cmake.org/cmake/help/latest/manual/cmake-policies.7.html>`_,
DUNE has a policy mechanism to maintain backward compatibility of build-system
behavior across releases. A policy selects either ``OLD`` or ``NEW`` behavior
for a given module.

The ``OLD`` behavior is deprecated by default and triggers an author warning
about unset policies. That warning can be disabled globally by setting
``DUNE_POLICY_DISABLE_WARNING`` to ``TRUE``.

Policies do not switch automatically to ``NEW`` based on module versions or
release numbers. Module authors need to opt in explicitly with
``dune_policy(SET ...)``.

Policies Introduced by Dune 2.10
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``DP_DEFAULT_INCLUDE_DIRS``
    *OLD behavior:* Set global ``include_directories`` when creating a new
    ``dune_project``.

    *NEW behavior:* Include directories must be set on module library targets
    and are not set globally anymore.

    The ``NEW`` behavior requires module authors to set include directories on
    all their targets, or to link against another target that already carries
    those include directories. For common directory layouts, the helper
    ``dune_default_include_directories(<target> <scope>)`` can be used.

Policies Introduced by Dune 2.11
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``DP_TEST_ADD_ALL_FLAGS``
    *OLD behavior:* Automatically call ``add_dune_all_flags`` on all test
    targets inside ``dune_add_test``.

    *NEW behavior:* Flags must be set for each test target separately, for
    example by using ``add_dune_pkg_flags`` or in directory scope using
    ``dune_enable_all_packages``.

Testing Options
^^^^^^^^^^^^^^^

The global option ``DUNE_ENABLE_TESTING`` controls whether DUNE modules add
their test subdirectories by default. It defaults to ``ON``.

Each DUNE module also gets a module-specific option
``<MODULE>_ENABLE_TESTING`` when ``dune_project()`` is called. If set, it
overrides the global default for that module. For example, ``dune-common``
uses ``DUNE_COMMON_ENABLE_TESTING``.

Within a module, the effective value is available as
``DUNE_MODULE_ENABLE_TESTING`` and can be used to guard
``add_subdirectory(test)``.
