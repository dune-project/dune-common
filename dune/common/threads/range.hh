// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_THREADS_RANGE_HH
#define DUNE_COMMON_THREADS_RANGE_HH

#include <cstddef>

#include <tbb/blocked_range.h>

namespace Dune {
  namespace Threads {

    /**
     *  \addtogroup tbb Threading Building Blocks extensions
     *  \ingroup Common
     *  \{
     */


    /** \file
     * \brief Special-purpose ranges for TBB algorithms.
     */

    //! TBB-compatible range that respects fixed block sizes when splitting.
    /**
     * \tparam V  value type of the range
     */
    template<typename V>
    class fixed_block_size_range
    {

    public:

      //! Size type.
      typedef std::size_t size_type;

      //! Iterator type returned from begin() and end().
      typedef V const_iterator;

      //! Block iterator type returned from begin_block() and end_block().
      typedef size_type block_iterator;

    private:

      // This method performs the actual range splitting, assigning the left part to the
      // existing range r. It returns the splitting point element to be used as the lower bound
      // in the constructor of the new range. As the algorithm preserves the block structure of
      // the range, the two subranges will usually not be of equal size. The size of the left
      // subrange is guaranteed to be a multiple of the block size.
      static const_iterator do_split(fixed_block_size_range& r)
      {
        size_type middle_chunk = (r.block_count() / r._chunk_size) >> 1;
        V middle = r._begin + middle_chunk * r._block_size * r._chunk_size;
        r._end = middle;
        return middle;
      }

      // This method performs the range splitting for the blocks, assigning the left part to the
      // existing range r. It returns the splitting point element to be used as the lower bound
      // in the constructor of the new range. As the algorithm preserves the block structure of
      // the range, the two subranges will usually not be of equal size. The size of the left
      // subrange is guaranteed to be a multiple of the block size.
      static const_iterator do_block_split(fixed_block_size_range& r)
      {
        size_type middle_chunk = r._begin_block / r._chunk_size + ((r.block_count() / r._chunk_size) >> 1);
        size_type middle_block = middle_chunk * r._chunk_size;
        r._end_block = middle_block;
        return middle_block;
      }

      static block_iterator calculate_end_block(V begin, V end, size_type block_size)
      {
        if (!(begin < end))
          return 0;
        block_iterator e = (end - begin) / block_size;
        block_iterator remainder = (end - begin) % block_size;
        return e + (remainder > 0);
      }

    public:

      //! Constructs a new fixed_block_size_range over the half-open interval [begin,end).
      /**
       * \param begin      the lower bound of the range.
       * \param end        the upper bound of the range.
       * \param block_size the block size of the range.
       * \param grain_size the grain size of the range. Will automatically be raised to be >= block_size * chunk_size.
       * \param chunk_size the minimum chunk size that the range will be chopped into, measured in blocks.
       */
      fixed_block_size_range(V begin, V end, size_type block_size = 1, size_type grain_size = 1, size_type chunk_size = 1)
        : _end(end)
        , _begin(begin)
        , _end_block(calculate_end_block(begin,end,block_size))
        , _begin_block(0)
        , _block_size(block_size)
        , _grain_size(std::max(block_size * chunk_size,grain_size))
        , _chunk_size(chunk_size)
      {}

      //! Constructs a new fixed_block_size_range by splitting the existing range r.
      fixed_block_size_range(fixed_block_size_range& r, tbb::split)
        : _end(r._end)
        , _begin(do_split(r))
        , _end_block(r._end_block)
        , _begin_block(do_block_split(r))
        , _block_size(r._block_size)
        , _grain_size(r._grain_size)
        , _chunk_size(r._chunk_size)
      {}

      //! Returns the lower bound of the range.
      const_iterator begin() const
      {
        return _begin;
      }

      //! Returns the upper bound of the range.
      const_iterator end() const
      {
        return _end;
      }

      //! Returns the lower bound of the block indices spanned by the range.
      const_iterator begin_block() const
      {
        return _begin_block;
      }

      //! Returns the upper bound of the block indices spanned by the range.
      const_iterator end_block() const
      {
        return _end_block;
      }

      //! Returns the grain size of the range.
      /**
       * The grain size is always at least as large as the block size.
       */
      size_type grainsize() const
      {
        return _grain_size;
      }

      //! Returns the block size of the range.
      /**
       * Subranges are always guaranteed to be aligned to a multiple of the block size
       * and all subranges apart from the last one have a size that is a multiple of
       * the block size.
       */
      size_type blocksize() const
      {
        return _grain_size;
      }

      //! Returns the size of the range.
      size_type size() const
      {
        return _end - _begin;
      }

      //! Returns the number of blocks spanned by the range.
      size_type block_count() const
      {
        return _end_block - _begin_block;
      }

      //! Returns whether the range is empty.
      /**
       * May be faster than testing for size() == 0.
       */
      bool empty() const
      {
        return !(_begin < _end);
      }

      //! Returns whether this range can be split into two subranges.
      /**
       * A range can be split if it is at least twice as large as the grain size,
       * i.e. if 2 * grainsize() <= size().
       *
       * \note This criterion differs from the standard TBB blocked_range, which uses
       *       grainsize() < size(), but that criterion is hard to sensibly integrate
       *       into the block-preserving splitting semantics of fixed_block_size_range.
       */
      bool is_divisible() const
      {
        return 2 * _grain_size <= size();
      }

    private:

      const_iterator _end;
      const_iterator _begin;
      block_iterator _end_block;
      block_iterator _begin_block;
      const size_type _block_size;
      const size_type _grain_size;
      const size_type _chunk_size;

    };

    //! \} group tbb

  } // namespace tbb
} // namespace Dune

#endif // DUNE_COMMON_THREADS_RANGE_HH
