# Release 2.6

- New class `IntegralRange<integral_type>` and free standing function
  `range` added providing a feature similar to Python's `range` function:
  ```
    for (const auto &i : range(5,10))
  ```

  [dune-common!325]: https://gitlab.dune-project.org/core/dune-common/merge_requests/325

- `Dune::array` was deprecated use `std::array` from <array> instead.
   Instead of `Dune::make_arra`, use `Dune::Std::make_array`
   from dune/common/std/make_array.hh
   and `Dune::filledArray` instead of `Dune::fill_array`
   from dune/common/filledarray.hh.`

  [dune-common!359]: https://gitlab.dune-project.org/core/dune-common/merge_requests/359

- The `DUNE_VERSION...` maccros are deprecated use the new macros
  `DUNE_VERSION_GT`, `DUNE_VERSION_GTE`, `DUNE_VERSION_LTE`, and
  `DUNE_VERSION_LT` instead.

  [dune-common!329]: https://gitlab.dune-project.org/core/dune-common/merge_requests/329

- Added some additional fallback implementation to C++17 features:
  (e.g. `optional`, `conjunction`, `discjunction`)

- `makeVirtualFunction`:
  allows to easily convert any function object (e.g. lambda) to a `VirtualFunction`

  [dune-common!282]: https://gitlab.dune-project.org/core/dune-common/merge_requests/282
