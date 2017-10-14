# Release 2.6

- New class `IntegralRange<integral_type>` and free standing function
  `range` added providing a feature similar to Python's `range` function:
  ```
    for (const auto &i : range(5,10))
  ```
