// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
namespace Dune {

  template< int dim, int dimworld>
  inline UGGridIntersectionIterator<dim,dimworld>::
  UGGridIntersectionIterator() : center_(NULL), neighborCount_(-1),
                                 virtualEntity_(-1)
  {}

  template< int dim, int dimworld>
  inline UG3d::ELEMENT* UGGridIntersectionIterator<dim,dimworld>::
  target() const
  {
#define TAG(p) ReadCW(p, UG3d::TAG_CE)
#define SIDES_OF_ELEM(p) (UG3d::element_descriptors[TAG(p)]->sides_of_elem)
    if (!center_
        || neighborCount_<0
        || neighborCount_>=SIDES_OF_ELEM(center_))
      return NULL;
#undef SIDES_OF_ELEM



#define NBELEM(p,i) ((UG3d::ELEMENT *) (p)->ge.refs[UG3d::nb_offset[TAG(p)]+(i)])
    return NBELEM(center_, neighborCount_);
#undef NBELEM
#undef TAG
  }

  template< int dim, int dimworld>
  inline void UGGridIntersectionIterator<dim,dimworld>::
  setToTarget(UG3d::element* center, int nb)
  {
    //printf("entering II::setToTarget %d %d\n", center, nb);
    center_ = center;
    neighborCount_ = nb;
    virtualEntity_.setToTarget(target());
  }

  template< int dim, int dimworld>
  inline bool UGGridIntersectionIterator<dim,dimworld>::
  operator ==(const UGGridIntersectionIterator& I) const
  {
    return (target() == I.target());
  }

  template< int dim, int dimworld>
  inline bool UGGridIntersectionIterator<dim,dimworld>::
  operator !=(const UGGridIntersectionIterator& I) const
  {
    return target()!=I.target();
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

  template<>
  inline UGGridIntersectionIterator < 3,3 >&
  UGGridIntersectionIterator < 3,3 >::operator++()
  {
    //printf("This is II::operator++\n");
    if (!target())
      return (*this);


    setToTarget(center_, neighborCount_+1);

    return (*this);
  }

  template<>
  inline bool
  UGGridIntersectionIterator < 3,3 >::boundary()
  {
#define TAG(p) ReadCW(p, UG3d::TAG_CE)
#define NBELEM(p,i) ((UG3d::ELEMENT *) (p)->ge.refs[UG3d::nb_offset[TAG(p)]+(i)])
    return NBELEM(center_, neighborCount_) == NULL;
#undef TAG
#undef NBELEM
  }

  /** \brief Why can't I leave dimworld unspezialized? */
  template<>
  inline Vec<3,UGCtype>&
  UGGridIntersectionIterator < 3,3 >::unit_outer_normal ()
  {
    std::cerr << "unit_outer_normal<3,3> not yet implemented!\n";
    return outNormal_;

  }

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
#if 0
    if(!manageInterEl_)
    {
      manageInterEl_ = grid_->interSelfProvider_.getNewObjectEntity();
      fakeNeigh_ = manageInterEl_->item;
    }

    fakeNeigh_->builtGeom(elInfo_,neighborCount_,0,0);
#endif
    return (*fakeNeigh_);
  }

  template< int dim, int dimworld>
  inline UGGridElement< dim-1, dimworld >&
  UGGridIntersectionIterator<dim,dimworld>::
  intersection_self_global()
  {
    std::cout << "\nintersection_self_global not implemented yet!\n";
#if 0
    if(!manageNeighEl_)
    {
      manageNeighEl_ = grid_->interNeighProvider_.getNewObjectEntity();
      neighGlob_ = manageNeighEl_->item;
    }

    if(neighGlob_->builtGeom(elInfo_,neighborCount_,0,0))
      return (*neighGlob_);
    else
      abort();
#endif
    return (*neighGlob_);
  }

  template< int dim, int dimworld>
  inline UGGridElement< dim-1, dim >&
  UGGridIntersectionIterator<dim,dimworld>::
  intersection_neighbor_local()
  {
    std::cout << "\nintersection_neighbor_local not implemented yet!\n";
#if 0
    if(!manageInterEl_)
    {
      manageInterEl_ = grid_->interSelfProvider_.getNewObjectEntity();
      fakeNeigh_ = manageInterEl_->item;
    }

    if(!builtNeigh_)
    {
      setupVirtEn();
    }

    fakeNeigh_->builtGeom(neighElInfo_,neighborCount_,0,0);
#endif
    return (*fakeNeigh_);
  }

  template< int dim, int dimworld>
  inline UGGridElement< dim-1, dimworld >&
  UGGridIntersectionIterator<dim,dimworld>::
  intersection_neighbor_global()
  {
    std::cout << "\nintersection_neighbor_global not implemented yet!\n";
#if 0
    if(!manageNeighEl_)
    {
      manageNeighEl_ = grid_->interNeighProvider_.getNewObjectEntity();
      neighGlob_ = manageNeighEl_->item;
    }

    // built neighGlob_ first
    if(!builtNeigh_)
    {
      setupVirtEn();
    }
    neighGlob_->builtGeom(elInfo_,neighborCount_,0,0);
#endif
    return (*neighGlob_);
  }

  template< int dim, int dimworld>
  inline int UGGridIntersectionIterator<dim,dimworld>::
  number_in_self ()
  {
    return neighborCount_;
  }

  template< int dim, int dimworld>
  inline int UGGridIntersectionIterator<dim,dimworld>::
  number_in_neighbor ()
  {
#define TAG(p) ReadCW(p, UG3d::TAG_CE)
#define SIDES_OF_ELEM(p) (UG3d::element_descriptors[TAG(p)]->sides_of_elem)
#define NBELEM(p,i) ((UG3d::ELEMENT *) (p)->ge.refs[UG3d::nb_offset[TAG(p)]+(i)])
    const UG3d::element* other = target();
    int i;
    for (i=0; i<SIDES_OF_ELEM(other); i++)
      if (NBELEM(other,i) == center_)
        break;

    return i;
#undef SIDES_OF_ELEM
#undef NBELEM
#undef TAG
  }

};
