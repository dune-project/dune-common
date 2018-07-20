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
  class ConcurrentCache
  {
    using key_type = Key;
    using data_type = Data;
    using hasher = Hash;
    using key_equal = KeyEqual;

  public:

    /// \brief Return the data associated to the key. If not yet initialized, call functor f.
    template <class F,
      std::enable_if_t<Dune::Std::is_invocable<F,data_type*,key_type>::value, int> = 0>
    static data_type const& get(key_type key, F&& f)
    {
      return instance(Policy{}).get_or_init(key, std::forward<F>(f));
    }

  private:

    auto& guarded_get(key_type const& key, SharedPolicy)
    {
      std::lock_guard<std::mutex> lock(access_mutex);
      return cached_data[key];
    }

    auto& guarded_get(key_type const& key, ThreadLocalPolicy)
    {
      return cached_data[key];
    }

    template <class F>
    data_type const& get_or_init(key_type const& key, F&& f)
    {
      auto& data = guarded_get(key, Policy{});

      std::call_once(data.first, std::forward<F>(f), &data.second, key);
      return data.second;
    }

  private:

    // Return an instance of this class with static storage
    static ConcurrentCache& instance(SharedPolicy)
    {
      static ConcurrentCache cache;
      return cache;
    }

    // Return an instance of this class with thread_local storage
    static ConcurrentCache& instance(ThreadLocalPolicy)
    {
      thread_local ConcurrentCache cache;
      return cache;
    }

    // private constructor
    ConcurrentCache() = default;

    // mutex used to access the data in the container, necessary since
    // access by operator[] is read+write access, i.e. an empty data element
    // is created if it does not exist yet.
    std::mutex access_mutex;

    // Container to store the cached values
    std::unordered_map<key_type, std::pair<std::once_flag, data_type>, hasher, key_equal> cached_data;
  };

} // end namespace Dune

#endif // DUNE_COMMON_CONCURRENT_CACHE_HH
