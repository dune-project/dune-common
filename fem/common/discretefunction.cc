// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DISCRETE_FUNCTION_CC__
#define __DUNE_DISCRETE_FUNCTION_CC__

#include <fstream>
#include <dune/io/file/asciiparser.hh>

namespace Dune
{


  //************************************************************
  //  Default Implementations
  //************************************************************
  // scalarProductDofs
  template<class DiscreteFunctionSpaceType, class DofIteratorImp,
      template <class,class> class LocalFunctionIteratorImp, class DiscreteFunctionImp >
  inline typename DiscreteFunctionSpaceType::RangeField
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , LocalFunctionIteratorImp,DiscreteFunctionImp >::
  scalarProductDofs( const DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
                         DofIteratorImp ,LocalFunctionIteratorImp, DiscreteFunctionImp > &g ) const
  {
    typedef typename DiscreteFunctionSpaceType::RangeField RangeFieldType;
    typedef ConstDofIteratorDefault<DofIteratorImp> ConstDofIter;

    RangeFieldType skp = 0.;

    ConstDofIter endit = this->dend ();
    ConstDofIter git =  g.dbegin ();

    // multiply
    for(ConstDofIter it = this->dbegin(); it != endit; ++it,++git)
    {
      skp += (*it) * (*git);
    }

    return skp;
  }


  // assign on maxlevel
  template<class DiscreteFunctionSpaceType, class DofIteratorImp,
      template <class,class> class LocalFunctionIteratorImp, class DiscreteFunctionImp >
  inline Vector< typename DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , LocalFunctionIteratorImp, DiscreteFunctionImp >::
  assign( const Vector< typename DiscreteFunctionSpaceType::RangeField > & g )
  {
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , LocalFunctionIteratorImp, DiscreteFunctionImp > DiscreteFunctionDefaultType;

    const DiscreteFunctionDefaultType &gc =
      static_cast<const DiscreteFunctionDefaultType &> ( g );

    DofIteratorImp it = this->dbegin();
    DofIteratorImp endit = this->dend ();
    ConstDofIteratorDefault<DofIteratorImp> git = gc.dbegin ();

    for(; it != endit; ++it, ++git)
      *it = *git;

    return *this;
  }


  // operator =
  template<class DiscreteFunctionSpaceType, class DofIteratorImp,
      template <class,class> class LocalFunctionIteratorImp, class DiscreteFunctionImp >
  inline Vector< typename DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , LocalFunctionIteratorImp,DiscreteFunctionImp >::
  operator = ( const Vector< typename DiscreteFunctionSpaceType::RangeField > & g )
  {
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , LocalFunctionIteratorImp, DiscreteFunctionImp > DiscreteFunctionDefaultType;

    const DiscreteFunctionDefaultType &gc =
      static_cast<const DiscreteFunctionDefaultType &> ( g );

    DofIteratorImp endit = this->dend ();
    ConstDofIteratorDefault<DofIteratorImp> git = gc.dbegin ();

    for(DofIteratorImp it = this->dbegin(); it != endit; ++it,++git )
    {
      *it = *git;
    }
    return *this;
  }

  // operator +=
  /** \todo This operator can add a discretefunction defined on all levels to another
   * one defined only on one level.  We should somehow issue a warning in this case.
   */
  template<class DiscreteFunctionSpaceType, class DofIteratorImp,
      template <class,class> class LocalFunctionIteratorImp, class DiscreteFunctionImp >
  inline Vector< typename DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , LocalFunctionIteratorImp,DiscreteFunctionImp >::
  operator += ( const Vector< typename DiscreteFunctionSpaceType::RangeField > & g )
  {
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , LocalFunctionIteratorImp, DiscreteFunctionImp > DiscreteFunctionDefaultType;

    const DiscreteFunctionDefaultType &gc =
      static_cast<const DiscreteFunctionDefaultType &> ( g );

    DofIteratorImp endit = this->dend ();
    ConstDofIteratorDefault<DofIteratorImp> git = gc.dbegin ();
    for(DofIteratorImp it = this->dbegin(); it != endit; ++it, ++git)
    {
      *it += *git;
    }
    return *this;
  }


  // operator -=
  template<class DiscreteFunctionSpaceType, class DofIteratorImp,
      template <class,class> class LocalFunctionIteratorImp, class DiscreteFunctionImp >
  inline Vector< typename DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , LocalFunctionIteratorImp,DiscreteFunctionImp >::
  operator -= ( const Vector< typename DiscreteFunctionSpaceType::RangeField > & g )
  {
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , LocalFunctionIteratorImp, DiscreteFunctionImp > DiscreteFunctionDefaultType;

    // cast to class discrete functions
    const DiscreteFunctionDefaultType &gc =
      static_cast<const DiscreteFunctionDefaultType &> ( g );

    DofIteratorImp endit = this->dend ();
    ConstDofIteratorDefault<DofIteratorImp> git = gc.dbegin ();
    for(DofIteratorImp it = this->dbegin(); it != endit; ++it, ++git)
    {
      *it -= *git;
    }
    return *this;
  }

  // operator *=
  template<class DiscreteFunctionSpaceType, class DofIteratorImp,
      template <class,class> class LocalFunctionIteratorImp, class DiscreteFunctionImp >
  inline Vector< typename DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , LocalFunctionIteratorImp,DiscreteFunctionImp >::
  operator *= ( const typename DiscreteFunctionSpaceType::RangeField & scalar )
  {
    DofIteratorImp endit = this->dend ();
    for(DofIteratorImp it = this->dbegin(); it != endit; ++it)
      *it *= scalar;

    return *this;
  }

  // operator /=
  template<class DiscreteFunctionSpaceType, class DofIteratorImp,
      template <class,class> class LocalFunctionIteratorImp, class DiscreteFunctionImp >
  inline Vector< typename DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , LocalFunctionIteratorImp,DiscreteFunctionImp >::
  operator /= ( const typename DiscreteFunctionSpaceType::RangeField & scalar )
  {
    (*this) *= (1./scalar);
    return *this;
  }


  // add
  template<class DiscreteFunctionSpaceType, class DofIteratorImp,
      template <class,class> class LocalFunctionIteratorImp, class DiscreteFunctionImp >
  inline Vector< typename DiscreteFunctionSpaceType::RangeField > &
  DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
      DofIteratorImp , LocalFunctionIteratorImp,DiscreteFunctionImp >::
  add ( const Vector< typename DiscreteFunctionSpaceType::RangeField > & g ,
        typename DiscreteFunctionSpaceType::RangeField scalar )
  {
    typedef DiscreteFunctionDefault<DiscreteFunctionSpaceType ,
        DofIteratorImp , LocalFunctionIteratorImp, DiscreteFunctionImp > DiscreteFunctionDefaultType;

    const DiscreteFunctionDefaultType &gc =
      static_cast<const DiscreteFunctionDefaultType &> ( g );
    // we would need const_iterators.....

    DofIteratorImp endit = this->dend ();
    ConstDofIteratorDefault<DofIteratorImp> git = gc.dbegin ();
    for(DofIteratorImp it = this->dbegin(); it != endit; ++it, ++git)
    {
      *it += (*git) * scalar;
    }
    return *this;
  }


} // end namespace Dune

#endif
