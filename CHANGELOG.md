<!--
SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
-->

# Master (will become release 2.9)

- Added feature test for lambdas in unevaluated contexts
  `DUNE_HAVE_CXX_UNEVALUATED_CONTEXT_LAMBDA`. When defined, the C++ language
  allows to declare lambdas in unevaluated contexts, e.g., `F = decltype([](){})`.

## Dependencies
In order to build the DUNE core modules you need at least the following software:

- C++ compiler supporting c++-17 language standard, e.g., LLVM Clang >= 5, GCC g++ >= 7
- CMake >= 3.13
- Optional: pkg-config to find other optional dependencies
- Optional: Python >= 3.7 for Python bindings

## Changelog
- Multiplication of two matrices using `a*b` is now also implemented if `a` or `b`
  is a `FieldMatrix` or if both are `DiagonalMatrices`.

- The utility function `transpose(m)` will now return `m.transposed()` if available.
  Otherwise it returns a wrapper storing a copy (this was a reference before) of `m`.
  References to matrices can still be captured using `transpose(std::ref(m))` or
  `transposedView(m)`.

- The transposed of a `FieldMatrix`, `DiagonalMatrix`, and `DynamicMatrix`
  is now available using the `transposed()` member function.

- Add helper function `resolveRef()` to transparently use `std::reference_wrapper`.

- Add `pragma omp simd` annotations in the LoopSIMD class to improve compiler optimizations

- deprecate Factorial in common/math.hh (use factorial function)

- Add `python -m dune [info|configure|list|remove|dunetype|fix-dunepy]` command to manage just-in-time generated python modules in dune-py

- The storage type `ReservedVector` is extended to follow more closely the `std::vector` and
  `std::array` interfaces.

## Build System

- Improve the the function `dune_add_library` by separating the target types normal, interface, and
  object. Additional properties can be passed to the cmake function like `LINK_LIBRARIES`,
  `OUTPUT_NAME`, and `EXPORT_NAME`

- Remove the variable `DUNE_DEFAULT_LIBS`

- Deprecate cmake file `DuneCMakeCompat.cmake` that just contained the removed function
  `dune_list_filter`

- Remove deprecated cmake file `DuneMPI.cmake`

- Overhaul of the handling of Dune python packages:
  python bindings are now enabled by default. Packages are automatically installed
  either in an internal virtual environment or in an active environment during the
  module build process.
  See https://gitlab.dune-project.org/core/dune-common/-/merge_requests/960
  which also contains a detailed set of instructions on how to update
  existing python bindings.

- Deprecated `dune_python_install_package`. Use either
  `dune_python_configure_bindings` (for Python bindings)
  `dune_python_configure_package` (for pure Python package).
  See https://gitlab.dune-project.org/core/dune-common/-/merge_requests/1148
  for more details.
  Note that this MR also includes
  https://gitlab.dune-project.org/core/dune-common/-/merge_requests/1103:
  the Python bindings are not installed editable during the configure stage
  instead of the build stage.

- dune-py is now build using a simple 'Makefile' per module instead of
  relying on cmake for each module. The old cmake builder can still be used
  by exporting the environment variable `DUNE_PY_USE_CMAKEBUILDER=1`.

- Remove deprecated cmake function overload `target_link_libraries`

- Deprecate cmake function `remove_processed_modules`

- The CI system now checks for common spelling mistakes using the `codespell` tool.

## Deprecations and removals

- Helper fallback implementations for `Std::to_false_type`, `Std::to_true_type`,
  `Std::is_invocable`, and `Std::is_invocable_r` have been removed. Instead,
  use `Dune::AlwaysFalse`, `Dune::AlwaysTrue`, `std::is_invocable`, and
  `std::is_invocable_r`.

- The deprecated headers `gcd.hh` and `lcm.hh` are removed. Use `std::gcd`
  and `std::lcm` instead.

- Both deprecated macros `DUNE_DEPRECATED` and `DUNE_DEPRECATED_MSG(text)`
  are removed. Use C++14 attribute `[[deprecated]]`. However, be aware
  that it is no drop-in replacement, as it must be sometimes placed at
  different position in the code.

- The macros `DUNE_UNUSED` is removed Use C++17's attribute
  `[[maybe_unused]]` instead, but be aware that it is no drop-in
  replacement, as it must be sometimes placed at a different position
  in the code.

- In `callFixedSize`, support handles with `fixedsize()` (lower case s)
  is removed. Implement `fixedSize()` (camelCase) instead.

- `CollectiveCommunication` and `getCollectiveCommunication` are deprecated
  and will be removed after Dune 2.9. Use `Communication` respectively
  `getCommunication` instead.

- The header `power.hh` is deprecated. Use `Dune::power` from
  `math.hh` instead.

- The deprecated compatibility header `optional.hh` is removed. Include
  `<optional>` instead.

- The compatibility header `make_array.hh` is deprecated and will be
  removed after Dune 2.8. Use deduction guide of `std::array` or
  `std::to_array`.

# Release 2.8

- Set minimal required CMake version in cmake to >= 3.13.

- Python bindings have been moved from dune-python to the core
  respective core modules.

- Add `instance` method to MPIHelper that does not expect arguments for access
  to the singleton object after initialization.

- Remove the cmake check for `HAVE_MPROTECT` and also do not define this variable in the
  `config.h` file. It is defined only inside the header `debugallocator.hh`.

- Remove deprecated type-traits `has_nan`, `is_indexable`, and
  `is_range`, use the CamelCase versions instead.

- Deprecate fallback implementations `Dune::Std::apply`, `Dune::Std::bool_constant`, and
  `Dune::Std::make_array` in favor of std c++ implementations.

- Deprecate type traits `Dune::Std::to_false_type`, `Dune::Std::to_true_type`.
  `Dune::AlwaysFalse` and `Dune::AlwaysTrue` (from header `dune/common/typetraits.hh`)
  now inherit from `std::true_type` and `std::false_type` and are therefore
  exact replacements for these two type traits.

- Deprecate fallback implementation `Dune::Std::conjunction`, `Dune::Std::disjunction`,
  and `Dune::Std::negation`. Use std c++17 implementations.

- Deprecate fallback implementations `Dune::Std::is_callable` and `Dune::Std::is_invocable`.
  Use C++17 std implementation `std::is_invocable` instead. Be aware that
  `Dune::Std::is_callable` and `std::is_invocable` are slightly different concepts,
  since `std::is_invocable` also covers invocation of pointers to member functions
  and pointers to data members. To additionally constrain for that case,
  there is now `Dune::IsCallable` (in `dune/common/typetraits.hh`)

- Added `Dune::IsCallable` (in `dune/common/typetraits.hh`) which is
  an improved version of the deprecated `Dune::Std::is_callable` and allows
  for checking if a type is a function object type,
  i.e. has a ()-operator than can be invoked with the given argument types and
  returns a specified return type.

- Remove c++ feature tests in cmake for existing c++-17 standards. Add default
  defines for `DUNE_HAVE_CXX_BOOL_CONSTANT`, `DUNE_HAVE_CXX_EXPERIMENTAL_BOOL_CONSTANT`,
  `DUNE_HAVE_HEADER_EXPERIMENTAL_TYPE_TRAITS`, `DUNE_HAVE_CXX_APPLY`,
  `DUNE_HAVE_CXX_EXPERIMENTAL_APPLY`, `HAVE_IS_INDEXABLE_SUPPORT` in `config.h` for one
  more release.

- Add backport of `FindPkgConfig.cmake` from cmake 3.19.4 since there was a bug in
  an older find module leading to problems finding tbb in debian:10.

- Update the FindTBB cmake module to search for the `TBBConfig.cmake` or the `tbb.pc`
  file containing the configuration. Add the `AddTBBFlags.cmake` file containing
  the macro `add_dune_tbb_flags` that must be called to use TBB.

- Set minimal required MPI version to >= 3.0.

- Previous versions of dune-common imported `std::shared_ptr` and `std::make_shared`
  into the `Dune` namespace.  dune-common-2.8 stops doing that.

- The file `function.hh` is deprecated.  It contained the two base classes
  `Function` and `VirtualFunction`.  In downstream codes, these should be
  replaced by C++ function objects, `std::function` etc.

- Python bindings have been moved from the `dune-python` module which is now
  obsolete. To activate Python bindings the CMake flag
  `DUNE_ENABLE_PYTHONBINDINGS` needs to be turned on (default is off).
  Furthermore, flags for either shared library or position independent code
  needs to be used.

- Support for distributing DUNE modules as python packages has been added.
  Package meta data is parsed in `packagemetadata.py` from the dune.module file.
  A script `/bin/dunepackaging.py` was added to generate package files
  (`setup.py`, `pyproject.toml`) that can also be used to upload packages to
  the Python Package Index. For a brief description of what is required to add
  this support to existing dune modules see
  https://gitlab.dune-project.org/core/dune-common/-/merge_requests/900
  Note that this can also be used to generate a package for dune modules
  that don't provide Python bindings.

- Eigenvectors of symmetric 2x2 `FieldMatrix`es are now computed correctly
  even when they have zero eigenvalues.

- Eigenvectors and values are now also supported for matrices and
  vectors with field_type being float.

- The `ParameterTreeParser::readINITree` can now directly construct and
  return a parameter tree by using the new overload without parameter tree
  argument.

- MPIHelper::instance can now be called without parameters if it was
  already initialized.

- MPITraits now support complex.

- There is now a matrix wrapper transpose(M) that represents the
  transpose of a matrix.

## build-system

- The name mangling for Fortran libraries like BLAS and LAPACK is now done
  without a Fortran compiler. So a Fortran compiler is no longer a built
  requirement.

- `dune_list_filter` is deprecated and will be removed after Dune 2.8. Use
  `list(FILTER ...)` introduced by CMake 3.6 instead.

- `ToUniquePtr` is deprecated and will be removed after Dune 2.8. Use
  `std::unique_ptr` or `std::shared_ptr` instead.

- Remove the CMake options `DUNE_BUILD_BOTH_LIBS` and
  `DUNE_USE_ONLY_STATIC_LIBS`. Use the default CMake way instead by
  setting `BUILD_SHARED_LIBS` accordingly. Building both static
  and shared libraries is no longer supported.

- Remove the CMake function deprecated `inkscape_generate_png_from_svg`.

- Remove the old and deprecated use of UseLATEX.cmake.
  `dune_add_latex_document' is a redirection to `add_latex_document`
  which internally uses `latexmk`.

- Many of the CMake find modules habe been rewritten to use CMake's
  imported targets. These targets are also used in the DUNE CMake
  package configuration files, where they might appear in e.g. the
  dune-module_LIBRARIES. If you do not use the DUNE CMake build system
  the linker might complain about e.g. METIS::METIS not being
  found. In that case your either need to use the CMake modules shipped with
  DUNE or create these targets manually.

## Deprecations and removals

- Remove deprecated header `dune/common/std/memory.hh`; use `<memory>`
  instead.

- Deprecate header `dune/common/std/utility.hh`; use `<utility>` instead.

- Deprecate header `dune/common/std/variant.hh`; use `<variant>` instead.

- Remove incomplete CPack support that was never used to make an official
  build or tarball.

- Both macros `DUNE_DEPRECATED` and `DUNE_DEPRECATED_MSG(text)` are
  deprecated and will be removed after Dune 2.8. Use C++14 attribute
  `[[deprecated]]` but be aware that it is no drop-in replacement,
  as it must be sometimes placed at different position in the code.

- The macros `DUNE_UNUSED` is deprecated and will be removed after
  Dune 2.8. Use C++17's attribute `[[maybe_unused]]` instead, but be
  aware that it is no drop-in replacement, as it must be sometimes
  placed at different position in the code.
  The use of `DUNE_UNUSED_PARAMETER` is discouraged.

- Dune::void_t has been deprecated and will be removed. Please use
  std::void_t

- Dune::lcd and Dune::gcd are deprecated and will be removed. Please
  use std::lcd and std::gcd.

- VariableSizeCommunicator::fixedsize has been renamed to FixedSize in
  line with the communicator changes of dune-grid. The old method will
  be removed in 2.9.

# Release 2.7

- Added fallback implementation to C++20 feature: `std::identity`.

- A helper class `TransformedRangeView` was added representing a
  transformed version of a given range using an unary transformation
  function. The transformation is done on the fly leaving the wrapped
  range unchanged.

- `dune-common` now provides an implementation of `std::variant` for all compilers
  that support C++14.  It is contained in the file `dune/common/std/variant.hh`,
  in the namespace `Dune::Std::`.  If your compiler does support C++17 the
  implementation in `dune-common` is automatically disabled, and the official
  implementation from the standard library is used instead.

- By popular demand, dense vectors and matrices like `FieldVector` and `FieldMatrix`
  now have additional operators.  In particular, there are
  - Vector = - Vector
  - Matrix = - Matrix
  While these two work for any vector or matrix class that inherits from `DenseVector`
  or `DenseMatrix`, the following additional methods only work for `FieldVector`:
  - Vector = Scalar * Vector
  - Vector = Vector * Scalar
  - Vector = Vector / Scalar
  Correspondingly, the `FieldMatrix` class now has
  - Matrix = Matrix + Matrix
  - Matrix = Matrix - Matrix
  - Matrix = Scalar * Matrix
  - Matrix = Matrix * Scalar
  - Matrix = Matrix / Scalar
  - Matrix = Matrix * Matrix
  Note that the operators
  - Vector = Vector + Vector
  - Vector = Vector - Vector
  have been introduced earlier.

- The matrix size functions `N()` and `M()` of `FieldMatrix` and `DiagonalMatrix` can now be used
  in a `constexpr` context.

- There is now (finally!) a method `power` in the file `math.hh` that computes
  powers with an integer exponent, and is usable in compile-time expressions.
  The use of the old power methods in `power.hh` is henceforth discouraged.

-   `FieldMatrix` and `FieldVector` are now [trivially copyable types]
    if the underlying field type is trivially copyable.

    As a consequence the copy assignment operator of the `DenseVector`
    class can no longer be used; just avoid going through
    `DenseVector` and use the real vector type instead
    (e.g. `FieldVector`).

    [trivially copyable types]: https://en.cppreference.com/w/cpp/named_req/TriviallyCopyable

## Deprecations and removals

- The `VectorSize` helper has been deprecated.  The `size()` method of
  vectors should be called directly instead.

- Drop support for Python 2. Only Python 3 works with Dune 2.7.

- Support for older version than METIS 5.x and ParMETIS 4.x is deprecated and will be
  removed after Dune 2.7.

- Deprecated header `dune/common/parallel/collectivecommunication.hh` which will be
  removed after Dune 2.7. Use dune/common/parallel/communication.hh instead!

- Deprecated header `dune/common/parallel/mpicollectivecommunication.hh` which will be
  removed after Dune 2.7. Use dune/common/parallel/mpicommunication.hh instead!

## build-system

- When run with an absolute build directory, `dunecontrol` now exposes the root build
  directory to CMake in the variable `DUNE_BUILD_DIRECTORY_ROOT_PATH`.

  See core/dune-common!542

- The `dune_symlink_to_sources_files` CMake function now has a `DESTINATION` argument.

- Dune no longer applies architecture flags detected by the Vc library
  automatically.  This applies to all targets that link to Vc explicitly (with
  `add_dune_vc_flags()`) or implicitly (with `dune_enable_all_packages()`).
  If you do want to make use of extended architecture features, set the
  architecture explicitly in the compiler options, e.g. by specifying
  ```sh
  CMAKE_FLAGS="-DCMAKE_CXX_FLAGS=-march=native"
  ```
  in your opts-file.  Vc also sets compiler options to select a particular C++
  abi (`-fabi-version` and `-fabi-compat-version`), these continue to be
  applied automatically.

  See core/dune-common!677

- `FindParMETIS.cmake` assumes METIS was found first using `FindMETIS.cmake` and does not
  longer try to find METIS itself.

- The `inkscape_generate_png_from_svg` CMake function is deprecated and will be removed
  after 2.7.

- LaTeX documents can now be built using `latexmk` with the help of UseLatexmk.cmake's
  `add_latex_document`. `dune_add_latex_document` will use the new way of calling
  LaTeX when the first argument is `SOURCE`. As a side effect, in-source builds are
  supported, too. The old function call and UseLATEX.cmake are deprecated and will be
  removed after 2.7.

  See core/dune-common!594

- The build system has learned some new tricks when creating or looking for the Python virtualenv:
  When using an absolute build directory with `dunecontrol`, the virtualenv will now be placed
  directly inside the root of the build directory hierarchy in the directory `dune-python-env`.
  This should make it much easier to actually find the virtualenv and also avoids some corner
  cases where the build system would create multiple virtualenvs that did not know about each
  other. This behavior can be disabled by setting
  `DUNE_PYTHON_EXTERNAL_VIRTUALENV_FOR_ABSOLUTE_BUILDDIR=0`.
  If you need even more precise control about the location of the virtualenv, you can now also
  directly set the CMake variable `DUNE_PYTHON_VIRTUALENV_PATH` to the directory in which to
  create the virtualenv.

# Release 2.6

**This release is dedicated to Elias Pipping (1986-2017).**

- New class `IntegralRange<integral_type>` and free standing function
  `range` added, providing a feature similar to Python's `range` function:
  ```
    for (const auto &i : range(5,10))
  ```
  See core/dune-common!325

- `Dune::array` was deprecated, use `std::array` from <array> instead.
   Instead of `Dune::make_array`, use `Dune::Std::make_array`
   from dune/common/std/make_array.hh
   and instead of `Dune::fill_array` use `Dune::filledArray`
   from dune/common/filledarray.hh.`

    See core/dune-common!359

- The `DUNE_VERSION...` macros are deprecated use the new macros
  `DUNE_VERSION_GT`, `DUNE_VERSION_GTE`, `DUNE_VERSION_LTE`, and
  `DUNE_VERSION_LT` instead.

    See core/dune-common!329

- Added some additional fallback implementation to C++17 features:
  (e.g. `optional`, `conjunction`, `disjunction`)

- `makeVirtualFunction`:
  allows to easily convert any function object (e.g. lambda) to a `VirtualFunction`

    See core/dune-common!282

- Added infrastructure for explicit vectorization *(experimental)*

    We added experimental support for SIMD data types. We currently
    provide infrastructure to use [Vc](https://github.com/VcDevel/Vc)
    and some helper functions to transparently switch between scalar data
    types and SIMD data types.

- `FieldMatrix` now has experimental support for SIMD types from
  [Vc](https://github.com/VcDevel/Vc) as field types.

    See core/dune-common!121

## build-system

- Variables passed via `dunecontrol`'s command `--configure-opts=..` are now
  added to the CMake flags.

- Bash-style variables which are passed to `dunecontrol`'s command `configure-opts`
  are no longer transformed to their equivalent CMake command. Pass
  `-DCMAKE_C_COMPILER=gcc` instead of `CC=gcc`.

- Added support for modules providing additional Python modules or bindings.
