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

  idx_t rank, size;
  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &size);

  // This test is design for 4 cores
  assert(size == 4);

  // local adjacency structure of the graph
  std::vector<idx_t> xadj;    // size n+1
  std::vector<idx_t> adjncy;  // size 2*m
  if (rank == 0) {
    xadj = std::vector<idx_t>{0,2,3};
    adjncy = std::vector<idx_t>{4,1,0,5,2};
  }
  else if (rank == 1)  {
    xadj = std::vector<idx_t>{0,3,5};
    adjncy = std::vector<idx_t>{1,6,3,2,7};
  }
  else if (rank == 2) {
    xadj = std::vector<idx_t>{0,2,5};
    adjncy = std::vector<idx_t>{5,0,6,1,4};
  }
  else if (rank == 3) {
    xadj = std::vector<idx_t>{0,3,5};
    adjncy = std::vector<idx_t>{7,2,5,3,6};
  }

  // Array describing how the vertices of the graph are distributed among the processors.
  std::vector<idx_t> vtxdist{0,2,4,6,8};

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

  int err = ParMETIS_V3_PartKway(vtxdist.data(), xadj.data(), adjncy.data(),
    nullptr, nullptr, &wgtflag, &numflag, &ncon, &nparts, tpwgts.data(),
    ubvec.data(), options.data(), &edgecut, part.data(), &comm);

  if (err != METIS_OK)
    return 1;

  for (std::size_t part_i = 0; part_i < part.size(); ++part_i) {
    std::cout << "[" << rank << "] " << part_i << " => " << part[part_i] << std::endl;
  }

  MPI_Finalize();
  return 0;
}