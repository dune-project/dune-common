// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
//***************************************************************
//
//  --UGGridHierarchicIterator
//  --HierarchicIterator
//
//***************************************************************
template< int dim, int dimworld>
inline
UGGridHierarchicIterator< dim,dimworld >::UGGridHierarchicIterator(int actLevel,int maxLevel)
  : virtualEntity_(0)
{
  //level_ = actLevel;
  maxlevel_ = maxLevel;

  virtualEntity_.setToTarget(NULL);
}


template< int dim, int dimworld>
inline UGGridHierarchicIterator<dim,dimworld>&
UGGridHierarchicIterator< dim,dimworld >::operator ++()
{
  if (elemStack.isempty())
    return (*this);

  StackEntry old_target = elemStack.pop_front();

  // Traverse the tree no deeper than maxlevel
  if (old_target.level < maxlevel_) {

    // The 30 is the macro MAX_SONS from ug/gm/gm.h
    UGElementType* sonList[30];
#ifdef _2
    UG2d::GetSons(old_target.element,sonList);
#else
    UG3d::GetSons(old_target.element,sonList);
#endif

#ifdef _2
#define NSONS(p) UG2d::ReadCW(p, UG2d::NSONS_CE)
#else
#define NSONS(p) UG3d::ReadCW(p, UG3d::NSONS_CE)
#endif
    // Load sons of old target onto the iterator stack
    for (unsigned int i=0; i<NSONS(old_target.element); i++) {
      StackEntry se;
      se.element = sonList[i];
      se.level   = old_target.level + 1;
      elemStack.push_front(se);
    }
#undef NSONS
  }

  if (elemStack.isempty())
    virtualEntity_.setToTarget(0);
  else
    virtualEntity_.setToTarget(elemStack.front().element, elemStack.front().level);

  return (*this);
}


template< int dim, int dimworld>
inline bool UGGridHierarchicIterator<dim,dimworld>::
operator ==(const UGGridHierarchicIterator& I) const
{
  return ( (elemStack.size()==0 && I.elemStack.size()==0) ||
           ((elemStack.size() == I.elemStack.size()) &&
            (elemStack.front().element == I.elemStack.front().element)));
}

template< int dim, int dimworld>
inline bool UGGridHierarchicIterator<dim,dimworld>::
operator !=(const UGGridHierarchicIterator& I) const
{
  return !((*this) == I);
}

template< int dim, int dimworld>
inline UGGridEntity < 0, dim ,dimworld >&
UGGridHierarchicIterator<dim,dimworld>::
operator *()
{
  return virtualEntity_;
}

template< int dim, int dimworld>
inline UGGridEntity < 0, dim ,dimworld >*
UGGridHierarchicIterator<dim,dimworld>::
operator ->()
{
  return &virtualEntity_;
}
