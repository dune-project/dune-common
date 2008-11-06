// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_BLOCK_BITFIELD_HH
#define DUNE_BLOCK_BITFIELD_HH

/** \file
    \brief Efficient implementation of a dynamic array of static arrays of booleans
 */

#include <vector>
#include <bitset>
#include <iostream>

#include <dune/common/genericiterator.hh>
#include <dune/common/exceptions.hh>

namespace Dune {

  template <int block_size, class Alloc> class BlockBitField;

  template <int block_size, class Alloc>
  class BlockBitFieldConstReference
  {
  protected:

    typedef Dune::BlockBitField<block_size, Alloc> BlockBitField;
    friend class Dune::BlockBitField<block_size, Alloc>;

    BlockBitFieldConstReference(const BlockBitField& blockBitField, int block_number) :
      blockBitField(blockBitField),
      block_number(block_number)
    {};

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
      return nSetBits();
    }

    //! Returns true if no bits are set.
    bool none() const
    {
      return ! any();
    }

    //! Returns true if bit n is set.
    bool test(size_type n) const
    {
      return getBit(n);
    }

    const_reference operator[](size_type i) const
    {
      return getBit(i);
    }

    //! Returns the total number of set bits
    size_type nSetBits() const
    {
      return count();
    }

    //! cast to bitset
    operator bitset() const
    {
      return blockBitField.getLocalBits(block_number);
    }

    /*!
       missing operators:

       - bool operator==(const bitset&) const
       - bool operator==(const BlockBitFieldReferenceBase&) const
       - bool operator!=(const bitset&) const
       - bool operator!=(const BlockBitFieldReferenceBase&) const
       - unsigned long to_ulong() const
     */

    friend std::ostream& operator<< (std::ostream& s, const BlockBitFieldConstReference& v)
    {
      s << "(";
      for(int i=0; i<block_size; ++i)
        s << v[i];
      s << ")";
      return s;
    }

  protected:
    const BlockBitField& blockBitField;
    int block_number;

    const_reference getBit(size_type i) const
    {
      return blockBitField.getBit(block_number,i);
    }

  };

  template <int block_size, class Alloc>
  class BlockBitFieldReference : public BlockBitFieldConstReference<block_size,Alloc>
  {
  protected:

    typedef Dune::BlockBitField<block_size, Alloc> BlockBitField;
    friend class Dune::BlockBitField<block_size, Alloc>;

    typedef Dune::BlockBitFieldConstReference<block_size,Alloc> BlockBitFieldConstReference;

    BlockBitFieldReference(BlockBitField& blockBitField, int block_number) :
      BlockBitFieldConstReference(blockBitField, block_number),
      blockBitField(blockBitField)
    {};

  public:
    typedef std::bitset<block_size> bitset;

    // bitset interface typedefs
    typedef typename std::vector<bool, Alloc>::reference reference;
    typedef typename std::vector<bool, Alloc>::const_reference const_reference;
    typedef size_t size_type;

    BlockBitFieldReference& operator=(bool b)
    {
      for(int i=0; i<block_size; ++i)
        getBit(i) = b;

      return (*this);
    }

    BlockBitFieldReference& operator=(const bitset & b)
    {
      for(int i=0; i<block_size; ++i)
        getBit(i) = b[i];

      return (*this);
    }

    BlockBitFieldReference& operator=(const BlockBitFieldConstReference & b)
    {
      for(int i=0; i<block_size; ++i)
        getBit(i) = b[i];

      return (*this);
    }

    //! Bitwise and.
    BlockBitFieldReference& operator&=(const BlockBitFieldConstReference& x)
    {
      for (size_type i=0; i<block_size; i++)
        set(i, getBit(i) & x.getBit(i));
      return *this;
    }

    //! Bitwise inclusive or.
    BlockBitFieldReference& operator|=(const BlockBitFieldConstReference& x)
    {
      for (size_type i=0; i<block_size; i++)
        set(i, getBit(i) | x.getBit(i));
      return *this;
    }

    //! Bitwise exclusive or.
    BlockBitFieldReference& operator^=(const BlockBitFieldConstReference& x)
    {
      for (size_type i=0; i<block_size; i++)
        set(i, getBit(i) ^ x.getBit(i));
      return *this;
    }

    //! Left shift.
    BlockBitFieldReference& operator<<=(size_type n)
    {
      for (size_type i=0; i<block_size-n; i++)
        set(i, getBit(i+n));
      return *this;
    }

    //! Right shift.
    BlockBitFieldReference& operator>>=(size_type n)
    {
      for (size_type i=0; i<block_size-n; i++)
        set(i+n, getBit(i));
      return *this;
    }

    // Sets every bit.
    BlockBitFieldReference& set()
    {
      for (size_type i=0; i<block_size; i++)
        set(i);
      return *this;
    }

    //! Flips the value of every bit.
    BlockBitFieldReference& flip()
    {
      for (size_type i=0; i<block_size; i++)
        flip(i);
      return *this;
    }

    //! Clears every bit.
    BlockBitFieldReference& reset()
    {}

    //! Sets bit n if val is nonzero, and clears bit n if val is zero.
    BlockBitFieldReference& set(size_type n, int val = 1)
    {
      getBit(n) = val;
      return *this;
    }

    //! Clears bit n.
    BlockBitFieldReference& reset(size_type n)
    {
      set(n, false);
      return *this;
    }

    //! Flips bit n.
    BlockBitFieldReference& flip(size_type n)
    {
      bool val = ! getBit(n);
      set(n, val);
      return *this;
    }

    reference operator[](size_type i)
    {
      return getBit(i);
    }

  protected:
    BlockBitField& blockBitField;

    reference getBit(size_type i)
    {
      return blockBitField.getBit(this->block_number,i);
    }
  };

  /**
     \brief A dynamic array of blocks of booleans
   */
  template <int block_size, class Allocator=std::allocator<bool> >
  class BlockBitField : private std::vector<bool, Allocator>
  {
    /** \brief The implementation class: an unblocked bitfield */
    typedef std::vector<bool, Allocator> BlocklessBaseClass;

  public:
    //! container interface typedefs
    //! {
    typedef std::bitset<block_size> value_type;
    typedef BlockBitFieldReference<block_size,Allocator> reference;
    typedef BlockBitFieldConstReference<block_size,Allocator> const_reference;
    typedef typename std::vector<bool, Allocator>::size_type size_type;
    //! }

    //! iterators
    //! {
    typedef Dune::GenericIterator<BlockBitField<block_size,Allocator>, value_type, reference> iterator;
    typedef Dune::GenericIterator<const BlockBitField<block_size,Allocator>, const value_type, const reference> const_iterator;
    //! }

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
      return iterator(*this, 100);
    }

    //! Returns a const_iterator pointing to the end of the vector.
    const_iterator end() const {
      return const_iterator(*this, 100);
    }

    //! Default constructor
    BlockBitField() :
      BlocklessBaseClass()
    {}

    //! Construction from an unblocked bitfield
    BlockBitField(const BlocklessBaseClass& blocklessBitField) :
      BlocklessBaseClass(blocklessBitField)
    {
      if (blocklessBitField.size()%block_size != 0)
        DUNE_THROW(RangeError, "Vector size is not a multiple of the block size!");
    }

    /** Constructor with a given length
        \param n Number of blocks
     */
    explicit BlockBitField(int n) :
      BlocklessBaseClass(n*block_size)
    {}

    //! Constructor which initializes the field with true or false
    BlockBitField(int n, bool v) :
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
    int size() const
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

    //! Returns the total number of set bits
    size_type nSetBits() const
    {
      size_type n = 0;
      for(size_type i=0; i<BlocklessBaseClass::size(); ++i)
        n += BlocklessBaseClass::operator[](i);
      return n;
    }

    //! Returns the number of set bits for given component
    int nSetBits(int j) const
    {
      size_type n = 0;
      size_type blocks = size();
      for(size_type i=0; i<blocks; ++i)
        n += getBit(i,j);
      return n;
    }


    /** \brief Return i-th block by value */
    value_type getLocalBits(int i) const
    {
      value_type bits;
      for(int j=0; j<block_size; ++j)
        bits.set(j, getBit(i,j));
      return bits;
    }

    //! Send bitfield to an output stream
    friend std::ostream& operator<< (std::ostream& s, const BlockBitField& v)
    {
      for (size_t i=0; i<v.size(); i++)
        s << v[i] << "  ";

      s << std::endl;
      return s;
    }

  private:

    typename std::vector<bool>::reference getBit(size_type i, size_type j) {
      return BlocklessBaseClass::operator[](i*block_size+j);
    }

    typename std::vector<bool>::const_reference getBit(size_type i, size_type j) const {
      return BlocklessBaseClass::operator[](i*block_size+j);
    }

    friend class BlockBitFieldReference<block_size,Allocator>;
    friend class BlockBitFieldConstReference<block_size,Allocator>;
  };

}  // namespace Dune

#endif
