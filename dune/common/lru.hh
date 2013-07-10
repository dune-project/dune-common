// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_LRU_HH
#define DUNE_COMMON_LRU_HH

#include <list>
#include <utility>
#include <map>

/** @file
    @author Christian Engwer
    @brief LRU Cache Container, using an STL like interface
 */

namespace Dune {

  namespace {

    /*
       hide the default traits in an empty namespace
     */
    template <typename _Key, typename _Tp,
        typename _Alloc = std::allocator<_Tp> >
    struct _lru_default_traits
    {
      typedef _Key key_type;
      typedef _Alloc allocator;
      typedef std::list< std::pair<_Key, _Tp> > list_type;
      typedef typename list_type::iterator iterator;
      typedef typename std::less<key_type> cmp;
      typedef std::map< key_type, iterator, cmp,
          typename allocator::template rebind<std::pair<const key_type, iterator> >::other > map_type;
    };

  } // end empty namespace

  /**
      @brief LRU Cache Container

      Implementatation of an LRU (least recently used) cache
      container. This implementation follows the approach presented in
      http://aim.adc.rmit.edu.au/phd/sgreuter/papers/graphite2003.pdf
   */
  template <typename _Key, typename _Tp,
      typename _Traits = _lru_default_traits<_Key, _Tp> >
  class lru
  {
    typedef typename _Traits::list_type list_type;
    typedef typename _Traits::map_type map_type;
    typedef typename _Traits::allocator allocator;
    typedef typename map_type::iterator map_iterator;
    typedef typename map_type::const_iterator const_map_iterator;

  public:
    typedef typename _Traits::key_type key_type;
    typedef typename allocator::value_type value_type;
    typedef typename allocator::pointer pointer;
    typedef typename allocator::const_pointer const_pointer;
    typedef typename allocator::const_reference const_reference;
    typedef typename allocator::reference reference;
    typedef typename allocator::size_type size_type;
    typedef typename list_type::iterator iterator;
    typedef typename list_type::const_iterator const_iterator;

    /**
     *  Returns a read/write reference to the data of the most
     *  recently used entry.
     */
    reference front()
    {
      return _data.front().second;
    }

    /**
     *  Returns a read-only (constant) reference to the data of the
     *  most recently used entry.
     */
    const_reference front() const
    {
      return _data.front().second;
    }

    /**
     *  Returns a read/write reference to the data of the least
     *  recently used entry.
     */
    reference back()
    {
      return _data.back().second;
    }

    /**
     *  Returns a read-only (constant) reference to the data of the
     *  least recently used entry.
     */
    const_reference back (int i) const
    {
      return _data.back().second;
    }


    /**
     * @brief Removes the first element.
     */
    const void pop_front()
    {
      key_type k = _data.front().first;
      _data.pop_front();
      _index.erase(k);
    }
    /**
     * @brief Removes the last element.
     */
    const void pop_back()
    {
      key_type k = _data.back().first;
      _data.pop_back();
      _index.erase(k);
    }

    /**
     * @brief Finds the element whose key is k.
     *
     * @return iterator
     */
    iterator find (const key_type & key)
    {
      const map_iterator it = _index.find(key);
      if (it == _index.end()) return _data.end();
      return it->second;
    }

    /**
     * @brief Finds the element whose key is k.
     *
     * @return const_iterator
     */
    const_iterator find (const key_type & key) const
    {
      const map_iterator it = _index.find(key);
      if (it == _index.end()) return _data.end();
      return it->second;
    }

    /**
     * @brief Insert a value into the container
     *
     * Stores value under key and marks it as most recent. If this key
     * is already present, the associated data is replaced.
     *
     * @param key   associated with data
     * @param data  to store
     *
     * @return reference of stored data
     */
    reference insert (const key_type & key, const_reference data)
    {
      std::pair<key_type, value_type> x(key, data);
      /* insert item as mru */
      iterator it = _data.insert(_data.begin(), x);
      /* store index */
      _index.insert(std::make_pair(key,it));

      return it->second;
    }

    /**
     * @copydoc touch
     */
    reference insert (const key_type & key)
    {
      return touch (key);
    }

    /**
     * @brief mark data associateed with key as most recent
     *
     * @return reference of stored data
     */
    reference touch (const key_type & key)
    {
      /* query _index for iterator */
      iterator it = _index[key];
      if (it == _index.end())
        DUNE_THROW(Dune::RangeError,
          "Failed to touch key " << key << ", it is not in the lru container");
       /* update _data
         move it to the front
       */
      _data.splice(_data.begin(), _data, it);
      return it->second;
    }

    /**
     * @brief retief number of entries in the container
     */
    size_type size() const
    {
      return _data.size();
    }

    /**
     * @brief ensure a maximum size of the container
     * If new_size is smaller than size the oldest elements are
     * dropped. Otherwise nothing happens.
     */
    void resize(size_type new_size)
    {
      assert(new_size <= size());

      while (new_size < size())
        pop_back();
    }

    /**
     *
     */
    void clear()
    {
      _data.clear();
      _index.clear();
    }

  private:
    list_type _data;
    map_type _index;

  };

} // namespace Dune

#endif // DUNE_COMMON_LRU_HH
