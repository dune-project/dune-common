// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_BIGUNSIGNEDINT_HH
#define DUNE_BIGUNSIGNEDINT_HH

#include <iostream>
#include <limits>
#include <cstdlib>
#include <dune/common/exceptions.hh>
#include <dune/common/hash.hh>

/**
 * @file
 * @brief  Portable very large unsigned integers
 * @author Peter Bastian
 */

namespace Dune
{
  /** @addtogroup Common
   *
   * @{
   */

#if HAVE_MPI
  template<class K>
  struct MPITraits;
#endif

  /**
   * @brief Portable very large unsigned integers
   *
   * Implements (arbitrarily) large unsigned integers to be used as global
   * ids in some grid managers. Size is a template parameter.
   *
   * \tparam k Number of bits of the integer type
   */

  template<int k>
  class bigunsignedint {
  public:

    // unsigned short is 16 bits wide, n is the number of digits needed
    enum { bits=std::numeric_limits<unsigned short>::digits, n=k/bits+(k%bits!=0),
           hexdigits=4, bitmask=0xFFFF, compbitmask=0xFFFF0000,
           overflowmask=0x1 };

    //! Construct uninitialized
    bigunsignedint ();

    //! Construct from signed int
    bigunsignedint (int x);

    //! Construct from unsigned int
    bigunsignedint (std::size_t x);

    //! Print number in hex notation
    void print (std::ostream& s) const ;

    //! add
    bigunsignedint<k> operator+ (const bigunsignedint<k>& x) const;

    //! subtract
    bigunsignedint<k> operator- (const bigunsignedint<k>& x) const;

    //! multiply
    bigunsignedint<k> operator* (const bigunsignedint<k>& x) const;

    //! prefix increment
    bigunsignedint<k>& operator++ ();

    //! divide
    //! \warning This function is very slow and its usage should be
    //! prevented if possible
    bigunsignedint<k> operator/ (const bigunsignedint<k>& x) const;

    //! modulo
    //! \warning This function is very slow and its usage should be
    //! prevented if possible
    bigunsignedint<k> operator% (const bigunsignedint<k>& x) const;


    //! bitwise and
    bigunsignedint<k> operator& (const bigunsignedint<k>& x) const;

    //! bitwise exor
    bigunsignedint<k> operator^ (const bigunsignedint<k>& x) const;

    //! bitwise or
    bigunsignedint<k> operator| (const bigunsignedint<k>& x) const;

    //! bitwise komplement
    bigunsignedint<k> operator~ () const;


    //! left shift1/
    bigunsignedint<k> operator<< (int i) const;

    //! right shift
    bigunsignedint<k> operator>> (int i) const;


    //! less than
    bool operator< (const bigunsignedint<k>& x) const;

    //! less than or equal
    bool operator<= (const bigunsignedint<k>& x) const;

    //! greater than
    bool operator> (const bigunsignedint<k>& x) const;

    //! greater or equal
    bool operator>= (const bigunsignedint<k>& x) const;

    //! equal
    bool operator== (const bigunsignedint<k>& x) const;

    //! not equal
    bool operator!= (const bigunsignedint<k>& x) const;


    //! export to other types
    //	operator unsigned int () const;
    unsigned int touint() const;
    /**
     * @brief Convert to a double.
     *
     * @warning Subject to rounding errors!
     */
    double todouble() const;

    friend class bigunsignedint<k/2>;
    friend struct std::numeric_limits< bigunsignedint<k> >;

    inline friend std::size_t hash_value(const bigunsignedint& arg)
    {
      return hash_range(arg.digit,arg.digit + arg.n);
    }

  private:
    unsigned short digit[n];
#if HAVE_MPI
    friend struct MPITraits<bigunsignedint<k> >;
#endif
    inline void assign(std::size_t x);


  } ;

  // Constructors
  template<int k>
  bigunsignedint<k>::bigunsignedint ()
  {
    assign(0u);
  }

  template<int k>
  bigunsignedint<k>::bigunsignedint (int y)
  {
    std::size_t x = std::abs(y);
    assign(x);
  }

  template<int k>
  bigunsignedint<k>::bigunsignedint (std::size_t x)
  {
    assign(x);
  }
  template<int k>
  void bigunsignedint<k>::assign(std::size_t x)
  {
    int no=std::min(static_cast<int>(n),
                    static_cast<int>(std::numeric_limits<std::size_t>::digits/bits));

    for(int i=0; i<no; ++i) {
      digit[i] = (x&bitmask);
      x=x>>bits;
    }
    for (unsigned int i=no; i<n; i++) digit[i]=0;
  }

  // export
  template<int k>
  inline unsigned int bigunsignedint<k>::touint () const
  {
    return (digit[1]<<bits)+digit[0];
  }

  template<int k>
  inline double bigunsignedint<k>::todouble() const
  {
    int firstInZeroRange=n;
    for(int i=n-1; i>=0; --i)
      if(digit[i]!=0)
        break;
      else
        --firstInZeroRange;
    int representableDigits=std::numeric_limits<double>::digits/bits;
    int lastInRepresentableRange=0;
    if(representableDigits<firstInZeroRange)
      lastInRepresentableRange=firstInZeroRange-representableDigits;
    double val=0;
    for(int i=firstInZeroRange-1; i>=lastInRepresentableRange; --i)
      val =val*(1<<bits)+digit[i];
    return val*(1<<(bits*lastInRepresentableRange));
  }
  // print
  template<int k>
  inline void bigunsignedint<k>::print (std::ostream& s) const
  {
    bool leading=false;

    // print from left to right
    for (int i=n-1; i>=0; i--)
      for (int d=hexdigits-1; d>=0; d--)
      {
        // extract one hex digit
        int current = (digit[i]>>(d*4))&0xF;
        if (current!=0)
        {
          //			  s.setf(std::ios::noshowbase);
          s << std::hex << current;
          leading = false;
        }
        else if (!leading) s << std::hex << current;
      }
    if (leading) s << "0";
    s << std::dec;
  }

  template <int k>
  inline std::ostream& operator<< (std::ostream& s, const bigunsignedint<k>& x)
  {
    x.print(s);
    return s;
  }


  // Operators
  template <int k>
  inline bigunsignedint<k> bigunsignedint<k>::operator+ (const bigunsignedint<k>& x) const
  {
    bigunsignedint<k> result;
    int overflow=0;

    for (unsigned int i=0; i<n; i++)
    {
      int sum = ((int)digit[i]) + ((int)x.digit[i]) + overflow;
      result.digit[i] = sum&bitmask;
      overflow = (sum>>bits)&overflowmask;
    }
    return result;
  }

  template <int k>
  inline bigunsignedint<k> bigunsignedint<k>::operator- (const bigunsignedint<k>& x) const
  {
    bigunsignedint<k> result;
    int overflow=0;

    for (unsigned int i=0; i<n; i++)
    {
      int diff = ((int)digit[i]) - (((int)x.digit[i]) + overflow);
      if (diff>=0)
        result.digit[i] = (unsigned short) diff;
      else
      {
        result.digit[i] = (unsigned short) (diff+bitmask);
        overflow = 1;
      }
    }
    return result;
  }

  template <int k>
  inline bigunsignedint<k> bigunsignedint<k>::operator* (const bigunsignedint<k>& x) const
  {
    bigunsignedint<2*k> finalproduct(0);

    for (unsigned int m=0; m<n; m++)     // digit in right factor
    {
      bigunsignedint<2*k> singleproduct(0);
      unsigned int overflow(0);
      for (unsigned int i=0; i<n; i++)
      {
        unsigned int digitproduct = ((unsigned int)digit[i])*((unsigned int)x.digit[m])+overflow;
        singleproduct.digit[i+m] = (unsigned short) (digitproduct&bitmask);
        overflow = (digitproduct>>bits)&bitmask;
      }
      finalproduct = finalproduct+singleproduct;
    }

    bigunsignedint<k> result;
    for (unsigned int i=0; i<n; i++) result.digit[i] = finalproduct.digit[i];
    return result;
  }

  template <int k>
  inline bigunsignedint<k>& bigunsignedint<k>::operator++ ()
  {
    int overflow=1;

    for (unsigned int i=0; i<n; i++)
    {
      int sum = ((int)digit[i]) + overflow;
      digit[i] = sum&bitmask;
      overflow = (sum>>bits)&overflowmask;
    }
    return *this;
  }

  template <int k>
  inline bigunsignedint<k> bigunsignedint<k>::operator/ (const bigunsignedint<k>& x) const
  {
    if(x==0)
      DUNE_THROW(Dune::MathError, "division by zero!");

    // better slow than nothing
    bigunsignedint<k> temp(*this);
    bigunsignedint<k> result(0);

    while (temp>=x)
    {
      ++result;
      temp = temp-x;
    }

    return result;
  }

  template <int k>
  inline bigunsignedint<k> bigunsignedint<k>::operator% (const bigunsignedint<k>& x) const
  {
    // better slow than nothing
    bigunsignedint<k> temp(*this);
    bigunsignedint<k> result(0);

    while (temp>=x)
    {
      ++result;
      temp = temp-x;
    }

    return temp;
  }


  template <int k>
  inline bigunsignedint<k> bigunsignedint<k>::operator& (const bigunsignedint<k>& x) const
  {
    bigunsignedint<k> result;
    for (unsigned int i=0; i<n; i++)
      result.digit[i] = digit[i]&x.digit[i];
    return result;
  }

  template <int k>
  inline bigunsignedint<k> bigunsignedint<k>::operator^ (const bigunsignedint<k>& x) const
  {
    bigunsignedint<k> result;
    for (unsigned int i=0; i<n; i++)
      result.digit[i] = digit[i]^x.digit[i];
    return result;
  }

  template <int k>
  inline bigunsignedint<k> bigunsignedint<k>::operator| (const bigunsignedint<k>& x) const
  {
    bigunsignedint<k> result;
    for (unsigned int i=0; i<n; i++)
      result.digit[i] = digit[i]|x.digit[i];
    return result;
  }

  template <int k>
  inline bigunsignedint<k> bigunsignedint<k>::operator~ () const
  {
    bigunsignedint<k> result;
    for (unsigned int i=0; i<n; i++)
      result.digit[i] = ~digit[i];
    return result;
  }

  template <int k>
  inline bigunsignedint<k> bigunsignedint<k>::operator<< (int shift) const
  {
    bigunsignedint<k> result(0);

    // multiples of bits
    int j=shift/bits;
    for (int i=n-1-j; i>=0; i--)
      result.digit[i+j] = digit[i];

    // remainder
    j=shift%bits;
    for (int i=n-1; i>=0; i--)
    {
      unsigned int temp = result.digit[i];
      temp = temp<<j;
      result.digit[i] = (unsigned short) (temp&bitmask);
      temp = temp>>bits;
      if (i+1<(int)n)
        result.digit[i+1] = result.digit[i+1]|temp;
    }

    return result;
  }

  template <int k>
  inline bigunsignedint<k> bigunsignedint<k>::operator>> (int shift) const
  {
    bigunsignedint<k> result(0);

    // multiples of bits
    int j=shift/bits;
    for (unsigned int i=0; i<n-j; i++)
      result.digit[i] = digit[i+j];

    // remainder
    j=shift%bits;
    for (unsigned int i=0; i<n; i++)
    {
      unsigned int temp = result.digit[i];
      temp = temp<<(bits-j);
      result.digit[i] = (unsigned short) ((temp&compbitmask)>>bits);
      if (i>0)
        result.digit[i-1] = result.digit[i-1] | (temp&bitmask);
    }

    return result;
  }

  template <int k>
  inline bool bigunsignedint<k>::operator!= (const bigunsignedint<k>& x) const
  {
    for (unsigned int i=0; i<n; i++)
      if (digit[i]!=x.digit[i]) return true;
    return false;
  }

  template <int k>
  inline bool bigunsignedint<k>::operator== (const bigunsignedint<k>& x) const
  {
    return !((*this)!=x);
  }

  template <int k>
  inline bool bigunsignedint<k>::operator< (const bigunsignedint<k>& x) const
  {
    for (int i=n-1; i>=0; i--)
      if (digit[i]<x.digit[i]) return true;
      else if (digit[i]>x.digit[i]) return false;
    return false;
  }

  template <int k>
  inline bool bigunsignedint<k>::operator<= (const bigunsignedint<k>& x) const
  {
    for (int i=n-1; i>=0; i--)
      if (digit[i]<x.digit[i]) return true;
      else if (digit[i]>x.digit[i]) return false;
    return true;
  }

  template <int k>
  inline bool bigunsignedint<k>::operator> (const bigunsignedint<k>& x) const
  {
    return !((*this)<=x);
  }

  template <int k>
  inline bool bigunsignedint<k>::operator>= (const bigunsignedint<k>& x) const
  {
    return !((*this)<x);
  }


  template <int k>
  inline bigunsignedint<k> operator+ (const bigunsignedint<k>& x, std::size_t y)
  {
    bigunsignedint<k> temp(y);
    return x+temp;
  }

  template <int k>
  inline bigunsignedint<k> operator- (const bigunsignedint<k>& x, std::size_t y)
  {
    bigunsignedint<k> temp(y);
    return x-temp;
  }

  template <int k>
  inline bigunsignedint<k> operator* (const bigunsignedint<k>& x, std::size_t y)
  {
    bigunsignedint<k> temp(y);
    return x*temp;
  }

  template <int k>
  inline bigunsignedint<k> operator/ (const bigunsignedint<k>& x, std::size_t y)
  {
    bigunsignedint<k> temp(y);
    return x/temp;
  }

  template <int k>
  inline bigunsignedint<k> operator% (const bigunsignedint<k>& x, std::size_t y)
  {
    bigunsignedint<k> temp(y);
    return x%temp;
  }

  template <int k>
  inline bigunsignedint<k> operator+ (std::size_t x, const bigunsignedint<k>& y)
  {
    bigunsignedint<k> temp(x);
    return temp+y;
  }

  template <int k>
  inline bigunsignedint<k> operator- (std::size_t x, const bigunsignedint<k>& y)
  {
    bigunsignedint<k> temp(x);
    return temp-y;
  }

  template <int k>
  inline bigunsignedint<k> operator* (std::size_t x, const bigunsignedint<k>& y)
  {
    bigunsignedint<k> temp(x);
    return temp*y;
  }

  template <int k>
  inline bigunsignedint<k> operator/ (std::size_t x, const bigunsignedint<k>& y)
  {
    bigunsignedint<k> temp(x);
    return temp/y;
  }

  template <int k>
  inline bigunsignedint<k> operator% (std::size_t x, const bigunsignedint<k>& y)
  {
    bigunsignedint<k> temp(x);
    return temp%y;
  }


  /** @} */
}

namespace std
{
  template<int k>
  struct numeric_limits<Dune::bigunsignedint<k> >
  {
  public:
    static const bool is_specialized = true;

    static Dune::bigunsignedint<k> min()
    {
      return static_cast<Dune::bigunsignedint<k> >(0);
    }

    static Dune::bigunsignedint<k> max()
    {
      Dune::bigunsignedint<k> max_;
      for(std::size_t i=0; i < Dune::bigunsignedint<k>::n; ++i)
        max_.digit[i]=std::numeric_limits<unsigned short>::max();
      return max_;
    }


    static const int digits = Dune::bigunsignedint<k>::bits *
                              Dune::bigunsignedint<k>::n;
    static const bool is_signed = false;
    static const bool is_integer = true;
    static const bool is_exact = true;
    static const int radix = 2;

    static Dune::bigunsignedint<k> epsilon()
    {
      return static_cast<Dune::bigunsignedint<k> >(0);
    }

    static Dune::bigunsignedint<k> round_error()
    {
      return static_cast<Dune::bigunsignedint<k> >(0);
    }

    static const int min_exponent = 0;
    static const int min_exponent10 = 0;
    static const int max_exponent = 0;
    static const int max_exponent10 = 0;

    static const bool has_infinity = false;
    static const bool has_quiet_NaN = false;
    static const bool has_signaling_NaN = false;

    static const float_denorm_style has_denorm = denorm_absent;
    static const bool has_denorm_loss = false;

    static Dune::bigunsignedint<k> infinity() throw()
    {
      return static_cast<Dune::bigunsignedint<k> >(0);
    }

    static Dune::bigunsignedint<k> quiet_NaN() throw()
    {
      return static_cast<Dune::bigunsignedint<k> >(0);
    }

    static Dune::bigunsignedint<k> signaling_NaN() throw()
    {
      return static_cast<Dune::bigunsignedint<k> >(0);
    }

    static Dune::bigunsignedint<k> denorm_min() throw()
    {
      return static_cast<Dune::bigunsignedint<k> >(0);
    }

    static const bool is_iec559 = false;
    static const bool is_bounded = true;
    static const bool is_modulo = true;

    static const bool traps = false;
    static const bool tinyness_before = false;
    static const float_round_style round_style = round_toward_zero;

  };

}

DUNE_DEFINE_HASH(DUNE_HASH_TEMPLATE_ARGS(int k),DUNE_HASH_TYPE(Dune::bigunsignedint<k>))

#endif
