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

namespace Dune {

  template <int block_size> class BlockBitField;

  template <int block_size>
  class BlockBitFieldReference
  {
  protected:
    BlockBitFieldReference(BlockBitField<block_size>& blockBitField, int block_number) :
      blockBitField(blockBitField),
      block_number(block_number)
    {};

    typedef typename BlockBitField<block_size>::BlockLessBaseClass BlockLess;

  public:
    typedef std::vector<bool>::reference BitReference;

    BlockBitFieldReference<block_size>& operator=(bool b)
    {
      for(int i=0; i<block_size; ++i)
        blockBitField.BlockLess::operator[](block_number*block_size+i) = b;

      return (*this);
    }

    BitReference operator[](int i)
    {
      return blockBitField.BlockLess::operator[](block_number*block_size+i);
    }

    const BitReference operator[](int i) const
    {
      return blockBitField.BlockLess::operator[](block_number*block_size+i);
    }

    friend std::ostream& operator<< (std::ostream& s, const BlockBitFieldReference<block_size>& v)
    {
      s << "(";
      for(int i=0; i<block_size; ++i)
        s << v[i];
      s << ")";
      return s;
    }

  private:
    BlockBitField<block_size>& blockBitField;
    int block_number;

    friend class BlockBitField<block_size>;
  };



  /** \brief A dynamic array of blocks of booleans
   *
   */
  template <int block_size>
  class BlockBitField : private std::vector<bool>
  {
  protected:

    /** \brief The implementation class: an unblocked bitfield */
    typedef std::vector<bool> BlocklessBaseClass;

  public:
    typedef std::bitset<block_size> LocalBits;

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

    //! Resize field
    void resize(int n)
    {
      BlocklessBaseClass::resize(n*block_size);
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
    BlockBitFieldReference<block_size> operator[](int i)
    {
      return BlockBitFieldReference<block_size>(*this, i);
    }

    /** \brief Return const reference to i-th block */
    const BlockBitFieldReference<block_size> operator[](int i) const
    {
      return BlockBitFieldReference<block_size>(const_cast<BlockBitField<block_size>& >(*this), i);
    }

    /** \brief Return reference to last block */
    BlockBitFieldReference<block_size> back()
    {
      return BlockBitFieldReference<block_size>(*this, size()-1);
    }

    /** \brief Return const reference to last block */
    const BlockBitFieldReference<block_size> back() const
    {
      return BlockBitFieldReference<block_size>(const_cast<BlockBitField<block_size>& >(*this), size()-1);
    }

    //! Returns the total number of set bits
    int nSetBits() const
    {
      int n = 0;
      for(int i=0; i<BlocklessBaseClass::size(); ++i)
        n += BlocklessBaseClass::operator[](i);
      return n;
    }

    //! Returns the number of set bits for given component
    int nSetBits(int j) const
    {
      int n = 0;
      int blocks = size();
      for(int i=0; i<blocks; ++i)
        n += (BlocklessBaseClass::operator[](i*block_size+j)) ? 1 : 0;
      return n;
    }


    /** \brief Return i-th block by value */
    LocalBits getLocalBits(int i) const
    {
      LocalBits bits;
      for(int j=0; j<block_size; ++j)
        bits.set(j, BlocklessBaseClass::operator[](i*block_size+j));
      return bits;
    }

    //! Send bitfield to an output stream
    friend std::ostream& operator<< (std::ostream& s, const BlockBitField<block_size>& v)
    {
      for (size_t i=0; i<v.size(); i++)
        s << v[i] << "  ";

      s << std::endl;
      return s;
    }

    friend class BlockBitFieldReference<block_size>;
  };

}  // namespace Dune

#endif
