// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DOFMAPPERINTERFACE_HH__
#define __DUNE_DOFMAPPERINTERFACE_HH__

namespace Dune {

  /** @defgroup DofMapper  DofMapperInterface

     @{
   */


  //************************************************************************
  //
  //  --MapperInterface
  //
  //! Interface for calculating the size of a function space for a grid on a
  //! specified level.
  //! Furthermore the local to global mapping of dof number is done.
  //
  //************************************************************************
  class DofMapperInterface
  {
  public:
    //! if grid has changed determine new size
    //! (to be called once per timestep, therefore virtual )
    virtual int newSize(int level) const = 0;

    //! calc new insertion points for dof of different codim
    //! (to be called once per timestep, therefore virtual )
    virtual void calcInsertPoints () = 0;
  };

  //***********************************************************************
  //
  //!  Provide default implementation of DofMapperInterface
  //
  //***********************************************************************
  template <class DofMapperImp>
  class DofMapperDefault : public DofMapperInterface
  {
  public:
    //! return number of dofs for special function space and grid on
    //! specified level
    int size (int level ) const
    {
      return asImp().size(level);
    };

    //! map a local dof num of a given entity to a global dof num
    template <class EntityType>
    int mapToGlobal ( EntityType &en, int localNum ) const
    {
      return asImp().mapToGlobal( en , localNum );
    };

    //! default implementation if not overlaoded
    virtual int newSize(int level) const
    {
      // overload this method in derived class
      assert(false);
      return -1;
    }

    //! default implementation if not overlaoded
    virtual void calcInsertPoints ()
    {
      // overload this method in derived class
      assert(false);
    }

  private:
    //! Barton-Nackman trick
    DofMapperImp &asImp()  { return static_cast<DofMapperImp &>(*this); };
    //! Barton-Nackman trick
    const DofMapperImp &asImp() const { return static_cast<const DofMapperImp &>(*this); };
  };

  /** @} end documentation group */

} // end namespace Dune

#endif
