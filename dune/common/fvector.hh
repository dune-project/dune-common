// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_FVECTOR_HH
#define DUNE_FVECTOR_HH

#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <complex>
#include <cstring>
#include <limits>

#include "exceptions.hh"
#include "genericiterator.hh"

#include "ftraits.hh"

#ifdef DUNE_EXPRESSIONTEMPLATES
#include "exprtmpl.hh"
#endif

namespace Dune {

  // forward declaration of template
  template<class K, int SIZE> class FieldVector;

  template<class K, int SIZE>
  struct FieldTraits< FieldVector<K,SIZE> >
  {
    typedef const typename FieldTraits<K>::field_type field_type;
    typedef const typename FieldTraits<K>::real_type real_type;
  };

  /** @defgroup DenseMatVec Dense Matrix and Vector Template Library
      @ingroup Common
      @{
   */

  /*! \file
   * \brief This file implements a vector constructed from a given type
     representing a field and a compile-time given size.
   */

#ifndef DUNE_EXPRESSIONTEMPLATES

  namespace fvmeta
  {
    /**
       \private
       \memberof FieldVector
     */
    template<class K>
    inline typename FieldTraits<K>::real_type absreal (const K& k)
    {
      return std::abs(k);
    }

    /**
       \private
       \memberof FieldVector
     */
    template<class K>
    inline typename FieldTraits<K>::real_type absreal (const std::complex<K>& c)
    {
      return std::abs(c.real()) + std::abs(c.imag());
    }

    /**
       \private
       \memberof FieldVector
     */
    template<class K>
    inline typename FieldTraits<K>::real_type abs2 (const K& k)
    {
      return k*k;
    }

    /**
       \private
       \memberof FieldVector
     */
    template<class K>
    inline typename FieldTraits<K>::real_type abs2 (const std::complex<K>& c)
    {
      return c.real()*c.real() + c.imag()*c.imag();
    }

    /**
       \private
       \memberof FieldVector
     */
    template<class K, bool isInteger = std::numeric_limits<K>::is_integer>
    struct Sqrt
    {
      static inline typename FieldTraits<K>::real_type sqrt (const K& k)
      {
        return std::sqrt(k);
      }
    };

    /**
       \private
       \memberof FieldVector
     */
    template<class K>
    struct Sqrt<K, true>
    {
      static inline typename FieldTraits<K>::real_type sqrt (const K& k)
      {
        return std::sqrt((double)k);
      }
    };

    /**
       \private
       \memberof FieldVector
     */
    template<class K>
    inline typename FieldTraits<K>::real_type sqrt (const K& k)
    {
      return Sqrt<K>::sqrt(k);
    }

  }

#endif

  //! Iterator class for sequential access to FieldVector and FieldMatrix
  template<class C, class T>
  class FieldIterator :
    public Dune::RandomAccessIteratorFacade<FieldIterator<C,T>,T, T&, int>
  {
    friend class FieldIterator<typename remove_const<C>::type, typename remove_const<T>::type >;
    friend class FieldIterator<const typename remove_const<C>::type, const typename remove_const<T>::type >;

  public:

    /**
     * @brief The type of the difference between two positions.
     */
    typedef std::ptrdiff_t DifferenceType;

    // Constructors needed by the base iterators.
    FieldIterator()
      : container_(0), position_(0)
    {}

    FieldIterator(C& cont, DifferenceType pos)
      : container_(&cont), position_(pos)
    {}

    FieldIterator(const FieldIterator<typename remove_const<C>::type, typename remove_const<T>::type >& other)
      : container_(other.container_), position_(other.position_)
    {}

#if 0
    FieldIterator(const FieldIterator<const typename remove_const<C>::type, const typename remove_const<T>::type >& other)
      : container_(other.container_), position_(other.position_)
    {}
#endif
#if 0
    FieldIterator(const FieldIterator<C,T>& other)
      : container_(other.container_), position_(other.position_)
    {}
#endif
    // Methods needed by the forward iterator
    bool equals(const FieldIterator<typename remove_const<C>::type,typename remove_const<T>::type>& other) const
    {
      return position_ == other.position_ && container_ == other.container_;
    }


    bool equals(const FieldIterator<const typename remove_const<C>::type,const typename remove_const<T>::type>& other) const
    {
      return position_ == other.position_ && container_ == other.container_;
    }

    T& dereference() const {
      return container_->operator[](position_);
    }

    void increment(){
      ++position_;
    }

    // Additional function needed by BidirectionalIterator
    void decrement(){
      --position_;
    }

    // Additional function needed by RandomAccessIterator
    T& elementAt(DifferenceType i) const {
      return container_->operator[](position_+i);
    }

    void advance(DifferenceType n){
      position_=position_+n;
    }

    std::ptrdiff_t distanceTo(FieldIterator<const typename remove_const<C>::type,const typename remove_const<T>::type> other) const
    {
      assert(other.container_==container_);
      return other.position_ - position_;
    }

    std::ptrdiff_t distanceTo(FieldIterator<typename remove_const<C>::type, typename remove_const<T>::type> other) const
    {
      assert(other.container_==container_);
      return other.position_ - position_;
    }

    //! return index
    DifferenceType index () const
    {
      return this->position_;
    }

  private:
    C *container_;
    DifferenceType position_;
  };

  //! Type Traits for Vector::Iterator vs (const Vector)::ConstIterator
  template<class T>
  struct IteratorType
  {
    typedef typename T::Iterator type;
  };

  template<class T>
  struct IteratorType<const T>
  {
    typedef typename T::ConstIterator type;
  };

#ifdef DUNE_EXPRESSIONTEMPLATES
  //! Iterator class for flat sequential access to a nested Vector
  template<class V>
  class FlatIterator :
    public ForwardIteratorFacade<FlatIterator<V>,
        typename Dune::FieldType<V>::type,
        typename Dune::FieldType<V>::type&,
        int>
  {
  public:
    typedef typename IteratorType<V>::type BlockIterator;
    typedef std::ptrdiff_t DifferenceType;
    //    typedef typename BlockIterator::DifferenceType DifferenceType;
    typedef typename BlockType<V>::type block_type;
    typedef typename FieldType<V>::type field_type;
    typedef FlatIterator<block_type> SubBlockIterator;
    FlatIterator(const BlockIterator & i) :
      it(i), bit(i->begin()), bend(i->end()) {};
    void increment ()
    {
      ++bit;
      if (bit == bend)
      {
        ++it;
        bit = it->begin();
        bend = it->end();
      }
    }
    bool equals (const FlatIterator & fit) const
    {
      return fit.it == it && fit.bit == bit;
    }
    field_type& dereference() const
    {
      return *bit;
    }
    //! return index
    DifferenceType index () const
    {
      return bit.index();
    }
  private:
    BlockIterator it;
    SubBlockIterator bit;
    SubBlockIterator bend;
  };

  //! Specialization for FieldVector
  //! acts as the end of the recursive template
  template<class K, int N>
  class FlatIterator< FieldVector<K,N> > :
    public ForwardIteratorFacade<FlatIterator< FieldVector<K,N> >,
        K, K&, int>
  {
  public:
    typedef typename FieldVector<K,N>::Iterator BlockIterator;
    typedef std::ptrdiff_t DifferenceType;
    //    typedef typename BlockIterator::DifferenceType DifferenceType;
    typedef typename FieldVector<K,N>::field_type field_type;
    FlatIterator(const BlockIterator & i) : it(i) {};
    void increment ()
    {
      ++it;
    }
    bool equals (const FlatIterator & fit) const
    {
      return fit.it == it;
    }
    field_type& dereference() const
    {
      return *it;
    }
    //! return index
    DifferenceType index () const
    {
      return it.index();
    }
  private:
    BlockIterator it;
  };

  //! Specialization for const FieldVector
  //! acts as the end of the recursive template
  template<class K, int N>
  class FlatIterator< const FieldVector<K,N> > :
    public ForwardIteratorFacade<FlatIterator< const FieldVector<K,N> >,
        const K, const K&, int>
  {
  public:
    typedef typename FieldVector<K,N>::ConstIterator BlockIterator;
    typedef std::ptrdiff_t DifferenceType;
    //    typedef typename BlockIterator::DifferenceType DifferenceType;
    typedef typename FieldVector<K,N>::field_type field_type;
    FlatIterator(const BlockIterator & i) : it(i) {};
    void increment ()
    {
      ++it;
    }
    bool equals (const FlatIterator & fit) const
    {
      return fit.it == it;
    }
    const field_type& dereference() const
    {
      return *it;
    }
    //! return index
    DifferenceType index () const
    {
      return it.index();
    }
  private:
    BlockIterator it;
  };
#endif


  /** \brief Construct a vector space out of a tensor product of fields.
   *
   *  K is the field type (use float, double, complex, etc) and SIZE
   *  is the number of components.
   *
   *  It is generally assumed that K is a numerical type compatible with double
   *  (E.g. norms are always computed in double precision).
   */
  template< class K, int SIZE >
  class FieldVector
#ifdef DUNE_EXPRESSIONTEMPLATES
    : public Dune :: ExprTmpl :: Vector< FieldVector< K, SIZE > >
#endif
  {
  public:
    // remember size of vector
    enum { dimension = SIZE };

    // standard constructor and everything is sufficient ...

    //===== type definitions and constants

    //! export the type representing the field
    typedef K field_type;

    //! export the type representing the components
    typedef K block_type;

    //! The type used for the index access and size operation
    typedef std::size_t size_type;

    //! We are at the leaf of the block recursion
    enum {
      //! The number of block levels we contain
      blocklevel = 1
    };

    //! export size
    enum {
      //! The size of this vector.
      size = SIZE
    };

    //! Constructor making uninitialized vector
    FieldVector() {}

#ifndef DUNE_EXPRESSIONTEMPLATES
    //! Constructor making vector with identical coordinates
    explicit FieldVector (const K& t)
    {
      for (size_type i=0; i<SIZE; i++) p[i] = t;
    }

    //===== assignment from scalar
    //! Assignment operator for scalar
    FieldVector& operator= (const K& k)
    {
      for (size_type i=0; i<SIZE; i++)
        p[i] = k;
      return *this;
    }

#else
    //! Constructor making vector with identical coordinates
    explicit FieldVector (const K& t)
    {
#ifdef DUNE_VVERBOSE
      Dune::dvverb << INDENT << "FieldVector Copy Constructor Scalar\n";
#endif
      assignFrom(t);
    }
    //! Assignment operator for scalar
    FieldVector& operator= (const K& k)
    {
#ifdef DUNE_VVERBOSE
      Dune::dvverb << INDENT << "FieldVector Assignment Operator Scalar\n";
#endif
      return assignFrom(k);
    }
    template <class E>
    FieldVector (Dune::ExprTmpl::Expression<E> op) {
#ifdef DUNE_VVERBOSE
      Dune::dvverb << INDENT << "FieldVector Copy Constructor Expression\n";
#endif
      assignFrom(op);
    }
    template <class V>
    FieldVector (const Dune::ExprTmpl::Vector<V> & op) {
#ifdef DUNE_VVERBOSE
      Dune::dvverb << INDENT << "FieldVector Copy Operator Vector\n";
#endif
      assignFrom(op);
    }
    template <class E>
    FieldVector&  operator = (Dune::ExprTmpl::Expression<E> op) {
#ifdef DUNE_VVERBOSE
      Dune::dvverb << INDENT << "FieldVector Assignment Operator Expression\n";
#endif
      return assignFrom(op);
    }
    template <class V>
    FieldVector& operator = (const Dune::ExprTmpl::Vector<V> & op) {
#ifdef DUNE_VVERBOSE
      Dune::dvverb << INDENT << "FieldVector Assignment Operator Vector\n";
#endif
      return assignFrom(op);
    }
#endif

    /** \brief copy constructor */
    FieldVector ( const FieldVector &other )
    {
      // for( size_type i = 0; i < SIZE; ++i )
      //   p[ i ] = other[ i ];
      std::memcpy(p,other.p,SIZE*sizeof(K));
    }

    /** \brief Assigment from other vector */
    FieldVector& operator= (const FieldVector& other) {
      std::memcpy(p,other.p,SIZE*sizeof(K));
      // for (size_type i=0; i<SIZE; i++)
      //     p[i] = other[i];
      return *this;
    }


    //===== access to components

    //! random access
    K& operator[] (size_type i)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i<0 || i>=SIZE) DUNE_THROW(MathError,"index out of range");
#endif
      return p[i];
    }

    //! same for read only access
    const K& operator[] (size_type i) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i<0 || i>=SIZE) DUNE_THROW(MathError,"index out of range");
#endif
      return p[i];
    }

    //! Iterator class for sequential access
    typedef FieldIterator<FieldVector<K,SIZE>,K> Iterator;
    //! typedef for stl compliant access
    typedef Iterator iterator;

    //! begin iterator
    Iterator begin ()
    {
      return Iterator(*this,0);
    }

    //! end iterator
    Iterator end ()
    {
      return Iterator(*this,SIZE);
    }

    //! begin iterator
    Iterator rbegin ()
    {
      return Iterator(*this,SIZE-1);
    }

    //! end iterator
    Iterator rend ()
    {
      return Iterator(*this,-1);
    }

    //! return iterator to given element or end()
    Iterator find (size_type i)
    {
      if (i<SIZE)
        return Iterator(*this,i);
      else
        return Iterator(*this,SIZE);
    }

    //! ConstIterator class for sequential access
    typedef FieldIterator<const FieldVector<K,SIZE>,const K> ConstIterator;
    //! typedef for stl compliant access
    typedef ConstIterator const_iterator;

    //! begin ConstIterator
    ConstIterator begin () const
    {
      return ConstIterator(*this,0);
    }

    //! end ConstIterator
    ConstIterator end () const
    {
      return ConstIterator(*this,SIZE);
    }

    //! begin ConstIterator
    ConstIterator rbegin () const
    {
      return ConstIterator(*this,SIZE-1);
    }

    //! end ConstIterator
    ConstIterator rend () const
    {
      return ConstIterator(*this,-1);
    }

    //! return iterator to given element or end()
    ConstIterator find (size_type i) const
    {
      if (i<SIZE)
        return ConstIterator(*this,i);
      else
        return ConstIterator(*this,SIZE);
    }

#ifndef DUNE_EXPRESSIONTEMPLATES
    //===== vector space arithmetic

    //! vector space addition
    FieldVector& operator+= (const FieldVector& y)
    {
      for (size_type i=0; i<SIZE; i++)
        p[i] += y.p[i];
      return *this;
    }

    //! vector space subtraction
    FieldVector& operator-= (const FieldVector& y)
    {
      for (size_type i=0; i<SIZE; i++)
        p[i] -= y.p[i];
      return *this;
    }

    //! Binary vector addition
    FieldVector<K, size> operator+ (const FieldVector<K, size>& b) const
    {
      FieldVector<K, size> z = *this;
      return (z+=b);
    }

    //! Binary vector subtraction
    FieldVector<K, size> operator- (const FieldVector<K, size>& b) const
    {
      FieldVector<K, size> z = *this;
      return (z-=b);
    }

    //! vector space add scalar to all comps
    FieldVector& operator+= (const K& k)
    {
      for (size_type i=0; i<SIZE; i++)
        p[i] += k;
      return *this;
    }

    //! vector space subtract scalar from all comps
    FieldVector& operator-= (const K& k)
    {
      for (size_type i=0; i<SIZE; i++)
        p[i] -= k;
      return *this;
    }

    //! vector space multiplication with scalar
    FieldVector& operator*= (const K& k)
    {
      for (size_type i=0; i<SIZE; i++)
        p[i] *= k;
      return *this;
    }

    //! vector space division by scalar
    FieldVector& operator/= (const K& k)
    {
      for (size_type i=0; i<SIZE; i++)
        p[i] /= k;
      return *this;
    }

#endif

    //! Binary vector comparison
    bool operator== (const FieldVector& y) const
    {
      for (size_type i=0; i<SIZE; i++)
        if (p[i]!=y.p[i])
          return false;

      return true;
    }

    //! Binary vector incomparison
    bool operator!= (const FieldVector& y) const
    {
      return !operator==(y);
    }


    //! vector space axpy operation ( *this += a y )
    FieldVector& axpy (const K& a, const FieldVector& y)
    {
#ifndef DUNE_EXPRESSIONTEMPLATES
      for (size_type i=0; i<SIZE; i++)
        p[i] += a*y.p[i];
#else
      *this += a*y;
#endif
      return *this;
    }

#ifndef DUNE_EXPRESSIONTEMPLATES
    //===== Euclidean scalar product

    //! scalar product (x^T y)
    K operator* (const FieldVector& y) const
    {
      K result = 0;
      for (int i=0; i<size; i++)
        result += p[i]*y[i];
      return result;
    }

    //===== norms

    //! one norm (sum over absolute values of entries)
    typename FieldTraits<K>::real_type one_norm() const {
      typename FieldTraits<K>::real_type result = 0;
      for (int i=0; i<size; i++)
        result += std::abs(p[i]);
      return result;
    }


    //! simplified one norm (uses Manhattan norm for complex values)
    typename FieldTraits<K>::real_type one_norm_real () const
    {
      typename FieldTraits<K>::real_type result = 0;
      for (int i=0; i<size; i++)
        result += fvmeta::absreal(p[i]);
      return result;
    }

    //! two norm sqrt(sum over squared values of entries)
    typename FieldTraits<K>::real_type two_norm () const
    {
      typename FieldTraits<K>::real_type result = 0;
      for (int i=0; i<size; i++)
        result += fvmeta::abs2(p[i]);
      return fvmeta::sqrt(result);
    }

    //! square of two norm (sum over squared values of entries), need for block recursion
    typename FieldTraits<K>::real_type two_norm2 () const
    {
      typename FieldTraits<K>::real_type result = 0;
      for (int i=0; i<size; i++)
        result += fvmeta::abs2(p[i]);
      return result;
    }

    //! infinity norm (maximum of absolute values of entries)
    typename FieldTraits<K>::real_type infinity_norm () const
    {
      typename FieldTraits<K>::real_type result = 0;
      for (int i=0; i<size; i++)
        result = std::max(result, std::abs(p[i]));
      return result;
    }

    //! simplified infinity norm (uses Manhattan norm for complex values)
    typename FieldTraits<K>::real_type infinity_norm_real () const
    {
      typename FieldTraits<K>::real_type result = 0;
      for (int i=0; i<size; i++)
        result = std::max(result, fvmeta::absreal(p[i]));
      return result;
    }
#endif

    //===== sizes

    //! number of blocks in the vector (are of size 1 here)
    size_type N () const
    {
      return SIZE;
    }

    //! dimension of the vector space
    size_type dim () const
    {
      return SIZE;
    }

  private:
    // the data, very simply a built in array
    K p[(SIZE > 0) ? SIZE : 1];
  };

  /** \brief Write a FieldVector to an output stream
   *  \relates FieldVector
   *
   *  \param[in]  s  std :: ostream to write to
   *  \param[in]  v  FieldVector to write
   *
   *  \returns the output stream (s)
   */
  template<typename K, int n>
  std::ostream& operator<< (std::ostream& s, const FieldVector<K,n>& v)
  {
    for (typename FieldVector<K,n>::size_type i=0; i<n; i++)
      s << ((i>0) ? " " : "") << v[i];
    return s;
  }

  /** \brief Read a FieldVector from an input stream
   *  \relates FieldVector
   *
   *  \note This operator is STL compilant, i.e., the content of v is only
   *        changed if the read operation is successful.
   *
   *  \param[in]  in  std :: istream to read from
   *  \param[out] v   FieldVector to be read
   *
   *  \returns the input stream (in)
   */
  template< class K, int SIZE >
  inline std :: istream &operator>> ( std :: istream &in,
                                      FieldVector< K, SIZE > &v )
  {
    FieldVector< K, SIZE > w;
    for( typename FieldVector< K, SIZE > :: size_type i = 0; i < SIZE; ++i )
      in >> w[ i ];
    if( in )
      v = w;
    return in;
  }


  // forward declarations
  template<class K, int n, int m> class FieldMatrix;


#ifndef DOXYGEN
  /** \brief Vectors containing only one component
   */
  template< class K >
  class FieldVector< K, 1 >
#ifdef DUNE_EXPRESSIONTEMPLATES
    : public Dune :: ExprTmpl :: Vector< FieldVector< K, 1 > >
#endif
  {
    enum { n = 1 };
  public:
    friend class FieldMatrix<K,1,1>;

    //===== type definitions and constants

    //! export the type representing the field
    typedef K field_type;

    //! export the type representing the components
    typedef K block_type;

    //! The type for the index access and size operations.
    typedef std::size_t size_type;

    //! We are at the leaf of the block recursion
    enum {blocklevel = 1};

    //! export size
    enum {size = 1};

    //! export size
    enum {dimension = 1};

    //===== construction

    /** \brief Default constructor */
    FieldVector ()
    {       }

    /** \brief Constructor with a given scalar */
    FieldVector (const K& k)
    {
      p = k;
    }

    /** \brief Assignment from the base type */
    FieldVector& operator= (const K& k)
    {
      p = k;
      return *this;
    }

#ifdef DUNE_EXPRESSIONTEMPLATES
    template <class E>
    FieldVector (Dune::ExprTmpl::Expression<E> op) {
#ifdef DUNE_VVERBOSE
      Dune::dvverb << INDENT << "FieldVector<1> Copy Constructor Expression\n";
#endif
      assignFrom(op);
    }
    template <class V>
    FieldVector (const Dune::ExprTmpl::Vector<V> & op) {
#ifdef DUNE_VVERBOSE
      Dune::dvverb << INDENT << "FieldVector<1> Copy Operator Vector\n";
#endif
      assignFrom(op);
    }
    template <class E>
    FieldVector&  operator = (Dune::ExprTmpl::Expression<E> op) {
#ifdef DUNE_VVERBOSE
      Dune::dvverb << INDENT << "FieldVector<1> Assignment Operator Expression\n";
#endif
      return assignFrom(op);
    }
    template <class V>
    FieldVector& operator = (const Dune::ExprTmpl::Vector<V> & op) {
#ifdef DUNE_VVERBOSE
      Dune::dvverb << INDENT << "FieldVector<1> Assignment Operator Vector\n";
#endif
      return assignFrom(op);
    }
#endif

    //===== access to components

    //! random access
    K& operator[] (size_type i)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i != 0) DUNE_THROW(MathError,"index out of range");
#endif
      return p;
    }

    //! same for read only access
    const K& operator[] (size_type i) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i != 0) DUNE_THROW(MathError,"index out of range");
#endif
      return p;
    }

    //! Iterator class for sequential access
    typedef FieldIterator<FieldVector<K,n>,K> Iterator;
    //! typedef for stl compliant access
    typedef Iterator iterator;

    //! begin iterator
    Iterator begin ()
    {
      return Iterator(*this,0);
    }

    //! end iterator
    Iterator end ()
    {
      return Iterator(*this,n);
    }

    //! begin iterator
    Iterator rbegin ()
    {
      return Iterator(*this,n-1);
    }

    //! end iterator
    Iterator rend ()
    {
      return Iterator(*this,-1);
    }

    //! return iterator to given element or end()
    Iterator find (size_type i)
    {
      if (i<n)
        return Iterator(*this,i);
      else
        return Iterator(*this,n);
    }

    //! ConstIterator class for sequential access
    typedef FieldIterator<const FieldVector<K,n>,const K> ConstIterator;
    //! typedef for stl compliant access
    typedef ConstIterator const_iterator;

    //! begin ConstIterator
    ConstIterator begin () const
    {
      return ConstIterator(*this,0);
    }

    //! end ConstIterator
    ConstIterator end () const
    {
      return ConstIterator(*this,n);
    }

    //! begin ConstIterator
    ConstIterator rbegin () const
    {
      return ConstIterator(*this,n-1);
    }

    //! end ConstIterator
    ConstIterator rend () const
    {
      return ConstIterator(*this,-1);
    }

    //! return iterator to given element or end()
    ConstIterator find (size_type i) const
    {
      if (i<n)
        return ConstIterator(*this,i);
      else
        return ConstIterator(*this,n);
    }
    //===== vector space arithmetic

    //! vector space add scalar to each comp
    FieldVector& operator+= (const K& k)
    {
      p += k;
      return *this;
    }

    //! vector space subtract scalar from each comp
    FieldVector& operator-= (const K& k)
    {
      p -= k;
      return *this;
    }

    //! vector space multiplication with scalar
    FieldVector& operator*= (const K& k)
    {
      p *= k;
      return *this;
    }

    //! vector space division by scalar
    FieldVector& operator/= (const K& k)
    {
      p /= k;
      return *this;
    }

    //! vector space axpy operation ( *this += a y )
    FieldVector& axpy (const K& a, const FieldVector& y)
    {
      p += a*y.p;
      return *this;
    }

    //! scalar product (x^T y)
    inline K operator* ( const K & k ) const
    {
      return p * k;
    }

    //===== norms

    //! one norm (sum over absolute values of entries)
    typename FieldTraits<K>::real_type one_norm () const
    {
      return std::abs(p);
    }

    //! simplified one norm (uses Manhattan norm for complex values)
    typename FieldTraits<K>::real_type one_norm_real () const
    {
      return fvmeta::absreal(p);
    }

    //! two norm sqrt(sum over squared values of entries)
    typename FieldTraits<K>::real_type two_norm () const
    {
      return fvmeta::sqrt(fvmeta::abs2(p));
    }

    //! square of two norm (sum over squared values of entries), need for block recursion
    typename FieldTraits<K>::real_type two_norm2 () const
    {
      return fvmeta::abs2(p);
    }

    //! infinity norm (maximum of absolute values of entries)
    typename FieldTraits<K>::field_type infinity_norm () const
    {
      return std::abs(p);
    }

    //! simplified infinity norm (uses Manhattan norm for complex values)
    typename FieldTraits<K>::real_type infinity_norm_real () const
    {
      return fvmeta::absreal(p);
    }

    //===== sizes

    //! number of blocks in the vector (are of size 1 here)
    size_type N () const
    {
      return 1;
    }

    //! dimension of the vector space (==1)
    size_type dim () const
    {
      return 1;
    }

    //===== conversion operator

    /** \brief Conversion operator */
    operator K () {return p;}

    /** \brief Const conversion operator */
    operator K () const {return p;}

  private:
    // the data
    K p;
  };

#ifndef DUNE_EXPRESSIONTEMPLATES
  //! Binary vector addition
  template<class K>
  inline FieldVector<K,1> operator+ (const FieldVector<K,1>& a, const FieldVector<K,1>& b)
  {
    FieldVector<K,1> z = a;
    return (z+=b);
  }

  //! Binary vector subtraction
  template<class K>
  inline FieldVector<K,1> operator- (const FieldVector<K,1>& a, const FieldVector<K,1>& b)
  {
    FieldVector<K,1> z = a;
    return (z-=b);
  }

  //! Binary addition, when using FieldVector<K,1> like K
  template<class K>
  inline FieldVector<K,1> operator+ (const FieldVector<K,1>& a, const K b)
  {
    FieldVector<K,1> z = a;
    return (z[0]+=b);
  }

  //! Binary subtraction, when using FieldVector<K,1> like K
  template<class K>
  inline FieldVector<K,1> operator- (const FieldVector<K,1>& a, const K b)
  {
    FieldVector<K,1> z = a;
    return (z[0]-=b);
  }

  //! Binary addition, when using FieldVector<K,1> like K
  template<class K>
  inline FieldVector<K,1> operator+ (const K a, const FieldVector<K,1>& b)
  {
    FieldVector<K,1> z = a;
    return (z[0]+=b);
  }

  //! Binary subtraction, when using FieldVector<K,1> like K
  template<class K>
  inline FieldVector<K,1> operator- (const K a, const FieldVector<K,1>& b)
  {
    FieldVector<K,1> z = a;
    return (z[0]-=b);
  }
#endif
#endif

  /** @} end documentation */

} // end namespace

#endif
