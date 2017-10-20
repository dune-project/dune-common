#ifndef DUNE_PYTHON_COMMON_ITERATORRANGE_HH
#define DUNE_PYTHON_COMMON_ITERATORRANGE_HH

#include <dune/python/pybind11/pybind11.h>

namespace Dune {
namespace CorePy {

/**
 * Simple range between a begin and an end iterator, with optional length.
 *
 * This class is similar to dune-common's IteratorRange, but allows
 * for different types for begin and end, and includes an optional length
 * for Python's `len(...)` operator.
 *
 * Instances of this type should be registered with
 * \seealso registerIteratorRange
 *
 * \tparam BeginIt type of begin iterator
 * \tparam EndIt   type of end iterator
 */
template<typename BeginIt, typename EndIt = BeginIt>
class PyIteratorRange
{
public:
  using BeginIterator = BeginIt;
  using EndIterator = EndIt;

  /**
   * construct a new iterator range
   */
  PyIteratorRange(const BeginIterator& begin, const EndIterator& end, std::size_t length = 0)
    : begin_(begin)
    , end_(end)
    , length_(length)
    {}

  /**
   * returns begin iterator
   */
  const BeginIterator& begin() const
    { return begin_; }

  /**
   * returns end iterator
   */
  const EndIterator& end() const
    { return end_; }

  /**
   * returns distance between begin and end iterator.
   * Used to implement Python's `__len__` protocol.
   */
  std::size_t length() const
    { return length_; }

private:
  BeginIterator begin_;
  EndIterator end_;
  std::size_t length_;
};

/**
 * register a new iterator range Python type
 *
 * \tparam Range    type of iterator range; should be an instance of \ref PyIteratorRange
 *
 * \param scope     Python scope for the new class
 * \param name      Python name for the new class
 * \param hasLength indicates whether Python type should implement `__len__` protocol or not
 */
template<typename Range>
auto registerIteratorRange(pybind11::handle scope, const char* name, bool hasLength = false)
{
  using BeginIterator = typename Range::BeginIterator;
  using EndIterator = typename Range::EndIterator;

  // TODO use insertClass
  auto cls = pybind11::class_<Range>(scope, name)
    .def("__iter__",
         [](const Range& range) { return pybind11::make_iterator(range.begin(), range.end()); },
         pybind11::keep_alive<0, 1>());

  if (hasLength) {
    cls.def(pybind11::init<const BeginIterator&, const EndIterator&, std::size_t>());
    cls.def("__len__", &Range::length);
  }
  else {
    cls.def(pybind11::init<const BeginIterator&, const EndIterator&>());
  }

  return cls;
}

} /* namespace CorePy */
} /* namespace Dune */

#endif
