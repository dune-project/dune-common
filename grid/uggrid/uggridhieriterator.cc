// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
//***************************************************************
//
//  --UGGridHierarchicIterator
//  --HierarchicIterator
//
//***************************************************************
template<class GridImp>
inline
UGGridHierarchicIterator<GridImp>::UGGridHierarchicIterator(int maxLevel)
  : virtualEntity_(0)
{
  maxlevel_ = maxLevel;

  virtualEntity_.setToTarget(NULL);
}

template<class GridImp>
inline
UGGridHierarchicIterator<GridImp>::
UGGridHierarchicIterator(const UGGridHierarchicIterator& other)
  : virtualEntity_(other.virtualEntity_),
    maxlevel_(other.maxlevel_),
    elemStack(other.elemStack)
{}

template<class GridImp>
void UGGridHierarchicIterator<GridImp>::increment()
{
  if (elemStack.empty())
    return;

  StackEntry old_target = elemStack.pop();

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
      elemStack.push(se);
    }
#undef NSONS
  }

  if (elemStack.empty())
    virtualEntity_.setToTarget(0);
  else
    virtualEntity_.setToTarget(elemStack.top().element, elemStack.top().level);

}

template<class GridImp>
inline bool UGGridHierarchicIterator<GridImp>::
equals(const UGGridHierarchicIterator& I) const
{
  return ( (elemStack.size()==0 && I.elemStack.size()==0) ||
           ((elemStack.size() == I.elemStack.size()) &&
            (elemStack.top().element == I.elemStack.top().element)));
}

template<class GridImp>
inline typename UGGridHierarchicIterator<GridImp>::Entity&
UGGridHierarchicIterator<GridImp>::
dereference() const
{
  return virtualEntity_;
}
