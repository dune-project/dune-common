// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_FVECTOR_HH__
#define __DUNE_FVECTOR_HH__

#include <math.h>
#include <complex>

#include "../common/exceptions.hh"

#include "istl.hh"

/*! \file __FILE__

   This file implements a vector constructed from a given type
   representing a field and a compile-time given size.
 */

namespace Dune {

  /** @defgroup ISTL Iterative Solvers Template Library
              @addtogroup ISTL
              @{
   */

  // forward declaration of template
  template<class K, int n> class FieldVector;

  // template meta program for assignment from scalar
  template<int I>
  struct fvmeta_assignscalar {
    template<class K, int n>
    static K& assignscalar (FieldVector<K,n>& x, const K& k)
    {
      x[I] = fvmeta_assignscalar<I-1>::assignscalar(x,k);
      return x[I];
    }
  };
  template<>
  struct fvmeta_assignscalar<0> {
    template<class K, int n>
    static K& assignscalar (FieldVector<K,n>& x, const K& k)
    {
      x[0] = k;
      return x[0];
    }
  };

  // template meta program for operator+=
  template<int I>
  struct fvmeta_plusequal {
    template<class K, int n>
    static void plusequal (FieldVector<K,n>& x, const FieldVector<K,n>& y)
    {
      fvmeta_plusequal<I-1>::plusequal(x,y);
      x[I] += y[I];
    }
  };
  template<>
  struct fvmeta_plusequal<0> {
    template<class K, int n>
    static void plusequal (FieldVector<K,n>& x, const FieldVector<K,n>& y)
    {
      x[0] += y[0];
    }
  };

  // template meta program for operator-=
  template<int I>
  struct fvmeta_minusequal {
    template<class K, int n>
    static void minusequal (FieldVector<K,n>& x, const FieldVector<K,n>& y)
    {
      fvmeta_minusequal<I-1>::minusequal(x,y);
      x[I] -= y[I];
    }
  };
  template<>
  struct fvmeta_minusequal<0> {
    template<class K, int n>
    static void minusequal (FieldVector<K,n>& x, const FieldVector<K,n>& y)
    {
      x[0] -= y[0];
    }
  };

  // template meta program for operator*=
  template<int I>
  struct fvmeta_multequal {
    template<class K, int n>
    static void multequal (FieldVector<K,n>& x, const K& k)
    {
      fvmeta_multequal<I-1>::multequal(x,k);
      x[I] *= k;
    }
  };
  template<>
  struct fvmeta_multequal<0> {
    template<class K, int n>
    static void multequal (FieldVector<K,n>& x, const K& k)
    {
      x[0] *= k;
    }
  };

  // template meta program for operator/=
  template<int I>
  struct fvmeta_divequal {
    template<class K, int n>
    static void divequal (FieldVector<K,n>& x, const K& k)
    {
      fvmeta_divequal<I-1>::divequal(x,k);
      x[I] /= k;
    }
  };
  template<>
  struct fvmeta_divequal<0> {
    template<class K, int n>
    static void divequal (FieldVector<K,n>& x, const K& k)
    {
      x[0] /= k;
    }
  };

  // template meta program for axpy
  template<int I>
  struct fvmeta_axpy {
    template<class K, int n>
    static void axpy (FieldVector<K,n>& x, const K& a, const FieldVector<K,n>& y)
    {
      fvmeta_axpy<I-1>::axpy(x,a,y);
      x[I] += a*y[I];
    }
  };
  template<>
  struct fvmeta_axpy<0> {
    template<class K, int n>
    static void axpy (FieldVector<K,n>& x, const K& a, const FieldVector<K,n>& y)
    {
      x[0] += a*y[0];
    }
  };

  // template meta program for dot
  template<int I>
  struct fvmeta_dot {
    template<class K, int n>
    static K dot (const FieldVector<K,n>& x, const FieldVector<K,n>& y)
    {
      return x[I]*y[I] + fvmeta_dot<I-1>::dot(x,y);
    }
  };
  template<>
  struct fvmeta_dot<0> {
    template<class K, int n>
    static K dot (const FieldVector<K,n>& x, const FieldVector<K,n>& y)
    {
      return x[0]*y[0];
    }
  };

  // some abs functions needed for the norms
  template<class K>
  double fvmeta_abs (const K& k)
  {
    return (k >= 0) ? k : -k;
  }

  template<class K>
  double fvmeta_abs (const std::complex<K>& c)
  {
    return sqrt(c.real()*c.real() + c.imag()*c.imag());
  }

  template<class K>
  double fvmeta_absreal (const K& k)
  {
    return fvmeta_abs(k);
  }

  template<class K>
  double fvmeta_absreal (const std::complex<K>& c)
  {
    return fvmeta_abs(c.real()) + fvmeta_abs(c.imag());
  }

  template<class K>
  double fvmeta_abs2 (const K& k)
  {
    return k*k;
  }

  template<class K>
  double fvmeta_abs2 (const std::complex<K>& c)
  {
    return c.real()*c.real() + c.imag()*c.imag();
  }

  // template meta program for one_norm
  template<int I>
  struct fvmeta_one_norm {
    template<class K, int n>
    static double one_norm (const FieldVector<K,n>& x)
    {
      return fvmeta_abs(x[I]) + fvmeta_one_norm<I-1>::one_norm(x);
    }
  };
  template<>
  struct fvmeta_one_norm<0> {
    template<class K, int n>
    static double one_norm (const FieldVector<K,n>& x)
    {
      return fvmeta_abs(x[0]);
    }
  };

  // template meta program for one_norm_real
  template<int I>
  struct fvmeta_one_norm_real {
    template<class K, int n>
    static double one_norm_real (const FieldVector<K,n>& x)
    {
      return fvmeta_absreal(x[I]) + fvmeta_one_norm_real<I-1>::one_norm_real(x);
    }
  };
  template<>
  struct fvmeta_one_norm_real<0> {
    template<class K, int n>
    static double one_norm_real (const FieldVector<K,n>& x)
    {
      return fvmeta_absreal(x[0]);
    }
  };

  // template meta program for two_norm squared
  template<int I>
  struct fvmeta_two_norm2 {
    template<class K, int n>
    static double two_norm2 (const FieldVector<K,n>& x)
    {
      return fvmeta_abs2(x[I]) + fvmeta_two_norm2<I-1>::two_norm2(x);
    }
  };
  template<>
  struct fvmeta_two_norm2<0> {
    template<class K, int n>
    static double two_norm2 (const FieldVector<K,n>& x)
    {
      return fvmeta_abs2(x[0]);
    }
  };

  // template meta program for infinity norm
  template<int I>
  struct fvmeta_infinity_norm {
    template<class K, int n>
    static double infinity_norm (const FieldVector<K,n>& x)
    {
      return std::max(fvmeta_abs(x[I]),fvmeta_infinity_norm<I-1>::infinity_norm(x));
    }
  };
  template<>
  struct fvmeta_infinity_norm<0> {
    template<class K, int n>
    static double infinity_norm (const FieldVector<K,n>& x)
    {
      return fvmeta_abs(x[0]);
    }
  };

  // template meta program for simplified infinity norm
  template<int I>
  struct fvmeta_infinity_norm_real {
    template<class K, int n>
    static double infinity_norm_real (const FieldVector<K,n>& x)
    {
      return std::max(fvmeta_absreal(x[I]),fvmeta_infinity_norm_real<I-1>::infinity_norm_real(x));
    }
  };
  template<>
  struct fvmeta_infinity_norm_real<0> {
    template<class K, int n>
    static double infinity_norm_real (const FieldVector<K,n>& x)
    {
      return fvmeta_absreal(x[0]);
    }
  };

  /**! Construct a vector space out of a tensor product of fields.
           K is the field type (use float, double, complex, etc) and n
       is the number of components.

           It is generally assumed that K is a numerical type compatible with double
       (E.g. norms are always computed in double precision).

           Implementation of all members uses template meta programs where appropriate
   */
  template<class K, int n>
  class FieldVector
  {
  public:
    // standard constructor and everything is sufficient ...


    //===== type definitions and constants

    //! export the type representing the field
    typedef K field_type;

    //! export the type representing the components
    typedef K block_type;

    //! We are at the leaf of the block recursion
    enum {blocklevel = 1};

    //! export size
    enum {size = n};


    //===== assignment from scalar
    FieldVector& operator= (const K& k)
    {
      fvmeta_assignscalar<n-1>::assignscalar(*this,k);
      return *this;
    }


    //===== access to components

    //! random access
    K& operator[] (int i)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i<0 || i>=n) DUNE_THROW(ISTLError,"index out of range");
#endif
      return p[i];
    }

    //! same for read only access
    const K& operator[] (int i) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i<0 || i>=n) DUNE_THROW(ISTLError,"index out of range");
#endif
      return p[i];
    }

    //! Iterator class for sequential access
    class Iterator
    {
    public:
      //! constructor
      Iterator (K* _p, int _i)
      {
        p = _p;
        i = _i;
      }

      //! prefix increment
      Iterator& operator++()
      {
        ++i;
        return *this;
      }

      //! equality
      bool operator== (const Iterator& it) const
      {
        return (p+i)==(it.p+it.i);
      }

      //! inequality
      bool operator!= (const Iterator& it) const
      {
        return (p+i)!=(it.p+it.i);
      }

      //! dereferencing
      K& operator* ()
      {
        return p[i];
      }

      //! arrow
      K* operator-> ()
      {
        return p+i;
      }

      //! return index
      int index ()
      {
        return i;
      }

    private:
      K* p;
      int i;
    };

    //! begin iterator
    Iterator begin ()
    {
      return Iterator(p,0);
    }

    //! end iterator
    Iterator end ()
    {
      return Iterator(p,n);
    }


    //===== vector space arithmetic

    //! vector space addition
    FieldVector& operator+= (const FieldVector& y)
    {
      fvmeta_plusequal<n-1>::plusequal(*this,y);
      return *this;
    }

    //! vector space subtraction
    FieldVector& operator-= (const FieldVector& y)
    {
      fvmeta_minusequal<n-1>::minusequal(*this,y);
      return *this;
    }

    //! vector space multiplication with scalar
    FieldVector& operator*= (const K& k)
    {
      fvmeta_multequal<n-1>::multequal(*this,k);
      return *this;
    }

    //! vector space division by scalar
    FieldVector& operator/= (const K& k)
    {
      fvmeta_divequal<n-1>::divequal(*this,k);
      return *this;
    }

    //! vector space axpy operation
    FieldVector& axpy (const K& a, const FieldVector& y)
    {
      fvmeta_axpy<n-1>::axpy(*this,a,y);
      return *this;
    }


    //===== Euclidean scalar product

    //! scalar product
    const K operator* (const FieldVector& y) const
    {
      return fvmeta_dot<n-1>::dot(*this,y);
    }


    //===== norms

    //! one norm (sum over absolute values of entries)
    double one_norm () const
    {
      return fvmeta_one_norm<n-1>::one_norm(*this);
    }

    //! simplified one norm (uses Manhattan norm for complex values)
    double one_norm_real () const
    {
      return fvmeta_one_norm_real<n-1>::one_norm_real(*this);
    }

    //! two norm sqrt(sum over squared values of entries)
    double two_norm () const
    {
      return sqrt(fvmeta_two_norm2<n-1>::two_norm2(*this));
    }

    //! sqare of two norm (sum over squared values of entries), need for block recursion
    double two_norm2 () const
    {
      return fvmeta_two_norm2<n-1>::two_norm2(*this);
    }

    //! infinity norm (maximum of absolute values of entries)
    double infinity_norm () const
    {
      return fvmeta_infinity_norm<n-1>::infinity_norm(*this);
    }

    //! simplified infinity norm (uses Manhattan norm for complex values)
    double infinity_norm_real () const
    {
      return fvmeta_infinity_norm_real<n-1>::infinity_norm_real(*this);
    }


    //===== sizes

    //! number of blocks in the vector (are of size 1 here)
    int N () const
    {
      return n;
    }

    //! dimension of the vector space
    int dim () const
    {
      return n;
    }

  private:
    // the data, very simply a built in array
    K p[n];
  };

  /** @} end documentation */

} // end namespace

#endif
