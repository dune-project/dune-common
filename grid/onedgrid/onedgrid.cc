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

    vertices[0].insert_after(vertices[0].rbegin(), *newVertex);
  }

  // Init element set
  //elements[0].resize(coords.size()-1);

  DoubleLinkedList<OneDGridEntity<1,1,1> >::Iterator it = vertices[0].begin();
  for (int i=0; i<coords.size()-1; i++) {

    OneDGridEntity<0,1,1>* newElement = new OneDGridEntity<0,1,1>;
    newElement->geo_.vertex[0] = it;
    ++it;
    newElement->geo_.vertex[1] = it;
    newElement->index_ = i;

    elements[0].insert_after(elements[0].rbegin(), *newElement);

  }

}


template <>
inline OneDGridLevelIterator<1,1,1,All_Partition>
OneDGrid<1,1>::lbegin<1> (int level) const
{
  if (level<0 || level>maxlevel())
    DUNE_THROW(GridError, "LevelIterator in nonexisting level " << level << " requested!");

  OneDGridLevelIterator<1,1,1,All_Partition> it(vertices[level].begin());
  return it;
}

template <>
inline OneDGridLevelIterator<0,1,1,All_Partition>
OneDGrid<1,1>::lbegin<0> (int level) const
{
  if (level<0 || level>maxlevel())
    DUNE_THROW(GridError, "LevelIterator in nonexisting level " << level << " requested!");

  OneDGridLevelIterator<0,1,1,All_Partition> it(elements[level].begin());
  return it;
}

template <>
inline OneDGridLevelIterator<1,1,1,All_Partition>
OneDGrid<1,1>::lend<1> (int level) const
{
  if (level<0 || level>maxlevel())
    DUNE_THROW(GridError, "LevelIterator in nonexisting level " << level << " requested!");

  OneDGridLevelIterator<1,1,1,All_Partition> it(vertices[level].end());
  return it;
}

template <>
inline OneDGridLevelIterator<0,1,1,All_Partition>
OneDGrid<1,1>::lend<0> (int level) const
{
  if (level<0 || level>maxlevel())
    DUNE_THROW(GridError, "LevelIterator in nonexisting level " << level << " requested!");

  OneDGridLevelIterator<0,1,1,All_Partition> it(elements[level].end());
  return it;
}

template <int dim, int dimworld>
OneDGrid<dim,dimworld>::VertexContainer::Iterator
OneDGrid<dim,dimworld>::getLeftUpperVertex(const ElementContainer::Iterator& eIt)
{
  ElementContainer::Iterator l = eIt;
  --l;

  if (l==ElementContainer::Iterator())
    return VertexContainer::Iterator();

  // return NULL if there is no geometrical left neighbor
  if (l->geo_.vertex[1]!=eIt->geo_.vertex[0])
    return VertexContainer::Iterator();

  // return NULL if that neighbor doesn't have sons
  if (!l->hasChildren())
    return VertexContainer::Iterator();

  // return the right vertex of the right son
  ElementContainer::Iterator rightSon = l->sons_[1];
  VertexContainer::Iterator rightVertex = rightSon->geo_.vertex[1];
  const OneDGridElement<1,1>& geo = rightSon->geo_;

  return l->sons_[1]->geo_.vertex[1];

}

template <int dim, int dimworld>
OneDGrid<dim,dimworld>::VertexContainer::Iterator
OneDGrid<dim,dimworld>::getRightUpperVertex(const ElementContainer::Iterator& eIt)
{
  ElementContainer::Iterator r = eIt;
  ++r;

  if (r==ElementContainer::Iterator())
    return VertexContainer::Iterator();

  // return NULL if there is no geometrical right neighbor
  if (r->geo_.vertex[0]!=eIt->geo_.vertex[1])
    return VertexContainer::Iterator();

  // return NULL if that neighbor doesn't have sons
  if (!r->hasChildren())
    return VertexContainer::Iterator();

  // return the left vertex of the left son
  return r->sons_[0]->geo_.vertex[0];

}

template <int dim, int dimworld>
OneDGrid<dim,dimworld>::ElementContainer::Iterator OneDGrid<dim,dimworld>::getLeftNeighborWithSon(const ElementContainer::Iterator& eIt)
{
  ElementContainer::Iterator l = eIt;

  do {
    --l;
    // we may not be on level 0, but it doesn't matter because all rend are the same
  } while (l!=elements[0].rend() && !l->hasChildren());

  return l;
}


template <int dim, int dimworld>
bool OneDGrid<dim,dimworld>::adapt()
{
  std::cout << "######## adapt() ##########" << std::endl;

  ElementContainer::Iterator eIt;

  // remove all elements that have been marked for coarsening
  for (int i=1; i<=maxlevel(); i++) {

    for (eIt = elements[i].begin(); eIt!=elements[i].end(); ++eIt) {

      if (eIt->adaptationState == COARSEN
          && eIt->hasChildren()) {

        ElementContainer::Iterator leftElement = eIt;
        --leftElement;

        ElementContainer::Iterator rightElement = eIt;
        ++rightElement;

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

      }

    }

  }

  // /////////////////////////////////////////////////////////////////////////
  //  Check if one of the elements on the toplevel is marked for refinement
  //  In that case add another level
  // /////////////////////////////////////////////////////////////////////////
  bool toplevelRefinement = false;
  for (eIt = elements[maxlevel()].begin(); eIt!=elements[maxlevel()].end(); ++eIt)
    if (eIt->adaptationState == REFINED) {
      toplevelRefinement = true;
      break;
    }

  if (toplevelRefinement) {
    VertexContainer newVertices;
    ElementContainer newElements;
    vertices.push_back(newVertices);
    elements.push_back(newElements);
  }

  // //////////////////////////////
  // refine all marked elements
  // //////////////////////////////
  int oldMaxlevel = (toplevelRefinement) ? maxlevel()-1 : maxlevel();
  for (int i=0; i<=oldMaxlevel; i++) {

    //std::cout << "level " << i << ", maxlevel " << maxlevel() << std::endl;

    for (eIt = elements[i].begin(); eIt!=elements[i].end(); ++eIt) {

      std::cout << "element loop " << std::endl;

      if (eIt->adaptationState == REFINED
          && !eIt->hasChildren()) {

        if (eIt->index()!=0) continue;
#if 1
        std::cout << "Refining element " << eIt->index() << " on level " << i << std::endl;
        // Does the left vertex exist on the next-higher level?
        // If no create it
        VertexContainer::Iterator leftUpperVertex = getLeftUpperVertex(eIt);

        OneDGridEntity<1,1,1> *newRightUpperVertex, *newLeftUpperVertex, *centerVertex;
        if (leftUpperVertex==VertexContainer::Iterator())
          newLeftUpperVertex = new OneDGridEntity<1,1,1>(i+1, eIt->geometry()[0][0]);

        // Does the right vertex exist on the next-higher level?
        // If no create it
        VertexContainer::Iterator rightUpperVertex = getRightUpperVertex(eIt);

        if (rightUpperVertex==VertexContainer::Iterator()) {
          FieldVector<double, 1> mystery = eIt->geometry()[1];
          std::cout << "Creating right vertex at " << mystery << std::endl;
          newRightUpperVertex = new OneDGridEntity<1,1,1>(i+1, eIt->geometry()[1][0]);
        }

        // Create center vertex
        double a = eIt->geometry()[0][0];
        double b = eIt->geometry()[1][0];
        double p = 0.5*(a+b);

        // The following line didn't give the correct result.  And I dont't know why
        //double p = 0.5*(eIt->geometry()[0][0] + eIt->geometry()[1][0]);
        //                 std::cout << "left: " << eIt->geometry()[0][0]
        //                           << " right: " << eIt->geometry()[1][0] << " p: " << p << std::endl;

        centerVertex = new OneDGridEntity<1,1,1>(i+1, p);

        // //////////////////////////////////////
        // Insert new vertices into vertex list
        // //////////////////////////////////////

        ElementContainer::Iterator leftNeighbor = getLeftNeighborWithSon(eIt);
        if (leftNeighbor!=elements[i].rend()) {

          // leftNeighbor exists
          if ( leftNeighbor->sons_[1]->geo_.vertex[1] != leftUpperVertex)
            leftUpperVertex = vertices[i+1].insert_after(leftNeighbor->sons_[1]->geo_.vertex[1], *newLeftUpperVertex);

        } else {

          // leftNeighbor does not exist
          leftUpperVertex = vertices[i+1].insert_before(vertices[i+1].begin(), *newLeftUpperVertex);

        }

        VertexContainer::Iterator centerVIt = vertices[i+1].insert_after(leftUpperVertex, *centerVertex);

        // Check if rightUpperVertex is already in the list
        VertexContainer::Iterator succOfCenter = centerVIt;
        ++succOfCenter;

        if (succOfCenter==vertices[i+1].end() || succOfCenter != rightUpperVertex)
          if (rightUpperVertex != VertexContainer::Iterator())
            vertices[i+1].insert_after(centerVIt, *rightUpperVertex);
          else
            rightUpperVertex = vertices[i+1].insert_after(centerVIt, *newRightUpperVertex);

        // ///////////////////////
        // Create new elements
        // ///////////////////////
        OneDGridEntity<0,1,1>* newElement0 = new OneDGridEntity<0,1,1>;
        newElement0->geo_.vertex[0] = leftUpperVertex;
        newElement0->geo_.vertex[1] = centerVIt;
        newElement0->father_ = eIt;

        OneDGridEntity<0,1,1>* newElement1 = new OneDGridEntity<0,1,1>;
        newElement1->geo_.vertex[0] = centerVIt;
        newElement1->geo_.vertex[1] = rightUpperVertex;
        newElement1->father_ = eIt;

        // Insert new elements into element list
        ElementContainer::Iterator newIt0, newIt1;

        if (leftNeighbor!=elements[i].rend())
          // leftNeighbor exists
          newIt0 = elements[i+1].insert_after(leftNeighbor->sons_[1], *newElement0);
        else
          // leftNeighbor does not exist
          newIt0 = elements[i+1].insert_before(elements[i+1].begin(), *newElement0);

        newIt1 = elements[i+1].insert_after(newIt0, *newElement1);

        // Mark the two new elements as the sons of the refined element
        eIt->sons_[0] = newIt0;
        eIt->sons_[1] = newIt1;

#endif
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
    VertexContainer::Iterator vIt;
    for (vIt = vertices[i].begin(); vIt!=vertices[i].end(); ++vIt)
      vIt->index_ = idx++;

    idx = 0;
    for (eIt = elements[i].begin(); eIt!=elements[i].end(); ++eIt)
      eIt->index_ = idx++;

  }

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
