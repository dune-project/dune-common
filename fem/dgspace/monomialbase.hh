// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE__MONOMIALBASE_HH__
#define __DUNE__MONOMIALBASE_HH__

#include <dune/common/matvec.hh>
#include <dune/common/simplevector.hh>
#include <dune/fem/common/basefunctions.hh>

namespace Dune {

  /** @defgroup MonomialBaseFunctionSet The MonomialBaseFunctionSet
     @ingroup BaseFunctionSet
     The MonomialBaseFunctionSet is an special implementation of the interface
     BaseFunctionSetInterface. This class offers monomial functions as a base.
     Currently only functionspaces mapping from 2D real to real.

     @{
   */

  //*************************************************************************
  //
  //  --MonomialBaseFunctionSet
  //
  //*************************************************************************
  template<class FunctionSpaceType>
  class MonomialBaseFunctionSet
    : public BaseFunctionSetDefault
      <FunctionSpaceType, MonomialBaseFunctionSet<FunctionSpaceType> >
  {
    typedef typename FunctionSpaceType::Domain Domain;
    typedef typename FunctionSpaceType::Range Range;
    enum { DimDomain = FunctionSpaceType::DimDomain };
    enum { DimRange  = FunctionSpaceType::DimRange  };
  public:

    //! Constructor
    MonomialBaseFunctionSet (FunctionSpaceType & fuspace , int polOrder);

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

    void print (std::ostream& s, int baseFunct) const {
      assert(baseFunct < numOfBaseFct_);
      print(s, Phi_[baseFunct]);
    };
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

    //! vector which holds the base function pointers
    SimpleVector<FieldVector<int, DimDomain> > Phi_ ;

    double power(double x, int p) const {
      if (p <= 0)
        return 1.0;
      return x*power(x, p-1);
    }

    inline
    void real_evaluate ( int baseFunct, const FieldVector<deriType, 0> &diffVariable,
                         const Domain & x,  Range & phi ) const;
    inline
    void real_evaluate ( int baseFunct, const FieldVector<deriType, 1> &diffVariable,
                         const Domain & x,  Range & phi ) const;
    inline
    void real_evaluate ( int baseFunct, const FieldVector<deriType, 2> &diffVariable,
                         const Domain & x,  Range & phi ) const;

    void print (std::ostream& s, const FieldVector<int, 2> & pol) const;

  }; // end class MonomialBaseFunctionSet

  // overloading the out stream for printing of monomial bases
  template< class T >
  inline std::ostream&
  operator<< (std::ostream& s, MonomialBaseFunctionSet<T> & m)
  {
    m.print(s);
    return s;
  }

  /** @} end documentation group */

#include "monomialbase.cc"

} // end namespace Dune

#endif
