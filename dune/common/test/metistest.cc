#include <config.h>
#include <iostream>
#include <vector>

#if ! HAVE_METIS
#error "METIS is required for this test"
#endif

#if HAVE_PTSCOTCH
#include <scotch.h>
#endif

#include <metis.h>

int main()
{
#if defined(REALTYPEWIDTH)
  using real_t = ::real_t;
#else
  using real_t = float;
#endif

#if defined(IDXTYPEWIDTH)
  using idx_t = ::idx_t;
#elif defined(SCOTCH_METIS_PREFIX)
  using idx_t = SCOTCH_Num;
#else
  using idx_t = int;
#endif

  idx_t nVertices = 6;
  idx_t nWeights  = 1;
  idx_t nParts    = 2;

  std::vector<idx_t> part(nVertices, 0);

  // Indexes of starting points in adjacent array
  std::vector<idx_t> xadj{0,2,5,7,9,12,14};

  // Adjacent vertices in consecutive index order
  std::vector<idx_t> adjncy{1,3,0,4,2,1,5,0,4,3,1,5,4,2};

  // Weights of vertices
  // if all weights are equal then can be set to 0
  std::vector<idx_t> vwgt(nVertices * nWeights, 0);

  idx_t objval;
  int err = METIS_PartGraphKway(&nVertices, &nWeights, xadj.data(), adjncy.data(),
    nullptr, nullptr, nullptr, &nParts, nullptr,
    nullptr, nullptr, &objval, part.data());

  if (err != METIS_OK)
    return 1;

  for (std::size_t part_i = 0; part_i < part.size(); ++part_i) {
    // partition index must be in range [0,nParts)
    if (part[part_i] >= nParts)
      return 2;

    std::cout << part_i << " " << part[part_i] << std::endl;
  }

  return 0;
}