// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DISCRETE_FUNCTION_CC__
#define __DUNE_DISCRETE_FUNCTION_CC__

namespace Dune
{


  //************************************************************
  //  Default Implementations
  //************************************************************
  // scalarProductDofs
  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp, class DiscreteFunctionImp >
  inline DiscreteFunctionSpaceType::RangeField
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , DiscreteFunctionImp >::
  scalarProductDofs( const DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
                         DofIteratorImp , DiscreteFunctionImp > &g )
  {
    typedef DiscreteFunctionSpaceType::RangeField RangeFieldType;
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType , DofIteratorImp , DiscreteFunctionImp >
    DiscreteFunctionDefaultType ;

    RangeFieldType skp = 0.;

    int level = getFunctionSpace().getGrid().maxlevel();

    // get DofIterator from this
    DofIteratorImp endit = dend ( level );

    // hack
    DofIteratorImp git   = const_cast<DiscreteFunctionDefault &>( g ).dbegin ( level );
    // multiply
    for(DofIteratorImp it = dbegin( level ); it != endit; ++it)
    {
      skp += (*it) * (*git);
      ++git;
    }
    return skp;
  }

  // assign
  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp, class DiscreteFunctionImp >
  inline Vector< DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , DiscreteFunctionImp >::
  assign( const Vector< DiscreteFunctionSpaceType::RangeField > & g )
  {
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , DiscreteFunctionImp > DiscreteFunctionDefaultType;

    DiscreteFunctionDefaultType &gc =
      const_cast<DiscreteFunctionDefaultType &>( static_cast<const DiscreteFunctionDefaultType &> ( g ));
    // we would need const_iterators.....

    int level = getFunctionSpace().getGrid().maxlevel();

    DofIteratorImp endit = dend ( level );
    DofIteratorImp git = gc.dbegin ( level );
    for(DofIteratorImp it = dbegin( level ); it != endit; ++it)
    {
      *it = *git;
      ++git;
    }

    return *this;
  }

  // operator =
  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp, class DiscreteFunctionImp >
  inline Vector< DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , DiscreteFunctionImp >::
  operator = ( const Vector< DiscreteFunctionSpaceType::RangeField > & g )
  {
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , DiscreteFunctionImp > DiscreteFunctionDefaultType;

    DiscreteFunctionDefaultType &gc =
      const_cast<DiscreteFunctionDefaultType &>( static_cast<const DiscreteFunctionDefaultType &> ( g ));
    // we would need const_iterators.....

    int level = getFunctionSpace().getGrid().maxlevel();

    DofIteratorImp endit = dend ( level );
    DofIteratorImp git = gc.dbegin ( level );
    for(DofIteratorImp it = dbegin( level ); it != endit; ++it)
    {
      *it = *git;
      ++git;
    }

    return *this;
  }

  // operator +=
  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp, class DiscreteFunctionImp >
  inline Vector< DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , DiscreteFunctionImp >::
  operator += ( const Vector< DiscreteFunctionSpaceType::RangeField > & g )
  {
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , DiscreteFunctionImp > DiscreteFunctionDefaultType;

    DiscreteFunctionDefaultType &gc =
      const_cast<DiscreteFunctionDefaultType &>( static_cast<const DiscreteFunctionDefaultType &> ( g ));
    // we would need const_iterators.....

    int level = getFunctionSpace().getGrid().maxlevel();

    DofIteratorImp endit = dend ( level );
    DofIteratorImp git = gc.dbegin ( level );
    for(DofIteratorImp it = dbegin( level ); it != endit; ++it)
    {
      *it += *git;
      ++git;
    }
    return *this;
  }

  // operator -=
  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp, class DiscreteFunctionImp >
  inline Vector< DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , DiscreteFunctionImp >::
  operator -= ( const Vector< DiscreteFunctionSpaceType::RangeField > & g )
  {
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , DiscreteFunctionImp > DiscreteFunctionDefaultType;

    DiscreteFunctionDefaultType &gc =
      const_cast<DiscreteFunctionDefaultType &>( static_cast<const DiscreteFunctionDefaultType &> ( g ));
    // we would need const_iterators.....

    int level = getFunctionSpace().getGrid().maxlevel();

    DofIteratorImp endit = dend ( level );
    DofIteratorImp git = gc.dbegin ( level );
    for(DofIteratorImp it = dbegin( level ); it != endit; ++it)
    {
      *it -= *git;
      ++git;
    }
    return *this;
  }

  // operator *=
  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp, class DiscreteFunctionImp >
  inline Vector< DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , DiscreteFunctionImp >::
  operator *= ( const DiscreteFunctionSpaceType::RangeField & scalar )
  {
    int level = getFunctionSpace().getGrid().maxlevel();

    DofIteratorImp endit = dend ( level );
    for(DofIteratorImp it = dbegin( level ); it != endit; ++it)
      *it *= scalar;

    return *this;
  }

  // operator /=
  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp, class DiscreteFunctionImp >
  inline Vector< DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , DiscreteFunctionImp >::
  operator /= ( const DiscreteFunctionSpaceType::RangeField & scalar )
  {
    (*this) *= (1./scalar);
    return *this;
  }


  // add
  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp, class DiscreteFunctionImp >
  inline Vector< DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , DiscreteFunctionImp >::
  add ( const Vector< DiscreteFunctionSpaceType::RangeField > & g ,
        DiscreteFunctionSpaceType::RangeField scalar )
  {
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , DiscreteFunctionImp > DiscreteFunctionDefaultType;

    DiscreteFunctionDefaultType &gc =
      const_cast<DiscreteFunctionDefaultType &>( static_cast<const DiscreteFunctionDefaultType &> ( g ));
    // we would need const_iterators.....

    int level = getFunctionSpace().getGrid().maxlevel();

    DofIteratorImp endit = dend ( level );
    DofIteratorImp git = gc.dbegin ( level );
    for(DofIteratorImp it = dbegin( level ); it != endit; ++it)
    {
      *it += (*git) * scalar;
      ++git;
    }
    return *this;
  }

  // clear
  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp, class DiscreteFunctionImp >
  inline void DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , DiscreteFunctionImp >:: clearLevel ( int level  )
  {
    DofIteratorImp enddof = dend ( level_ );
    for(DofIteratorImp itdof = dbegin ( level_ );
        itdof != enddof; ++itdof)
    {
      *itdof = 0.;
    }
  }

} // end namespace Dune

#endif
