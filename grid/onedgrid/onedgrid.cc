// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
template <int dim, int dimworld>
OneDGrid<dim,dimworld>::OneDGrid(const SimpleVector<OneDCType>& coords)
{
  // Init grid hierarchy
  vertices.resize(1);
  elements.resize(1);

  // Init vertex set
  for (int i=0; i<coords.size(); i++) {
    OneDGridEntity<1,1,1>* newVertex = new OneDGridEntity<1,1,1>(0, coords[i]);
    newVertex->index_ = i;

    vertices[0].insert_after(vertices[0].rbegin, newVertex);
  }

  // Init element set
  OneDGridEntity<1,1,1>* it = vertices[0].begin;
  for (int i=0; i<coords.size()-1; i++) {

    OneDGridEntity<0,1,1>* newElement = new OneDGridEntity<0,1,1>(0);
    newElement->geo_.vertex[0] = it;
    it = it->succ_;
    newElement->geo_.vertex[1] = it;
    newElement->index_ = i;

    elements[0].insert_after(elements[0].rbegin, newElement);

  }

}


template <>
inline OneDGridLevelIterator<1,1,1,All_Partition>
OneDGrid<1,1>::lbegin<1> (int level) const
{
  if (level<0 || level>maxlevel())
    DUNE_THROW(GridError, "LevelIterator in nonexisting level " << level << " requested!");

  OneDGridLevelIterator<1,1,1,All_Partition> it(vertices[level].begin);
  return it;
}

template <>
inline OneDGridLevelIterator<0,1,1,All_Partition>
OneDGrid<1,1>::lbegin<0> (int level) const
{
  if (level<0 || level>maxlevel())
    DUNE_THROW(GridError, "LevelIterator in nonexisting level " << level << " requested!");

  OneDGridLevelIterator<0,1,1,All_Partition> it(elements[level].begin);
  return it;
}

template <>
inline OneDGridLevelIterator<1,1,1,All_Partition>
OneDGrid<1,1>::lend<1> (int level) const
{
  if (level<0 || level>maxlevel())
    DUNE_THROW(GridError, "LevelIterator in nonexisting level " << level << " requested!");

  OneDGridLevelIterator<1,1,1,All_Partition> it(0);
  return it;
}

template <>
inline OneDGridLevelIterator<0,1,1,All_Partition>
OneDGrid<1,1>::lend<0> (int level) const
{
  if (level<0 || level>maxlevel())
    DUNE_THROW(GridError, "LevelIterator in nonexisting level " << level << " requested!");

  OneDGridLevelIterator<0,1,1,All_Partition> it(0);
  return it;
}

template <int dim, int dimworld>
OneDGridEntity<1,1,1>*
OneDGrid<dim,dimworld>::getLeftUpperVertex(const OneDGridEntity<0,1,1>* eIt)
{
  OneDGridEntity<0,1,1>* l = eIt->pred_;

  if (!l)
    return 0;

  // return NULL if there is no geometrical left neighbor
  if (l->geo_.vertex[1]!=eIt->geo_.vertex[0])
    return 0;

  // return NULL if that neighbor doesn't have sons
  if (!l->hasChildren())
    return 0;

  // return the right vertex of the right son
  //     ElementContainer::Iterator rightSon = l->sons_[1];
  //     VertexContainer::Iterator rightVertex = rightSon->geo_.vertex[1];
  //const OneDGridElement<1,1>& geo = rightSon->geo_;

  return l->sons_[1]->geo_.vertex[1];

}

template <int dim, int dimworld>
OneDGridEntity<1,1,1>*
OneDGrid<dim,dimworld>::getRightUpperVertex(const OneDGridEntity<0,1,1>* eIt)
{
  OneDGridEntity<0,1,1>* r = eIt->succ_;

  if (!r)
    return 0;

  // return NULL if there is no geometrical right neighbor
  if (r->geo_.vertex[0]!=eIt->geo_.vertex[1])
    return 0;

  // return NULL if that neighbor doesn't have sons
  if (!r->hasChildren())
    return 0;

  // return the left vertex of the left son
  return r->sons_[0]->geo_.vertex[0];

}

template <int dim, int dimworld>
OneDGridEntity<0,1,1>* OneDGrid<dim,dimworld>::getLeftNeighborWithSon(OneDGridEntity<0,1,1>* eIt)
{
  OneDGridEntity<0,1,1>* l = eIt;

  do {
    l = l->pred_;
    // we may not be on level 0, but it doesn't matter because all rend are the same
  } while (l && !l->hasChildren());

  return l;
}


template <int dim, int dimworld>
bool OneDGrid<dim,dimworld>::adapt()
{
  OneDGridEntity<0,1,1>* eIt;

  // for the return value:  true if the grid was changed
  bool changedGrid = false;

#if 0
  // remove all elements that have been marked for coarsening
  for (int i=1; i<=maxlevel(); i++) {

    for (eIt = elements[i].begin(); eIt!=NULL; eIt = eIt->succ_) {

      if (eIt->adaptationState == COARSEN
          && eIt->hasChildren()) {

        OneDGridEntity<0,1,1>* leftElement = eIt->pred_;

        OneDGridEntity<0,1,1>* rightElement = eIt->succ_;

        // Is the left vertex obsolete?
        if (leftElement->geo_.vertex[1] != eIt->geo_.vertex[0])
          vertices[i].erase(eIt->geo_.vertex[0]);

        // Is the right vertex obsolete?
        if (rightElement->geo_.vertex[0] != eIt->geo_.vertex[1])
          vertices[i].erase(eIt->geo_.vertex[1]);

        // Remove reference from the father element
        if (eIt->father()->sons_[0] == eIt)
          eIt->father()->sons_[0] = elements[i].end();
        else {
          assert (eIt->father()->sons_[1] == eIt);
          eIt->father()->sons_[1] = elements[i].end();
        }

        // Actually delete element
        elements[i].erase(eIt);

        // The grid has been changed
        changedGrid = true;

      }

    }

  }
#endif
  // /////////////////////////////////////////////////////////////////////////
  //  Check if one of the elements on the toplevel is marked for refinement
  //  In that case add another level
  // /////////////////////////////////////////////////////////////////////////
  bool toplevelRefinement = false;
  for (eIt = elements[maxlevel()].begin; eIt!=NULL; eIt=eIt->succ_)
    if (eIt->adaptationState == REFINED) {
      toplevelRefinement = true;
      break;
    }

  if (toplevelRefinement) {
    List<OneDGridEntity<1,1,1> > newVertices;
    List<OneDGridEntity<0,1,1> >  newElements;
    vertices.push_back(newVertices);
    elements.push_back(newElements);
  }

  // //////////////////////////////
  // refine all marked elements
  // //////////////////////////////
  int oldMaxlevel = (toplevelRefinement) ? maxlevel()-1 : maxlevel();
  for (int i=0; i<=oldMaxlevel; i++) {

    //std::cout << "level " << i << ", maxlevel " << maxlevel() << std::endl;

    for (eIt = elements[i].begin; eIt!=NULL; eIt = eIt->succ_) {

      //std::cout << "element loop " << std::endl;

      if (eIt->adaptationState == REFINED
          && !eIt->hasChildren()) {

        //std::cout << "Refining element " << eIt->index() << " on level " << i << std::endl;

        // Does the left vertex exist on the next-higher level?
        // If no create it
        OneDGridEntity<1,1,1>* leftUpperVertex = getLeftUpperVertex(eIt);

        if (leftUpperVertex==NULL)
          leftUpperVertex = new OneDGridEntity<1,1,1>(i+1, eIt->geometry()[0][0]);

        // Does the right vertex exist on the next-higher level?
        // If no create it
        OneDGridEntity<1,1,1>* rightUpperVertex = getRightUpperVertex(eIt);

        if (rightUpperVertex==NULL)
          rightUpperVertex = new OneDGridEntity<1,1,1>(i+1, eIt->geometry()[1][0]);

        // Create center vertex
        double a = eIt->geometry()[0][0];
        double b = eIt->geometry()[1][0];
        double p = 0.5*(a+b);

        // The following line didn't give the correct result.  And I dont't know why
        //double p = 0.5*(eIt->geometry()[0][0] + eIt->geometry()[1][0]);
        //                 std::cout << "left: " << eIt->geometry()[0][0]
        //                           << " right: " << eIt->geometry()[1][0] << " p: " << p << std::endl;

        OneDGridEntity<1,1,1>* centerVertex = new OneDGridEntity<1,1,1>(i+1, p);

        // //////////////////////////////////////
        // Insert new vertices into vertex list
        // //////////////////////////////////////

        OneDGridEntity<0,1,1>* leftNeighbor = getLeftNeighborWithSon(eIt);

        if (leftNeighbor!=NULL) {

          // leftNeighbor exists
          if ( leftNeighbor->sons_[1]->geo_.vertex[1] != leftUpperVertex)
            vertices[i+1].insert_after(leftNeighbor->sons_[1]->geo_.vertex[1], leftUpperVertex);

        } else {
          // leftNeighbor does not exist
          vertices[i+1].insert_before(vertices[i+1].begin, leftUpperVertex);

        }

        vertices[i+1].insert_after(leftUpperVertex, centerVertex);

        // Check if rightUpperVertex is already in the list
        OneDGridEntity<1,1,1>* succOfCenter = centerVertex->succ_;

        if (succOfCenter==NULL || succOfCenter != rightUpperVertex)
          vertices[i+1].insert_after(centerVertex, rightUpperVertex);

        // ///////////////////////
        // Create new elements
        // ///////////////////////
        OneDGridEntity<0,1,1>* newElement0 = new OneDGridEntity<0,1,1>(i+1);
        newElement0->geo_.vertex[0] = leftUpperVertex;
        newElement0->geo_.vertex[1] = centerVertex;
        newElement0->father_ = eIt;

        OneDGridEntity<0,1,1>* newElement1 = new OneDGridEntity<0,1,1>(i+1);
        newElement1->geo_.vertex[0] = centerVertex;
        newElement1->geo_.vertex[1] = rightUpperVertex;
        newElement1->father_ = eIt;

        // Insert new elements into element list
        if (leftNeighbor!=NULL)
          // leftNeighbor exists
          elements[i+1].insert_after(leftNeighbor->sons_[1], newElement0);
        else
          // leftNeighbor does not exist
          elements[i+1].insert_before(elements[i+1].begin, newElement0);

        elements[i+1].insert_after(newElement0, newElement1);

        // Mark the two new elements as the sons of the refined element
        eIt->sons_[0] = newElement0;
        eIt->sons_[1] = newElement1;

        // The grid has been modified
        changedGrid = true;

      }

    }

  }

  // delete uppermost level if it doesn't contain elements anymore
  if (elements[maxlevel()].size()==0) {
    assert(vertices[maxlevel()].size()==0);
    elements.pop_back();
    vertices.pop_back();
  }


  // renumber vertices and elements
  for (int i=1; i<=maxlevel(); i++) {

    int idx = 0;
    OneDGridEntity<1,1,1>* vIt;
    for (vIt = vertices[i].begin; vIt!=NULL; vIt = vIt->succ_)
      vIt->index_ = idx++;

    idx = 0;
    for (eIt = elements[i].begin; eIt!=NULL; eIt = eIt->succ_)
      eIt->index_ = idx++;

  }

  return changedGrid;

}


template <int dim, int dimworld>
void OneDGrid<dim,dimworld>::globalRefine(int refCount)
{
  // mark all entities for grid refinement
  OneDGridLevelIterator<0, dim, dimworld, All_Partition> iIt    = lbegin<0>(maxlevel());
  OneDGridLevelIterator<0, dim, dimworld, All_Partition> iEndIt = lend<0>(maxlevel());

  for (; iIt!=iEndIt; ++iIt)
    iIt->mark(1);

  this->preAdapt();
  adapt();
  this->postAdapt();
}
