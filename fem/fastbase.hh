// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_FASTBASE_HH__
#define __DUNE_FASTBASE_HH__

#include "basefunctions.hh"

namespace Dune {

  template<class FunctionSpaceType>
  class FastBaseFunctionSet :
    public BaseFunctionSetInterface<FunctionSpaceType, FastBaseFunctionSet<FunctionSpaceType> >
  {

  public:
    typedef BaseFunctionInterface < FunctionSpaceType > BaseFunctionInterfaceType;

    FastBaseFunctionSet ( FunctionSpaceType & fuspace , int numOfBaseFct);

    template <int diffOrd>
    void evaluate ( int baseFunct, const Vec<diffOrd,char> &diffVariable, Domain & x,  Range & phi ) const;

    template <int diffOrd>
    void evaluate ( int baseFunct, const Vec<diffOrd,char> &diffVariable, QuadratureType & quad,
                    int quadPoint, Range & phi ) const;
    const BaseFunctionInterface<FunctionSpaceType> &getBaseFunction( int baseFunct ) const {
      std::cout << "getBaseFunction \n";
      return (*baseFunctionList_[baseFunct]);
    }

    enum { numDiffOrd = 3 };

  protected:
    void setBaseFunctionPointer ( int baseFunc, BaseFunctionInterfaceType * func) {
      baseFunctionList_[baseFunc] = func;
    };

  private:
    std::vector< BaseFunctionInterfaceType *  > baseFunctionList_ ;

    template <int diffOrd>
    int index( int baseFunc, const Vec<diffOrd,char> &diffVariable, int quadPt, int numQuadPoints ) const {
      int n = 0;
      for ( int i = 0; i < diffOrd; i++ ) {
        n = diffVariable(i) + n * dimDomain;
      }
      return numQuadPoints*(getNumberOfBaseFunctions()*n + baseFunc) + quadPt;
    };


    std::vector<std::vector<Range> > vecEvaluate_;
    IdentifierType evaluateQuad_[ numDiffOrd ];

    template <int diffOrd>
    void evaluateInit ( QuadratureType & quad, Vec<diffOrd,char> & diffVariable ) ;
  };


#include "fastbase.cc"
}

#endif
