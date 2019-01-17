# Master (will become release 2.7)

-   When run with an absolute build directory, `dunecontrol` now exposes the root build
    directory to CMake in the variable `DUNE_BUILD_DIRECTORY_ROOT_PATH`.

    See core/dune-common!542

-   The build system has learned some new tricks when creating or looking for the Python virtualenv:
    When using an absolute build directory with `dunecontrol`, the virtualenv will now be placed
    directly inside the root of the build directory hierarchy in the directory `dune-python-env`.
    This should make it much easier to actually find the virtualenv and also avoids some corner
    cases where the build system would create multiple virtualenvs that did not know about each
    other. This behavior can be disabled by setting
    `DUNE_PYTHON_EXTERNAL_VIRTUALENV_FOR_ABSOLUTE_BUILDDIR=0`.
    If you need even more precise control about the location of the virtualenv, you can now also
    directly set the CMake variable `DUNE_PYTHON_VIRTUALENV_PATH` to the directory in which to
    create the virtualenv.

-   The `dune_symlink_to_sources_files` cmake function now has a `DESTINATION` argument.

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
