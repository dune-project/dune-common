// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef DUNE_BIGUNSIGNEDINT_HH
#define DUNE_BIGUNSIGNEDINT_HH

#include <iostream>

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
  /**
   * @brief Portable very large unsigned integers
   *
   * Implements (arbitrarily) large unsigned integers to be used as global
   * ids in some grid managers. Size is a template parameter.
   */

  template<int k>
  class bigunsignedint {
  public:

    // unsigned short is 16 bits wide, n is the number of digits needed
    enum { n=k/16+(k%16!=0), bits=16, hexdigits=4, bitmask=0xFFFF, compbitmask=0xFFFF0000,
           overflowmask=0x1 };

    //! Construct as zero
    bigunsignedint ();

    //! Construct from unsigned int
    bigunsignedint (unsigned int x);

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
    bigunsignedint<k> operator/ (const bigunsignedint<k>& x) const;

    //! modulo
    bigunsignedint<k> operator% (const bigunsignedint<k>& x) const;


    //! bitwise and
    bigunsignedint<k> operator& (const bigunsignedint<k>& x) const;

    //! bitwise exor
    bigunsignedint<k> operator^ (const bigunsignedint<k>& x) const;

    //! bitwise or
    bigunsignedint<k> operator| (const bigunsignedint<k>& x) const;

    //! bitwise komplement
    bigunsignedint<k> operator~ () const;


    //! left shift
    bigunsignedint<k> operator<< (int i) const;

    //! right shift
    bigunsignedint<k> operator>> (int i) const;


    //! less than
    bool operator< (const bigunsignedint<k>& x) const;

    //! less than or equal
    bool operator<= (const bigunsignedint<k>& x) const;

    //! greater than
    bool operator> (const bigunsignedint<k>& x) const;

    //! greater or equalt
    bool operator>= (const bigunsignedint<k>& x) const;

    //! equal
    bool operator== (const bigunsignedint<k>& x) const;

    //! not equal
    bool operator!= (const bigunsignedint<k>& x) const;

    friend class bigunsignedint<k/2>;
  private:
    unsigned short digit[n];
  } ;


  // Constructors
  template<int k>
  bigunsignedint<k>::bigunsignedint ()
  {  }

  template<int k>
  bigunsignedint<k>::bigunsignedint (unsigned int x)
  {
    // assume unsigned int is 32 bits
    digit[0] = (x&bitmask);
    if (n>1) digit[1] = (x>>bits)&bitmask;
    for (unsigned int i=2; i<n; i++) digit[i]=0;
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
      if (i-1>=0)
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
  inline bigunsignedint<k> operator+ (const bigunsignedint<k>& x, unsigned int y)
  {
    bigunsignedint<k> temp(y);
    return x+temp;
  }

  template <int k>
  inline bigunsignedint<k> operator- (const bigunsignedint<k>& x, unsigned int y)
  {
    bigunsignedint<k> temp(y);
    return x-temp;
  }

  template <int k>
  inline bigunsignedint<k> operator* (const bigunsignedint<k>& x, unsigned int y)
  {
    bigunsignedint<k> temp(y);
    return x*temp;
  }

  template <int k>
  inline bigunsignedint<k> operator/ (const bigunsignedint<k>& x, unsigned int y)
  {
    bigunsignedint<k> temp(y);
    return x/temp;
  }

  template <int k>
  inline bigunsignedint<k> operator% (const bigunsignedint<k>& x, unsigned int y)
  {
    bigunsignedint<k> temp(y);
    return x%temp;
  }

  template <int k>
  inline bigunsignedint<k> operator+ (unsigned int x, const bigunsignedint<k>& y)
  {
    bigunsignedint<k> temp(x);
    return temp+y;
  }

  template <int k>
  inline bigunsignedint<k> operator- (unsigned int x, const bigunsignedint<k>& y)
  {
    bigunsignedint<k> temp(x);
    return temp-y;
  }

  template <int k>
  inline bigunsignedint<k> operator* (unsigned int x, const bigunsignedint<k>& y)
  {
    bigunsignedint<k> temp(x);
    return temp*y;
  }

  template <int k>
  inline bigunsignedint<k> operator/ (unsigned int x, const bigunsignedint<k>& y)
  {
    bigunsignedint<k> temp(x);
    return temp/y;
  }

  template <int k>
  inline bigunsignedint<k> operator% (unsigned int x, const bigunsignedint<k>& y)
  {
    bigunsignedint<k> temp(x);
    return temp%y;
  }


  /** @} */
}
#endif
