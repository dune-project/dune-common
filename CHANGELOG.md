<!--
SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
-->

# Master (will become release 2.11)

## Build system: Changelog

- Propagate dependencies of `dune-common` when consumed. This means that a CMake project may find
  `dune-common` and consume the `Dune::Common` target without the need of the dune build system.

- Change the way include directories are set in dune projects. OLD behavior: use `include_directories`
  in `dune_project` to set include dirs for the current project. NEW behavior: Provide a utility
  `dune_default_include_directories` to set include dirs on targets manually. Which behavior to
  activate can be decided in each module by using the new dune policy `DP_DEFAULT_INCLUDE_DIRS`, which can be set
  to `OLD` or `NEW` correspondingly.

- The CMake function `dune_target_enable_all_packages` can now handle Interface libraries too.

- Add a module-specific CMake target `build_<module>_tests` to compile only tests
  associated to a specific `<module>`. Additionally, add the `<module>`-name as
  `LABEL` property to all tests created with `dune_add_tests` in that module. This
  allows to run these tests with `ctest -L <module>`.

## C++: Changelog

- `DUNE_THROW` no longer prevents functions from being used in `constexpr` contexts,
  as long the exception is not thrown. As a sideproduct, the macros now
  also supports the syntax `DUNE_THROW(ExceptionType, a << b) << c << d` and
  `DUNE_THROW(ExceptionType) << a << b`.

- Add concepts `Std::three_way_comparable` and `Std::three_way_comparable_with` as well as an
  algorithm `Std::lexicographical_compare_three_way` to provide library utilities for the `<=>`
  comparison operator.

- Add deduction guides to `TupleVector` analogous to `std::tuple`.

- Add concept definition `Std::indirectly_copyable` to constrain the `DenseMatrixAssigner`.

- Add concept definition `Concept::Number` to represent scalar number types in containers.

## C++: Deprecations and removals

- Deprecate the utility `integerSequenceEntry` in favour of the shorter `get` from `integersequence.hh`.


# Release 2.10

## Dependencies
In order to build the DUNE core modules you need at least the following software:

- C++ compilers LLVM Clang >= 10 or GCC g++ >= 9
- CMake >= 3.16
- Optional: pkg-config to find other optional dependencies
- The Python bindings require at least Python 3.7 or higher. This is now enforced
  through CMake. The bindings are disabled prompting the user with a message containing
  the reason if no suitable Python version is found.


## C++: Changelog

- Fix bug where `AlignedNumber` could not check if placement `new` alignment is correct.

- `TupleVector` now implements the standard protocol for tuple-like types.

- There is a new base class `IteratorFacade` that unifies `ForwardIteratorFacade`,
  `BidirectionalIteratorFacade`, `RandomAccessIteratorFacade` by making the iterator
  category a template. Furthermore the new `IteratorFacade`  class allows to specify
  a `pointer` type other than `value_type*` to support proxy-iterators.
  The old facade classes remain unchanged for backward compatibility reasons.

- Add utilities `Std::span`, `Std::mdspan`, `Std::mdarray` and all its related classes into the `Dune::Std` namespace
  for working with multidimensional arrays. See core/dune-common!1334 for further details.

- The construction of `FiedlMatrix` and `FieldVector` from `std::initializer_list`
  is now `constexpr`.

- Add concepts directory `dune/common/concepts/` and some fundamental concept definitions using
  c++20-concepts. Those concepts are still experimental and are marked with the new `doxygen`
  command `\experimental`. Additionally, the preprocessor constant `DUNE_ENABLE_CONCEPTS` is
  provided when including `dune/common/concepts.hh` that tells whether the library and compiler
  understand the new c++20-concepts and the concept definitions can be used. This constant can also
  be set manually as a compilerflag to enforce enabling or disabling these features.

- Two concept definitions are added: `Dune::Concept::Hashable` and
  `Dune::Concept::[RandomAccess]Container` in `dune/common/concepts/` directory.

- Add user-defined literals `_ic`, `_uc` and `_sc` to represent integral constants.

- Add "hybrid" functors for basic math operations with integral constant arguments, i.e.,
  `Hybrid::max`, `Hybrid::min`, `Hybrid::plus`, `Hybrid::minus`, and `Hybrid::equal_to`. Operations
  between two integral constants result in an integral constant, whereas operations with at least
  one non integral constant argument is performed on the underlying value type.

- Make `filledArray` compatible with non-default-constructble types.

- Add utility `CopyableOptional` that allows to wrap types that are copy constructible but not
  copy assignable and provide assignment operations based on the constructors.

- Added the methods `checkThrow`,`requireThrow` and the corresponding `checkNoThrow`,
  `requireNoThrow` to the `Dune::TestSuite` to test for throwing and no throwing of exceptions.

- Add the utility `IsCompileTimeConstant` to check for integral constants and anything with
  the same interface.

- Add dedicated includes `dune/common/metis.hh` for METIS and `dune/common/parallel/parmetis.hh`
  for ParMETIS to be used instead of the direct includes `metis.h` and `parmetis.h`.

- Add utilities and algorithms to work with `std::integer_sequences`.

- Fixed MPI implementation of `Communication::isend`.

- Move special MPI-related compile flags, like `MPI_NO_CPPBIND`, from `config.h` into
  the cmake utiltiy `add_dune_mpi_flags` and the related global package registration.

- Add new utility type `IndexedIterator` that extends a given iterator by an `index()`
  method returning a traversal index.

- Add a macro `DUNE_NO_UNIQUE_ADDRESS` that expands to the attribute `[[no_unique_address]]`
  or something similar, if supported by the compiler.

## C++: Deprecations and removals

- Remove deprecated macros `DUNE_VERSION_NEWER` and `DUNE_VERSION_NEWER_REV`, use `DUNE_VERSION_GTE`
  and `DUNE_VERSION_GTE_REV` instead. There was no deprecation compiler warning.

- The deprecated header `dune/common/function.hh` has been removed. Use C++ function
  objects and `std::function` stuff instead!

- The deprecated header `dune/common/std/utility.hh` has been removed. Use `<utility>`
  instead.

- The deprecated header `dune/common/std/variant.hh` has been removed. Use `<variant>`
  instead.

- The deprecated header `dune/common/to_unique_ptr.hh` has been removed. Use
  `std::unique_ptr` or `std::shared_ptr` instead.

- Deprecated `conjunction`, `disjunction`, and `negation` have been removed. Instead,
  use the structs from `<type_traits>` introduced with C++17.

- Remove deprecated `dune/common/std/apply.hh`, use `std::apply` instead.

- Deprecated the file `dune/common/assertandreturn.hh` and the contained utility
  `DUNE_ASSERT_AND_RETURN`. Use `assert()` macro directly in `constexpr` functions.

- Remove deprecated header `power.hh`. Use `Dune::power` from `math.hh` instead.

- Deprecate class `SizeOf`. Use `sizeof...` instead.

- Deprecate header `dune/common/keywords.hh` and the provided macros
  `DUNE_GENERALIZED_CONSTEXPR` and `DUNE_INLINE_VARIABLE`. Use the key words directly.

- Remove deprecated header `dune/python/common/numpycommdatahandle.hh`. Use
  `dune/python/grid/numpycommdatahandle.hh` instead.

- Remove in `dune/python/common/dimrange.hh` the `DimRange` specializations for
  dune-typetree and dune-functions types. Those are moved to dune-functions.

- Deprecated `Hybrid::equals`. Use `Hybrid::equal_to` instead.

- The preprocessor constant `HAVE_UMFPACK` is deprecated. Use `HAVE_SUITESPARSE_UMFPACK` instead.

## Python: Changelog

- Python: Add `TupleVector` Python bindings

- Python: The function `cppType` now support Python tuples, which are converted to the C++ type `std::tuple`

- Python: Add a dump/load functions to dune.common.pickle which add support
  for storing and recreating the JIT generated modules required for the
  pickling of dune objects to work. In addition a class for writing time
  series of pickled data is provided.

- Python: Add a new argument class to the generator to add pickling support.
  The approach is similar to adding extra constructors or methods.

- Python: Add a new command to `dune.__main__` to compile existing modules
  in parallel, e.g., python -m dune make -j8 hierarchicalgrid
  Add the option to both 'remove' and 'make' commands to read
  module list from a file.

## Build system: Changelog

- Add a `REQUIRED` parameter to `dune_python_configure_bindings`. If set to
  `TRUE` the functions throws an error instead of a warning if the package
  setup fails. The default behavior (or setting `REQUIRE` to `FALSE`) is to
  show the warning during configuration and to continue.

- Dune package dependencies are now transitively resolved at `find_package(<dune-module>)` calls instead of waiting
  until the call to `dune_project()`. For example, a CMake call to `find_package(dune-grid)` will transitively
  find the dune packages `dune-common`, `dune-geometry` and (if available) `dune-uggrid`. Note that the targets
  provided by those found modules are still being set up at the `dune_project()` call.

- Documentation files in `doc/buildsystem/${ModuleName}.rst` are now only copied.
  Previously, they were configured through the CMake function `configure_file()`
  as a cmake template file.

- Try to find SuiteSparse during configuration.

- The function `dune_add_library(<lib> ...)` now requires to provide `EXPORT_NAME` or `NO_EXPORT`.
  Moreover, a namespace can be specified via the argument `NAMESPACE` which defaults to `Dune::` and is prepended to the export name.
  We recommend to choose an export name with a camel title case matching your
  library name (e.g., `Common`, `ISTL`, and `MultiDomainGrid` will be exported as
  `Dune::Common`, `Dune::ISTL`, and `Dune::MultiDomainGrid`).
  _Warning:_ Both `<lib>` and `Dune::${EXPORT_NAME}` are currently exported. Keep in mind that:
   * Libraries that _consume_ `Dune::${EXPORT_NAME}` will only be forward compatible with Dune 2.10.
   * Libraries that _consume_ `<lib>` will be supported until compatibility with Dune 2.9 is not required anymore.

- Generation of `config.h` is overhauled and split in a public and a private config file. Only
  the public file is installed and consumed by down-stream modules. For compatibility, the
  old single file is created, too.

- The CMake macro `finalize_dune_project` no longer has an optional argument, a config file is
  always created.

- Do not overwrite the `add_test` cmake function with an error message.

- Setting the minimal c++ standard in cmake is now done by a cmake feature-requirement
  `cxx_std_17` on the `dunecommon` library target. This requirement is propagated to all
  other modules by linking against `dunecommon`.

- We have changed the way optional dependencies are activated in the build-system internally.
  The cmake macros `add_dune_xy_flags` do not set the compiler flag `-DENABLE_XY=1` anymore, but instead
  set directly the flag `-DHAVE_XY=1`. Neither `ENABLE_XY` nor `HAVE_XY` should be modified manually
  by the user. Since the `HAVE_XY` flag is now set as a compiler flag, it is not included in the
  `config.h` files anymore.

- Add a policy system to smoothly change behavior in the build-system. This follows the cmake policy
  system but uses own IDs and is connected to dune module version instead of cmake versions.

- Rename `<module>_INTERFACE_LIBRARIES` into `<module>_LIBRARIES` (representing all module-libraries)
  and introduce `<module>_EXPORTED_LIBRARIES` as a list of all libraries exported by the module.


## Build system: Deprecations and removals

- Remove the search of (currently broken) `pkg-config` files for dune packages.

- Remove the `ALLOW_CXXFLAGS_OVERWRITE` configure option. The `CXXFLAGS`
  overload is still turned on for the JIT compiled Python modules. See the
  description of the MR
  https://gitlab.dune-project.org/core/dune-common/-/merge_requests/1251
  for more details on how to use this feature in the source modules and on
  some new feature.

- Remove deprecated `add_directory_test_target` function.

- The cmake options `CXX_MAX_STANDARD`, `CXX_MAX_SUPPORTED_STANDARD` and `DISABLE_CXX_VERSION_CHECK`
  are removed. The cmake function `dune_require_cxx_standard()` is now deprecated.

- Deprecate CMake macro `message_verbose`. Use `message(VERBOSE "message text")` instead.
  This macro will be removed after Dune 2.10.

- Remove deprecated CMake file `DuneCMakeCompat.cmake` that only contained a
  deprecation message.

- Remove deprecated CMake function `inkscape_generate_png_from_svg`. Use
  `dune_create_inkscape_image_converter_target` instead.

- Remove deprecated `rmgenerated.py`. Call `python -m dune remove` with the same
  arguments instead.

- Remove `DunePythonDeprecations.cmake` that was used to ease the overhaul
  of Python CMake integration.

- Remove deprecated CMake function `dune_python_install_package`. Use
  `dune_python_configure_bindings` or `dune_python_configure_package`
  according to the needed behavior.


# Release 2.9

## Dependencies
In order to build the DUNE core modules you need at least the following software:

- C++ compiler supporting c++-17 language standard, e.g., LLVM Clang >= 5, GCC g++ >= 7
- CMake >= 3.13
- Optional: pkg-config to find other optional dependencies
- Optional: Python >= 3.7 for Python bindings

## Changelog

- Added feature test for lambdas in unevaluated contexts
  `DUNE_HAVE_CXX_UNEVALUATED_CONTEXT_LAMBDA`. When defined, the C++ language
  allows to declare lambdas in unevaluated contexts, e.g., `F = decltype([](){})`.

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

- Add `python -m dune [info|configure|list|remove|dunetype|fix-dunepy]` command to manage
  just-in-time generated python modules in dune-py

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

- Deprecate the CMake function `inkscape_generate_png_from_svg`.

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
