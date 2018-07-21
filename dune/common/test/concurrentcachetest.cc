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
#include <dune/common/exceptions.hh>
#include <dune/common/hash.hh>


namespace Dune
{
  struct quadrature_key
  {
    int id; // topologyId
    int p;  // order
    int qt; // quadrature type

    struct hasher
    {
      std::size_t operator()(quadrature_key const& t) const
      {
        std::size_t seed = 0;
        hash_combine(seed, t.id);
        hash_combine(seed, t.p);
        hash_combine(seed, t.qt);
        return seed;
      }
    };

    friend bool operator==(quadrature_key const& lhs, quadrature_key const& rhs)
    {
      return std::tie(lhs.id,lhs.p,lhs.qt) == std::tie(rhs.id,rhs.p,rhs.qt);
    }
  };

} // end namespace Dune


using quadrature_key = Dune::quadrature_key;
using quadrature_data = std::vector<double>;

quadrature_data init_data(quadrature_key const& key)
{
  quadrature_data data(100);
  std::generate(data.begin(), data.end(), []{ return std::fmod(double(std::rand()), 10.0); });
  std::stringstream ss;
  ss << "init [" << key.id << "," << key.p << "," << key.qt << "]\n";
  std::cout << ss.str();
  return data;
}

template <template <class> class Policy>
void test()
{
  using QuadratureCache = Dune::ConcurrentCache<quadrature_key, quadrature_data, Policy,
    std::unordered_map<quadrature_key,quadrature_data,quadrature_key::hasher>>;

  std::random_device rd;

  constexpr unsigned threads_count = 16;
  std::thread threads[threads_count];
  for (auto& t: threads) {
    t = std::thread([&rd]() {
      std::mt19937 gen(rd());
      std::uniform_int_distribution<int> uniform_dist(1, 3);
      for (auto i = 0; i < 100; ++i) {
        int id = uniform_dist(gen);
        int p = uniform_dist(gen);
        int qt = uniform_dist(gen);

        auto const& data = QuadratureCache::get(quadrature_key{id,p,qt}, init_data);

        if (data.size() != 100) {
          DUNE_THROW(Dune::Exception, "Data must be initialized to size 100");
        }
      }
    });
  }

  for (auto& t: threads) {
    t.join();
  }
}

int main()
{
  test<Dune::ThreadLocalPolicy>();
  test<Dune::SharedPolicy>();
}
