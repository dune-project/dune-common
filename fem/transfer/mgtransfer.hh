// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_MG_TRANSFER_HH__
#define __DUNE_MG_TRANSFER_HH__

#include <dune/fem/feop/spmatrix.hh>

namespace Dune {

  /** \brief Standard multigrid restriction and prolongation operator
   *
   * Currently only works for first-order Lagrangian elements!
   */
  template<class DiscFuncType>
  class MGTransfer {

    typedef typename DiscFuncType::FunctionSpaceType FunctionSpaceType;

  public:

    /** \brief Sets up the operator between levels cL and fL
     *
     * \param fS The function space hierarchy between levels of which we're mapping
     * \param cL The coarse level
     * \param fL The fine level
     */
    void setup(const FunctionSpaceType& fS, int cL, int fL);

    /** \brief Restrict level fL of f and store the result in level cL of t
     */
    void restrict (const DiscFuncType & f, DiscFuncType &t) const;

    /** \brief Prolong level cL of f and store the result in level fL of t
     */
    void prolong(const DiscFuncType& f, DiscFuncType &t) const;

    /** \brief Galerkin assemble a coarse stiffness matrix
     */
    SparseRowMatrix<double> galerkinRestrict(const SparseRowMatrix<double>& fineMat) const;

    const SparseRowMatrix<double>& getMatrix() const {return matrix_;}

  protected:
    int coarseLevel;

    int fineLevel;

    /** \todo Should we extract the value type from DiscFuncType? */
    SparseRowMatrix<double> matrix_;

  };

}

#include "mgtransfer.cc"

#endif
