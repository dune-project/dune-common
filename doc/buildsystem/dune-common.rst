
===========
dune-common
===========

.. _buildsystem:

---------------------
The DUNE Build System
---------------------


DUNE provides a set of utilities to aid the set up of a project. Most of the
utilities are CMake scripts that automate common use cases or required steps
needed to use a dune project. Among other things, these utilities have

* Automatic intra-module project configuration
* Find package scripts for commonly used software in the DUNE context
* Testing utilities
* Configuration files
* Setup of DUNE python bindings

.. _duneproject:

Dune Project
============

In this documentation pages, we call *dune project* in the sense of the build
system those projects that call CMake functions ``dune_project()`` and
``finalize_dune_project()``. A typical *dune project* usually looks like this:

.. code-block::
    :caption: CMakeLists.txt

    cmake_minimum_required(VERSION 3.16)
    project("dune-foo" CXX)

    # find dune-common
    find_package(dune-common)

    # include dune-common modules in the current CMake path
    list(APPEND CMAKE_MODULE_PATH "${dune-common_MODULE_PATH}")

    # include the dune macros from dune-common
    include(DuneMacros)

    # initialize the dune project context
    dune_project()

    # ...

    # finalize the dune project context
    finalize_dune_project()

Then, the project will be able to use the build system utilities provided by dune.

The minimum required version to build Dune with CMake is 3.16.
You can install CMake through your favorite package manager or downloading source code from
`KITWare <http://www.cmake.org>`_

.. _configfile:

Configuration File Header
=========================

Dune projects may provide a configuration file template ``config.h.cmake``.
This file will be parsed by the build system and provides you with a C++ header
file based on the configuration options at configuration time.

.. code-block::
    :caption: config.h.cmake

    /* begin private */

    /* Everything within begin/end private will be exclusively generated to the current project */

    #define DUNE_FOO_BUILD_OPTION 1

    /* end private */

    /* Everything outside of begin/end private will be generated to all of the downstream projects using the module */

    #define DUNE_FOO_HEADER_OPTION 0


Additional to the template config definitions in ``config.h.cmake``, the dune
build system will also include a ``HAVE_${module_upper}`` preprocessor
definition for each found dune module, where ``module_upper`` is the module
name in upper case version (e.g., ``DUNE_COMMON`` for the ``dune-common`` module):

.. code-block::

    /* Define to 1 if you have module ${module} available */
    #ifndef HAVE_${module_upper}
    #cmakedefine01 HAVE_${module_upper}
    #endif


DUNE generates two version of configuration files based on the configuration template ``config.h.cmake``:

* ``config.h.cmake``                    [configuration template - installed]
* ``${ProjectName}-config.hh``          [eager configuration instantiation - used in header files - installed]
* ``${ProjectName}-config-private.hh``  [eager configuration instantiation - used in binaries - not installed]
* ``config.h``                          [lazy configuration instantiation - used in binaries - not installed]

**Eager generation**
The configuration files are generated at configuration time of the project in
question. This means that once the project runs the configuration stage on
cmake, its configuration template is instantiated in c++ header files and can
be consumed by other projects without any further intervention of cmake.

* ``${ProjectName}-config-private.hh``    [used in binaries - not installed]
* ``${ProjectName}-config.hh``            [used in header files - installed]

**Lazy generation**
The configuration files are generated at configuration time of the consumer
project. This means that every time a downstream project runs the
configuration stage on cmake, the configuration template of all preceding
projects are instantiated with the cmake options of the consumer project.
This requires the consumer project to be a dune-project in the sense of the
cmake build system.

* ``config.h``                            [used in binaries - not installed]


Example
^^^^^^^

Suppose that project ``dune-bar`` depends on ``dune-foo`` and they have the
following configuration template files:

.. code-block::
    :caption: dune-foo/config.h.cmake

    #cmakedefine OPTION_FOO 1


The options ``OPTION_FOO`` will be defined depending on whether the variable
``OPTION_FOO`` is present in cmake at the time of the generation of the
configuration template file. This means that the contents of the generated files
will look like this.

===========================================================  ========================  ========================
                                                               ``dune-foo-config.hh``         ``config.h``
===========================================================  ========================  ========================
``OPTION_FOO=1`` in dune-foo & ``OPTION_FOO=1`` in dune-bar  ``#define OPTION_FOO 1``  ``#define OPTION_FOO 1``
``OPTION_FOO=1`` in dune-foo & ``OPTION_FOO=0`` in dune-bar  ``#define OPTION_FOO 1``         -
``OPTION_FOO=0`` in dune-foo & ``OPTION_FOO=0`` in dune-bar           -                   -
``OPTION_FOO=0`` in dune-foo & ``OPTION_FOO=1`` in dune-bar           -                ``#define OPTION_FOO 1``
===========================================================  ========================  ========================

Whether the combination of those options are valid and possible, is a
responsibility of the developer of the ``dune-foo`` module. Very often the
second combination is invalid and impossible to generate whereas the fourth is a
feature for late discovery of dependencies.

Since both ``dune-foo-config.hh`` and ``config.h`` are guarded with a
``HAVE_DUNE_FOO_CONFIG_HH``, the first apearence will determine which version is
being used. Thus, an executable in dune-bar with the following structure

.. code-block::
    :caption: dune-bar.cc

    #ifdef HAVE_CONFIG_H
    #include "config.h"
    #endif

    #include <dune/foo/feature.hh> // already includes dune-foo-config.hh

    /*...*/


will be use the lazy or the eager configuration options depending on whether
``HAVE_CONFIG_H`` is defined.

In the mid term, we want to remove the lazy configuration file generation
because:

* Projects aren't fully configured even after installation and distribution.
  Thus, c++ source code has unclear interpretation for the consumer of the
  project.
* In practice, this forces all downstream users to also be *dune project* in the
  sense of the cmake build system.


.. _policies:

Dune Build System Policies
==========================

Similar to the cmake `policy system <https://cmake.org/cmake/help/latest/manual/cmake-policies.7.html>`_
we have introduced a way to maintain backwards compatibility in the behavior of our
build system across multiple versions of Dune. A policy is an identifier that controls
whether the ``OLD`` or the ``NEW`` introduced behavior should be used for a given
Dune module. By default, the ``OLD`` behavior is still enabled, unless a module author
explicitly changes it to  ``NEW``, or unless a specific version is reached where the ``NEW``
behavior is enabled by default.

The `OLD` behavior is deprecated by default and results in an author warning about unhandled
policies. However, if it is explicitly set to `OLD` or `NEW`, the warning disappears. If you do
not want to see any policy warnings and just stick with the defaults, the global option
``DUNE_POLICY_DISABLE_WARNING`` can be set to ``TRUE``.

Policies Introduced by Dune 2.10
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``DP_DEFAULT_INCLUDE_DIRS`` (dune-common, set to NEW by default in version 2.12)
    *OLD behavior:* Set global ``include_directories`` when creating a new ``dune_project``.
    *NEW behavior:* Include directories must be set on a module library targets and are not set globally anymore.

    The NEW behavior requires module authors to set include directories on all their targets
    or to link against another target that contains these include directories already. With
    CMake you can use the command ``target_include_directories(<target> <scope> <dirs>...)``
    to set directories ``<dirs>...`` as include directories for the target ``<target>``.

    If the module follows a common directory layout, a default set of include directories
    can be set on a target at once using the CMake function ``dune_default_include_directories(<target> <scope>)``.

Policies Introduced by Dune 2.11
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``DP_TEST_ADD_ALL_FLAGS`` (dune-common, set to NEW by default in version 2.13)
    *OLD behavior:* Automatically call ``add_dune_all_flags`` on all test targets inside of ``dune_add_test``.
    *NEW behavior:* Flags must be set for each test target separately, e.g., by using ``add_dune_pkg_flags``, or in directory scope using ``dune_enable_all_packages``.

.. _faq:

--------------------------------
Frequently Asked Questions (FAQ)
--------------------------------


How do I use Dune with CMake?
=============================

The build process is controlled by the script :code:`dunecontrol`, located in :code:`dune-common/bin`.
There is a compatibility layer that will translate all the configure flags from your opts file into the corresponding
CMake flags. While this is a great tool to determine how to do the transition, in the long run you should switch to
a CMake-only approach.

:code:`dunecontrol` will pickup the variable :code:`CMAKE_FLAGS` from your opts file and use it as command line options for
any call to CMake. There, you can define variables for the configure process with CMake's :code:`-D` option; just as
with the C pre-processor.

The most important part of the configure flags is to tell the build system where to look for external libraries.
You can browse the :ref:`variableref` section of this documentation for a list of variables that are picked up
by the Dune CMake build system.

.. _whatfiles:

What files in a dune module belong to the CMake build system?
=============================================================

Every directory in a project contains a file called :code:`CMakeLists.txt`, which is written in the CMake language.
You can think of these as a distributed configure script. Upon configure, the top-level :code:`CMakeLists.txt` is executed.
Whenever an :code:`add_subdirectory` command is encountered, the :code:`CMakeLists.txt` file of that sub-directory is executed.
The top-level :code:`CMakeLists.txt` file is special, because it sets up the entire Dune module correctly. You should not delete the
auto-generated parts of it.

Additionally, a Dune module can export some cmake modules. A cmake module is a file that contains one or
more build system macros meant for downstream use. If a module provides modules, they can be found in
the subfolder :code:`cmake/modules`. The module :code:`dune-foo/cmake/modules/DuneFooMacros.cmake` in a module
:code:`dune-foo` is special however: Its contents are always executed when configuring the module
:code:`dune-foo` or any other Dune module, that requires or suggests the module :code:`dune-foo`.
This is the perfect place to put your checks for external packages, see below.

The file :code:`config.h.cmake` defines a template for the section of :code:`config.h`, that is generated by the module.

.. _flags:

How do I modify the flags and linked libraries of a given target?
=================================================================

Again, there are multiple ways to do this. The Dune build system offers macros to make this task as
easy as possible. For each external module, there is a macro :code:`add_dune_*_flags`. Those macros should
cover most flags. Example usage:

.. code-block:: cmake

    add_executable(foo foo.cc)
    add_dune_umfpack_flags(foo)
    add_dune_mpi_flags(foo)

There is also the macro :ref:`add_dune_all_flags`, which uses the same flag registry mechanism as the simplified
build system in section :ref:`simplified`.

If you want to fully control the configuration of the targets, you can do so. Build system entities such
as targets, directories and tests do have so called properties in CMake. You can access and modify those
properties via the commands :code:`get_property` and :code:`set_property`. You can for example use those
to modify a targets :code:`COMPILE_DEFINITIONS` or :code:`INCLUDE_DIRECTORIES` property:

.. code-block:: cmake

    add_executable(foo foo.cc)
    set_property(TARGET foo APPEND PROPERTY COMPILE_DEFINITIONS <somedefinition>)
    set_property(TARGET foo APPEND PROPERTY INCLUDE_DIRECTORIES <somepath>)

For a full list of properties, check the manual:

.. code-block:: bash

    cmake --help-property-list

Manually linking libraries can be done through the :code:`target_link_libraries` command instead of manually
tweaking properties.

.. _external:

How do I link against external libraries, that are not checked for by Dune?
===========================================================================

While there might be many solutions that make your application work, there is only one clean solution to this: You have
to provide a find module for the package. A find module is a CMake module that follows a specific naming scheme: For
an external package called :code:`SomePackage` it is called :code:`FindSomePackage.cmake`. Note that CMake
treats package names case sensitive. If CMake encounters a :code:`find_package(SomePackage)` line, it searches
its module include paths for this find module. A good read to get started writing a find module is
`this page <http://www.cmake.org/Wiki/CMake:How_To_Find_Libraries>`_ in the CMake wiki.

Depending on how common your external package is, you may not even need to write the find module on your own.
You can have a look at the list of find modules shipped by CMake or simply search the
internet for the module name and profit from other open-source project's work.

It is considered good style to also provide a macro :code:`add_dune_somepackage_flags`.

.. _outofsource:

What is an out-of-source build?
===============================

An out-of-source build does leave the version-controlled source tree untouched and puts all files that are
generated by the build process into a different directory -- the build directory. The build directory does mirror
your source tree's structure as seen in the following. Assume the following source directory structure:

::

   dune-foo/
     CMakeLists.txt
     dune/
       foo/
         CMakeLists.txt
     src/
       CMakeLists.txt

The generated build directory will have the following structure, where the directory :code:`build-cmake`
is a subdirectory of the source directory:

::

  build-cmake/
    Makefile
    dune/
      foo/
        Makefile
    src/
      Makefile

Using the :code:`Unix Makefiles` generator, your Makefiles are generated in the build tree, so that is where you
have to call :code:`make`. There are multiple advantages with this approach, such as a clear separation between
version controlled and generated files and you can have multiple out-of-source builds with different configurations
at the same time.

Out-of-source builds are the default with CMake. In-source builds are strongly discouraged.

By default, a subfolder :code:`build-cmake` is generated within each dune module and is used as a build directory.
You can customize this folder through the :code:`--builddir` option of :code:`dunecontrol`. Give an absolute path to
the :code:`--builddir` option, you will get something like this:

::

  build/
    dune-common/
      Makefile
    dune-foo/
      Makefile

So, instead of one build directory in every dune module, you will be able to collect all build directories in one
directory. This makes it much easier to have multiple build directories and to remove build directories.

.. _simplified:

What is the simplified build system and how do I use it?
========================================================

Dune offers a simplified build system, where all flags are added to all targets and all libraries are linked to all targets. You can enable the feature
by calling :ref:`dune_enable_all_packages` in the top-level :code:`CMakeLists.txt` file of your project, before you add any subdirectories.

This will modify all targets in the directory of the :code:`CMakeLists.txt`, where you put this, and also in all
subdirectories. The compile flags for all found external packages are added to those targets and the target is
linked against all found external libraries.

To use this while using custom external packages, you have to register your flags to the mechanism.
Also, some special care has to be given, if your module does build one or more library which targets within the module do link against.

Carefully read the following documentation in those cases:

* :ref:`dune_enable_all_packages`
* :ref:`dune_register_package_flags`
* :ref:`dune_library_add_sources`

.. _compiler:

How do I change my compiler and compiler flags?
===============================================

In general, there are multiple ways to do this:

* Setting the CMake variables :code:`CMAKE_<LANG>_COMPILER` (with :code:`LANG` being :code:`C`
  or :code:`CXX`) from the opts file, e.g. via :code:`CMAKE_FLAGS="-DCMAKE_CXX_COMPILER=otherc++"`.
* Setting those variables within the project with the :code:`set` command
* Setting the environment variables :code:`CC`, :code:`CXX`, :code:`FC` etc.

The first option is the recommended way. Whenever you change your compiler, you should delete all build
directories. For some CMake versions, there is a known CMake bug, that requires you to give an absolute path
to your compiler, but Dune will issue a warning, if you violate that.

You can modify your default compiler flags by setting the variables
:code:`CMAKE_<LANG>_FLAGS` in your opts file (again with :code:`LANG` being :code:`C` or
:code:`CXX`).

.. _cxxstandard:

How to set the C++ standard?
============================

If you want to enforce a more recent C++ standard than used in the Dune core modules, use the CMake
command :code:`target_compile_features(<target> <PRIVATE|PUBLIC|INTERFACE> cxx_std_[17|20|23|...])`
to set the requirement on your target directly. Note, it is recommended to compile all modules with
the same C++ standard. This can be achieved by setting the global CMake variable
:code:`CMAKE_CXX_STANDARD` to the requested maximal standard for all dune modules.

.. _symlink:

How should I handle ini and grid files in an out-of-source-build setup?
=======================================================================

Such files are under version control, but they are needed in the build directory.
There are some CMake functions targeting this issue:

* :ref:`dune_symlink_to_source_tree`
* :ref:`dune_symlink_to_source_files`
* :ref:`dune_add_copy_command`
* :ref:`dune_add_copy_dependency`
* :ref:`dune_add_copy_target`

The simplest way to solve the problem is to set the variable :ref:`DUNE_SYMLINK_TO_SOURCE_TREE` to your opts file.
This will execute :ref:`dune_symlink_to_source_tree` in your top-level :code:`CMakeLists.txt`. This will add a symlink
:code:`src_dir` to all subdirectories of the build directory, which points to the corresponding directory of the source
tree. This will only work on platforms that support symlinking.

.. _ides:

How do I use CMake with IDEs?
=============================

As already said, CMake is merely a build system generator with multiple backends (called a generator). Using IDEs requires
a different generator. Check :code:`cmake --help` for a list of generators. You can then add the :code:`-G` to the :code:`CMAKE_FLAGS` in your opts file.
Note that the generator name has to match character by character, including case and spaces.

To configure highlighting of CMake errors in Emacs' compilation mode, include
the following in your :code:`~./emacs` (see the `Emacs bug
<http://debbugs.gnu.org/cgi/bugreport.cgi?bug=22944>`_):

.. code-block:: elisp

   (setq compilation-error-regexp-alist-alist
      `((cmake "^CMake \\(?:Error\\|\\(Warning\\)\\) at \\(.*\\):\\([1-9][0-9]*\\) ([^)]+):$"
               2 3 nil (1))
        (cmake-info "^  \\(?: \\*\\)?\\(.*\\):\\([1-9][0-9]*\\) ([^)]+)$"
               2 3 nil 0)
        . ,compilation-error-regexp-alist-alist))

Then customize the option :code:`compilation-error-regexp-alist` and add the
two predefined symbols :code:`cmake` and :code:`cmake-info` to the list.

.. _cxxflags:

I usually modify my CXXFLAGS upon calling make. How can I do this in CMake?
===========================================================================

This violates the CMake philosophy and there is no clean solution to achieve it. The CMake-ish solution would be
to have for each configuration one out-of-source build. We have nevertheless implemented a workaround. It can be enable
by setting the variable :ref:`ALLOW_CXXFLAGS_OVERWRITE` in your opts file. You can then type:

.. code-block:: bash

   make CXXFLAGS="<your flags>" <target>

Furthermore any C pre-processor variable of the form :code:`-DVAR=<value>` can be overloaded on the command line
and the grid type can be set via :code:`GRIDTYPE="<grid type>"`.

Note this only works with generators that are based on Makefiles and several Unix tools like bash must be
available.

.. _test:

How do I run the test suite from CMake?
=======================================

The built-in target to run the tests is called :code:`test` instead of Autotools' :code:`check`.
It is a mere wrapper around CMake's own testing tool CTest. You can check :code:`ctest --help`
for a lot of useful options, such as choosing the set of tests to be run by matching regular expressions or
showing the output of failed tests.

The test programs are not built automatically. You need to build them manually
before running them using :code:`make build_tests`.

The Dune test suite also defines tests that run in parallel. You may set an upper bound to the number
of cores in use for a single test by setting :ref:`DUNE_MAX_TEST_CORES`.

.. _disable:

Can I disable an external dependency?
=====================================

To disable an external dependency :code:`Foo`, add

::

   -DCMAKE_DISABLE_FIND_PACKAGE_Foo=TRUE

to your opts file. The name of the dependency is case sensitive but there is no canonical naming
scheme. See the output of configure to get the right name.

Make sure to not use cached configure results by deleting the cache file or the build directory, cf.
:ref:`troubleshoot`.

.. _parallel:

How do I switch between parallel and sequential builds?
=======================================================

Dune builds with CMake are parallel if and only if MPI is found. To have a sequential build despite an
installed MPI library, you have to explicitly disable the corresponding find module by setting

::

   -DCMAKE_DISABLE_FIND_PACKAGE_MPI=TRUE

in the :code:`CMAKE_FLAGS` of your opts file, as described in section :ref:`disable`.

.. _headercheck:

Why is it not possible anymore to do make headercheck?
======================================================

The headercheck feature has been disabled by default. You can enable it by setting the CMake variable :ref:`ENABLE_HEADERCHECK`
through your opts file. This step has been necessary, because of the large amount of additional file the headercheck adds to the
build directory. A better implementation has not been found yet, because it simply does not fit the CMake philosophy.

.. _packages:

How do I create tarballs or packages?
=====================================

To create source code packages, also known as tarballs, run `git archive` within your
module's Git repository.

There is no default way to create binary packages like Deb or RPM packages. You can use
the Open Build Service for openSuse RPMs and related distributions. Or create packages according
to the distribution of your choice like the tools around dpkg-buildpackage and debuild
for Debian.

CMake has a packaging tool CPack, but with CPack you are on your own. In the past, our
results based on CPack were not satisfying.

.. _dune-python:

How does the Dune build system handle Python?
=============================================

dune-common contains a build system extension to handle many python-related aspects. You can
read more on this in the module description :ref:`DunePythonCommonMacros` and the pieces of
documentation mentioned inthere.

.. _troubleshoot:

How do I troubleshoot?
======================

CMake caches aggressively which makes it bad at recognizing changed configurations.
To trigger a fresh run of configure, you can delete the :code:`CMakeCache.txt` file from
the build directory and maybe save some compilation time afterward.

Whenever you experience any problems, your first step should be to delete all build directories. Nice trick:

::

   dunecontrol exec "rm -rf build-cmake"

This will remove all build directories from all DUNE modules.

Later on you can get an error log from the file :code:`CMakeError.log` in the :code:`CMakeFiles`
subdirectory of your build directory. This is what you should send to the mailing list alongside the
description of your setup and efforts to help us help you.

Where can I get help?
=====================

The CMake manual is available on the command line:

* :code:`cmake --help-command-list`
* :code:`cmake --help-command <command>`
* :code:`cmake --help-property-list`
* :code:`cmake --help-property <property>`
* :code:`cmake --help-module-list`
* :code:`cmake --help-module <module>`

To get help on which variables are picked up by CMake, there is a CMake wiki page collecting them.
Of course, there is also Google, StackOverflow and the CMake Mailing list (archive).
For problems specific to DUNE's build system, ask on our mailing lists.
