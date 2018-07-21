// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_CONCURRENT_CACHE_HH
#define DUNE_COMMON_CONCURRENT_CACHE_HH

#include <mutex>
#include <shared_mutex>
#include <thread>
#include <tuple>
#include <unordered_map>

#include <dune/common/hash.hh>
#include <dune/common/std/type_traits.hh>

namespace Dune
{
  /// Store cache thread local, requires no locking.
  template <class Container>
  struct ThreadLocalPolicy;

  /// Stores cache global static, requires locking on write access.
  template <class Container>
  struct SharedPolicy;


  /// \brief Cache for data of type `Data` that allows concurrent accessed by key of type `Key`.
  /**
   * Cache data of arbitray type that needs initialization on the first access. The data is thereby
   * initialized thread-wise or globally only once, and guarantees that you always get initialized data.
   *
   * \tparam Key        Type of key to access the data.
   * \tparam Data       Type of the data to store in the cache.
   * \tparam Policy     E.g. one of \ref ThreadLocalPolicy, or \ref SharedPolicy. [`ThreadLocalPolicy`].
   *                    \see ConcurrentCachePolicy
   * \tparam Container  Associative container to store the data in. Default is an unordered_map,
   *                    requiring the key to be hashable. [`std::unordered_map<Key,Data>`]
   *
   * The `Policy` is a template parametrizable with the container type, that provides a static `get_or_init()`
   * method, that is called with the key, and a functor for creation of new data elements.
   **/
  template <class Key,
            class Data,
            template <class> class Policy = ThreadLocalPolicy,
            class Container = std::unordered_map<Key, Data>>
  class ConcurrentCache;


#ifdef DOXYGEN
  /// \brief Implementation of concrete policies for the \ref ConcurrentCache. May be specialized for
  /// user-defined storage policies. \see ThreadLocalPolicy, \see SharedPolicy.
  /**
   * An implementation must provide a static `get_or_init()` method that returns a `const&` to the stored data.
   *
   * \tparam Container  The Type of the associative container key->data to store the cached data.
   **/
  template <class Container>
  class ConcurrentCachePolicy;
#endif

  // implementation of the ThreadLocal policy
  template <class Container>
  struct ThreadLocalPolicy
  {
    using key_type = typename Container::key_type;
    using data_type = typename Container::mapped_type;
    using container_type = Container;

    template <class F, class... Args>
    static data_type const& get_or_init(key_type const& key, F&& f, Args&&... args)
    {
      // Container to store the cached values
      thread_local container_type cached_data;

      data_type data = f(key, std::forward<Args>(args)...);
      auto it = cached_data.emplace(key, std::move(data));
      return it.first->second;
    }
  };


  // implementation of the Shared policy
  template <class Container>
  struct SharedPolicy
  {
    using key_type = typename Container::key_type;
    using data_type = typename Container::mapped_type;
    using container_type = Container;

    template <class F, class... Args>
    static data_type const& get_or_init(key_type const& key, F&& f, Args&&... args)
    {
      // Container to store the cached values
      static container_type cached_data;

      // mutex used to access the data in the container, necessary since
      // access emplace is read-write.
      using mutex_type = std::shared_timed_mutex;
      static mutex_type access_mutex;

      // first try to lock for read-only, if an element for key is found, return it,
      // if not, obtain a unique_lock to insert a new element and initialize it.
      std::shared_lock<mutex_type> read_lock(access_mutex);
      auto it = cached_data.find(key);
      if (it != cached_data.end())
        return it->second;
      else {
        read_lock.unlock();
        data_type data = f(key, std::forward<Args>(args)...);
        std::unique_lock<mutex_type> write_lock(access_mutex);
        auto new_it = cached_data.emplace(key, std::move(data));
        return new_it.first->second;
      }
    }
  };


  template <class Key, class Data, template <class> class Policy, class Container>
  class ConcurrentCache
      : private Policy<Container>
  {
    using key_type = Key;
    using data_type = Data;

  public:

    /// \brief Return the data associated to the key. If not yet initialized, call functor f.
    /**
     * \tparam F        A functor of signature data_type(key_type, Args...)
     * \tparam Args...  Type of arguments passed to the functor f
     *
     * Return the data associated to key. If no data is found, create a new entry in the container
     * with a value obtainer from the functor, by calling `f(key, args...)`.
     **/
    template <class F, class... Args>
    static data_type const& get(key_type key, F&& f, Args&&... args)
    {
      static_assert(Std::is_callable<F(key_type, Args...), data_type>::value,
        "Functor F must have the signature data_type(key_type, Args...)");

      return ConcurrentCache::get_or_init(key, std::forward<F>(f), std::forward<Args>(args)...);
    }
  };

} // end namespace Dune

#endif // DUNE_COMMON_CONCURRENT_CACHE_HH
