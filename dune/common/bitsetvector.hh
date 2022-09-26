// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_BLOCK_BITFIELD_HH
#define DUNE_BLOCK_BITFIELD_HH

/** \file
    \brief Efficient implementation of a dynamic array of static arrays of booleans
 */

#include <vector>
#include <bitset>
#include <iostream>
#include <algorithm>

#include <dune/common/boundschecking.hh>
#include <dune/common/genericiterator.hh>
#include <dune/common/exceptions.hh>

namespace Dune {

  template <int block_size, class Alloc> class BitSetVector;
  template <int block_size, class Alloc> class BitSetVectorReference;

  /**
     \brief A proxy class that acts as a const reference to a single
     bitset in a BitSetVector.

     It contains a conversion to std::bitset and most of the
     interface of const std::bitset.

     \warning As this is only a proxy class, you can not get the
     address of the bitset.
   */
  template <int block_size, class Alloc>
  class BitSetVectorConstReference
  {
  protected:

    typedef Dune::BitSetVector<block_size, Alloc> BitSetVector;
    friend class Dune::BitSetVector<block_size, Alloc>;

    BitSetVectorConstReference(const BitSetVector& blockBitField_, int block_number_) :
      blockBitField(blockBitField_),
      block_number(block_number_)
    {
      DUNE_ASSERT_BOUNDS(blockBitField_.size() > static_cast<size_t>(block_number_));
    }

    //! hide assignment operator
    BitSetVectorConstReference& operator=(const BitSetVectorConstReference & b);

  public:

    typedef std::bitset<block_size> bitset;

    // bitset interface typedefs
    typedef typename std::vector<bool, Alloc>::const_reference reference;
    typedef typename std::vector<bool, Alloc>::const_reference const_reference;
    typedef size_t size_type;

    //! Returns a copy of *this shifted left by n bits.
    bitset operator<<(size_type n) const
    {
      bitset b = *this;
      b <<= n;
      return b;
    }

    //! Returns a copy of *this shifted right by n bits.
    bitset operator>>(size_type n) const
    {
      bitset b = *this;
      b >>= n;
      return b;
    }

    //! Returns a copy of *this with all of its bits flipped.
    bitset operator~() const
    {
      bitset b = *this;
      b.flip();
      return b;
    }

    //! Returns block_size.
    size_type size() const
    {
      return block_size;
    }

    //! Returns the number of bits that are set.
    size_type count() const
    {
      size_type n = 0;
      for(size_type i=0; i<block_size; ++i)
        n += getBit(i);
      return n;
    }

    //! Returns true if any bits are set.
    bool any() const
    {
      return count();
    }

    //! Returns true if no bits are set.
    bool none() const
    {
      return ! any();
    }

    //! Returns true if all bits are set
    bool all() const
    {
      for(size_type i=0; i<block_size; ++i)
        if(not test(i))
          return false;
      return true;
    }

    //! Returns true if bit n is set.
    bool test(size_type n) const
    {
      return getBit(n);
    }

    //! Return reference to the `i`-th bit
    const_reference operator[](size_type i) const
    {
      return getBit(i);
    }

    //! cast to bitset
    operator bitset() const
    {
      return blockBitField.getRepr(block_number);
    }

    //! Equality of reference and std::bitset
    bool operator== (const bitset& bs) const
    {
      return equals(bs);
    }

    //! Equality of reference and other reference
    bool operator== (const BitSetVectorConstReference& bs) const
    {
      return equals(bs);
    }

    //! Inequality of reference and std::bitset
    bool operator!= (const bitset& bs) const
    {
      return ! equals(bs);
    }

    //! Inequality of reference and other reference
    bool operator!= (const BitSetVectorConstReference& bs) const
    {
      return ! equals(bs);
    }

    /*!
       missing operators:

       - unsigned long to_ulong() const
     */

    friend std::ostream& operator<< (std::ostream& s, const BitSetVectorConstReference& v)
    {
      s << "(";
      for(int i=0; i<block_size; ++i)
        s << v[i];
      s << ")";
      return s;
    }

  protected:
    const BitSetVector& blockBitField;
    int block_number;

    const_reference getBit(size_type i) const
    {
      return blockBitField.getBit(block_number,i);
    }

    template<class BS>
    bool equals(const BS & bs) const
    {
      bool eq = true;
      for(int i=0; i<block_size; ++i)
        eq &= (getBit(i) == bs[i]);
      return eq;
    }

  private:
    /**
       This is only a Proxy class, you can't get the address of the
       object it references
     */
    void operator & () = delete;

    friend class BitSetVectorReference<block_size, Alloc>;
  };

  /**
     \brief A proxy class that acts as a mutable reference to a
     single bitset in a BitSetVector.

     It contains an assignment operator from std::bitset.  It
     inherits the const std::bitset interface provided by
     BitSetVectorConstReference and adds most of the non-const
     methods of std::bitset.

     \warning As this is only a proxy class, you can not get the
     address of the bitset.
   */
  template <int block_size, class Alloc>
  class BitSetVectorReference : public BitSetVectorConstReference<block_size,Alloc>
  {
  protected:

    typedef Dune::BitSetVector<block_size, Alloc> BitSetVector;
    friend class Dune::BitSetVector<block_size, Alloc>;

    typedef Dune::BitSetVectorConstReference<block_size,Alloc> BitSetVectorConstReference;

    BitSetVectorReference(BitSetVector& blockBitField_, int block_number_) :
      BitSetVectorConstReference(blockBitField_, block_number_),
      blockBitField(blockBitField_)
    {}

  public:
    typedef std::bitset<block_size> bitset;

    //! bitset interface typedefs
    //! \{
    //! A proxy class that acts as a reference to a single bit.
    typedef typename std::vector<bool, Alloc>::reference reference;
    //! A proxy class that acts as a const reference to a single bit.
    typedef typename std::vector<bool, Alloc>::const_reference const_reference;
    //! \}

    //! size_type typedef (an unsigned integral type)
    typedef size_t size_type;

    //! Assignment from bool, sets each bit in the bitset to b
    BitSetVectorReference& operator=(bool b)
    {
      for(int i=0; i<block_size; ++i)
        getBit(i) = b;
      return (*this);
    }

    //! Assignment from bitset
    BitSetVectorReference& operator=(const bitset & b)
    {
      for(int i=0; i<block_size; ++i)
        getBit(i) = b.test(i);
      return (*this);
    }

    //! Assignment from BitSetVectorConstReference
    BitSetVectorReference& operator=(const BitSetVectorConstReference & b)
    {
      for(int i=0; i<block_size; ++i)
        getBit(i) = b.test(i);
      return (*this);
    }

    //! Assignment from BitSetVectorReference
    BitSetVectorReference& operator=(const BitSetVectorReference & b)
    {
      for(int i=0; i<block_size; ++i)
        getBit(i) = b.test(i);
      return (*this);
    }

    //! Bitwise and (for bitset).
    BitSetVectorReference& operator&=(const bitset& x)
    {
      for (size_type i=0; i<block_size; i++)
        getBit(i) = (test(i) & x.test(i));
      return *this;
    }

    //! Bitwise and (for BitSetVectorConstReference and BitSetVectorReference)
    BitSetVectorReference& operator&=(const BitSetVectorConstReference& x)
    {
      for (size_type i=0; i<block_size; i++)
        getBit(i) = (test(i) & x.test(i));
      return *this;
    }

    //! Bitwise inclusive or (for bitset)
    BitSetVectorReference& operator|=(const bitset& x)
    {
      for (size_type i=0; i<block_size; i++)
        getBit(i) = (test(i) | x.test(i));
      return *this;
    }

    //! Bitwise inclusive or (for BitSetVectorConstReference and BitSetVectorReference)
    BitSetVectorReference& operator|=(const BitSetVectorConstReference& x)
    {
      for (size_type i=0; i<block_size; i++)
        getBit(i) = (test(i) | x.test(i));
      return *this;
    }

    //! Bitwise exclusive or (for bitset).
    BitSetVectorReference& operator^=(const bitset& x)
    {
      for (size_type i=0; i<block_size; i++)
        getBit(i) = (test(i) ^ x.test(i));
      return *this;
    }

  private:

    // For some reason, the following variant of operator^= triggers an ICE or a hanging
    // compiler on Debian 9 with GCC 6.3 and full optimizations enabled (-O3).
    // The only way to reliably avoid the issue is by making sure that the compiler does not
    // see the XOR in the context of the function, so here is a little helper that will normally
    // be inlined, but not on the broken compiler. This incurs a substantial overhead (a function
    // call), but until someone else has a better idea, it's the only way to make it work reliably.

    static bool xor_helper(bool a, bool b)
#if defined(__GNUC__) && ! defined(__clang__) && __GNUC__ == 6 && __GNUC_MINOR__ == 3 && __cplusplus \
  == 201402L
      __attribute__((noinline))
#endif
      ;

  public:

    //! Bitwise exclusive or (for BitSetVectorConstReference and BitSetVectorReference)
    BitSetVectorReference& operator^=(const BitSetVectorConstReference& x)
    {
      // This uses the helper from above to hoist the actual XOR computation out of the function for
      // the buggy version of GCC.
      for (size_type i=0; i<block_size; i++)
        getBit(i) = xor_helper(test(i),x.test(i));
      return *this;
    }

    //! Left shift.
    BitSetVectorReference& operator<<=(size_type n)
    {
      for (size_type i=0; i<block_size-n; i++)
        getBit(i) = test(i+n);
      return *this;
    }

    //! Right shift.
    BitSetVectorReference& operator>>=(size_type n)
    {
      for (size_type i=0; i<block_size-n; i++)
        getBit(i+n) = test(i);
      return *this;
    }

    //! Sets every bit.
    BitSetVectorReference& set()
    {
      for (size_type i=0; i<block_size; i++)
        set(i);
      return *this;
    }

    //! Flips the value of every bit.
    BitSetVectorReference& flip()
    {
      for (size_type i=0; i<block_size; i++)
        flip(i);
      return *this;
    }

    //! Clears every bit.
    BitSetVectorReference& reset()
    {
      *this = false;
      return *this;
    }

    //! Sets bit n if val is nonzero, and clears bit n if val is zero.
    BitSetVectorReference& set(size_type n, int val = 1)
    {
      getBit(n) = val;
      return *this;
    }

    //! Clears bit n.
    BitSetVectorReference& reset(size_type n)
    {
      set(n, false);
      return *this;
    }

    //! Flips bit n.
    BitSetVectorReference& flip(size_type n)
    {
      getBit(n).flip();
      return *this;
    }

    using BitSetVectorConstReference::test;
    using BitSetVectorConstReference::operator[];

    //! Return reference to the `i`-th bit
    reference operator[](size_type i)
    {
      return getBit(i);
    }

  protected:
    BitSetVector& blockBitField;

    using BitSetVectorConstReference::getBit;

    reference getBit(size_type i)
    {
      return blockBitField.getBit(this->block_number,i);
    }
  };

  // implementation of helper - I put it into the template to avoid having
  // to compile it in a dedicated compilation unit
  template<int block_size, class Alloc>
  bool BitSetVectorReference<block_size,Alloc>::xor_helper(bool a, bool b)
  {
    return a ^ b;
  }

  /**
     typetraits for BitSetVectorReference
   */
  template<int block_size, class Alloc>
  struct const_reference< BitSetVectorReference<block_size,Alloc> >
  {
    typedef BitSetVectorConstReference<block_size,Alloc> type;
  };

  template<int block_size, class Alloc>
  struct const_reference< BitSetVectorConstReference<block_size,Alloc> >
  {
    typedef BitSetVectorConstReference<block_size,Alloc> type;
  };

  template<int block_size, class Alloc>
  struct mutable_reference< BitSetVectorReference<block_size,Alloc> >
  {
    typedef BitSetVectorReference<block_size,Alloc> type;
  };

  template<int block_size, class Alloc>
  struct mutable_reference< BitSetVectorConstReference<block_size,Alloc> >
  {
    typedef BitSetVectorReference<block_size,Alloc> type;
  };

  /**
     \brief A dynamic %array of blocks of booleans
   */
  template <int block_size, class Allocator=std::allocator<bool> >
  class BitSetVector : private std::vector<bool, Allocator>
  {
    /** \brief The implementation class: an unblocked bitfield */
    typedef std::vector<bool, Allocator> BlocklessBaseClass;

  public:
    //! container interface typedefs
    //! \{

    /** \brief Type of the values stored by the container */
    typedef std::bitset<block_size> value_type;

    /** \brief Reference to a small block of bits */
    typedef BitSetVectorReference<block_size,Allocator> reference;

    /** \brief Const reference to a small block of bits */
    typedef BitSetVectorConstReference<block_size,Allocator> const_reference;

    /** \brief Pointer to a small block of bits */
    typedef BitSetVectorReference<block_size,Allocator>* pointer;

    /** \brief Const pointer to a small block of bits */
    typedef BitSetVectorConstReference<block_size,Allocator>* const_pointer;

    /** \brief size type */
    typedef typename std::vector<bool, Allocator>::size_type size_type;

    /** \brief The type of the allocator */
    typedef Allocator allocator_type;
    //! \}

    //! iterators
    //! \{
    typedef Dune::GenericIterator<BitSetVector<block_size,Allocator>, value_type, reference, std::ptrdiff_t, ForwardIteratorFacade> iterator;
    typedef Dune::GenericIterator<const BitSetVector<block_size,Allocator>, const value_type, const_reference, std::ptrdiff_t, ForwardIteratorFacade> const_iterator;
    //! \}

    //! Returns a iterator pointing to the beginning of the vector.
    iterator begin(){
      return iterator(*this, 0);
    }

    //! Returns a const_iterator pointing to the beginning of the vector.
    const_iterator begin() const {
      return const_iterator(*this, 0);
    }

    //! Returns an iterator pointing to the end of the vector.
    iterator end(){
      return iterator(*this, size());
    }

    //! Returns a const_iterator pointing to the end of the vector.
    const_iterator end() const {
      return const_iterator(*this, size());
    }

    //! Default constructor
    BitSetVector() :
      BlocklessBaseClass()
    {}

    //! Construction from an unblocked bitfield
    BitSetVector(const BlocklessBaseClass& blocklessBitField) :
      BlocklessBaseClass(blocklessBitField)
    {
      if (blocklessBitField.size()%block_size != 0)
        DUNE_THROW(RangeError, "Vector size is not a multiple of the block size!");
    }

    /** Constructor with a given length
        \param n Number of blocks
     */
    explicit BitSetVector(int n) :
      BlocklessBaseClass(n*block_size)
    {}

    //! Constructor which initializes the field with true or false
    BitSetVector(int n, bool v) :
      BlocklessBaseClass(n*block_size,v)
    {}

    //! Erases all of the elements.
    void clear()
    {
      BlocklessBaseClass::clear();
    }

    //! Resize field
    void resize(int n, bool v = bool())
    {
      BlocklessBaseClass::resize(n*block_size, v);
    }

    /** \brief Return the number of blocks */
    size_type size() const
    {
      return BlocklessBaseClass::size()/block_size;
    }

    //! Sets all entries to <tt> true </tt>
    void setAll() {
      this->assign(BlocklessBaseClass::size(), true);
    }

    //! Sets all entries to <tt> false </tt>
    void unsetAll() {
      this->assign(BlocklessBaseClass::size(), false);
    }

    /** \brief Return reference to i-th block */
    reference operator[](int i)
    {
      return reference(*this, i);
    }

    /** \brief Return const reference to i-th block */
    const_reference operator[](int i) const
    {
      return const_reference(*this, i);
    }

    /** \brief Return reference to last block */
    reference back()
    {
      return reference(*this, size()-1);
    }

    /** \brief Return const reference to last block */
    const_reference back() const
    {
      return const_reference(*this, size()-1);
    }

    //! Returns the number of bits that are set.
    size_type count() const
    {
      return std::count(BlocklessBaseClass::begin(), BlocklessBaseClass::end(), true);
    }

    //! Returns the number of set bits, while each block is masked with 1<<i
    size_type countmasked(int j) const
    {
      size_type n = 0;
      size_type blocks = size();
      for(size_type i=0; i<blocks; ++i)
        n += getBit(i,j);
      return n;
    }

    //! Send bitfield to an output stream
    friend std::ostream& operator<< (std::ostream& s, const BitSetVector& v)
    {
      for (size_t i=0; i<v.size(); i++)
        s << v[i] << "  ";
      return s;
    }

  private:

    //! Get a representation as value_type
    value_type getRepr(int i) const
    {
      value_type bits;
      for(int j=0; j<block_size; ++j)
        bits.set(j, getBit(i,j));
      return bits;
    }

    typename std::vector<bool>::reference getBit(size_type i, size_type j) {
      DUNE_ASSERT_BOUNDS(j < block_size);
      DUNE_ASSERT_BOUNDS(i < size());
      return BlocklessBaseClass::operator[](i*block_size+j);
    }

    typename std::vector<bool>::const_reference getBit(size_type i, size_type j) const {
      DUNE_ASSERT_BOUNDS(j < block_size);
      DUNE_ASSERT_BOUNDS(i < size());
      return BlocklessBaseClass::operator[](i*block_size+j);
    }

    friend class BitSetVectorReference<block_size,Allocator>;
    friend class BitSetVectorConstReference<block_size,Allocator>;
  };

}  // namespace Dune

#endif
