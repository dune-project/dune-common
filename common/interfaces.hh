// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_INTERFACES_HH
#define DUNE_INTERFACES_HH

#include "typetraits.hh"

namespace Dune {

  //! An interface class for cloneable objects
  struct Cloneable {

    /** \brief Clones the object
       clone needs to be redefined by an implementation class, with the
       return type covariantly adapted. Remember to
       delete the resulting pointer.
     */
    virtual Cloneable* clone() const = 0;

    /** \brief Destructor */
    virtual ~Cloneable()
    {}

  };

  //! Tagging interface to indicate that Grid provides typedef ObjectStreamType
  struct HasObjectStream {};

  //! Helper template (implicit specialisation if GridImp exports an object
  //! stream
  template <bool hasStream, class GridImp, class DefaultImp>
  struct GridObjectStreamOrDefaultHelper {
    typedef typename GridImp::ObjectStreamType ObjectStreamType;
  };

  //! Helper template (explicit specialisation if GridImp doesn't export an
  //! object stream -> DefaultImplementation is exported)
  template <class GridImp, class DefaultImp>
  struct GridObjectStreamOrDefaultHelper<false, GridImp, DefaultImp> {
    typedef DefaultImp ObjectStreamType;
  };

  //! Template to choose right Object stream type for a given class
  template <class GridImp, class DefaultImp>
  struct GridObjectStreamOrDefault {
    typedef typename GridObjectStreamOrDefaultHelper<
        Conversion<GridImp, HasObjectStream>::exists,
        GridImp,
        DefaultImp>::ObjectStreamType ObjectStreamType;
  };

} // end namespace Dune

#endif
