// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DGMAPPER_HH__
#define __DUNE_DGMAPPER_HH__

#include <dune/fem/common/dofmapperinterface.hh>

namespace Dune {

  //***************************************************************************
  //
  //!  DG Mapper for mapping of local dof numbers to global dof numbers,
  //!  i.e. the entry in the vector of unknowns
  //
  //***************************************************************************
  template <class IndexSetType, int polOrd, int dimRange>
  class DGMapper
    : public DofMapperDefault < DGMapper <IndexSetType,polOrd, dimRange > >
  {
    // index set of grid, i.e. the element indices
    IndexSetType &indexSet_;

    // number of dofs on element
    int numberOfDofs_;
    int level_;
  public:
    //! Constructor
    DGMapper ( IndexSetType &iset , int numDof , int level) :
      indexSet_ (iset), numberOfDofs_ (numDof), level_(level)  {};

    //! return size of function space
    //! see dofmanager.hh for definition of IndexSet, which
    //! is a wrapper for en.index
    int size () const
    {
      // return number of dofs * number of elements
      return (numberOfDofs_ * indexSet_.size( level_ , 0 ));
    }

    //! return boundary type for given boundary id
    //! uses the parameter class BoundaryIdentifierType
    BoundaryType boundaryType ( int id ) const
    {
      // to be expanded for user needs
      return (id < 0) ? Dirichlet : Neumann;
    }

    //! map Entity an local Dof number to global Dof number
    //! see dofmanager.hh for definition of IndexSet, which
    //! is a wrapper for en.index
    template <class EntityType>
    int mapToGlobal (EntityType &en, int localNum ) const
    {
      // indexSet_.index<0>(en,0) return 0 entity of codim the codim 0
      // entitys of en which is the element index
      return ((indexSet_.template index<0>(en,0) * numberOfDofs_) + localNum);
    };

    //** size methods needed by the dofmanager ***
    //! calc new start point for element dof , vertex dof ...
    //not needed here at the moment
    virtual void calcInsertPoints () {};

    //! default implementation if not overlaoded
    virtual int numberOfDofs () const
    {
      return numberOfDofs_;
    }

    //! only called once, if grid was adapted
    virtual int newSize() const
    {
      return this->size();
    }
  };

} // end namespace Dune

#endif
