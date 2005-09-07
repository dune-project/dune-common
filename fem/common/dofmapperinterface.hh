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
    /** \brief Virtual destructor */
    virtual ~DofMapperInterface() {}

    //! if grid has changed determine new size
    //! (to be called once per timestep, therefore virtual )
    virtual int newSize() const = 0;

    //! calc new insertion points for dof of different codim
    //! (to be called once per timestep, therefore virtual )
    virtual void calcInsertPoints () = 0;

    //! return max number of local dofs per entity
    virtual int numberOfDofs () const DUNE_DEPRECATED = 0;

    //! return max number of local dofs per entity
    virtual int numDofs () const = 0;

    //! returns true if index is new ( for dof compress )
    virtual bool indexNew (int num) const = 0;

    //! return old index in dof array of given index ( for dof compress )
    virtual int oldIndex (int num) const = 0;

    //! return new index in dof array
    virtual int newIndex (int num) const = 0;

    /*! return estimate for size that is addtional needed
        for restriction of data */
    virtual int additionalSizeEstimate() const = 0;
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

    //! default implementation if not overlaoded
    virtual int newSize() const
    {
      return asImp().size();
    }

    //! default implementation if not overlaoded
    virtual void calcInsertPoints ()
    {
      // overload this method in derived class
      assert(false);
    }

    //! default implementation if not overlaoded
    virtual int numberOfDofs () const DUNE_DEPRECATED
    {
      // overload this method in derived class
      assert(false);
      return -1;
    }

    //! default implementation if not overlaoded
    virtual int numDofs () const
    {
      // overload this method in derived class
      assert(false);
      return -1;
    }

    //! returns true if index is new ( for dof compress )
    virtual bool indexNew (int num) const
    {
      assert(false);
      return false;
    }

    //! return old index in dof array of given index ( for dof compress )
    virtual int oldIndex (int num) const
    {
      assert(false);
      return -1;
    }

    // return new index in dof array
    virtual int newIndex (int num) const
    {
      assert(false);
      return -1;
    }

    // return estimate for size additional need for restriction of data
    virtual int additionalSizeEstimate() const
    {
      assert(false);
      return -1;
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
