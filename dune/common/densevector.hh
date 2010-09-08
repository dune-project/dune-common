// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DENSEVECTOR_HH
#define DUNE_DENSEVECTOR_HH

#include <limits>

#include "genericiterator.hh"
#include "ftraits.hh"

namespace Dune {

  // forward declaration of template
  template<class S> class DenseVector;

  template<class S>
  struct FieldTraits< DenseVector<S> >
  {
    typedef const typename FieldTraits<typename S::value_type>::field_type field_type;
    typedef const typename FieldTraits<typename S::value_type>::real_type real_type;
  };

  /** @addtogroup DenseMatVec
      @{
   */

  /*! \todo doc me
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

  //! Iterator class for sequential access to DenseVector, FieldVector and FieldMatrix
  template<class C, class T>
  class DenseIterator :
    public Dune::RandomAccessIteratorFacade<DenseIterator<C,T>,T, T&, int>
  {
    friend class DenseIterator<typename remove_const<C>::type, typename remove_const<T>::type >;
    friend class DenseIterator<const typename remove_const<C>::type, const typename remove_const<T>::type >;

  public:

    /**
     * @brief The type of the difference between two positions.
     */
    typedef std::ptrdiff_t DifferenceType;

    // Constructors needed by the base iterators.
    DenseIterator()
      : container_(0), position_(0)
    {}

    DenseIterator(C& cont, DifferenceType pos)
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

    std::ptrdiff_t distanceTo(DenseIterator<const typename remove_const<C>::type,const typename remove_const<T>::type> other) const
    {
      assert(other.container_==container_);
      return other.position_ - position_;
    }

    std::ptrdiff_t distanceTo(DenseIterator<typename remove_const<C>::type, typename remove_const<T>::type> other) const
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

  /** \brief Construct a vector space out of a tensor product of fields.
   *
   *  K is the field type (use float, double, complex, etc) and SIZE
   *  is the number of components.
   *
   *  It is generally assumed that K is a numerical type compatible with double
   *  (E.g. norms are always computed in double precision).
   *
   * \tparam S storage class (e.g. std::array<K,Size> or std::vector<K>)
   */
  template<typename S>
  class DenseVector : S
  {
    typedef typename S::value_type K;

  public:
    //===== type definitions and constants

    //! export the type representing the field
    typedef typename S::value_type field_type;

    //! export the type representing the components
    typedef typename S::value_type block_type;

    //! The type used for the index access and size operation
    typedef typename S::size_type size_type;

    //! We are at the leaf of the block recursion
    enum {
      //! The number of block levels we contain
      blocklevel = 1
    };

    // pull in methods from the storage class

    //! random access
    using S::operator[];

    //! size method
    using S::size;

    //! Constructor making uninitialized vector
    DenseVector() {}

    //! Constructor making vector with identical values
    DenseVector (size_type n, const K& t) : S(n,t) {}

    //===== assignment from scalar
    //! Assignment operator for scalar
    DenseVector& operator= (const K& k)
    {
      for (size_type i=0; i<size(); i++)
        (*this)[i] = k;
      return *this;
    }

    //===== dynamic size related methods
    //! Resize the vector. (only possible if S support resizing)
    void resize(size_type size)
    {
      S::resize(size);
    }

    //! Get the capacity of the vector.
    size_type capacity() const
    {
      return S::capacity();
    }

    //===== access to components

    //! Iterator class for sequential access
    typedef DenseIterator<S,K> Iterator;
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
    typedef DenseIterator<const S,const K> ConstIterator;
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
      return Iterator(*this,std::min(i,size()));
    }

    //===== vector space arithmetic

    //! vector space addition
    DenseVector& operator+= (const DenseVector& y)
    {
      assert(y.size() == size());
      for (size_type i=0; i<size(); i++)
        (*this)[i] += y[i];
      return *this;
    }

    //! vector space subtraction
    DenseVector& operator-= (const DenseVector& y)
    {
      assert(y.size() == size());
      for (size_type i=0; i<size(); i++)
        (*this)[i] -= y[i];
      return *this;
    }

    //! Binary vector addition
    DenseVector operator+ (const DenseVector& b) const
    {
      DenseVector z = *this;
      return (z+=b);
    }

    //! Binary vector subtraction
    DenseVector operator- (const DenseVector& b) const
    {
      DenseVector z = *this;
      return (z-=b);
    }

    //! vector space add scalar to all comps
    DenseVector& operator+= (const K& k)
    {
      for (size_type i=0; i<size(); i++)
        (*this)[i] += k;
      return *this;
    }

    //! vector space subtract scalar from all comps
    DenseVector& operator-= (const K& k)
    {
      for (size_type i=0; i<size(); i++)
        (*this)[i] -= k;
      return *this;
    }

    //! vector space multiplication with scalar
    DenseVector& operator*= (const K& k)
    {
      for (size_type i=0; i<size(); i++)
        (*this)[i] *= k;
      return *this;
    }

    //! vector space division by scalar
    DenseVector& operator/= (const K& k)
    {
      for (size_type i=0; i<size(); i++)
        (*this)[i] /= k;
      return *this;
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
    DenseVector& axpy (const K& a, const DenseVector& y)
    {
      assert(y.size() == size());
      for (size_type i=0; i<size(); i++)
        (*this)[i] += a*y[i];
      return *this;
    }

    //===== Euclidean scalar product

    //! scalar product (x^T y)
    K operator* (const DenseVector& y) const
    {
      assert(y.size() == size());
      K result = 0;
      for (size_type i=0; i<size(); i++)
        result += (*this)[i]*y[i];
      return result;
    }

    //===== norms

    //! one norm (sum over absolute values of entries)
    typename FieldTraits<K>::real_type one_norm() const {
      typename FieldTraits<K>::real_type result = 0;
      for (size_type i=0; i<size(); i++)
        result += std::abs((*this)[i]);
      return result;
    }


    //! simplified one norm (uses Manhattan norm for complex values)
    typename FieldTraits<K>::real_type one_norm_real () const
    {
      typename FieldTraits<K>::real_type result = 0;
      for (size_type i=0; i<size(); i++)
        result += fvmeta::absreal((*this)[i]);
      return result;
    }

    //! two norm sqrt(sum over squared values of entries)
    typename FieldTraits<K>::real_type two_norm () const
    {
      typename FieldTraits<K>::real_type result = 0;
      for (size_type i=0; i<size(); i++)
        result += fvmeta::abs2((*this)[i]);
      return fvmeta::sqrt(result);
    }

    //! square of two norm (sum over squared values of entries), need for block recursion
    typename FieldTraits<K>::real_type two_norm2 () const
    {
      typename FieldTraits<K>::real_type result = 0;
      for (size_type i=0; i<size(); i++)
        result += fvmeta::abs2((*this)[i]);
      return result;
    }

    //! infinity norm (maximum of absolute values of entries)
    typename FieldTraits<K>::real_type infinity_norm () const
    {
      typename FieldTraits<K>::real_type result = 0;
      for (size_type i=0; i<size(); i++)
        result = std::max(result, std::abs((*this)[i]));
      return result;
    }

    //! simplified infinity norm (uses Manhattan norm for complex values)
    typename FieldTraits<K>::real_type infinity_norm_real () const
    {
      typename FieldTraits<K>::real_type result = 0;
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

  private:
  };

  /** \brief Write a DenseVector to an output stream
   *  \relates DenseVector
   *
   *  \param[in]  s  std :: ostream to write to
   *  \param[in]  v  DenseVector to write
   *
   *  \returns the output stream (s)
   */
  template<typename S>
  std::ostream& operator<< (std::ostream& s, const DenseVector<S>& v)
  {
    for (typename DenseVector<S>::size_type i=0; i<v.size(); i++)
      s << ((i>0) ? " " : "") << v[i];
    return s;
  }

  /** @} end documentation */

} // end namespace

#endif // DUNE_DENSEVECTOR_HH
