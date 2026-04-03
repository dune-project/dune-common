.. _faq:

--------------------------------
Frequently Asked Questions (FAQ)
--------------------------------

How do I use Dune with CMake?
=============================

The build process can be orchestrated by ``dunecontrol``, located in
``dune-common/bin``. It translates configure flags from the opts file into
corresponding CMake flags. Over time, a direct CMake-based workflow is usually
the cleaner long-term approach.

``dunecontrol`` picks up ``CMAKE_FLAGS`` from the opts file and forwards them
to CMake. This is the primary mechanism for setting build-system variables such
as search paths or compiler settings.

The most important part of the configure flags is telling the build system
where to look for external libraries. The generated variable reference contains
the variables that are picked up by the DUNE CMake build system.

.. _whatfiles:

What files in a DUNE module belong to the CMake build system?
=============================================================

Every directory in a project contains a ``CMakeLists.txt`` file written in the
CMake language. The top-level ``CMakeLists.txt`` sets up the DUNE module.
Whenever an ``add_subdirectory`` command is encountered, the ``CMakeLists.txt``
of that subdirectory is executed.

Additionally, a DUNE module can export CMake modules in ``cmake/modules``.
The file ``config.h.cmake`` defines the template for the generated
configuration-header section of ``config.h``.

.. _flags:

How do I modify the flags and linked libraries of a given target?
=================================================================

There are multiple ways to do this.

For many external packages, DUNE provides helper commands of the form
``add_dune_*_flags``. Example:

.. code-block:: cmake

   add_executable(foo foo.cc)
   add_dune_mpi_flags(foo)

If you want full control over a target, use standard CMake commands such as
``target_link_libraries()``, ``target_compile_definitions()``,
``target_compile_options()``, and ``target_include_directories()``.

The simplified directory-scope mechanism is provided by
``dune_enable_all_packages()`` and ``dune_target_enable_all_packages()``.

.. _external:

How do I link against external libraries that are not checked for by DUNE?
==========================================================================

The clean solution is to provide a find module for the package. For an
external package called ``SomePackage``, the conventional CMake module name is
``FindSomePackage.cmake``. If CMake encounters ``find_package(SomePackage)``,
it searches its module paths for such a file.

Depending on how common the package is, you may not need to write this module
from scratch. It is often enough to rely on an upstream or third-party find
module.

It is usually good style to also provide a helper command like
``add_dune_somepackage_flags`` if the package is intended for regular target
use in DUNE modules.

.. _outofsource:

What is an out-of-source build?
===============================

An out-of-source build keeps generated files out of the version-controlled
source tree and puts them into a separate build directory.

Out-of-source builds are the default with CMake. In-source builds are strongly
discouraged.

By default, DUNE uses a ``build-cmake`` subdirectory in each module. The build
directory can be customized with the ``--builddir`` option of ``dunecontrol``.

.. _simplified:

What is the simplified build system and how do I use it?
========================================================

DUNE offers a simplified build system mode where package flags and libraries
are applied broadly in directory scope. You can enable this by calling
``dune_enable_all_packages()`` in the top-level ``CMakeLists.txt`` of your
project before adding subdirectories.

This modifies targets in that directory and below, adding compile flags and
link libraries for all found external packages.

The most relevant commands for this mode are:

* ``dune_enable_all_packages``
* ``dune_register_package_flags``
* ``dune_library_add_sources``

.. _compiler:

How do I change my compiler and compiler flags?
===============================================

Common approaches are:

* setting ``CMAKE_<LANG>_COMPILER`` via ``CMAKE_FLAGS`` in the opts file
* setting those variables directly in CMake
* setting environment variables such as ``CC`` and ``CXX``

The recommended approach is to set the compiler through ``CMAKE_FLAGS``. When
changing compilers, existing build directories should usually be removed.

Default compiler flags can be modified via ``CMAKE_<LANG>_FLAGS``.

.. _cxxstandard:

How do I set the C++ standard?
==============================

If you want to require a more recent C++ standard than used in the DUNE core
modules, use ``target_compile_features(<target> ... cxx_std_17)`` or a newer
standard directly on your target.

For a consistent build across modules, set the global CMake variable
``CMAKE_CXX_STANDARD``.

.. _symlink:

How should I handle ini and grid files in an out-of-source build?
=================================================================

Such files are under version control, but often need to be available in the
build directory as well.

Relevant helpers are:

* ``dune_symlink_to_source_tree``
* ``dune_symlink_to_source_files``
* ``dune_add_copy_command``
* ``dune_add_copy_dependency``
* ``dune_add_copy_target``

The simplest solution is often setting ``DUNE_SYMLINK_TO_SOURCE_TREE`` in the
opts file.

.. _ides:

How do I use CMake with IDEs?
=============================

CMake is a build-system generator with multiple backends. IDE integration
typically requires selecting a different CMake generator. Check ``cmake --help``
for available generators and pass the desired one via ``-G`` in
``CMAKE_FLAGS``.

.. _cxxflags:

I usually modify my CXXFLAGS when calling make. How can I do this in CMake?
===========================================================================

This is not aligned with the normal CMake workflow. The preferred solution is
to use separate out-of-source build directories for different configurations.

The old ``ALLOW_CXXFLAGS_OVERWRITE`` mechanism is deprecated. Use
``dune-common/bin/compiler_launcher.sh`` instead if you need a comparable
workflow.

.. _test:

How do I run the test suite from CMake?
=======================================

The built-in target to run tests is called ``test`` and wraps CMake's own
testing tool ``ctest``.

Test programs are not built automatically. Build them first, for example with
``make build_tests``, and then run the tests.

The DUNE test suite also defines parallel tests. You may set an upper bound on
the number of cores used by a single test via ``DUNE_MAX_TEST_CORES``.

.. _disable:

Can I disable an external dependency?
=====================================

To disable an external dependency ``Foo``, add:

.. code-block:: cmake

   -DCMAKE_DISABLE_FIND_PACKAGE_Foo=TRUE

The name of the dependency is case sensitive, but there is no canonical naming
scheme. Check the configure output to determine the exact name.

This is also the mechanism to force a sequential build when MPI is installed.
For example:

.. code-block:: cmake

   -DCMAKE_DISABLE_FIND_PACKAGE_MPI=TRUE

disables MPI discovery and therefore prevents a parallel DUNE build.

When changing dependency-discovery settings, it is usually safest to delete the
build directory or at least the CMake cache.

.. _troubleshoot:

How do I troubleshoot?
======================

CMake caches aggressively, which makes it bad at recognizing changed
configurations. To trigger a fresh configure run, delete ``CMakeCache.txt``
from the build directory or remove the build directory entirely.

Whenever you experience problems, the first step should usually be to delete
all build directories. A useful shortcut is:

.. code-block:: bash

   dunecontrol exec "rm -rf build-cmake"

This removes the default build directories from all DUNE modules.

If configure fails, inspect ``CMakeError.log`` in the ``CMakeFiles``
subdirectory of the build directory. That log is usually what you should send
along when asking for help.

Where can I get help?
=====================

The CMake manual is available on the command line:

* ``cmake --help-command-list``
* ``cmake --help-command <command>``
* ``cmake --help-property-list``
* ``cmake --help-property <property>``
* ``cmake --help-module-list``
* ``cmake --help-module <module>``

For DUNE-specific buildsystem issues, ask on the DUNE mailing lists. For
general CMake usage, the upstream CMake documentation and broader community
resources are often useful too.
