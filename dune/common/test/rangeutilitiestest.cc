// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <array>
#if __has_include(<concepts>)
  #include <concepts>
#endif
#include <map>
#include <utility>
#include <vector>
#include <numeric>
#include <type_traits>
#include <optional>

#include <dune/common/concepts.hh>
#include <dune/common/hybridutilities.hh>
#include <dune/common/iteratorrange.hh>
#include <dune/common/rangeutilities.hh>
#include <dune/common/typetraits.hh>
#include <dune/common/diagonalmatrix.hh>
#include <dune/common/test/testsuite.hh>
#include <dune/common/test/iteratortest.hh>
#include <dune/common/tupleutility.hh>
#include <dune/common/integersequence.hh>



// This utility helps to avoid an inconvenience of lambda functions:
// If a lambda is mutable and contains by value captures, the generated
// class only contains a mutable "...operator()(...)"
// but const version "...operator()(...) const" passing "this" as const.
//
// auto v = std::vector{0,2,3};
// auto f = [v=std::move(v)](auto i) mutable ->decltype(auto) {return v[i];};
// f(0);      //OK, non-const operator()
// f(0) = 42; //OK, modifies captured vector
//
// const auto& fc = f;
// fc(0);  //Compile error, there's no, const operator()
//
// This class will store copies of a collection of values and a callback.
// It provides const and a non-const "...operator()(...)", which is forwarded
// the callback passing the stored values as leading argument followed by
// any additional argument passed to "operator()". Hence for
//
// auto v = std::vector{0,2,3};
// auto f = Capture(std::move(v), [](auto&& v, auto i) ->decltype(auto) {return v[i];});
// f(0);      //OK, calls non-const operator()
// f(0) = 42; //OK, modifies captured vector
//
// const auto& fc = f;
// fc(0);      //OK, calls const operator()
// fc(0) = 42; //Compile error, for const capture this returns a const ref
template<class... T>
class Capture
{

  template<class Tuple, class... Args>
  static decltype(auto) invoke(Tuple&& capture, Args&&...args)
  {
    constexpr auto N = sizeof...(T)-1;
    constexpr auto indices = Dune::push_front(std::make_index_sequence<N>(), Dune::index_constant<N>());
    return Dune::applyPartial([&](auto&... t) -> decltype(auto) {
      return std::invoke(t..., std::forward<Args>(args)...);
    }, capture, indices);
  }

public:
  Capture(T&&... t) :
    t_(std::move(t)...)
  {}

  template<class... Args>
  decltype(auto) operator()(Args&&... args)
  {
    return invoke(t_, std::forward<Args>(args)...);
  }

  template<class... Args>
  decltype(auto) operator()(Args&&... args) const
  {
    return invoke(t_, std::forward<Args>(args)...);
  }

private:
  std::tuple<T...> t_;
};



template<class R1, class BeginIt2, class EndIt2>
auto checkSameRange(R1&& r1, BeginIt2&& it2, EndIt2&& end2)
{
  auto it1 = r1.begin();
  auto end1 = r1.end();
  for(; (it1 < end1) and (it2 < end2); ++it1, ++it2)
    if (*it1 != *it2)
      return false;
  if ((it1 != end1) or (it2 != end2))
    return false;
  return true;
}

template<class R1, class R2>
auto checkSameRange(R1&& r1, R2&& r2)
{
  return checkSameRange(r1, r2.begin(), r2.end());
}

template<class R>
auto checkRangeConstIterators(R&& r)
{
  auto it = r.begin();
  auto end = r.end();
  auto op = [](const auto& x){};
  return (testConstIterator(it, end, op)==0);
}

template<class R, class Category>
auto checkRangeIterators(R&& r, Category category)
{
  bool result = true;
  auto it = r.begin();
  auto end = r.end();
  auto op = [](const auto& x){};
  result &= (testConstIterator(it, end, op)==0);
  result &= (testIterator(it, end, op, category)==0);
  return result;
}

template<class R>
auto checkRangeSize(R&& r)
{
  std::size_t counter = 0;
  for([[maybe_unused]] auto&& dummy : r)
    ++counter;
  return (r.size()==counter) && (!r.empty() || counter==0);
}

template<class R, class V>
auto checkRandomAccessNumberRangeSums(R&& r, V sum, V first, V last)
{
  bool passed = true;
  passed = passed and (std::accumulate(r.begin(), r.end(), 0) == sum);
  passed = passed and (std::accumulate(r.begin()+1, r.end(), 0) == (sum-first));
  passed = passed and (std::accumulate(r.begin(), r.end()-1, 0) == (sum-last));
  passed = passed and (std::accumulate(r.begin()+1, r.end()-1, 0) == (sum-first-last));

  // check direct random index access
  V s = 0;
  for (std::size_t i = 0; i < std::size_t(r.size()); ++i)
    s += r[i];
  passed = passed and (s == sum);

  return passed;
}

template<class T>
struct is_const_reference : public std::conjunction<std::is_reference<T>, std::is_const<std::remove_reference_t<T>>>
{};

template<class T>
struct is_mutable_reference : public std::conjunction<std::is_reference<T>, std::negation<is_const_reference<T>>>
{};

template<typename F, typename T, T... I>
void checkIndexExpansion(std::integer_sequence<T, I...> seq, F&& f){
  f(seq);
}

auto testTransformedRangeView()
{
  Dune::TestSuite suite("Check transformedRangeView()");

  // Check transformedRangeView with container range
  Dune::Hybrid::forEach(std::make_tuple(std::array<int,3>({1,2,3}), std::vector<int>({1,2,3})), [&](auto&& a)
  {
    auto a_backup = a;
    // Pass original range by l-value, modify it, and then traverse.
    // This should traverse the modified original range.
    {
      auto r = Dune::transformedRangeView(a, [](auto&& x) { return 2*x;});
      a[0] = 2;
      suite.check(checkRandomAccessNumberRangeSums(r, 14, 4, 6))
        << "incorrect values in transformedRangeView of l-value";
      suite.check(checkRangeIterators(r, std::random_access_iterator_tag()))
        << "iterator test fails for transformedRangeView of l-value";
      suite.check(checkRangeSize(r))
        << "checking size fails for transformedRangeView of l-value";
      a = a_backup;
    }
    // Pass original range by const l-value, modify it, and then traverse.
    // This should traverse the modified original range.
    {
      const auto& a_const = a;
      auto r = Dune::transformedRangeView(a_const, [](auto&& x) { return 2*x;});
      a[0] = 2;
      suite.check(checkRandomAccessNumberRangeSums(r, 14, 4, 6))
        << "incorrect values in transformedRangeView of const l-value";
      suite.check(checkRangeIterators(r, std::random_access_iterator_tag()))
        << "iterator test fails for transformedRangeView of const l-value";
      suite.check(checkRangeSize(r))
        << "checking size fails for transformedRangeView of const l-value";
      a = a_backup;
    }
    // Modify original range, pass it by r-value, restore it, and then traverse.
    // This should traverse a modified copy of the range and not the restored
    // original one.
    {
      a[0] = 2;
      auto r = Dune::transformedRangeView(std::move(a), [](auto&& x) { return 2*x;});
      a = a_backup;
      suite.check(checkRandomAccessNumberRangeSums(r, 14, 4, 6))
        << "incorrect values in transformedRangeView of r-value";
      suite.check(checkRangeIterators(r, std::random_access_iterator_tag()))
        << "iterator test fails for transformedRangeView of r-value";
      suite.check(checkRangeSize(r))
        << "checking size fails for transformedRangeView of r-value";
    }
    // Check if returning real references in the transformation works
    {
      auto r = Dune::transformedRangeView(a, [](auto&& x) -> decltype(auto) { return x;});
      suite.check(is_mutable_reference<decltype(*r.begin())>::value)
        << "iterator with mutable-reference returning transformation does not return mutable references";
      suite.check(&(*(r.begin())) == &(a[0]))
        << "reference points to wrong location";
      (*r.begin()) = 0;
      suite.check(a[0] == 0)
        << "modifying range by reference returning transformation failed";
      *(r.begin().operator->()) = 42;
      suite.check(a[0] == 42)
        << "modifying range by reference returning transformation failed using operator-> failed";
      a = a_backup;
    }
    // Check if returning real references in the transformation works
    {
      const auto& a_const = a;
      auto r = Dune::transformedRangeView(a_const, [](auto&& x) -> decltype(auto) { return x;});
      suite.check(is_const_reference<decltype(*r.begin())>::value)
        << "iterator with const-reference returning transformation does not return const references";
      suite.check(&(*(r.begin())) == &(a[0]))
        << "reference points to wrong location";
    }
    // Check if returning copies leaves original range untouched
    {
      auto r = Dune::transformedRangeView(a, [](auto x) { return x;});
      suite.check(not std::is_lvalue_reference_v<decltype(*r.begin())>)
        << "dereferenced iterator returned reference instead of copy";
      suite.check(not std::is_lvalue_reference_v<decltype(r[0])>)
        << "operator[] returned reference instead of copy";
      auto a0 = a[0];
      *(r.begin().operator->().operator->()) = a0+3;
      suite.check(a[0] == a0)
        << "iterator::operator-> for non-reference transformation modifies the original range";
      a = a_backup;
    }
    // Check iterator based transformation
    {
      auto r = Dune::iteratorTransformedRangeView(a, [&](auto&& it) { return (*it)+(it-a.begin());});
      suite.check(checkRandomAccessNumberRangeSums(r, 9, 1, 5))
        << "incorrect values in transformedRangeView of l-value";
      suite.check(checkRangeIterators(r, std::random_access_iterator_tag()))
        << "iterator test fails for transformedRangeView of l-value";
      suite.check(checkRangeSize(r))
        << "checking size fails for transformedRangeView of l-value";
      a = a_backup;
    }
  });
  // Check transformedRangeView with on the fly range
  {
    auto r = Dune::transformedRangeView(Dune::range(10), [](auto&& x) { return 2*x;});
    suite.check(checkRandomAccessNumberRangeSums(r, 90, 0, 18))
      << "transformation of on-the-fly range gives incorrect results";
    suite.check(checkRangeIterators(r, std::random_access_iterator_tag()))
      << "iterator test fails for transformedRangeView";
    suite.check(checkRangeSize(r))
      << "checking size fails for transformedRangeView of on-the-fly range";
  }
  // Check if we can indirectly sort subrange via reference returning transformations
  {
    auto a = std::vector<int>{4,3,2,1,0};
    auto r = Dune::transformedRangeView(Dune::range(1,4), [&](auto&& i) -> decltype(auto){ return a[i];});
    std::sort(r.begin(), r.end());
    suite.check(a == std::vector<int>{4,1,2,3,0})
      << "sorting reference returning transformedRangeView failed";

    auto r2 = Dune::transformedRangeView(std::array<int,3>{0, 2, 4}, [&](auto&& i) -> decltype(auto){ return a[i];});
    std::sort(r2.begin(), r2.end());
    suite.check(a == std::vector<int>{0,1,2,3,4})
      << "sorting reference returning transformedRangeView failed";

    // Remap values of certain keys in a std::map such that
    // they are sorted according to the keys.
    auto m = std::map<int, int>{{-1,5},{0,4}, {1,3}, {2,2}};
    auto r3 = Dune::transformedRangeView(std::array<int,3>{1, -1, 2}, [&](auto&& i) -> decltype(auto){ return m[i];});
    std::sort(r3.begin(), r3.end());
    suite.check(m == std::map<int, int>{{1,2},{-1,3}, {2,5},{0,4}})
      << "sorting reference returning transformedRangeView failed";
  }
  // Check if using a mutable callback allows to use const and non-const calls
  {
    auto a = std::vector<int>{4,3,2,1,0};
    auto r = Dune::transformedRangeView(Dune::range(1,4), Capture(std::move(a), [](auto&& a, auto i) -> decltype(auto) {return a[i];}));
    suite.check(checkSameRange(r, std::vector{3, 2, 1}));
    std::sort(r.begin(), r.end());
    suite.check(checkSameRange(r, std::vector{1, 2, 3}))
      << "sorting mutable captured range failed";
    const auto& rc = r;
    suite.check(checkSameRange(rc, std::vector{1, 2, 3}))
      << "accessing mutable range via const reference failed";
  }
  // Check creation of free iterators storing raw lambdas of different type
  {
    auto transformedIterator = [](auto&& it, auto&& f) {
      using It = std::decay_t<decltype(it)>;
      using F = std::decay_t<decltype(f)>;
      using TIt = Dune::Impl::TransformedRangeIterator<It, F, Dune::ValueTransformationTag>;
      return TIt(std::forward<decltype(it)>(it), std::forward<decltype(f)>(f));
    };
    auto it = transformedIterator(Dune::range(0,5).begin(), [](auto x) { return 2*x; });
    auto end = transformedIterator(Dune::range(0,4).end(), [](auto x) {return 0;});
    suite.check(checkSameRange(std::vector{0, 2, 4, 6}, it, end))
      << "free TransformedRangeIterator's with raw lambdas yield wrong result";

#if DUNE_ENABLE_CONCEPTS
    static_assert(std::random_access_iterator<decltype(it)>);
#endif
  }
  // Check creation of free iterators storing std::optional lambdas of different type
  {
    auto transformedIterator = [](auto&& it, auto&& f) {
      using It = std::decay_t<decltype(it)>;
      using F = std::decay_t<decltype(f)>;
      using TIt = Dune::Impl::TransformedRangeIterator<It, std::optional<F>, Dune::ValueTransformationTag>;
      return TIt(std::forward<decltype(it)>(it), std::forward<decltype(f)>(f));
    };
    auto it = transformedIterator(Dune::range(0,5).begin(), [](auto x) { return 2*x; });
    auto end = transformedIterator(Dune::range(0,4).end(), [](auto x) {return 0;});
    suite.check(checkSameRange(std::vector{0, 2, 4, 6}, it, end))
      << "free TransformedRangeIterator's with lambdas in std::optional yield wrong result";

#if DUNE_ENABLE_CONCEPTS
    static_assert(std::random_access_iterator<decltype(it)>);
#endif
  }
  return suite;
}


auto testSparseRange()
{
  Dune::TestSuite suite("Check sparseRange()");

  auto checkWithMatrix = [&suite](auto&& M) {
    for(std::size_t i=0; i<M.size(); ++i)
    {
      auto it = M[i].begin();
      auto end = M[i].end();
      for(auto&& [M_ij, j] : Dune::sparseRange(M[i]))
      {
        suite.check(it!=end)
          << "sparseRange() contains more entries than the original range";
        suite.check(&M_ij == &M[i][j])
          << "Entry obtained by sparseRange() does not point to actual range entry";
        suite.check(&M_ij == &(*it))
          << "Entry obtained by sparseRange() does not point to actual range entry";
        ++it;
      }
      suite.check(it==end)
        << "sparseRange() contains less entries than the original range";
    }
  };

  auto M1 = Dune::DiagonalMatrix<double,1>({42});
  checkWithMatrix(M1);
  checkWithMatrix(std::as_const(M1));

  auto M2 = Dune::DiagonalMatrix<double,2>({42, 41});
  checkWithMatrix(M2);
  checkWithMatrix(std::as_const(M2));

  auto M3 = Dune::DiagonalMatrix<double,3>({42, 41, 40});
  checkWithMatrix(M3);
  checkWithMatrix(std::as_const(M3));

  return suite;
}



int main()
{
  // Check IsIterable<> for https://gitlab.dune-project.org/core/dune-common/issues/58
  static_assert(Dune::IsIterable< std::array<int, 3> >::value, "std::array<int> must be a range");
  static_assert(Dune::IsIterable< Dune::IteratorRange<int*> >::value, "IteratorRange must be a range");
  static_assert(!Dune::IsIterable< int >::value, "int must not be a range");

  Dune::TestSuite suite;

  // max_value, min_value
  {
    const int value = 12;
    suite.check(Dune::max_value(value) == value);
    suite.check(Dune::min_value(value) == value);

    std::array<int, 3> values{-42, 0, 42};
    suite.check(Dune::max_value(values) == 42)
      << "maximum of values is 42, but got " << Dune::max_value(values);
    suite.check(Dune::min_value(values) == -42)
      << "minimum of values is -42, but got " << Dune::min_value(values);

    std::array<int, 3> positiveValues{1, 2, 3};
    suite.check(Dune::max_value(positiveValues) == 3)
      << "maximum of positiveValues is 3, but got " << Dune::max_value(positiveValues);
    suite.check(Dune::min_value(positiveValues) == 1)
      << "minimum of positiveValues is 1, but got " << Dune::min_value(positiveValues);

    std::array<int, 3> negativeValues{-1, -3, -1};
    suite.check(Dune::max_value(negativeValues) == -1)
      << "maximum of negativeValues is -1, but got " << Dune::max_value(negativeValues);
    suite.check(Dune::min_value(negativeValues) == -3)
      << "minimum of negativeValues is -3, but got " << Dune::min_value(negativeValues);
  }

  // any_true, all_true
  {
    const std::array<bool, 3> allTrue{true, true, true};
    const std::array<bool, 3> allFalse{false, false, false};
    const std::array<bool, 3> someTrue{false, true, false};

    suite.check(Dune::any_true(allTrue))
      << "any_true(allTrue) must be true";
    suite.check(!Dune::any_true(allFalse))
      << "any_true(allFalse) must be false";
    suite.check(Dune::any_true(someTrue))
      << "any_true(someTrue) must be true";

    suite.check(Dune::all_true(allTrue))
      << "all_true(allTrue) must be true";
    suite.check(!Dune::all_true(allFalse))
      << "all_true(allFalse) must be false";
    suite.check(!Dune::all_true(someTrue))
      << "all_true(someTrue) must be false";

    const bool t = true;
    const bool f = false;

    suite.check(Dune::any_true(t))
      << "any_true(true) must be true";
    suite.check(!Dune::any_true(f))
      << "any_true(false) must be false";

    suite.check(Dune::all_true(t))
      << "all_true(true) must be true";
    suite.check(!Dune::all_true(f))
      << "all_true(false) must be false";
  }

  // integer ranges
  using Dune::range;
  std::vector<int> numbers(range(6).begin(), range(6).end());
  int sum = 0;
  for( auto i : range(numbers.size()) )
    sum += numbers[i];
  suite.check(sum == 15) << "sum over range( 0, 6 ) must be 15.";
  suite.check(range(sum, 100)[5] == 20) << "range(sum, 100)[5] must be 20.";
  sum = 0;
  for( auto i : range(-10, 11) )
    sum += i;
  suite.check(sum == 0) << "sum over range( -10, 11 ) must be 0.";

  // check whether entries are contained in a range
  suite.check(range(6).contains(5));
  suite.check(not range(6).contains(6));

  auto range4 = range(std::integral_constant<int,4>());
  using StaticRange4 = decltype(range4);
  static_assert(StaticRange4::contains(std::integral_constant<int,3>()));
  static_assert(not StaticRange4::contains(4));

  static_assert(std::is_same<StaticRange4::integer_sequence, std::make_integer_sequence<int, 4>>::value,
                "decltype(range(std::integral_constant<int, 4>))::integer_sequence must be the same as std::make_integer_sequence<int, 4>");

  checkIndexExpansion(range4.to_integer_sequence(), [](auto seq){
    static_assert(std::is_same<decltype(seq), std::make_integer_sequence<int, 4>>::value,
                  "decltype(range(std::integral_constant<int, 4>))::integer_sequence must be the same as std::make_integer_sequence<int, 4>");
  });

  // Hybrid::forEach for integer ranges
  Dune::Hybrid::forEach(range(std::integral_constant<int, 1>()), [] (auto &&i) {
      static_assert(std::is_same<std::decay_t<decltype(i)>, std::integral_constant<int, 0>>::value,
                    "Hybrid::forEach(range(std::integral_constant<int, 1>()), ...) should only visit std::integral_constant<int, 0>.");
    });


  {
    auto r = range(-10,11);
    auto it = r.begin();
    auto end = r.end();
    auto op = [](const auto& x){};
    suite.check(testConstIterator(it, end, op)==0)
      << "iterator test fails for range(-10,11)";
  }

  suite.subTest(testTransformedRangeView());

  suite.subTest(testSparseRange());

  return suite.exit();

}
