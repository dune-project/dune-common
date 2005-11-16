// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_MULTIGRID_TRANSFER_HH
#define DUNE_MULTIGRID_TRANSFER_HH

#include <dune/istl/bcrsmatrix.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/bitfield.hh>

// for backward compatibility
#include <dune/fem/feop/spmatrix.hh>

namespace Dune {


  /** \brief Restriction and prolongation operator for standard multigrid
   *
   * This class implements the standard prolongation and restriction
   * operators for standard multigrid solvers.  Restriction and prolongation
   * of block vectors is provided.  Internally, the operator is stored
   * as a BCRSMatrix.  Therefore, the template parameter DiscFuncType
   * has to comply with the ISTL requirements.

   * \todo Currently only works for first-order Lagrangian elements!
   */
  template<class DiscFuncType>
  class MultiGridTransfer {

    enum {blocksize = DiscFuncType::block_type::size};

    /** \todo Should we extract the value type from DiscFuncType? */
    typedef FieldMatrix<double, blocksize, blocksize> MatrixBlock;


  public:

    typedef BCRSMatrix<MatrixBlock> OperatorType;

    virtual ~MultiGridTransfer() {}

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

    /** \brief Sets up the operator between two P1 spaces
     *
     * Does not use any of the Freiburg fem stuff
     * \param grid The grid
     * \param cL The coarse grid level
     * \param fL The fine grid level
     */
    template <class GridType>
    void setup(const GridType& grid, int cL, int fL);

    /** \brief Restrict a function from the fine onto the coarse grid
     */
    virtual void restrict (const DiscFuncType & f, DiscFuncType &t) const;

    /** \brief Restrict a bitfield from the fine onto the coarse grid
     */
    virtual void restrict (const BitField & f, BitField& t) const;

    /** \brief Prolong a function from the coarse onto the fine grid
     */
    virtual void prolong(const DiscFuncType& f, DiscFuncType &t) const;

    /** \brief Galerkin assemble a coarse stiffness matrix
     */
    virtual void galerkinRestrict(const OperatorType& fineMat, OperatorType& coarseMat) const;

    /** \brief Direct access to the operator matrix, if you absolutely want it! */
    virtual const OperatorType& getMatrix() const {return matrix_;}

    /** \brief Galerkin assemble a coarse stiffness matrix

       \deprecated Only exists for backward compatibility
     */
    SparseRowMatrix<double> galerkinRestrict(const SparseRowMatrix<double>& fineMat) const;

    /** \brief Restrict a DiscFuncArray from the fine onto the coarse grid
       \deprecated Only exists for backward compatibility
     */
    void restrictDFA(const DiscFuncType& f, DiscFuncType &t) const;

    /** \brief Prolong a DiscFuncArray from the coarse onto the fine grid
       \deprecated Only exists for backward compatibility
     */
    void prolongDFA(const DiscFuncType& f, DiscFuncType &t) const;

  protected:

    OperatorType matrix_;

  };

}   // end namespace Dune

#include "multigridtransfer.cc"

#endif
