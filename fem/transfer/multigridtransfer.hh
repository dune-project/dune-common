// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_MULTIGRID_TRANSFER_HH
#define DUNE_MULTIGRID_TRANSFER_HH

#include <dune/istl/bcrsmatrix.hh>
#include <dune/istl/fmatrix.hh>

namespace Dune {


  /** \brief Restriction and prolongation operator for standard multigrid
   *
   * \todo Currently only works for first-order Lagrangian elements!
   */
  template<class DiscFuncType>
  class MultiGridTransfer {

    enum {blocksize = DiscFuncType::block_type::size};

    /** \todo Should we extract the value type from DiscFuncType? */
    typedef FieldMatrix<double, blocksize, blocksize> MatrixBlock;


  public:

    typedef BCRSMatrix<MatrixBlock> OperatorType;

    /** \brief Sets up the operator between two given function spaces
     *
     * It is implicitly assumed that the two function spaces are nested.
     *
     * \param coarseFSpace The coarse grid function space
     * \param fineFSpace The fine grid function space
     */
    template <class FunctionSpaceType>
    void setup(const FunctionSpaceType& coarseFSpace,
               const FunctionSpaceType& fineFSpace);

    /** \brief Restrict a function from the fine onto the coarse grid
     */
    void restrict (const DiscFuncType & f, DiscFuncType &t) const;

    /** \brief Prolong a function from the coarse onto the fine grid
     */
    void prolong(const DiscFuncType& f, DiscFuncType &t) const;

    /** \brief Galerkin assemble a coarse stiffness matrix
     */
    void galerkinRestrict(const OperatorType& fineMat, OperatorType& coarseMat) const;

    /** \brief Direct access to the operator matrix, if you absolutely want it! */
    const OperatorType& getMatrix() const {return matrix_;}

  protected:

    OperatorType matrix_;

  };

}   // end namespace Dune

#include "multigridtransfer.cc"

#endif
