// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE__ORTHONORMALBASE_HH
#define DUNE__ORTHONORMALBASE_HH

#include <dune/common/matvec.hh>
#include <dune/common/simplevector.hh>
#include <dune/fem/common/basefunctions.hh>

namespace Dune {

  /** @defgroup OrthonormalBaseFunctionSet The OrthonormalBaseFunctionSet
     @ingroup BaseFunctionSet
     The OrthonormalBaseFunctionSet is an special implementation of the interface
     BaseFunctionSetInterface. This class offers orthonormal functions as a base.
     Currently only functionspaces mapping from 2D and 3D real to real.

     @{
   */

  //*************************************************************************
  //
  //  --OrthonormalBaseFunctionSet
  //
  //*************************************************************************

  //! \todo Please doc me!
  template<class FunctionSpaceType>
  class OrthonormalBaseFunctionSet
    : public BaseFunctionSetDefault
      <FunctionSpaceType, OrthonormalBaseFunctionSet<FunctionSpaceType> >
  {
    typedef typename FunctionSpaceType::Domain Domain;
    typedef typename FunctionSpaceType::DomainField DomainField;
    typedef typename FunctionSpaceType::Range Range;
    typedef typename FunctionSpaceType::JacobianRange JacobianRange;
    enum { DimDomain = FunctionSpaceType::DimDomain };
    enum { DimRange  = FunctionSpaceType::DimRange  };
  public:

    //! Constructor
    OrthonormalBaseFunctionSet (FunctionSpaceType & fuspace, int polOrder);

    //! change type
    void changeType(ElementType & type)
    {
      type_ = type;
    }

    //! return the number of base functions for this BaseFunctionSet
    int getNumberOfBaseFunctions () const
    {
      return numOfBaseFct_;
    };

    //! evaluate base function baseFunct with the given diffVariable and a
    //! point x and range phi
    template <int diffOrd>
    inline
    void evaluate ( int baseFunct, const FieldVector<deriType, diffOrd> &diffVariable,
                    const Domain & x,  Range & phi ) const;

    //! evaluate base fucntion baseFunct at a given quadrature point
    //! the identifier of the quadrature is stored to check , whether the
    //! qaudrature has changed an the values at the quadrature have to be
    //! calulated again
    template <int diffOrd, class QuadratureType>
    inline
    void evaluate ( int baseFunct, const FieldVector<deriType, diffOrd> &diffVariable,
                    QuadratureType & quad, int quadPoint, Range & phi ) const;

    //! evaluate the gradient
    void jacobian ( int baseFunct, const Domain & x, JacobianRange & grad ) const;

    //! \todo Please doc me!
    void print (std::ostream& s, int baseFunct) const {
      s << "print is missing\n";
    };
    //! \todo Please doc me!
    void print (std::ostream& s) const {
      s << "[";
      for (int i = 0; i < numOfBaseFct_ - 1; i++) {
        s << "["; print(s, i); s << "], ";
      }
      s << "["; print(s, numOfBaseFct_); s << "]]";
    }

  private:
    //! polynomial order of the base
    int polOrder_;

    //! number of different basefunctions
    int numOfBaseFct_;

    //! Elementtype for which this basis is orthonormal
    ElementType type_;

    //! eval
    inline void real_evaluate ( int baseFunct,
                                const FieldVector<deriType, 0> &diffVariable,
                                const Domain & x,
                                Range & phi ) const;

    //! grad
    inline void real_evaluate ( int baseFunct,
                                const FieldVector<deriType, 1> &diffVariable,
                                const Domain & x,
                                Range & phi ) const;

    double eval_triangle_2d ( int i, const Domain & xi ) const;
    double eval_quadrilateral_2d ( int i, const Domain & xi ) const;
    double eval_tetrahedron_3d ( int i, const Domain & xi ) const;
    double eval_pyramid_3d ( int i, const Domain & xi ) const;
    double eval_prism_3d ( int i, const Domain & xi ) const;
    double eval_hexahedron_3d ( int i, const Domain & xi ) const;

    void grad_triangle_2d ( int i, const Domain & xi,
                            JacobianRange & grad ) const;
    void grad_quadrilateral_2d ( int i, const Domain & xi,
                                 JacobianRange & grad ) const;
    void grad_tetrahedron_3d ( int i, const Domain & xi,
                               JacobianRange & grad ) const;
    void grad_pyramid_3d ( int i, const Domain & xi,
                           JacobianRange & grad ) const;
    void grad_prism_3d ( int i, const Domain & xi,
                         JacobianRange & grad ) const;
    void grad_hexahedron_3d ( int i, const Domain & xi,
                              JacobianRange & grad ) const;

  }; // end class OrthonormalBaseFunctionSet

  //! overloading the out stream for printing of monomial bases
  template< class T >
  inline std::ostream&
  operator<< (std::ostream& s, OrthonormalBaseFunctionSet<T> & m)
  {
    m.print(s);
    return s;
  }

  /** @} end documentation group */

#include "orthonormalbase.cc"
#include "orthonormalbase_impl.cc"

} // end namespace Dune

#endif // DUNE_ORTHONORMALBASE_HH
