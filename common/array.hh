// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_ARRAY_HH
#define __DUNE_ARRAY_HH


//***********************************************************************
//
//  implementation of peter array
//
//***********************************************************************

#include <iostream>
#include <iomanip>
#include <string>

namespace Dune
{
  /** @addtogroup Common

     @{
   */

  //! simple fixed size array class
  template<class T, int n>
  class FixedArray {
  public:
    //! create empty array
    FixedArray () {}

    //! initialize all components with same size
    FixedArray (T t)
    {
      for (int i=0; i<n; i++) a[i]=t;
    }

    //! assign value to all entries
    FixedArray<T,n>& operator= (const T& t)
    {
      for (int i=0; i<n; i++) a[i]=t;
      return (*this);
    }

    //! component access
    T& operator[] (int i)
    {
      return a[i];
    }

    //! const component access
    const T& operator[] (int i) const
    {
      return a[i];
    }

    FixedArray<T,n-1> shrink (int comp)
    {
      FixedArray<T,n-1> x;
      for (int i=0; i<comp; i++) x[i] = a[i];
      for (int i=comp+1; i<n; i++) x[i-1] = a[i];
      return x;
    }

    FixedArray<T,n+1> expand (int comp, T value)
    {
      FixedArray<T,n+1> x;
      for (int i=0; i<comp; i++) x[i] = a[i];
      x[comp] = value;
      for (int i=comp+1; i<n+1; i++) x[i] = a[i-1];
      return x;
    }

  private:
    T a[n];
  };

  //! Output operator for FixedArray
  template <class T, int n>
  inline std::ostream& operator<< (std::ostream& s, FixedArray<T,n> e)
  {
    s << "[";
    for (int i=0; i<n-1; i++) s << e[i] << ",";
    s << e[n-1] << "]";
    return s;
  }

  //! a simple dynamic array class with copy semantics
  template <class T>
  class Array {
  public:

    //! an Iterator to access all components of array
    class Iterator {      // Iteratorklasse zum
    private:              // Durchlaufen der Elemente des Containers
      T* p;             // Iterator ist Zeiger auf Feldelement
    public:               // Iterator ohne Bereichstest !
      //! make iterator
      Iterator();

      //! equality of two iterators
      bool operator!= (Iterator x);

      //! inequality of two iterators
      bool operator== (Iterator x);

      //! prefix increment
      Iterator operator++ ();       // prefix Stroustrup p. 292

      //! postfix increment
      Iterator operator++ (int);    // postfix

      //! dereferencing
      T& operator* () const;

      //! selector
      T* operator-> () const;     // Stroustrup p. 289

      friend class Array<T>;
    } ;

    //! return iterator refering to first element in array
    Iterator begin () const;

    //! return iterator refering to on past the last element of the array
    Iterator end () const;

    //! make empty array
    Array();

    //! make array with m components
    Array(int m);

    //! copy constructor making shallow copy
    Array (const Array<T>&);

    //! destructor
    ~Array();

    //! assignment of two arrays
    Array<T>& operator= (const Array<T>&);

    //! assign value to all components
    Array<T>& operator= (const T t);

    //! reallocate array with size m
    void resize (int m);

    //! return number of components in array
    int size () const;

    //! random access operator
    T& operator[](int i);

    //! export base type of array
    typedef T MemberType;      // Merke den Grundtyp ...

    //! print contents of array, k elements in a row
    void print (int k, std::string s, std::string row)
    {
      char buf[96];
      std::cout << s << " size=" << n << " {" << std::endl;
      for (int i=0; i<n/k; i++)
      {
        sprintf(buf,"%4d",k*i);
        std::cout << buf << " " << row << " ";
        for (int j=i*k; j<(i+1)*k; j++)
        {
          sprintf(buf,"%10.3E ",p[j]);
          std::cout << buf;
        }
        std::cout << std::endl;
      }
      int i = (n/k)*k;
      if (i<n)
      {
        sprintf(buf,"%4d",k*i);
        std::cout << buf << " " << row << " ";
        for (int j=i; j<n; j++)
        {
          sprintf(buf,"%10.3E ",p[j]);
          std::cout << buf;
        }
        std::cout << std::endl;
      }
      std::cout << "}" << std::endl;
    }

  protected:
    int n;      // Anzahl Elemente; n=0 heisst, dass kein array allokiert ist!
    T *p;       // Zeiger auf built-in array
  } ;


  // Iterator interface
  template<class T>
  inline typename Array<T>::Iterator Array<T>::begin () const
  {
    Iterator tmp;      // hat Zeiger 0
    tmp.p = p;         // Zeiger auf Feldelement 0
    return tmp;
  }

  template<class T>
  inline typename Array<T>::Iterator Array<T>::end () const
  {
    Iterator tmp;
    tmp.p = &(p[n]);     // Zeiger auf Feldelement NACH letztem !
    return tmp;          // das funktioniert: Stroustrup p. 92.
  }

  // Destruktor
  template <class T>
  inline Array<T>::~Array ()
  {
    if (n>0) delete[] p;
  }

  // Konstruktoren
  template <class T>
  inline Array<T>::Array ()
  {
    n = 0;
  }

  template <class T>
  inline Array<T>::Array (int m)
  {
    n = m;
    if (n<=0)
    {
      n = 0;
      return;
    }
    try {
      p = new T[n];
    }
    catch (std::bad_alloc) {
      std::cerr << "nicht genug Speicher!" << std::endl;
      throw;
    }
  }

  template <class T>
  inline void Array<T>::resize (int m)
  {
    if (m!=n)
    {
      if (n>0) delete[] p;
      n = m;
      if (n==0) return;
      try {
        p = new T[n];
      }
      catch (std::bad_alloc) {
        std::cerr << "nicht genug Speicher!" << std::endl;
        throw;
      }
    }
  }

  // Copy-Konstruktor
  template <class T>
  inline Array<T>::Array (const Array<T>& a)
  {
    // Erzeuge Feld mit selber Groesse wie a
    n = a.n;
    if (n>0)
    {
      try {
        p = new T[n];
      }
      catch (std::bad_alloc) {
        std::cerr << "nicht genug Speicher!" << std::endl;
        throw;
      }
    }

    // und kopiere Elemente
    for (int i=0; i<n; i=i+1) p[i]=a.p[i];
  }

  // Zuweisung
  template <class T>
  inline Array<T>& Array<T>::operator= (const Array<T>& a)
  {
    if (&a!=this)     // nur bei verschiedenen Objekten was tun
    {
      if (n!=a.n)
      {
        // allokiere fuer this ein Feld der Groesse a.n
        if (n>0) delete[] p;                 // altes Feld loeschen
        n = a.n;
        if (n>0)
        {
          try {
            p = new T[n];
          }
          catch (std::bad_alloc) {
            std::cerr << "nicht genug Speicher!" << std::endl;
            throw;
          }
        }
      }
      for (int i=0; i<n; i++) p[i]=a.p[i];
    }
    return *this;     // Gebe Referenz zurueck damit a=b=c; klappt
  }

  // Zuweisung mit member type
  template <class T>
  inline Array<T>& Array<T>::operator= (T a)
  {
    for (int i=0; i<n; i++) p[i]=a;
    return *this;     // Gebe Referenz zurueck damit a=b=c; klappt
  }

  // Indizierung
  template <class T>
  inline T& Array<T>::operator[] (int i)
  {
    return p[i];
  }

  // Groesse
  template <class T>
  inline int Array<T>::size () const
  {
    return n;
  }

  // Ausgabe
  template <class T>
  std::ostream& operator<< (std::ostream& s, Array<T>& a)
  {
    s << "array " << a.size() << " elements = [" << std::endl;
    for (int i=0; i<a.size(); i++)
      s << "    " << i << "  " << a[i] << std::endl;
    s << "]" << std::endl;
    return s;
  }

  template<class T>
  inline Array<T>::Iterator::Iterator ()
  {
    p=0;     // nicht initialisierter Iterator
  }

  template<class T>
  inline bool Array<T>::Iterator::operator!=
    (Array<T>::Iterator x)
  {
    return p != x.p;
  }

  template<class T>
  inline bool Array<T>::Iterator::operator==
    (Array::Iterator x)
  {
    return p == x.p;
  }

  template<class T>
  inline typename Array<T>::Iterator Array<T>::Iterator::operator++ () // prefix
  {
    p++;      // C Zeigerarithmetik: p zeigt auf naechstes Feldelement
    return *this;
  }

  template<class T>
  inline typename Array<T>::Iterator Array<T>::Iterator::operator++ (int) // postfix
  {
    Iterator tmp = *this;
    ++*this;
    return tmp;
  }

  template<class T>
  inline T& Array<T>::Iterator::operator* () const
  {
    return *p;
  }

  template<class T>
  inline T* Array<T>::Iterator::operator-> () const
  {
    return p;
  }

  //! a simple vector class derived from array
  template <class T>
  class Vector : public Array<T> {
  public:
    //! make empty vector
    Vector() {};

    //! make array with m components
    Vector(int m) : Array<T>::Array(m) {}

    //! assignment from scalar
    Vector<T>& operator= (const T t)
    {
      for (int i=0; i<n; ++i) p[i] = t;
      return *this;
    }

    //! scalar product of two vectors, no check for size !
    T ddot (const Vector<T>& x)
    {
      T sum = 0;
      for (int i=0; i<n; ++i) sum += p[i]*x.p[i];
      return sum;
    }

    // add scalar times other vector
    void daxpy (T a, const Vector<T>& x)
    {
      for (int i=0; i<n; ++i) p[i] += a*x.p[i];
    }
  };

  /** @} */

} // end namespace Dune

#endif
