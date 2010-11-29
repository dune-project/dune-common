// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id: fvector.hh 5262 2008-09-07 09:03:38Z christi $
#ifndef DUNE_FTRAITS_HH
#define DUNE_FTRAITS_HH

#include <complex>
#include "typetraits.hh"
namespace Dune {

  /**
     @addtogroup DenseMatVec
     \brief Type Traits to retrieve the field and the real type of classes

     Type Traits to retrieve the field and the real type of classes
     e.g. that of FieldVector or FieldMatrix
   */
  template<class T>
  struct FieldTraits
  {
    //! export the type representing the field
    typedef typename T::field_type field_type;
    //! export the type representing the real type of the field
    typedef typename T::real_type real_type;
  };

#define ComposeFTraits(p,f,r)                          \
  template<>                                           \
  struct FieldTraits<p>{                               \
    typedef remove_const<f>::type field_type; \
    typedef remove_const<r>::type real_type;  \
  };                                                   \


  ComposeFTraits(double,double,double);
  ComposeFTraits(float,float,float);
  ComposeFTraits(int,int,int);
  ComposeFTraits(long,long,long);
  ComposeFTraits(std::size_t,std::size_t,std::size_t);
  ComposeFTraits(bool,bool,bool);

#undef ComposeFTraits

  template<class T>
  struct FieldTraits<const T>
  {
    typedef typename FieldTraits<T>::field_type field_type;
    typedef typename FieldTraits<T>::real_type real_type;
  };

  template<class T>
  struct FieldTraits< std::complex<T> >
  {
    typedef std::complex<T> field_type;
    typedef T real_type;
  };

} // end namespace Dune

#endif // DUNE_FTRAITS_HH
