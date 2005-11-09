// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DGMAPPER_HH
#define DUNE_DGMAPPER_HH

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
  public:
    //! Constructor
    DGMapper(IndexSetType& iset , int numDof) :
      indexSet_ (iset), numberOfDofs_ (numDof*dimRange)  {};

    //! return size of function space
    //! see dofmanager.hh for definition of IndexSet, which
    //! is a wrapper for en.index
    int size () const
    {
      // return number of dofs * number of elements
      return (numberOfDofs_ * indexSet_.size( 0 ));
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
    void calcInsertPoints () {};

    //! default implementation if not overlaoded
    int numDofs () const
    {
      return numberOfDofs_;
    }

    //! only called once, if grid was adapted
    int newSize() const
    {
      return this->size();
    }

    //! for dof manager, to check whether it has to copy dof or not
    bool indexNew (int num) const
    {
      const int newn = static_cast<int> (num / numberOfDofs_);
      return indexSet_.template indexNew(newn,0);
    }

    //! return old index, for dof manager only
    int oldIndex (int num) const
    {
      // corresponding number of set is newn
      const int newn  = static_cast<int> (num / numberOfDofs_);
      // local number of dof is local
      const int local = (num % numberOfDofs_);
      return (numberOfDofs_ * indexSet_.oldIndex(newn,0)) + local;
    }

    //! return new index, for dof manager only
    int newIndex (int num) const
    {
      // corresponding number of set is newn
      const int newn = static_cast<int> (num / numberOfDofs_);
      // local number of dof is local
      const int local = (num % numberOfDofs_);
      return (numberOfDofs_ * indexSet_.newIndex(newn,0)) + local;
    }

    //! return size of grid entities per level and codim
    //! for dof mapper
    int oldSize () const
    {
      // this index set works only for codim = 0 at the moment
      return numberOfDofs_ * indexSet_.oldSize(0);
    }

    // is called once and calcs the insertion points too
    int additionalSizeEstimate () const
    {
      return numberOfDofs_ * indexSet_.additionalSizeEstimate();
    }
  };


  // specialisation for dimRange = 1
  template <class IndexSetType, int polOrd>
  class DGMapper<IndexSetType,polOrd,1>
    : public DofMapperDefault < DGMapper <IndexSetType,polOrd,1> >
  {
    // index set of grid, i.e. the element indices
    IndexSetType &indexSet_;

    // number of dofs on element
    int numberOfDofs_;
  public:
    //! Constructor
    DGMapper ( IndexSetType &iset , int numDof) :
      indexSet_ (iset), numberOfDofs_ (numDof) {}

    //! return size of function space
    //! see dofmanager.hh for definition of IndexSet, which
    //! is a wrapper for en.index
    int size () const
    {
      // return number of dofs * number of elements
      return (numberOfDofs_ * indexSet_.size( 0 ));
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
    void calcInsertPoints () {};

    //! default implementation if not overlaoded
    int numDofs () const
    {
      return numberOfDofs_;
    }

    //! only called once, if grid was adapted
    int newSize() const
    {
      return this->size();
    }

    //! for dof manager, to check whether it has to copy dof or not
    bool indexNew (int num) const
    {
      const int newn = static_cast<int> ( num / numberOfDofs_ );
      return indexSet_.template indexNew(newn,0);
    }

    //! return old index, for dof manager only
    int oldIndex (int num) const
    {
      // corresponding number of set is newn
      const int newn  = static_cast<int> ( num / numberOfDofs_ );
      // local number of dof is local
      const int local = (num % numberOfDofs_);
      return (numberOfDofs_ * indexSet_.oldIndex(newn,0)) + local;
    }

    //! return new index, for dof manager only
    int newIndex (int num) const
    {
      // corresponding number of set is newn
      const int newn = static_cast<int> ( num / numberOfDofs_ );
      // local number of dof is local
      const int local = (num % numberOfDofs_);
      return (numberOfDofs_ * indexSet_.newIndex(newn,0)) + local;
    }
    //! return size of grid entities per level and codim
    //! for dof mapper
    int oldSize () const
    {
      // this index set works only for codim = 0 at the moment
      return numberOfDofs_ * indexSet_.oldSize(0);
    }

    // is called once and calcs the insertion points too
    int additionalSizeEstimate () const
    {
      return numberOfDofs_ * indexSet_.additionalSizeEstimate();
    }
  };

} // end namespace Dune

#endif
