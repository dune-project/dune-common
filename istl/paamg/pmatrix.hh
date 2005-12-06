// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_AMG_PMATRIX_HH
#define DUNE_AMG_PMATRIX_HH

#include <dune/common/helpertemplates.hh>
#include <dune/common/typetraits.hh>
#include <dune/istl/solvercategory.hh>
#include <dune/istl/operators.hh>
#include "pinfo.hh"

namespace Dune
{
  namespace Amg
  {

    /**
     * @brief Matrix together with information about the distribution
     * between the processes.
     *
     */
    template<class M, class IS, class X, class Y>
    class ParallelMatrix : public AssembledLinearOperator<M,X,Y>
    {
    public:
      /** @brief The type of the matrix. */
      typedef M Matrix;
      /**
       * @brief The type of the matrix.
       *
       * For use as an ISTL Operator.
       */
      typedef M matrix_type;
      /** @brief The type of the index set. */
      typedef ParallelInformation<IS> ParallelInformation;

      enum {
        category = SolverCategory::overlapping
      };

      ParallelMatrix(const Matrix& matrix, const ParallelInformation& info)
        : matrix_(&matrix), info_(&info)
      {}


      /**
       * @brief Get the locally stored matrix.
       * @return The locally stored matrix.
       */
      const Matrix& getmat() const
      {
        return *matrix_;
      }
      /**
       * @brief Get the index set that maps global indices to matrix rows.
       *  @return The index set.
       */
      const ParallelInformation& info() const
      {
        return *info_;
      }

      void apply(const X& x, Y&) const
      {
        DUNE_THROW(NotImplemented, "Markus was too lazy to implement this.");
      }

      void applyscaleadd(typename X::field_type a, const X& x, Y&) const
      {
        DUNE_THROW(NotImplemented, "Markus was too lazy to implement this.");
      }
    private:
      /** @brief The local part of the matrix. */
      const Matrix* matrix_;
      /** @brief The index set. */
      const ParallelInformation* info_;
    };

    template<class M, class IS>
    struct ParallelMatrixArgs
    {
      M& matrix;
      ParallelInformation<IS>& info;

      ParallelMatrixArgs(M& m, ParallelInformation<IS>& i)
        : matrix(m), info(i)
      {}
    };

    template<class T>
    class ConstructionTraits;

    template<class M, class IS, class X, class Y>
    class ConstructionTraits<ParallelMatrix<M,IS,X,Y> >
    {
    public:
      typedef const ParallelMatrixArgs<M,IS> Arguments;

      static inline ParallelMatrix<M,IS,X,Y>* construct(Arguments& args)
      {
        return new ParallelMatrix<M,IS,X,Y>(args.matrix, args.info);
      }
    };
  } // end namespace Amg

} //end namespace Dune
#endif
