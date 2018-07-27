// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_LEXICALCAST_HH
#define DUNE_COMMON_LEXICALCAST_HH

#include <cstdlib>

#include <dune/common/quadmath.hh>

namespace Dune
{
  // forward declarations
  template<class K, int SIZE> class FieldVector;
  template<class K, int ROWS, int COLS> class FieldMatrix;

  namespace Impl
  {
    // default behavior: call the string constructor
    template<typename T>
    struct LexicalCast {
      static T eval (const char* str) { return T(str); }
    };

    // signed integer types
    template<> struct LexicalCast<signed char> {
      static signed char eval (const char* str) { return (signed char)(std::atoi(str)); }
    };
    template<> struct LexicalCast<signed short> {
      static signed short eval (const char* str) { return (signed short)(std::atoi(str)); }
    };
    template<> struct LexicalCast<int> {
      static int eval (const char* str) { return std::atoi(str); }
    };
    template<> struct LexicalCast<long> {
      static long eval (const char* str) { return std::strtol(str, NULL, 10); }
    };
    template<> struct LexicalCast<long long> {
      static long long eval (const char* str) { return std::strtoll(str, NULL, 10); }
    };


    // unsigned integer types
    template<> struct LexicalCast<bool> {
      static bool eval (const char* str) { return (bool)(std::atoi(str)); }
    };
    template<> struct LexicalCast<unsigned char> {
      static unsigned char eval (const char* str) { return (unsigned char)(std::strtoul(str, NULL, 10)); }
    };
    template<> struct LexicalCast<unsigned short> {
      static unsigned short eval (const char* str) { return (unsigned short)(std::strtoul(str, NULL, 10)); }
    };
    template<> struct LexicalCast<unsigned int> {
      static unsigned int eval (const char* str) { return (unsigned int)(std::strtoul(str, NULL, 10)); }
    };
    template<> struct LexicalCast<unsigned long> {
      static unsigned long eval (const char* str) { return std::strtoul(str, NULL, 10); }
    };
    template<> struct LexicalCast<unsigned long long> {
      static unsigned long long eval (const char* str) { return std::strtoull(str, NULL, 10); }
    };


    // floating point types
    template<> struct LexicalCast<float> {
      static float eval (const char* str) { return std::strtof(str, NULL); }
    };
    template<> struct LexicalCast<double> {
      static double eval (const char* str) { return std::strtod(str, NULL); }
    };
    template<> struct LexicalCast<long double> {
      static long double eval (const char* str) { return std::strtold(str, NULL); }
    };


    // Dune scalar-like vectors/matrices
    template<typename T> struct LexicalCast<FieldVector<T,1>> {
      static FieldVector<T,1> eval (const char* str) { return LexicalCast<T>::eval (str); }
    };
    template<typename T> struct LexicalCast<FieldMatrix<T,1,1>> {
      static FieldMatrix<T,1,1> eval (const char* str) { return LexicalCast<T>::eval (str); }
    };


#if HAVE_QUADMATH
    // specialization for quadprecision floating-point type.
    template<> struct LexicalCast<__float128> {
      static __float128 eval (const char* str) { return strtoflt128(str, NULL); }
    };

    template<> struct LexicalCast<Float128> {
      static Float128 eval (const char* str) { return {strtoflt128(str, NULL)}; }
    };
#endif

  } // end namespace Impl


  /// \brief Convert a character sequence to a number type `T`
  /**
   * The cast from character sequence to any type is implemented in terms of std library functions
   * like `atoi()` and `strf()` but parametrized with the concrete target type for ease of usage
   * in generic code.
   *
   * The usage of these std library functions has the drawback of little error safety, i.e. for `atoi`
   * the default behavior is: If the converted value falls out of range of corresponding return type,
   * the return value is undefined. If no conversion can be performed, ​0​ is returned. But on the other
   * hand it gives high performance in conversion, what is intended here.
   *
   * Use `std::stringstream` for an exception safe string to number conversion, if necessary!
   *
   * Only types that provide a specialization of `Impl::LexicalCast` can call the special functions
   * for conversion. All other types by default call the constructor with strings.
   **/
  template<typename T>
  T lexicalCast(const char* str) { return Impl::LexicalCast<T>::eval(str); }

} // end namespace Dune

#endif // DUNE_COMMON_LEXICALCAST_HH
