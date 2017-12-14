# Master (will become release 2.7)

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

- `FieldMatrix` now has experimental support for SIMD types from
  [Vc](https://github.com/VcDevel/Vc) as field types.

    See core/dune-common!121

## build-system

- Variables passed via `dunecontrol`'s command `--configure-opts=..` are now
  added to the CMake flags.

- Bash-style variables which are passed to `dunecontrol`'s command `configure-opts`
  are no longer transformed to their equivalent CMake command. Pass
  `-DCMAKE_C_COMPILER=gcc` instead of `CC=gcc`.
