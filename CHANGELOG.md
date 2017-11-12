# Release 2.6

- New class `IntegralRange<integral_type>` and free standing function
  `range` added providing a feature similar to Python's `range` function:
  ```
    for (const auto &i : range(5,10))
  ```
  See core/dune-common!325

- `Dune::array` was deprecated use `std::array` from <array> instead.
   Instead of `Dune::make_arra`, use `Dune::Std::make_array`
   from dune/common/std/make_array.hh
   and `Dune::filledArray` instead of `Dune::fill_array`
   from dune/common/filledarray.hh.`

    See core/dune-common!359
  
- The `DUNE_VERSION...` maccros are deprecated use the new macros
  `DUNE_VERSION_GT`, `DUNE_VERSION_GTE`, `DUNE_VERSION_LTE`, and
  `DUNE_VERSION_LT` instead.

    See core/dune-common!329
  
- Added some additional fallback implementation to C++17 features:
  (e.g. `optional`, `conjunction`, `discjunction`)

- `makeVirtualFunction`:
  allows to easily convert any function object (e.g. lambda) to a `VirtualFunction`

    See core/dune-common!282