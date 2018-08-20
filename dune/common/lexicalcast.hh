// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_LEXICALCAST_HH
#define DUNE_COMMON_LEXICALCAST_HH

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <limits>

#include <dune/common/exceptions.hh>

namespace Dune
{
  /// An exception thrown whenever interpretation of the argument is not possible.
  class InvalidArgument : public Exception {};

  namespace Impl
  {
    // default behavior: call the string constructor
    template<typename T>
    struct LexicalCast {
      static T eval (const char* str) { return T(str); }
    };

    /// \brief Implementation of the acual number converter.
    /**
     * Allows leading and trailing whitespace characters.
     *
     * \tparam T  Target numeric type
     * \tparam S  Output type of the parser [=T]
     **/
    template<typename T, typename S=T>
    struct LexicalCastImpl
    {
      // The parser has the signature `T(const char*, char**)` and may set the errno in case of a range error.
      template<typename Parser>
      static T evalImpl (const char* str, Parser parser)
      {
        char* end;
        errno = 0;
        S x = parser(str, &end);

        // test whether all non-space characters are consumed during conversion
        bool all_consumed = (end != str);
        while (all_consumed && (*end != '\0'))
          all_consumed = std::isspace(*end++);

        if (!all_consumed) {
          DUNE_THROW(InvalidArgument,
            "Conversion to number failed. Possible reason: invalid string or locale format");
        }
        if (errno == ERANGE) {
          DUNE_THROW(RangeError, std::strerror(errno));
        }

        return convert(x);
      }

      // Check whether a numeric conversion is safe
      template<typename U>
      static T convert (U const& x)
      {
        if (sizeof(U) > sizeof(T) && (x > U(std::numeric_limits<T>::max()) ||
                                      x < U(std::numeric_limits<T>::min()))) {
          DUNE_THROW(RangeError, "Numerical result out of range");
        }
        return T(x);
      }

      static T const& convert (T const& x)
      {
        return x;
      }
    };

    // signed integer types
    template<> struct LexicalCast<signed char> {
      static signed char eval (const char* str)
      {
        auto parser = [](const char* str, char** end) { return std::strtol(str,end,10); };
        return LexicalCastImpl<signed char, long>::evalImpl(str, parser);
      }
    };

    template<> struct LexicalCast<signed short> {
      static signed short eval (const char* str)
      {
        auto parser = [](const char* str, char** end) { return std::strtol(str,end,10); };
        return LexicalCastImpl<signed short, long>::evalImpl(str, parser);
      }
    };

    template<> struct LexicalCast<int> {
      static int eval (const char* str)
      {
        auto parser = [](const char* str, char** end) { return std::strtol(str,end,10); };
        return LexicalCastImpl<int, long>::evalImpl(str, parser);
      }
    };

    template<> struct LexicalCast<long> {
      static long eval (const char* str)
      {
        auto parser = [](const char* str, char** end) { return std::strtol(str,end,10); };
        return LexicalCastImpl<long>::evalImpl(str, parser);
      }
    };

    template<> struct LexicalCast<long long> {
      static long long eval (const char* str)
      {
        auto parser = [](const char* str, char** end) { return std::strtoll(str,end,10); };
        return LexicalCastImpl<long>::evalImpl(str, parser);
      }
    };


    // unsigned integer types
    template<> struct LexicalCast<bool> {
      static unsigned char eval (const char* str)
      {
        auto parser = [](const char* str, char** end) { return std::strtoul(str,end,10); };
        return LexicalCastImpl<bool, unsigned long>::evalImpl(str, parser);
      }
    };

    template<>
    struct LexicalCast<unsigned char> {
      static unsigned char eval (const char* str)
      {
        auto parser = [](const char* str, char** end) { return std::strtoul(str,end,10); };
        return LexicalCastImpl<unsigned char, unsigned long>::evalImpl(str, parser);
      }
    };

    template<>
    struct LexicalCast<unsigned short> {
      static unsigned short eval (const char* str)
      {
        auto parser = [](const char* str, char** end) { return std::strtoul(str,end,10); };
        return LexicalCastImpl<unsigned short, unsigned long>::evalImpl(str, parser);
      }
    };

    template<>
    struct LexicalCast<unsigned int> {
      static unsigned int eval (const char* str)
      {
        auto parser = [](const char* str, char** end) { return std::strtoul(str,end,10); };
        return LexicalCastImpl<unsigned int, unsigned long>::evalImpl(str, parser);
      }
    };

    template<>
    struct LexicalCast<unsigned long> {
      static unsigned long eval (const char* str)
      {
        auto parser = [](const char* str, char** end) { return std::strtoul(str,end,10); };
        return LexicalCastImpl<unsigned long>::evalImpl(str, parser);
      }
    };

    template<>
    struct LexicalCast<unsigned long long> {
      static unsigned long long eval (const char* str)
      {
        auto parser = [](const char* str, char** end) { return std::strtoull(str,end,10); };
        return LexicalCastImpl<unsigned long long>::evalImpl(str, parser);
      }
    };


    // floating-point types
    template<>
    struct LexicalCast<float> {
      static float eval (const char* str)
      {
        return LexicalCastImpl<float>::evalImpl(str, std::strtof);
      }
    };

    template<>
    struct LexicalCast<double> {
      static double eval (const char* str)
      {
        return LexicalCastImpl<double>::evalImpl(str, std::strtod);
      }
    };

    template<>
    struct LexicalCast<long double> {
      static long double eval (const char* str)
      {
        return LexicalCastImpl<long double>::evalImpl(str, std::strtold);
      }
    };

  } // end namespace Impl


  /// \brief Convert a character sequence to a number type `T`
  /**
   * The cast from character sequence to any numeric type is implemented in terms of std library functions
   * like `strtol()` and `strtod()` but parametrized with the concrete target type for ease of usage
   * in generic code.
   *
   * Only types that provide a specialization of `Impl::LexicalCast` can call the special functions
   * for conversion. All other types by default call the constructor with strings.
   *
   * The conversion is locale-dependent and throws an \ref InvalidArgument exception if not all
   * characters are consumed during conversion, except leading and trailing whitespaces.
   *
   * In case the represented number is out of range of the number type T, a \ref RangeError exception
   * is thrown.
   *
   * \tparam T   The target number type to convert the string to.
   * \param str  A pointer to the null-terminated byte string to be interpreted.
   *
   * \throws InvalidArgument
   * \throws RangeError
   **/
  template<typename T>
  T lexicalCast(const char* str) { return Impl::LexicalCast<T>::eval(str); }

} // end namespace Dune

#endif // DUNE_COMMON_LEXICALCAST_HH
