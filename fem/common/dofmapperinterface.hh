// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DOFMAPPERINTERFACE_HH
#define DUNE_DOFMAPPERINTERFACE_HH

namespace Dune {

  /** @defgroup DofMapper  DofMapperInterface

     @{
   */

  //***********************************************************************
  //
  //  --MapperInterface
  //
  //! Interface for calculating the size of a function space for a grid on a
  //! specified level.
  //! Furthermore the local to global mapping of dof number is done.
  //! Also during grid adaptation this mapper knows about old and new indices
  //! of entities.
  //
  //***********************************************************************
  template <class DofMapperImp>
  class DofMapperInterface
  {
  public:
    //! return number of dofs for special function space and grid on
    //! specified level
    int size () const
    {
      return asImp().size();
    };

    //! map a local dof num of a given entity to a global dof num
    template <class EntityType>
    int mapToGlobal ( EntityType &en, int localNum ) const
    {
      return asImp().mapToGlobal( en , localNum );
    };

    //! return new size of space, i.e. after adaptation
    int newSize() const
    {
      return asImp().newSize();
    }

    //! return number of dofs on element
    int numDofs () const
    {
      return asImp().numDofs();
    }

    //! returns true if index is new ( for dof compress )
    bool indexNew (int num) const
    {
      return asImp().indexNew(num);
    }

    //! return old index in dof array of given index ( for dof compress )
    int oldIndex (int num) const
    {
      return asImp().oldIndex();
    }

    // return new index in dof array of given index ( for dof compress )
    int newIndex (int num) const
    {
      return asImp().newIndex();
    }

    // return estimate for size additional need for restriction of data
    int additionalSizeEstimate() const
    {
      return asImp().additionalSizeEstimate();
    }

  private:
    //! Barton-Nackman trick
    DofMapperImp &asImp()  { return static_cast<DofMapperImp &>(*this); };
    //! Barton-Nackman trick
    const DofMapperImp &asImp() const { return static_cast<const DofMapperImp &>(*this); };
  };

  //! Default implementation for DofMappers, empty at this moment
  template <class DofMapperImp>
  class DofMapperDefault : public DofMapperInterface<DofMapperImp>
  {
    //! nothing here at the moment
  private:
    //! Barton-Nackman trick
    DofMapperImp &asImp()  { return static_cast<DofMapperImp &>(*this); };
    //! Barton-Nackman trick
    const DofMapperImp &asImp() const { return static_cast<const DofMapperImp &>(*this); };
  };
  /** @} end documentation group */

} // end namespace Dune

#endif
