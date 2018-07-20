// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_CONCURRENT_CACHE_HH
#define DUNE_COMMON_CONCURRENT_CACHE_HH

#include <mutex>
#include <thread>
#include <tuple>
#include <unordered_map>

#include <dune/common/hash.hh>
#include <dune/common/std/type_traits.hh>

namespace Dune
{
  /// Store cache thread local, requires no locking.
  struct ThreadLocalPolicy {};

  /// Stores cache global static, requires locking on write access.
  struct SharedPolicy {};


  /// \brief Cache for data of type `Data` that allows concurrent accessed by key of type `Key`.
  /**
   * Cache data of arbitray type that needs initialization on the first access. The data is thereby
   * initialized thread-wise or globally only once, and guarantees that you always get initialized data.
   *
   * \tparam Key     Type of key to access the data. Must be Hashable. \see DUNE_DEFINE_STD_HASH
   * \tparam Data    Type of the data to store in the cache.
   * \tparam Policy  One of \ref ThreadLocalPolicy, or \ref SharedPolicy. [`ThreadLocalPolicy`].
   * \tparam Hash    Type of a hasher. [`Dune::hash<Key>`]
   * \tparam KeyEqual  Type of a predicate to test for equailty of the keys. [`std::equal_to<Key>`]
   *
   * The types `Hash` and `KeyEqual` must be DefaultConstructible, since only the default constructor
   * for the internal cache is called.
   **/
  template <class Key,
            class Data,
            class Policy = ThreadLocalPolicy,
            class Hash = hash<Key>,
            class KeyEqual = std::equal_to<Key>>
  class ConcurrentCache;


  namespace Impl
  {
    /// \brief Implementation of concrete policies for the ConcurrentCache. May be specialized for
    /// user-defined storage policies.
    /**
     * An implementation must provide a static `get_or_init()` method that returns a `const&` to the stored data.
     **/
    template <class Key, class Data, class Policy, class Hash, class KeyEqual>
    class ConcurrentCacheImpl;

    // implementation of the ThreadLocal policy
    template <class Key, class Data, class Hash, class KeyEqual>
    class ConcurrentCacheImpl<Key,Data,ThreadLocalPolicy,Hash,KeyEqual>
    {
      friend class ConcurrentCache<Key,Data,ThreadLocalPolicy,Hash,KeyEqual>;

      using key_type = Key;
      using data_type = Data;
      using hasher = Hash;
      using key_equal = KeyEqual;

      using container_type = std::unordered_map<key_type, data_type, hasher, key_equal>;

      template <class F, class... Args>
      static data_type const& get_or_init(key_type const& key, F&& f, Args&&... args)
      {
        // Container to store the cached values
        thread_local container_type cached_data;

        auto it = cached_data.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(args...));
        if (it.second)
          f(&(it.first->second), key);
        return it.first->second;
      }
    };


    // implementation of the Shared policy
    template <class Key, class Data, class Hash, class KeyEqual>
    class ConcurrentCacheImpl<Key,Data,SharedPolicy,Hash,KeyEqual>
    {
      friend class ConcurrentCache<Key,Data,SharedPolicy,Hash,KeyEqual>;

      using key_type = Key;
      using data_type = Data;
      using hasher = Hash;
      using key_equal = KeyEqual;

      using container_type = std::unordered_map<key_type, data_type, hasher, key_equal>;

      template <class F, class... Args>
      static data_type const& get_or_init(key_type const& key, F&& f, Args&&... args)
      {
        // Container to store the cached values
        static container_type cached_data;

        // mutex used to access the data in the container, necessary since
        // access emplace is read-write.
        static std::mutex access_mutex;

        std::lock_guard<std::mutex> lock(access_mutex);
        auto it = cached_data.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(args...));
        if (it.second)
          f(&(it.first->second), key);
        return it.first->second;
      }
    };

  } // end namespace Impl


  template <class Key, class Data, class Policy, class Hash, class KeyEqual>
  class ConcurrentCache
  {
    using key_type = Key;
    using data_type = Data;

    /// The actual implementation. Must provide a static get_or_init() method
    using Implementation = Impl::ConcurrentCacheImpl<Key,Data,Policy,Hash,KeyEqual>;

  public:

    /// \brief Return the data associated to the key. If not yet initialized, call functor f.
    /**
     * \tparam F        A functor of signature void(data_type*, key_type)
     * \tparam Args...  Type of arguments passed to the constructor of data_type
     *
     * Return the data associated to key. If no data is found, create a new entry in the map
     * with a value `data_type(args...)`, i.e. forward the additional arguments to the constructor
     * of `data_type`. Then, call the functor `f` on the new created entry.
     **/
    template <class F, class... Args,
      std::enable_if_t<Std::is_invocable<F,data_type*,key_type>::value, int> = 0>
    static data_type const& get(key_type key, F&& f, Args&&... args)
    {
      static_assert(std::is_constructible<data_type, Args...>::value,
        "Data stored in ConcurrentCache must be constructible with provided Args...");

      return Implementation::get_or_init(key, std::forward<F>(f), std::forward<Args>(args)...);
    }
  };

} // end namespace Dune

#endif // DUNE_COMMON_CONCURRENT_CACHE_HH
