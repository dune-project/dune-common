// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_TRUNCATED_MG_TRANSFER_HH
#define DUNE_TRUNCATED_MG_TRANSFER_HH

#include <dune/istl/bcrsmatrix.hh>
#include <dune/fem/transfer/multigridtransfer.hh>


namespace Dune {


  /** \brief Restriction and prolongation operator for truncated multigrid
   *
   * This class provides prolongation and restriction operators for truncated
   * multigrid.  That means that you can explicitly switch off certain
   * fine grid degrees-of-freedom.  This often leads to better coarse
   * grid corrections when treating obstacle problems.  For an introduction
   * to the theory of truncated multigrid see 'Adaptive Monotone Multigrid
   * Methods for Nonlinear Variational Problems' by R. Kornhuber.
   *
   * Currently only works for first-order Lagrangian elements!
   */
  template<class DiscFuncType>
  class TruncatedMGTransfer : public MultiGridTransfer<DiscFuncType> {

    enum {blocksize = DiscFuncType::block_type::size};

    /** \todo Should we extract the value type from DiscFuncType? */
    typedef Dune::FieldMatrix<double, blocksize, blocksize> MatrixBlock;


  public:

    typedef Dune::BCRSMatrix<MatrixBlock> OperatorType;

    /** \brief Default constructor */
    TruncatedMGTransfer() : recompute_(NULL)
    {}

    /** \brief Restrict level fL of f and store the result in level cL of t
     *
     * \param critical Has to contain an entry for each degree of freedom.
     *        Those dofs with a set bit are treated as critical.
     */
    void restrict (const DiscFuncType & f, DiscFuncType &t, const Dune::BitField& critical) const;

    /** \brief Prolong level cL of f and store the result in level fL of t
     *
     * \param critical Has to contain an entry for each degree of freedom.
     *        Those dofs with a set bit are treated as critical.
     */
    void prolong(const DiscFuncType& f, DiscFuncType &t, const Dune::BitField& critical) const;

    /** \brief Galerkin assemble a coarse stiffness matrix
     *
     * \param critical Has to contain an entry for each degree of freedom.
     *        Those dofs with a set bit are treated as critical.
     */
    void galerkinRestrict(const OperatorType& fineMat, OperatorType& coarseMat,
                          const Dune::BitField& critical) const;

    /** \brief Bitfield specifying a subsets of dofs which need to be recomputed
     * when doing Galerkin restriction
     *
     * If this pointer points to NULL it has no effect.  If not it is expected
     * to point to a bitfield with the size of the coarse function space.
     * Then, when calling galerkinRestrict(), only those matrix entries which
     * involve at least one dof which has a set bit get recomputed.  Depending
     * on the problem this can lead to considerable time savings.
     */
    const Dune::BitField* recompute_;
  };


}  // end namespace Dune


#include "truncatedtransfer.cc"

#endif
