// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#include "config.h"

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <iomanip>

#include <dune/common/classname.hh>
#include <dune/common/hybridutilities.hh>
#include <dune/common/float_cmp.hh>
#include <dune/common/strtonumber.hh>
#include <dune/common/quadmath.hh>
#include <dune/common/timer.hh>
#include <dune/common/test/testsuite.hh>

using namespace Dune;

template <class T, class = void>
struct Distribution
{
  using type = std::uniform_real_distribution<T>;
};

template <class T>
struct Distribution<T, std::enable_if_t<std::is_integral<T>::value>>
{
  using type = std::uniform_int_distribution<T>;
};

// Random number generator used to generate test data
template <class T, class = void>
struct Generator
{
  Generator(T lo = std::numeric_limits<T>::min(),
            T hi = std::numeric_limits<T>::max())
    : gen{}
    , dis(lo, hi)
  {}

  T operator()()
  {
    return dis(gen);
  }

  std::mt19937 gen;
  typename Distribution<T>::type dis;
};

// specialization for bool, since uniform_int_distribution not implemented for bool
template <>
struct Generator<bool,void>
{
  using T = signed short;

  Generator(T lo = std::numeric_limits<T>::min(),
            T hi = std::numeric_limits<T>::max())
    : gen{}
    , dis(lo, hi)
  {}

  bool operator()()
  {
    return dis(gen) >= 0;
  }

  std::mt19937 gen;
  typename Distribution<T>::type dis;
};

// fall-back implementatin for all types that are neither
// is_floating_point nor is_integral, using std::rand()
template <class T>
struct Generator<T, std::enable_if_t<not std::is_arithmetic<T>::value
#ifdef HAVE_QUADMATH
  || std::is_same<T,__float128>::value || std::is_same<T,Float128>::value
#endif
  >>
{
  Generator(T lo = std::numeric_limits<T>::min(),
            T hi = std::numeric_limits<T>::max())
    : lo_(lo)
    , hi_(hi)
  {
    std::srand(5489);
  }

  T operator()() const
  {
    T x = T(std::rand())/RAND_MAX;
    return lo_ + hi_*x - lo_*x;
  }

  T lo_, hi_;
};


template <class T, class = void>
struct Comparator
{
  bool operator()(T const& x, T const& y) { return Dune::FloatCmp::eq(x, y); }
};

template <class T>
struct Comparator<T, std::enable_if_t<std::is_integral<T>::value>>
{
  bool operator()(T const& x, T const& y) { return x == y; }
};


int main()
{
  std::srand(std::time(nullptr));

  using Types = std::tuple<bool, signed short, signed int, signed long, signed long long,
    unsigned short, unsigned int, unsigned long, unsigned long long,
    float, double, long double
#if HAVE_QUADMATH
    , Float128
#endif
    >;

  TestSuite test;

  test.check(strTo<char>("0") == '\0');
  test.check(strTo<signed char>("0") == '\0');
  test.check(strTo<unsigned char>("0") == '\0');

  Hybrid::forEach(Types{}, [&test](auto value)
  {
    using namespace Dune;
    using T = decltype(value);
    std::cout << "test<" << className<T>() << ">...\n";

    Comparator<T> cmp{};
    Generator<T> generate{};
    for (int i = 0; i < 100; ++i) { // repeat the test 10 times
      value = generate();
      std::ostringstream oss;
      oss << std::setprecision(std::numeric_limits<T>::digits10 + 3) << value;
      T val1 = strTo<T>(oss.str().c_str());

      // compare with original value
      test.check(bool(value == val1));
    }

    T value2 = std::numeric_limits<T>::max();
    std::ostringstream oss2;
    oss2 << std::setprecision(std::numeric_limits<T>::digits10 + 4) << value2;

    T val2 = strTo<T>(oss2.str().c_str());
    test.check(cmp(value2, val2));

    bool exception = false;
    try {
      T val3 = strTo<T>(("1" + oss2.str()).c_str()); // add one more digit in front of number
    } catch(Dune::RangeError const&) {
      exception = true;
    }
    test.check(exception, "RangeError Exception");
  });


  using FloatingPointTypes = std::tuple<float, double, long double
#if HAVE_QUADMATH
    , Float128
#endif
    >;

  Hybrid::forEach(FloatingPointTypes{}, [&test](auto value)
  {
    using namespace Dune;
    using T = decltype(value);
    Comparator<T> cmp{};
    std::cout << "test<" << className<T>() << ">...\n";

    value = 1.5;

    std::setlocale(LC_NUMERIC, "C");
    T val0 = strTo<T>("1.5");
    test.check(cmp(value, val0), "Locale_C");

    char* new_loc = std::setlocale(LC_NUMERIC, "de_DE.UTF-8");
    if (new_loc) {
      T val1 = strTo<T>("1,5");
      test.check(cmp(value, val1), "Locale_de_DE");
    } else {
      std::cout << "### skipped locale de_DE.UTF-8 test\n";
    }

    std::setlocale(LC_NUMERIC, "C");
    bool exception = false;
    try {
      T val2 = strTo<T>("1,5");
    } catch(Dune::InvalidArgument const&) {
      exception = true;
    }
    test.check(exception, "InvalidArgument Exception: locale");

    exception = false;
    try {
      T val2 = strTo<T>("1.5__");
    } catch(Dune::InvalidArgument const&) {
      exception = true;
    }
    test.check(exception, "InvalidArgument Exception: trailing characters");

    exception = false;
    try {
      T val2 = strTo<T>("1.5 ");
    } catch(Dune::InvalidArgument const&) {
      exception = true;
    }
    test.check(!exception, "InvalidArgument Exception: trailing whitespace");

    exception = false;
    try {
      T val2 = strTo<T>(" 1.5");
    } catch(Dune::InvalidArgument const&) {
      exception = true;
    }
    test.check(!exception, "InvalidArgument Exception: leading whitespace");
  });

}
