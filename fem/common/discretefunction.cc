// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DISCRETEFUNCTION_CC
#define DUNE_DISCRETEFUNCTION_CC

#include <fstream>
#include <dune/io/file/asciiparser.hh>

namespace Dune
{

  //************************************************************
  //  Default Implementations
  //************************************************************
  template <class DiscreteFunctionTraits>
  void DiscreteFunctionDefault<DiscreteFunctionTraits>::clear() {
    DofIteratorType endit = this->dend();
    for (DofIteratorType it = this->dbegin(); it != endit; ++it) {
      *it = 0.0;
    }
  }

  template <class DiscreteFunctionTraits>
  void DiscreteFunctionDefault<DiscreteFunctionTraits>::
  addScaled(const DiscreteFunctionType& g, const RangeFieldType& c) {
    DofIteratorType endit = this->dend();
    ConstDofIteratorType oit = g.dbegin();
    for (DofIteratorType it = this->dbegin(); it != endit; ++it, ++oit) {
      *it += *oit*c;
    }
  }

  // scalarProductDofs
  template <class DiscreteFunctionTraits>
  inline typename DiscreteFunctionTraits::DiscreteFunctionSpaceType::RangeFieldType
  DiscreteFunctionDefault<DiscreteFunctionTraits>::
  scalarProductDofs(const DiscreteFunctionType& g) const
  {
    typedef typename DiscreteFunctionSpaceType::RangeFieldType RangeFieldType;
    RangeFieldType skp = 0.;

    ConstDofIteratorType endit = this->dend ();
    ConstDofIteratorType git =  g.dbegin ();

    // multiply
    for(ConstDofIteratorType it = this->dbegin(); it != endit; ++it,++git)
    {
      skp += (*it) * (*git);
    }

    return skp;
  }

  // operator=
  template<class DiscreteFunctionTraits>
  DiscreteFunctionDefault<DiscreteFunctionTraits >&
  DiscreteFunctionDefault<DiscreteFunctionTraits >::
  assign(const MappingType& g)
  {
    typedef DiscreteFunctionDefault<
        DiscreteFunctionTraits
        > DiscreteFunctionDefaultType;

    const DiscreteFunctionDefaultType &gc =
      static_cast<const DiscreteFunctionDefaultType &> ( g );

    DofIteratorType endit = this->dend ();
    ConstDofIteratorType git = gc.dbegin ();
    for (DofIteratorType it = this->dbegin(); it != endit; ++it, ++git) {
      *it = *git;
    }
    return *this;
  }


  // operator +=
  /** \todo This operator can add a discretefunction defined on all levels to another
   * one defined only on one level.  We should somehow issue a warning in this case.
   */
  template<class DiscreteFunctionTraits>
  DiscreteFunctionDefault<DiscreteFunctionTraits >&
  DiscreteFunctionDefault<DiscreteFunctionTraits >::
  operator += (const MappingType& g)
  {
    typedef DiscreteFunctionDefault<
        DiscreteFunctionTraits
        > DiscreteFunctionDefaultType;

    const DiscreteFunctionDefaultType &gc =
      static_cast<const DiscreteFunctionDefaultType &> ( g );

    DofIteratorType endit = this->dend ();
    ConstDofIteratorType git = gc.dbegin ();
    for(DofIteratorType it = this->dbegin(); it != endit; ++it, ++git)
    {
      *it += *git;
    }
    return *this;
  }


  // operator -=
  template<class DiscreteFunctionTraits>
  DiscreteFunctionDefault<DiscreteFunctionTraits> &
  DiscreteFunctionDefault<DiscreteFunctionTraits >::
  operator -= ( const MappingType& g )
  {
    typedef DiscreteFunctionDefault<
        DiscreteFunctionTraits
        > DiscreteFunctionDefaultType;

    // cast to class discrete functions
    const DiscreteFunctionDefaultType &gc =
      static_cast<const DiscreteFunctionDefaultType &> ( g );

    DofIteratorType endit = this->dend ();
    ConstDofIteratorType git = gc.dbegin ();
    for(DofIteratorType it = this->dbegin(); it != endit; ++it, ++git)
    {
      *it -= *git;
    }
    return asImp();
  }

  // operator *=
  template<class DiscreteFunctionTraits >
  inline DiscreteFunctionDefault<DiscreteFunctionTraits>&
  DiscreteFunctionDefault<DiscreteFunctionTraits >::
  operator*=(const typename DiscreteFunctionSpaceType::RangeFieldType & scalar)
  {
    DofIteratorType endit = this->dend ();
    for(DofIteratorType it = this->dbegin(); it != endit; ++it)
      *it *= scalar;

    return *this;
  }

  // operator /=
  template<class DiscreteFunctionTraits>
  inline DiscreteFunctionDefault<DiscreteFunctionTraits > &
  DiscreteFunctionDefault<DiscreteFunctionTraits >::
  operator/=(const typename DiscreteFunctionSpaceType::RangeFieldType & scalar)
  {
    (*this) *= (1./scalar);
    return *this;
  }


  // add
  template<class DiscreteFunctionTraits >
  typename DiscreteFunctionTraits::DiscreteFunctionType&
  DiscreteFunctionDefault<DiscreteFunctionTraits >::
  add(const DiscreteFunctionType& g, RangeFieldType scalar)
  {
    typedef DiscreteFunctionDefault<
        DiscreteFunctionTraits
        > DiscreteFunctionDefaultType;

    const DiscreteFunctionDefaultType &gc =
      static_cast<const DiscreteFunctionDefaultType &> ( g );
    // we would need const_iterators.....

    DofIteratorType endit = this->dend ();
    ConstDofIteratorType git = gc.dbegin ();
    for(DofIteratorType it = this->dbegin(); it != endit; ++it, ++git)
    {
      *it += (*git) * scalar;
    }
    return asImp();
  }

} // end namespace Dune

#endif
