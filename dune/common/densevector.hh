// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DENSEVECTOR_HH
#define DUNE_DENSEVECTOR_HH

#include <limits>

#include "genericiterator.hh"
#include "ftraits.hh"
#include "matvectraits.hh"

namespace Dune {

  // forward declaration of template
  template<typename V> class DenseVector;

  template<typename V>
  struct FieldTraits< DenseVector<V> >
  {
    typedef typename FieldTraits< typename DenseMatVecTraits<V>::value_type >::field_type field_type;
    typedef typename FieldTraits< typename DenseMatVecTraits<V>::value_type >::real_type real_type;
  };

  /** @defgroup DenseMatVec Dense Matrix and Vector Template Library
      @ingroup Common
      @{
   */

  /*! \file
   * \brief This file implements a the dense vector interface, with an exchangeable storage class
   */

  namespace fvmeta
  {
    /**
       \private
       \memberof Dune::DenseVector
     */
    template<class K>
    inline typename FieldTraits<K>::real_type absreal (const K& k)
    {
      return std::abs(k);
    }

    /**
       \private
       \memberof Dune::DenseVector
     */
    template<class K>
    inline typename FieldTraits<K>::real_type absreal (const std::complex<K>& c)
    {
      return std::abs(c.real()) + std::abs(c.imag());
    }

    /**
       \private
       \memberof Dune::DenseVector
     */
    template<class K>
    inline typename FieldTraits<K>::real_type abs2 (const K& k)
    {
      return k*k;
    }

    /**
       \private
       \memberof Dune::DenseVector
     */
    template<class K>
    inline typename FieldTraits<K>::real_type abs2 (const std::complex<K>& c)
    {
      return c.real()*c.real() + c.imag()*c.imag();
    }

    /**
       \private
       \memberof Dune::DenseVector
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
       \memberof Dune::DenseVector
     */
    template<class K>
    struct Sqrt<K, true>
    {
      static inline typename FieldTraits<K>::real_type sqrt (const K& k)
      {
        return typename FieldTraits<K>::real_type(std::sqrt(double(k)));
      }
    };

    /**
       \private
       \memberof Dune::DenseVector
     */
    template<class K>
    inline typename FieldTraits<K>::real_type sqrt (const K& k)
    {
      return Sqrt<K>::sqrt(k);
    }

  }

  /*! \brief Generic iterator class for dense vector and matrix implementations

     provides sequential access to DenseVector, FieldVector and FieldMatrix
   */
  template<class C, class T>
  class DenseIterator :
    public Dune::RandomAccessIteratorFacade<DenseIterator<C,T>,T, T&, std::ptrdiff_t>
  {
    friend class DenseIterator<typename remove_const<C>::type, typename remove_const<T>::type >;
    friend class DenseIterator<const typename remove_const<C>::type, const typename remove_const<T>::type >;

  public:

    /**
     * @brief The type of the difference between two positions.
     */
    typedef std::ptrdiff_t DifferenceType;

    /**
     * @brief The type to index the underlying container.
     */
    typedef typename C::size_type SizeType;

    // Constructors needed by the base iterators.
    DenseIterator()
      : container_(0), position_()
    {}

    DenseIterator(C& cont, SizeType pos)
      : container_(&cont), position_(pos)
    {}

    DenseIterator(const DenseIterator<typename remove_const<C>::type, typename remove_const<T>::type >& other)
      : container_(other.container_), position_(other.position_)
    {}

    // Methods needed by the forward iterator
    bool equals(const DenseIterator<typename remove_const<C>::type,typename remove_const<T>::type>& other) const
    {
      return position_ == other.position_ && container_ == other.container_;
    }


    bool equals(const DenseIterator<const typename remove_const<C>::type,const typename remove_const<T>::type>& other) const
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

    DifferenceType distanceTo(DenseIterator<const typename remove_const<C>::type,const typename remove_const<T>::type> other) const
    {
      assert(other.container_==container_);
      return other.position_ - position_;
    }

    DifferenceType distanceTo(DenseIterator<typename remove_const<C>::type, typename remove_const<T>::type> other) const
    {
      assert(other.container_==container_);
      return other.position_ - position_;
    }

    //! return index
    SizeType index () const
    {
      return this->position_;
    }

  private:
    C *container_;
    SizeType position_;
  };

  /** \brief Interface for a class of dense vectors over a given field.
   *
   * \tparam V implementation class of the vector
   * \tparam T value type
   * \tparam S size type
   *
   * V has to provide the following members:
   * @code
   * T &       _access (size_type);
   * const T & _access (size_type) const;
   * size_type _size   () const;
   * @endcode
   */
  template<typename V>
  class DenseVector
  {
    typedef DenseMatVecTraits<V> Traits;
    // typedef typename Traits::value_type K;

    // Curiously recuring template pattern
    V & asImp() { return static_cast<V&>(*this); }
    const V & asImp() const { return static_cast<const V&>(*this); }

  public:
    //===== type definitions and constants

    //! type of derived vector class
    typedef typename Traits::derived_type derived_type;

    //! export the type representing the field
    typedef typename Traits::value_type value_type;

    //! export the type representing the field
    typedef typename Traits::value_type field_type;

    //! export the type representing the components
    typedef typename Traits::value_type block_type;

    //! The type used for the index access and size operation
    typedef typename Traits::size_type size_type;

    //! We are at the leaf of the block recursion
    enum {
      //! The number of block levels we contain
      blocklevel = 1
    };

    //===== assignment from scalar
    //! Assignment operator for scalar
    derived_type& operator= (const value_type& k)
    {
      for (size_type i=0; i<size(); i++)
        (*this)[i] = k;
      return asImp();
    }

    //===== access to components

    //! random access
    value_type & operator[] (size_type i)
    {
      return asImp().vec_access(i);
    }

    const value_type & operator[] (size_type i) const
    {
      return asImp().vec_access(i);
    }

    //! size method
    size_type size() const
    {
      return asImp().vec_size();
    }

    //! Iterator class for sequential access
    typedef DenseIterator<DenseVector,value_type> Iterator;
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
      return Iterator(*this,size());
    }

    //! begin iterator
    Iterator rbegin ()
    {
      return Iterator(*this,size()-1);
    }

    //! end iterator
    Iterator rend ()
    {
      return Iterator(*this,-1);
    }

    //! return iterator to given element or end()
    Iterator find (size_type i)
    {
      return Iterator(*this,std::min(i,size()));
    }

    //! ConstIterator class for sequential access
    typedef DenseIterator<const DenseVector,const value_type> ConstIterator;
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
      return ConstIterator(*this,size());
    }

    //! begin ConstIterator
    ConstIterator rbegin () const
    {
      return ConstIterator(*this,size()-1);
    }

    //! end ConstIterator
    ConstIterator rend () const
    {
      return ConstIterator(*this,-1);
    }

    //! return iterator to given element or end()
    ConstIterator find (size_type i) const
    {
      return ConstIterator(*this,std::min(i,size()));
    }

    //===== vector space arithmetic

    //! vector space addition
    derived_type& operator+= (const DenseVector& y)
    {
      assert(y.size() == size());
      for (size_type i=0; i<size(); i++)
        (*this)[i] += y[i];
      return asImp();
    }

    //! vector space subtraction
    derived_type& operator-= (const DenseVector& y)
    {
      assert(y.size() == size());
      for (size_type i=0; i<size(); i++)
        (*this)[i] -= y[i];
      return asImp();
    }

    //! Binary vector addition
    derived_type operator+ (const DenseVector& b) const
    {
      derived_type z = asImp();
      return (z+=b);
    }

    //! Binary vector subtraction
    derived_type operator- (const DenseVector& b) const
    {
      derived_type z = asImp();
      return (z-=b);
    }

    //! vector space add scalar to all comps
    derived_type& operator+= (const value_type& k)
    {
      for (size_type i=0; i<size(); i++)
        (*this)[i] += k;
      return asImp();
    }

    //! vector space subtract scalar from all comps
    derived_type& operator-= (const value_type& k)
    {
      for (size_type i=0; i<size(); i++)
        (*this)[i] -= k;
      return asImp();
    }

    //! vector space multiplication with scalar
    derived_type& operator*= (const value_type& k)
    {
      for (size_type i=0; i<size(); i++)
        (*this)[i] *= k;
      return asImp();
    }

    //! vector space division by scalar
    derived_type& operator/= (const value_type& k)
    {
      for (size_type i=0; i<size(); i++)
        (*this)[i] /= k;
      return asImp();
    }

    //! Binary vector comparison
    bool operator== (const DenseVector& y) const
    {
      assert(y.size() == size());
      for (size_type i=0; i<size(); i++)
        if ((*this)[i]!=y[i])
          return false;

      return true;
    }

    //! Binary vector incomparison
    bool operator!= (const DenseVector& y) const
    {
      return !operator==(y);
    }


    //! vector space axpy operation ( *this += a y )
    derived_type& axpy (const value_type& a, const DenseVector& y)
    {
      assert(y.size() == size());
      for (size_type i=0; i<size(); i++)
        (*this)[i] += a*y[i];
      return asImp();
    }

    //===== Euclidean scalar product

    //! scalar product (x^T y)
    value_type operator* (const DenseVector& y) const
    {
      assert(y.size() == size());
      value_type result( 0 );
      for (size_type i=0; i<size(); i++)
        result += (*this)[i]*y[i];
      return result;
    }

    //===== norms

    //! one norm (sum over absolute values of entries)
    typename FieldTraits<value_type>::real_type one_norm() const {
      typename FieldTraits<value_type>::real_type result( 0 );
      for (size_type i=0; i<size(); i++)
        result += std::abs((*this)[i]);
      return result;
    }


    //! simplified one norm (uses Manhattan norm for complex values)
    typename FieldTraits<value_type>::real_type one_norm_real () const
    {
      typename FieldTraits<value_type>::real_type result( 0 );
      for (size_type i=0; i<size(); i++)
        result += fvmeta::absreal((*this)[i]);
      return result;
    }

    //! two norm sqrt(sum over squared values of entries)
    typename FieldTraits<value_type>::real_type two_norm () const
    {
      typename FieldTraits<value_type>::real_type result( 0 );
      for (size_type i=0; i<size(); i++)
        result += fvmeta::abs2((*this)[i]);
      return fvmeta::sqrt(result);
    }

    //! square of two norm (sum over squared values of entries), need for block recursion
    typename FieldTraits<value_type>::real_type two_norm2 () const
    {
      typename FieldTraits<value_type>::real_type result( 0 );
      for (size_type i=0; i<size(); i++)
        result += fvmeta::abs2((*this)[i]);
      return result;
    }

    //! infinity norm (maximum of absolute values of entries)
    typename FieldTraits<value_type>::real_type infinity_norm () const
    {
      typename FieldTraits<value_type>::real_type result( 0 );
      for (size_type i=0; i<size(); i++)
        result = std::max(result, std::abs((*this)[i]));
      return result;
    }

    //! simplified infinity norm (uses Manhattan norm for complex values)
    typename FieldTraits<value_type>::real_type infinity_norm_real () const
    {
      typename FieldTraits<value_type>::real_type result( 0 );
      for (size_type i=0; i<size(); i++)
        result = std::max(result, fvmeta::absreal((*this)[i]));
      return result;
    }

    //===== sizes

    //! number of blocks in the vector (are of size 1 here)
    size_type N () const
    {
      return size();
    }

    //! dimension of the vector space
    size_type dim () const
    {
      return size();
    }

  };

  /** \brief Write a DenseVector to an output stream
   *  \relates DenseVector
   *
   *  \param[in]  s  std :: ostream to write to
   *  \param[in]  v  DenseVector to write
   *
   *  \returns the output stream (s)
   */
  template<typename V>
  std::ostream& operator<< (std::ostream& s, const DenseVector<V>& v)
  {
    for (typename DenseVector<V>::size_type i=0; i<v.size(); i++)
      s << ((i>0) ? " " : "") << v[i];
    return s;
  }

  /** @} end documentation */

} // end namespace

#endif // DUNE_DENSEVECTOR_HH
