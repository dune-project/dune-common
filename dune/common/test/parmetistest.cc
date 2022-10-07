// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <config.h>
#include <cassert>
#include <iostream>
#include <vector>

#if ! HAVE_PARMETIS
#error "ParMETIS is required for this test."
#endif

#include <mpi.h>

#if HAVE_PTSCOTCH_PARMETIS
extern "C" {
  #include <ptscotch.h>
}
#endif

extern "C" {
  #include <parmetis.h>
}

int main(int argc, char **argv)
{
#if defined(REALTYPEWIDTH)
  using real_t = ::real_t;
#else
  using real_t = float;
#endif

#if defined(IDXTYPEWIDTH)
  using idx_t = ::idx_t;
#elif HAVE_PTSCOTCH_PARMETIS
  using idx_t = SCOTCH_Num;
#else
  using idx_t = int;
#endif

  MPI_Init(&argc, &argv);

  MPI_Comm comm;
  MPI_Comm_dup(MPI_COMM_WORLD, &comm);

  int rank, size;
  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &size);

  // This test is design for 3 cores
  assert(size == 3);

  // local adjacency structure of the graph
  std::vector<idx_t> xadj;    // size n+1
  std::vector<idx_t> adjncy;  // size 2*m

  if (rank == 0) {
    xadj = std::vector<idx_t>{0,2,5,8,11,13};
    adjncy = std::vector<idx_t>{1,5,0,2,6,1,3,7,2,4,8,3,9};
  }
  else if (rank == 1)  {
    xadj = std::vector<idx_t>{0,3,7,11,15,18};
    adjncy = std::vector<idx_t>{0,6,10,1,5,7,11,2,6,8,12,3,7,9,13,4,8,14};
  }
  else if (rank == 2) {
    xadj = std::vector<idx_t>{0,2,5,8,11,13};
    adjncy = std::vector<idx_t>{5,11,6,10,12,7,11,13,8,12,14,9,13};
  }

  // Array describing how the vertices of the graph are distributed among the processors.
  std::vector<idx_t> vtxdist{0,5,10,15};

  // No weights
  idx_t wgtflag = 0;
  // C-style numbering that starts from 0.
  idx_t numflag = 0;
  // Number of weights that each vertex has
  idx_t ncon = 1;
  // Number of sub-domains
  idx_t nparts = size;
  // Fraction of vertex weight that should be distributed to each sub-domain for each
  // balance constraint
  std::vector<real_t> tpwgts(ncon * nparts, 1.0/nparts);
  std::vector<real_t> ubvec(ncon, 1.05);
  std::vector<idx_t> options{0, 0, 0};

  idx_t edgecut;
  std::vector<idx_t> part(xadj.size()-1, 0);

  ParMETIS_V3_PartKway(vtxdist.data(), xadj.data(), adjncy.data(),
    nullptr, nullptr, &wgtflag, &numflag, &ncon, &nparts, tpwgts.data(),
    ubvec.data(), options.data(), &edgecut, part.data(), &comm);

  for (std::size_t part_i = 0; part_i < part.size(); ++part_i) {
    std::cout << "[" << rank << "] " << part_i << " => " << part[part_i] << std::endl;
  }

  MPI_Finalize();
  return 0;
}