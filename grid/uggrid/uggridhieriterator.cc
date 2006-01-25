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
{
  maxlevel_ = maxLevel;

  this->virtualEntity_.setToTarget(NULL);
}

template<class GridImp>
void UGGridHierarchicIterator<GridImp>::increment()
{
  if (elemStack.empty())
    return;

  StackEntry old_target = elemStack.pop();

  // Traverse the tree no deeper than maxlevel
  if (old_target.level < maxlevel_) {

    // The 30 is the macro MAX_SONS from ug/gm/gm.h
    typename UGTypes<GridImp::dimension>::Element* sonList[30];
    UG_NS<GridImp::dimension>::GetSons(old_target.element,sonList);

    // Load sons of old target onto the iterator stack
    for (int i=0; i<UG_NS<GridImp::dimension>::nSons(old_target.element); i++) {
      StackEntry se;
      se.element = sonList[i];
      se.level   = old_target.level + 1;
      elemStack.push(se);
    }

  }

  if (elemStack.empty())
    this->virtualEntity_.setToTarget(0);
  else
    this->virtualEntity_.setToTarget(elemStack.top().element, elemStack.top().level);

}
