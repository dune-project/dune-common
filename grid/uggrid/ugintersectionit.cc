// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
namespace Dune {

  template< int dim, int dimworld>
  inline UGGridIntersectionIterator<dim,dimworld>::
  UGGridIntersectionIterator() : virtualEntity_(-1), center_(0), neighborCount_(-1)
  {}

  template< int dim, int dimworld>
  inline typename TargetType<0,dimworld>::T* UGGridIntersectionIterator<dim,dimworld>::
  target() const
  {
    if (!isValid())
      return NULL;

    return UG<dimworld>::NbElem(center_, neighborCount_);
  }

  template< int dim, int dimworld>
  inline void UGGridIntersectionIterator<dim,dimworld>::
  setToTarget(typename TargetType<0,dimworld>::T* center, int nb)
  {
    //printf("entering II::setToTarget %d %d\n", (int)center, nb);
    center_ = center;
    neighborCount_ = nb;
    virtualEntity_.setToTarget(target());
  }

  template< int dim, int dimworld>
  inline bool
  UGGridIntersectionIterator<dim,dimworld>::
  isValid() const
  {
    return center_
           && neighborCount_ >=0
           && neighborCount_ < UG<dimworld>::Sides_Of_Elem(center_);
  }

  template< int dim, int dimworld>
  inline bool UGGridIntersectionIterator<dim,dimworld>::
  operator ==(const UGGridIntersectionIterator& I) const
  {
    // Two intersection iterators are equal iff they have the same
    // validity.  Furthermore, if they are both valid, they have
    // to have the same center and neighborCount_
    return (!isValid() && !I.isValid())
           || (isValid() && I.isValid() &&
               (center_ == I.center_ && neighborCount_ == I.neighborCount_));
  }

  template< int dim, int dimworld>
  inline bool UGGridIntersectionIterator<dim,dimworld>::
  operator !=(const UGGridIntersectionIterator& I) const
  {
    return !((*this)==I);
  }

  template<int dim, int dimworld>
  inline UGGridEntity< 0,dim,dimworld >*
  UGGridIntersectionIterator< dim,dimworld >::operator ->()
  {
    return &virtualEntity_;
  }

  template<int dim, int dimworld>
  inline UGGridEntity< 0,dim,dimworld >&
  UGGridIntersectionIterator< dim,dimworld >::operator *()
  {
    return virtualEntity_;
  }

#ifdef _3
  template<>
  inline UGGridIntersectionIterator < 3,3 >&
  UGGridIntersectionIterator < 3,3 >::operator++()
  {
    //printf("This is II::operator++\n");
    setToTarget(center_, neighborCount_+1);

    return (*this);
  }
#endif

#ifdef _2
  template<>
  inline UGGridIntersectionIterator < 2,2 >&
  UGGridIntersectionIterator < 2,2 >::operator++()
  {
    setToTarget(center_, neighborCount_+1);

    return (*this);
  }
#endif

#ifdef _3
  template<>
  inline bool
  UGGridIntersectionIterator < 3,3 >::boundary()
  {
    return UG<3>::NbElem(center_, neighborCount_) == NULL;
  }
#endif

#ifdef _2
  template<>
  inline bool
  UGGridIntersectionIterator < 2,2 >::boundary()
  {
    return UG<2>::NbElem(center_, neighborCount_) == NULL;
  }
#endif

#ifdef _3
  /** \brief Why can't I leave dimworld unspezialized? */
  template<>
  inline Vec<3,UGCtype>&
  UGGridIntersectionIterator < 3,3 >::unit_outer_normal ()
  {
    std::cerr << "unit_outer_normal<3,3> not yet implemented!\n";
    return outNormal_;

  }
#endif

  template<>
  inline Vec<2,UGCtype>&
  UGGridIntersectionIterator < 2,2 >::unit_outer_normal ()
  {
    std::cerr << "unit_outer_normal<2,2> not yet implemented!\n";
    return outNormal_;
  }

  template<int dim, int dimworld>
  inline Vec<dimworld,UGCtype>&
  UGGridIntersectionIterator < dim,dimworld >::
  unit_outer_normal (Vec<dim-1,UGCtype>& local)
  {
    return unit_outer_normal();
  }

  template< int dim, int dimworld>
  inline UGGridElement< dim-1, dim >&
  UGGridIntersectionIterator<dim,dimworld>::
  intersection_self_local()
  {
    std::cout << "\nintersection_self_local not implemented yet!\n";
    return (*fakeNeigh_);
  }

  template< int dim, int dimworld>
  inline UGGridElement< dim-1, dimworld >&
  UGGridIntersectionIterator<dim,dimworld>::
  intersection_self_global()
  {
    std::cout << "\nintersection_self_global not implemented yet!\n";
    return (*neighGlob_);
  }

  template< int dim, int dimworld>
  inline UGGridElement< dim-1, dim >&
  UGGridIntersectionIterator<dim,dimworld>::
  intersection_neighbor_local()
  {
    std::cout << "\nintersection_neighbor_local not implemented yet!\n";
    return (*fakeNeigh_);
  }

  template< int dim, int dimworld>
  inline UGGridElement< dim-1, dimworld >&
  UGGridIntersectionIterator<dim,dimworld>::
  intersection_neighbor_global()
  {
    std::cout << "\nintersection_neighbor_global not implemented yet!\n";
    return (*neighGlob_);
  }

  template< int dim, int dimworld>
  inline int UGGridIntersectionIterator<dim,dimworld>::
  number_in_self ()
  {
    /** \todo Muﬂ ich die Seitennummer wirklich umrechnen? */
    const int nSides = UG<dimworld>::Sides_Of_Elem(center_);

    return (neighborCount_ + nSides -1)%nSides;
  }

  template< int dim, int dimworld>
  inline int UGGridIntersectionIterator<dim,dimworld>::
  number_in_neighbor ()
  {
    const typename TargetType<0,dimworld>::T* other = target();

    /** \todo Muﬂ ich die Seitennummer wirklich umrechnen? */
    const int nSides = UG<dimworld>::Sides_Of_Elem(other);

    int i;
    for (i=0; i<Sides_Of_Elem(other); i++)
      if (NbElem(other,i) == center_)
        break;

    return (i+nSides-1)%nSides;
  }

  template<int dim, int dimworld>
  inline
  UGGridBoundaryEntity<dim,dimworld>&
  UGGridIntersectionIterator<dim,dimworld>::
  boundaryEntity ()
  {
    return boundaryEntity_;
  }

};
