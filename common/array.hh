// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_ARRAY_HH
#define __DUNE_ARRAY_HH


//***********************************************************************
//
//  implementation of peter array
//
//***********************************************************************
namespace Dune
{

  template <class T> class Array {
  public:
    class Iterator {      // Iteratorklasse zum
    private:              // Durchlaufen der Elemente des Containers
      T* p;                   // Iterator ist Zeiger auf Feldelement
    public:               // Iterator ohne Bereichstest !
      Iterator();
      bool operator!= (Iterator x);
      bool operator== (Iterator x);
      Iterator operator++ ();             // prefix Stroustrup p. 292
      Iterator operator++ (int);          // postfix
      T& operator* () const;
      T* operator-> () const;           // Stroustrup p. 289
      friend class Array<T>;
    } ;


    Iterator begin () const;
    Iterator end () const;

    Array();
    Array(int m);
    Array (const Array<T>&);
    ~Array();
    Array<T>& operator= (const Array<T>&);
    Array<T>& operator= (const T t);
    void realloc (int m);
    int size () const;
    T& operator[](int i);
    typedef T MemberType;      // Merke den Grundtyp ...
  private:
    int n;      // Anzahl Elemente
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
    delete[] p;
  }

  // Konstruktoren
  template <class T>
  inline Array<T>::Array ()
  {
    n = 1;
    try {
      p = new T[n];
    }
    catch (std::bad_alloc) {
      std::cerr << "nicht genug Speicher!" << std::endl;
      throw;
    }
  }

  template <class T>
  inline Array<T>::Array (int m)
  {
    n = m;
    try {
      p = new T[n];
    }
    catch (std::bad_alloc) {
      std::cerr << "nicht genug Speicher!" << std::endl;
      throw;
    }
  }

  template <class T>
  inline void Array<T>::realloc (int m)
  {
    delete[] p;
    n = m;
    try {
      p = new T[n];
    }
    catch (std::bad_alloc) {
      std::cerr << "nicht genug Speicher!" << std::endl;
      throw;
    }
  }

  // Copy-Konstruktor
  template <class T>
  inline Array<T>::Array (const Array<T>& a)
  {
    // Erzeuge Feld mit selber Groesse wie a
    n = a.n;
    try {
      p = new T[n];
    }
    catch (std::bad_alloc) {
      std::cerr << "nicht genug Speicher!" << std::endl;
      throw;
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
        delete[] p;                 // altes Feld loeschen
        n = a.n;
        try {
          p = new T[n];
        }
        catch (std::bad_alloc) {
          std::cerr << "nicht genug Speicher!" << std::endl;
          throw;
        }
      }
      for (int i=0; i<n; i=i+1) p[i]=a.p[i];
    }
    return *this;     // Gebe Referenz zurueck damit a=b=c; klappt
  }

  inline Array<double >& Array<double >::operator= (const double t)
  {
    for (int i=0; i<n; i++) p[i] = t;
    return (*this);
  }

  template <class T>
  inline Array<T>& Array<T>::operator= (const T t)
  {
    //std::cout << "Array<T>::operator Warning, not implemented! \n";
    for (int i=0; i<n; i++) p[i] = t;
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
  std::ostream & operator<< (std::ostream& s, Array<T>& a)
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

} // end namespace Dune

#endif
