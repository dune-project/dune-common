// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ONEDGRID_ENTITY_POINTER_HH
#define DUNE_ONEDGRID_ENTITY_POINTER_HH


namespace Dune {

  /*! Acts as a pointer to an  entities of a given codimension.
   */
  template<int codim, class GridImp>
  class OneDGridEntityPointer
    : public EntityPointerDefaultImplementation <codim, GridImp, Dune::OneDGridEntityPointer<codim,GridImp> >
  {
    enum { dim = GridImp::dimension };
    friend class OneDGridIntersectionIterator<GridImp>;
    friend class OneDGridEntity<0,dim,GridImp>;

  public:
    typedef typename GridImp::template Codim<codim>::Entity Entity;

    //! equality
    bool equals(const OneDGridEntityPointer<codim,GridImp>& other) const {
      return other.virtualEntity_.target() == virtualEntity_.target();
    }

    //! dereferencing
    Entity& dereference() const {return virtualEntity_;}

    //! ask for level of entity
    int level () const {return virtualEntity_.level();}

    OneDGridEntityPointer() {}
  protected:

    /** \brief Constructor from a given iterator */
    OneDGridEntityPointer(OneDEntityImp<dim-codim>* it) {
      virtualEntity_.setToTarget(it);
    };

  protected:

    mutable OneDEntityWrapper<codim,GridImp::dimension,GridImp> virtualEntity_;

  };


} // end namespace Dune

#endif
