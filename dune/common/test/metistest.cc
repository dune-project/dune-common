// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <iostream>
#include <vector>

#if ! HAVE_METIS
#error "METIS is required for this test"
#endif

#include <dune/common/metis.hh>

int main()
{
  using real_t = Dune::Metis::real_t;
  using idx_t = Dune::Metis::idx_t;

  idx_t nVertices = 6; // number of vertices
  idx_t nCon      = 1; // number of constraints
  idx_t nParts    = 2; // number of partitions

  // Partition index for each vertex. Will be filled by METIS_PartGraphKway.
  std::vector<idx_t> part(nVertices, 0);

  // Indices of starting points in adjacent array
  std::vector<idx_t> xadj{0,2,5,7,9,12,14};

  // Adjacent vertices in consecutive order
  std::vector<idx_t> adjncy{1,3,0,4,2,1,5,0,4,3,1,5,4,2};

  // Weights of vertices. If all weights are equal, they can be set to 1.
  std::vector<idx_t> vwgt(nVertices * nCon, 1);

  // Load-imbalance tolerance for each constraint.
#if HAVE_SCOTCH_METIS
  // NOTE: scotchmetis interprets this parameter differently
  std::vector<real_t> ubvec(nCon, 0.01);
#else
  std::vector<real_t> ubvec(nCon, 1.001);
#endif

#if METIS_API_VERSION >= 5

  std::cout << "using METIS API version 5\n";

  idx_t objval;
  int err = METIS_PartGraphKway(&nVertices, &nCon, xadj.data(), adjncy.data(),
      vwgt.data(), nullptr, nullptr, &nParts, nullptr,
      ubvec.data(), nullptr, &objval, part.data());

#elif METIS_API_VERSION >= 3

  std::cout << "using METIS API version 3\n";

  int wgtflag = 2;
  int numflag = 0;
  int options = 0; // use default options

  int edgecut;
#if HAVE_SCOTCH_METIS && ! defined(SCOTCH_METIS_RETURN)
  METIS_PartGraphKway(&nVertices, xadj.data(), adjncy.data(), vwgt.data(),
    nullptr, &wgtflag, &numflag, &nParts, &options, &edgecut, part.data());
  int err = METIS_OK;
#else
  int err = METIS_PartGraphKway(&nVertices, xadj.data(), adjncy.data(), vwgt.data(),
    nullptr, &wgtflag, &numflag, &nParts, &options, &edgecut, part.data());
#endif

#endif // METIS_API_VERSION

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