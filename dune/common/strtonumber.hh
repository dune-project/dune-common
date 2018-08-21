// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_STRTONUMBER_HH
#define DUNE_COMMON_STRTONUMBER_HH

#include <cerrno>
#include <cstdlib>
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
    struct StrToNumber {
      static T eval (const char* str) { return T(str); }
    };

    /// \brief Implementation of the actual number converter.
    /**
     * Allows leading and trailing whitespace characters.
     *
     * \tparam T  Target numeric type
     **/
    template<typename T>
    struct StrToNumberImpl
    {
      // The parser has the signature `T(const char*, char**)` and may set the errno
      // in case of a range error.
      template<typename Parser>
      static T evalImpl (const char* str, Parser parser)
      {
        char* end;
        auto old_errno = errno;
        errno = 0;
        auto x = parser(str, &end); // maybe sets errno
        std::swap(errno, old_errno);

        if (old_errno == ERANGE) {
          DUNE_THROW(RangeError, std::strerror(old_errno));
        }

        // test whether all non-space characters are consumed during conversion
        bool all_consumed = (end != str);
        while (all_consumed && (*end != '\0'))
          all_consumed = std::isspace(*end++);

        if (!all_consumed) {
          DUNE_THROW(InvalidArgument,
            "Conversion of '" << str << "' to number failed. Possible reason: invalid string or locale format");
        }

        return convert(x);
      }

      // Check whether a numeric conversion is safe
      template<typename U>
      static T convert (U const& u)
      {
        if (sizeof(U) > sizeof(T) && (u > U(std::numeric_limits<T>::max()) ||
                                      u < U(std::numeric_limits<T>::min()))) {
          DUNE_THROW(RangeError, "Numerical result out of range");
        }
        return T(u);
      }
    };

    // signed integer types
    template<> struct StrToNumber<char> {
      static char eval (const char* str)
      {
        auto parser = [](const char* str, char** end) { return std::strtol(str,end,0); };
        return StrToNumberImpl<char>::evalImpl(str, parser);
      }
    };

    template<> struct StrToNumber<signed char> {
      static signed char eval (const char* str)
      {
        auto parser = [](const char* str, char** end) { return std::strtol(str,end,0); };
        return StrToNumberImpl<signed char>::evalImpl(str, parser);
      }
    };

    template<> struct StrToNumber<signed short> {
      static signed short eval (const char* str)
      {
        auto parser = [](const char* str, char** end) { return std::strtol(str,end,0); };
        return StrToNumberImpl<signed short>::evalImpl(str, parser);
      }
    };

    template<> struct StrToNumber<int> {
      static int eval (const char* str)
      {
        auto parser = [](const char* str, char** end) { return std::strtol(str,end,0); };
        return StrToNumberImpl<int>::evalImpl(str, parser);
      }
    };

    template<> struct StrToNumber<long> {
      static long eval (const char* str)
      {
        auto parser = [](const char* str, char** end) { return std::strtol(str,end,0); };
        return StrToNumberImpl<long>::evalImpl(str, parser);
      }
    };

    template<> struct StrToNumber<long long> {
      static long long eval (const char* str)
      {
        auto parser = [](const char* str, char** end) { return std::strtoll(str,end,0); };
        return StrToNumberImpl<long>::evalImpl(str, parser);
      }
    };


    // unsigned integer types
    template<> struct StrToNumber<bool> {
      static unsigned char eval (const char* str)
      {
        auto parser = [](const char* str, char** end) { return std::strtoul(str,end,0); };
        return StrToNumberImpl<bool>::evalImpl(str, parser);
      }
    };

    template<>
    struct StrToNumber<unsigned char> {
      static unsigned char eval (const char* str)
      {
        auto parser = [](const char* str, char** end) { return std::strtoul(str,end,0); };
        return StrToNumberImpl<unsigned char>::evalImpl(str, parser);
      }
    };

    template<>
    struct StrToNumber<unsigned short> {
      static unsigned short eval (const char* str)
      {
        auto parser = [](const char* str, char** end) { return std::strtoul(str,end,0); };
        return StrToNumberImpl<unsigned short>::evalImpl(str, parser);
      }
    };

    template<>
    struct StrToNumber<unsigned int> {
      static unsigned int eval (const char* str)
      {
        auto parser = [](const char* str, char** end) { return std::strtoul(str,end,0); };
        return StrToNumberImpl<unsigned int>::evalImpl(str, parser);
      }
    };

    template<>
    struct StrToNumber<unsigned long> {
      static unsigned long eval (const char* str)
      {
        auto parser = [](const char* str, char** end) { return std::strtoul(str,end,0); };
        return StrToNumberImpl<unsigned long>::evalImpl(str, parser);
      }
    };

    template<>
    struct StrToNumber<unsigned long long> {
      static unsigned long long eval (const char* str)
      {
        auto parser = [](const char* str, char** end) { return std::strtoull(str,end,0); };
        return StrToNumberImpl<unsigned long long>::evalImpl(str, parser);
      }
    };


    // floating-point types
    template<>
    struct StrToNumber<float> {
      static float eval (const char* str)
      {
        return StrToNumberImpl<float>::evalImpl(str, std::strtof);
      }
    };

    template<>
    struct StrToNumber<double> {
      static double eval (const char* str)
      {
        return StrToNumberImpl<double>::evalImpl(str, std::strtod);
      }
    };

    template<>
    struct StrToNumber<long double> {
      static long double eval (const char* str)
      {
        return StrToNumberImpl<long double>::evalImpl(str, std::strtold);
      }
    };

  } // end namespace Impl


  /// \brief Convert a character sequence to a number type `T`
  /**
   * The cast from character sequence to any numeric type is implemented in terms of std library functions
   * like `strtol()` and `strtod()` but parametrized with the concrete target type for ease of usage
   * in generic code.
   *
   * Only types that provide a specialization of `Impl::StrToNumber` can call the special functions
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
   *
   * Example of usage:
   * \code{.cpp}
   * double x = strTo<double>("1.2345");
   * \endcode
   **/
  template<typename T>
  T strTo (const char* str) { return Impl::StrToNumber<T>::eval(str); }

  /// Overload of \ref strTo for `std::string` arguments.
  template<typename T>
  T strTo (std::string const& str) { return strTo<T>(str.c_str()); }

} // end namespace Dune

#endif // DUNE_COMMON_STRTONUMBER_HH
