# Master (will become release 2.8)

- Set minimal required MPI version in cmake to >= 3.0.

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
  the Python Package Index.

- Eigenvectors of symmetric 2x2 `FieldMatrix`es are now computed correctly
  even when they have zero eigenvalues.

## build-system

- The name mangling for Fortran libraries like BLAS and LAPACK is now done
  without a Fortran compiler. So a Fortran compiler is no longer a built
  requirement.

- `dune_list_filter` is deprecated and will be removed after Dune 2.8. Use
  `list(FILTER ...)` introduced by CMake 3.6 instead.

- Remove the CMake options `DUNE_BUILD_BOTH_LIBS` and
  `DUNE_USE_ONLY_STATIC_LIBS`. Use the default CMake way instead by
  setting `BUILD_SHARED_LIBS` accordingly. Building both static
  and shared libraries is no longer supported.

- Remove the CMake function deprecated `inkscape_generate_png_from_svg`.

- Remove the old and deprecated use of UseLATEX.cmake.
  `dune_add_latex_document' is a redirection to `add_latex_document`
  which internally uses `latexmk`.

## Deprecations and removals

- Remove deprecated header `dune/common/std/memory.hh`; use `<memory>`
  instead.

- Deprecate header `dune/common/std/utility.hh`; use `<utility>` instead.

- Deprecate header `dune/common/std/variant.hh`; use `<variant>` instead.

- Remove incomplete CPack support that was never used to make an official
  build or tarball.

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
