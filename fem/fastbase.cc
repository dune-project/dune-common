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

template <class FunctionSpaceType> template <int diffOrd>
void FastBaseFunctionSet<FunctionSpaceType >::
evaluate( int baseFunct, const Vec<diffOrd,char> &diffVariable, QuadratureType & quad,
          int quadPoint, Range & phi ) const {
  if ( quad.getIdentifier() != evaluateQuad_[ diffOrd ] ) {
    evaluateInit( quad, diffOrd );
  }

  return vecEvaluate_[diffOrd][ index( baseFunc, diffVariable, quadPoint, quad.getNumberOfQuadPoints()) ];
}



template <class FunctionSpaceType>
template <int diffOrd>
void FastBaseFunctionSet<FunctionSpaceType >::
evaluateInit( QuadratureType &quad, Vec<diffOrd,char> &diffVariable ) {

  int p = 1;
  for ( int i = 0; i < diffOrd; i++ ) { p *= dimDomain; }

  vecEvaluate_[ diffOrd ].resize( p * baseFunctionList_.size() * quad.getNumberOfQuadPoints () );

  int count = index( 0, diffVariable, 0, quad.getNumberOfQuadPoints ());
  for ( int baseFunc = 0; baseFunc < baseFunctionList_.size(); baseFunc++ ) {
    for ( int quadPt = 0; quadPt < quad.getNumberOfQuadPoints (); quadPt++ ) {
      getBaseFunction( baseFunc ).evaluate( diffVariable, quad.getQuadraturePoints( quadPt ),
                                            vecEvaluate_[ diffOrd ][ count++ ] );
    }
  }
}
