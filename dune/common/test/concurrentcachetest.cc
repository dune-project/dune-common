// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <algorithm>
#include <array>
#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <thread>

#include <dune/common/concurrentcache.hh>
#include <dune/common/hash.hh>


namespace Dune
{
  namespace Impl
  {
    // Recursive template code derived from Matthieu M.
    template <class Tuple, std::size_t I = std::tuple_size<Tuple>::value - 1>
    struct HashTupleImpl
    {
      static void apply(size_t& seed, Tuple const& tuple)
      {
        HashTupleImpl<Tuple, I-1>::apply(seed, tuple);
        hash_combine(seed, std::get<I>(tuple));
      }
    };

    template <class Tuple>
    struct HashTupleImpl<Tuple, 0>
    {
      static void apply(std::size_t& seed, Tuple const& tuple)
      {
        hash_combine(seed, std::get<0>(tuple));
      }
    };

  } // end namespace Impl


  struct quadrature_key
  {
    // (geometryType, order, quadratureType)
    using type = std::tuple<int,int,int>;
    type key;

    friend std::size_t hash_value(quadrature_key const& t)
    {
      std::size_t seed = 0;
      Impl::HashTupleImpl<quadrature_key::type>::apply(seed, t.key);
      return seed;
    }

    friend bool operator==(quadrature_key const& lhs, quadrature_key const& rhs)
    {
      return lhs.key == rhs.key;
    }
  };

} // end namespace Dune

DUNE_DEFINE_STD_HASH( , Dune::quadrature_key)


using quadrature_key = Dune::quadrature_key;
using quadrature_data = std::vector<double>;

void init_data(quadrature_data* data, quadrature_key const& key)
{
  data->resize(1000);
  std::generate(data->begin(), data->end(), []{ return std::fmod(double(std::rand()), 10.0); });
  std::stringstream ss;
  ss << "init [" << std::get<0>(key.key) << "," << std::get<1>(key.key) << "," << std::get<2>(key.key) << "]\n";
  std::cout << ss.str();
}


int main()
{
  using QuadratureCache = Dune::ConcurrentCache<quadrature_key, quadrature_data, Dune::SharedPolicy>;
  std::random_device rd;

  constexpr unsigned threads_count = 16;
  std::thread threads[threads_count];
  for (auto& t: threads) {
    t = std::thread([&rd]() {
      std::mt19937 gen(rd());
      std::uniform_int_distribution<int> uniform_dist(1, 1);
      for (auto i = 0; i < 100; ++i) {
        int gt = uniform_dist(gen);
        int p = uniform_dist(gen);
        int qt = uniform_dist(gen);

        quadrature_key key{typename quadrature_key::type{gt,p,qt}};
        /*auto const& data =*/ QuadratureCache::get(key, init_data);
      }
    });
  }

  for (auto& t: threads) {
    t.join();
  }
}
