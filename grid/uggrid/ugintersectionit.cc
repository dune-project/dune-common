// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
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

  int i;
#define TAG(p) ReadCW(p, UG3d::TAG_CE)
#define NBELEM(p,i) ((UG3d::ELEMENT *) (p)->ge.refs[UG3d::nb_offset[TAG(p)]+(i)])
#define SIDES_OF_ELEM(p) (UG3d::element_descriptors[TAG(p)]->sides_of_elem)
  for (i=neighborCount_+1; i<SIDES_OF_ELEM(center_); i++) {
    if (NBELEM(center_, i) != NULL)
      break;
  }
#undef TAG
#undef NBELEM
#undef SIDES_OF_ELEM
  setToTarget(center_, i);

  //virtualEntity_.setToTarget(target());
  return (*this);
}
