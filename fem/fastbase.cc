// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

//template <class FunctionSpaceType, class FastBaseFunctionSetImp>
//FastBaseFunctionSet<FunctionSpaceType, FastBaseFunctionSetImp >::

template <class FunctionSpaceType>
FastBaseFunctionSet<FunctionSpaceType >::
FastBaseFunctionSet( FunctionSpaceType & fuspace, int numOfBaseFct )
  : BaseFunctionSetInterface<FunctionSpaceType,FastBaseFunctionSet <FunctionSpaceType > > ( fuspace ),
    vecEvaluate_( numDiffOrd ) {
  for(int i=0; i<numDiffOrd; i++)
    evaluateQuad_[i] = DynamicType::undefined;

  baseFunctionList_.resize( numOfBaseFct );

  for(int i=0; i<numOfBaseFct; i++)
    baseFunctionList_[i] = NULL;

}

template <class FunctionSpaceType> template <int diffOrd>
void FastBaseFunctionSet<FunctionSpaceType >::
evaluate( int baseFunct, const Vec<diffOrd,char> &diffVariable, const Domain & x,  Range & phi ) const
{
  std::cout << "FastBaseFunctionSet::evaluate \n";
  getBaseFunction( baseFunct ).evaluate( diffVariable, x, phi );
}

template <class FunctionSpaceType> template <int diffOrd, class QuadratureType>
void FastBaseFunctionSet<FunctionSpaceType >::
evaluate( int baseFunct, const Vec<diffOrd,char> &diffVariable, QuadratureType & quad,
          int quadPoint, Range & phi ) const
{
  // check if cache is for the used quadrature
  if ( quad.getIdentifier() != evaluateQuad_[ diffOrd ] )
  {
    std::cout << "call evaluateInit ()! \n";
    const_cast<FastBaseFunctionSet<FunctionSpaceType >& > (*this).evaluateInit( diffVariable , quad);
  }

  //! copy the value to phi
  phi = (vecEvaluate_[diffOrd])[ index( baseFunct, diffVariable, quadPoint, quad.getNumberOfQuadPoints()) ];
}


//! if the quadrature is new, then once cache the values
template <class FunctionSpaceType>
template <int diffOrd, class QuadratureType>
void FastBaseFunctionSet<FunctionSpaceType >::
evaluateInit( const Vec<diffOrd,char> &diffVariable,
              const QuadratureType &quad )
{
  int p = 1;
  for ( int i = 0; i < diffOrd; i++ ) { p *= DimDomain; }

  // resize the cache vector if nessesary
  vecEvaluate_[ diffOrd ].resize( p * baseFunctionList_.size() * quad.getNumberOfQuadPoints () );

  // cache the basefunction evaluation
  int count = index( 0, diffVariable, 0, quad.getNumberOfQuadPoints ());
  for ( int baseFunc = 0; baseFunc < baseFunctionList_.size(); baseFunc++ ) {
    for ( int quadPt = 0; quadPt < quad.getNumberOfQuadPoints (); quadPt++ ) {
      getBaseFunction( baseFunc ).evaluate( diffVariable, quad.getQuadraturePoint ( quadPt ),
                                            vecEvaluate_[ diffOrd ][ count++ ] );
    }
  }

  // remember the cached quadrature
  evaluateQuad_[ diffOrd ] = quad.getIdentifier();
}
