// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <dune/config.h>

#include "../onedgrid.hh"

// Explicitly instantiate the OnedGrid for dim == dimworld == 1,
// which is the only valid instantiation
template class Dune::OneDGrid<1,1>;

// Explicitly instantiate the necessary member templates contained in OneDGrid<1,1>
template Dune::OneDGrid<1,1>::Traits::codim<0>::LevelIterator Dune::OneDGrid<1,1>::lbegin<0>(int level) const;
template Dune::OneDGrid<1,1>::Traits::codim<1>::LevelIterator Dune::OneDGrid<1,1>::lbegin<1>(int level) const;

template Dune::OneDGrid<1,1>::Traits::codim<0>::LevelIterator Dune::OneDGrid<1,1>::lend<0>(int level) const;
template Dune::OneDGrid<1,1>::Traits::codim<1>::LevelIterator Dune::OneDGrid<1,1>::lend<1>(int level) const;


// ///////////////////////////////////////////////////////////////
//
//    OneDGridLevelIteratorFactory, a class used to simulate
//    specialization of member templates
//
// ///////////////////////////////////////////////////////////////
namespace Dune {

  template <int codim>
  struct OneDGridLevelIteratorFactory {};

  template <>
  struct OneDGridLevelIteratorFactory<1>
  {
    static Dune::OneDGridLevelIterator<1,Dune::All_Partition, const Dune::OneDGrid<1,1> >
    lbegin(const Dune::OneDGrid<1,1> * g, int level);
  };

  template <>
  struct OneDGridLevelIteratorFactory<0>
  {
    static Dune::OneDGridLevelIterator<0,Dune::All_Partition, const Dune::OneDGrid<1,1> >
    lbegin(const Dune::OneDGrid<1,1> * g, int level);
  };

}

inline Dune::OneDGridLevelIterator<1,Dune::All_Partition, const Dune::OneDGrid<1,1> >
Dune::OneDGridLevelIteratorFactory<1>::lbegin (const OneDGrid<1,1> * g, int level)
{
  if (level<0 || level>g->maxlevel())
    DUNE_THROW(GridError, "LevelIterator in nonexisting level " << level << " requested!");

  OneDGridLevelIterator<1,All_Partition, const Dune::OneDGrid<1,1> > it(g->vertices[level].begin);
  return it;
}

inline Dune::OneDGridLevelIterator<0,Dune::All_Partition, const Dune::OneDGrid<1,1> >
Dune::OneDGridLevelIteratorFactory<0>::lbegin (const OneDGrid<1,1> * g, int level)
{
  if (level<0 || level>g->maxlevel())
    DUNE_THROW(GridError, "LevelIterator in nonexisting level " << level << " requested!");

  OneDGridLevelIterator<0,All_Partition, const Dune::OneDGrid<1,1> > it(g->elements[level].begin);
  return it;
}


template <int dim, int dimworld>
Dune::OneDGrid<dim,dimworld>::OneDGrid(int numElements, double leftBoundary, double rightBoundary)
{
  typedef const OneDGrid<dim,dimworld> GridImp;

  // Init grid hierarchy
  vertices.resize(1);
  elements.resize(1);

  // Init vertex set
  for (int i=0; i<numElements+1; i++) {
    double newCoord = leftBoundary + i*(rightBoundary-leftBoundary) / numElements;

    OneDGridEntity<1,1,GridImp>* newVertex = new OneDGridEntity<1,1,GridImp>(0, newCoord);
    newVertex->index_ = i;

    vertices[0].insert_after(vertices[0].rbegin, newVertex);
  }

  // Init element set
  OneDGridEntity<1,1,GridImp>* it = vertices[0].begin;
  for (int i=0; i<numElements; i++) {

    OneDGridEntity<0,1,GridImp>* newElement = new OneDGridEntity<0,1,GridImp>(0);
    newElement->geo_.vertex(0) = it;
    it = it->succ_;
    newElement->geo_.vertex(1) = it;
    newElement->index_ = i;

    elements[0].insert_after(elements[0].rbegin, newElement);

  }

}

template <int dim, int dimworld>
Dune::OneDGrid<dim,dimworld>::OneDGrid(const SimpleVector<OneDCType>& coords)
{
  typedef const OneDGrid<dim,dimworld> GridImp;

  // Init grid hierarchy
  vertices.resize(1);
  elements.resize(1);

  // Init vertex set
  for (int i=0; i<coords.size(); i++) {
    OneDGridEntity<1,1,GridImp>* newVertex = new OneDGridEntity<1,1,GridImp>(0, coords[i]);
    newVertex->index_ = i;

    vertices[0].insert_after(vertices[0].rbegin, newVertex);
  }

  // Init element set
  OneDGridEntity<1,1,GridImp>* it = vertices[0].begin;
  for (int i=0; i<coords.size()-1; i++) {

    OneDGridEntity<0,1,GridImp>* newElement = new OneDGridEntity<0,1,GridImp>(0);
    newElement->geo_.vertex(0) = it;
    it = it->succ_;
    newElement->geo_.vertex(1) = it;
    newElement->index_ = i;

    elements[0].insert_after(elements[0].rbegin, newElement);

  }

}


template <int dim, int dimworld>
Dune::OneDGrid<dim,dimworld>::~OneDGrid()
{
  typedef const OneDGrid<dim,dimworld> GridImp;

  // Delete all vertices
  for (unsigned int i=0; i<vertices.size(); i++) {

    OneDGridEntity<1,1,GridImp>* v = vertices[i].begin;

    while (v) {

      OneDGridEntity<1,1,GridImp>* vSucc = v->succ_;
      vertices[i].remove(v);
      delete(v);
      v = vSucc;

    }

  }

  // Delete all elements
  for (unsigned int i=0; i<elements.size(); i++) {

    OneDGridEntity<0,1,GridImp>* e = elements[i].begin;

    while (e) {

      OneDGridEntity<0,1,GridImp>* eSucc = e->succ_;
      elements[i].remove(e);
      delete(e);
      e = eSucc;

    }

  }

}

template <int dim, int dimworld>
template <int codim>
typename Dune::OneDGrid<dim,dimworld>::Traits::template codim<codim>::LevelIterator
Dune::OneDGrid<dim,dimworld>::lbegin(int level) const
{
  return OneDGridLevelIteratorFactory<codim>::lbegin(this, level);
}

template <int dim, int dimworld>
template <int codim>
typename Dune::OneDGrid<dim,dimworld>::Traits::template codim<codim>::LevelIterator
Dune::OneDGrid<dim,dimworld>::lend(int level) const
{
  if (level<0 || level>maxlevel())
    DUNE_THROW(GridError, "LevelIterator in nonexisting level " << level << " requested!");

  OneDGridLevelIterator<codim,All_Partition, const Dune::OneDGrid<dim,dimworld> > it(0);
  return it;
}

template <int dim, int dimworld>
Dune::OneDGridEntity<1,1,const Dune::OneDGrid<dim,dimworld> >*
Dune::OneDGrid<dim,dimworld>::getLeftUpperVertex(const OneDGridEntity<0,1,const Dune::OneDGrid<dim,dimworld> >* eIt)
{
  OneDGridEntity<0,1,const Dune::OneDGrid<dim,dimworld> >* l = eIt->pred_;

  if (!l)
    return 0;

  // return NULL if there is no geometrical left neighbor
  if (l->geo_.vertex(1)!=eIt->geo_.vertex(0))
    return 0;

  // return NULL if that neighbor doesn't have sons
  if (l->isLeaf())
    return 0;

  // return the right vertex of the right son
  return l->sons_[1]->geo_.vertex(1);

}

template <int dim, int dimworld>
Dune::OneDGridEntity<1,1,const Dune::OneDGrid<dim,dimworld> >*
Dune::OneDGrid<dim,dimworld>::getRightUpperVertex(const OneDGridEntity<0,1,const Dune::OneDGrid<dim,dimworld> >* eIt)
{
  OneDGridEntity<0,1,const Dune::OneDGrid<dim,dimworld> >* r = eIt->succ_;

  if (!r)
    return 0;

  // return NULL if there is no geometrical right neighbor
  if (r->geo_.vertex(0)!=eIt->geo_.vertex(1))
    return 0;

  // return NULL if that neighbor doesn't have sons
  if (r->isLeaf())
    return 0;

  // return the left vertex of the left son
  return r->sons_[0]->geo_.vertex(0);

}

template <int dim, int dimworld>
Dune::OneDGridEntity<0,1,const Dune::OneDGrid<dim,dimworld> >*
Dune::OneDGrid<dim,dimworld>::getLeftNeighborWithSon(OneDGridEntity<0,1,const Dune::OneDGrid<dim,dimworld> >* eIt)
{
  OneDGridEntity<0,1,const Dune::OneDGrid<dim,dimworld> >* l = eIt;

  do {
    l = l->pred_;
    // we may not be on level 0, but it doesn't matter because all rend are the same
  } while (l && l->isLeaf());

  return l;
}


template <int dim, int dimworld>
bool Dune::OneDGrid<dim,dimworld>::adapt()
{
  typedef const OneDGrid<dim,dimworld> GridImp;

  OneDGridEntity<0,1,GridImp>* eIt;

  // for the return value:  true if the grid was changed
  bool changedGrid = false;

#if 0
  // remove all elements that have been marked for coarsening
  for (int i=1; i<=maxlevel(); i++) {

    for (eIt = elements[i].begin(); eIt!=NULL; eIt = eIt->succ_) {

      if (eIt->adaptationState == COARSEN
          && eIt->hasChildren()) {

        OneDGridEntity<0,1,GridImp>* leftElement = eIt->pred_;

        OneDGridEntity<0,1,GridImp>* rightElement = eIt->succ_;

        // Is the left vertex obsolete?
        if (leftElement->geo_.vertex(1) != eIt->geo_.vertex(0))
          vertices[i].erase(eIt->geo_.vertex(0));

        // Is the right vertex obsolete?
        if (rightElement->geo_.vertex(0) != eIt->geo_.vertex(1))
          vertices[i].erase(eIt->geo_.vertex(1));

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
    List<OneDGridEntity<1,1,GridImp> > newVertices;
    List<OneDGridEntity<0,1,GridImp> >  newElements;
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
          && eIt->isLeaf()) {

        //std::cout << "Refining element " << eIt->index() << " on level " << i << std::endl;

        // Does the left vertex exist on the next-higher level?
        // If no create it
        OneDGridEntity<1,1,GridImp>* leftUpperVertex = getLeftUpperVertex(eIt);

        if (leftUpperVertex==NULL)
          leftUpperVertex = new OneDGridEntity<1,1,GridImp>(i+1, eIt->geometry()[0][0]);

        // Does the right vertex exist on the next-higher level?
        // If no create it
        OneDGridEntity<1,1,GridImp>* rightUpperVertex = getRightUpperVertex(eIt);

        if (rightUpperVertex==NULL)
          rightUpperVertex = new OneDGridEntity<1,1,GridImp>(i+1, eIt->geometry()[1][0]);

        // Create center vertex
        double a = eIt->geometry()[0][0];
        double b = eIt->geometry()[1][0];
        double p = 0.5*(a+b);

        // The following line didn't give the correct result.  And I dont't know why
        //double p = 0.5*(eIt->geometry()[0][0] + eIt->geometry()[1][0]);
        //                 std::cout << "left: " << eIt->geometry()[0][0]
        //                           << " right: " << eIt->geometry()[1][0] << " p: " << p << std::endl;

        OneDGridEntity<1,1,GridImp>* centerVertex = new OneDGridEntity<1,1,GridImp>(i+1, p);

        // //////////////////////////////////////
        // Insert new vertices into vertex list
        // //////////////////////////////////////

        OneDGridEntity<0,1,GridImp>* leftNeighbor = getLeftNeighborWithSon(eIt);

        if (leftNeighbor!=NULL) {

          // leftNeighbor exists
          if ( leftNeighbor->sons_[1]->geo_.vertex(1) != leftUpperVertex)
            vertices[i+1].insert_after(leftNeighbor->sons_[1]->geo_.vertex(1), leftUpperVertex);

        } else {
          // leftNeighbor does not exist
          vertices[i+1].insert_before(vertices[i+1].begin, leftUpperVertex);

        }

        vertices[i+1].insert_after(leftUpperVertex, centerVertex);

        // Check if rightUpperVertex is already in the list
        OneDGridEntity<1,1,GridImp>* succOfCenter = centerVertex->succ_;

        if (succOfCenter==NULL || succOfCenter != rightUpperVertex)
          vertices[i+1].insert_after(centerVertex, rightUpperVertex);

        // ///////////////////////
        // Create new elements
        // ///////////////////////
        OneDGridEntity<0,1,GridImp>* newElement0 = new OneDGridEntity<0,1,GridImp>(i+1);
        newElement0->geo_.vertex(0) = leftUpperVertex;
        newElement0->geo_.vertex(1) = centerVertex;
        newElement0->father_ = eIt;

        OneDGridEntity<0,1,GridImp>* newElement1 = new OneDGridEntity<0,1,GridImp>(i+1);
        newElement1->geo_.vertex(0) = centerVertex;
        newElement1->geo_.vertex(1) = rightUpperVertex;
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


  // If the refinement mode is 'COPY', fill the empty slots in the grid
  // by copying elements
  if (refinementType_ == COPY) {

    for (int i=0; i<maxlevel(); i++) {

      OneDGridEntity<0,1,GridImp>* eIt;
      for (eIt = elements[i].begin; eIt!=NULL; eIt = eIt->succ_) {

        //printf("level %d, element %d \n", i, eIt->index());
        if (eIt->isLeaf()) {

          //printf("level %d, element %d is without children\n", i, eIt->index());

          // Does the left vertex exist on the next-higher level?
          // If no create it
          OneDGridEntity<1,1,GridImp>* leftUpperVertex = getLeftUpperVertex(eIt);

          if (leftUpperVertex==NULL)
            leftUpperVertex = new OneDGridEntity<1,1,GridImp>(i+1, eIt->geometry()[0][0]);

          // Does the right vertex exist on the next-higher level?
          // If no create it
          OneDGridEntity<1,1,GridImp>* rightUpperVertex = getRightUpperVertex(eIt);

          if (rightUpperVertex==NULL)
            rightUpperVertex = new OneDGridEntity<1,1,GridImp>(i+1, eIt->geometry()[1][0]);

          // //////////////////////////////////////
          // Insert new vertices into vertex list
          // //////////////////////////////////////

          OneDGridEntity<0,1,GridImp>* leftNeighbor = getLeftNeighborWithSon(eIt);

          if (leftNeighbor!=NULL) {

            // leftNeighbor exists
            if ( leftNeighbor->sons_[1]->geo_.vertex(1) != leftUpperVertex)
              vertices[i+1].insert_after(leftNeighbor->sons_[1]->geo_.vertex(1), leftUpperVertex);

          } else {
            // leftNeighbor does not exist
            vertices[i+1].insert_before(vertices[i+1].begin, leftUpperVertex);

          }

          // Check if rightUpperVertex is already in the list
          OneDGridEntity<1,1,GridImp>* succOfLeft = leftUpperVertex->succ_;

          if (succOfLeft==NULL || succOfLeft != rightUpperVertex)
            vertices[i+1].insert_after(leftUpperVertex, rightUpperVertex);

          // /////////////////////////
          //   Create new element
          // /////////////////////////
          OneDGridEntity<0,1,GridImp>* newElement = new OneDGridEntity<0,1,GridImp>(i+1);
          newElement->geo_.vertex(0) = leftUpperVertex;
          newElement->geo_.vertex(1) = rightUpperVertex;
          newElement->father_ = eIt;

          // Insert new elements into element list
          if (leftNeighbor!=NULL)
            // leftNeighbor exists
            elements[i+1].insert_after(leftNeighbor->sons_[1], newElement);
          else
            // leftNeighbor does not exist
            elements[i+1].insert_before(elements[i+1].begin, newElement);

          // Mark the new element as the sons of the refined element
          eIt->sons_[0] = eIt->sons_[1] = newElement;

        }

      }

    }

  }

  // ////////////////////////////////////
  //   renumber vertices and elements
  // ////////////////////////////////////
  for (int i=1; i<=maxlevel(); i++) {

    int idx = 0;
    OneDGridEntity<1,1,GridImp>* vIt;
    for (vIt = vertices[i].begin; vIt!=NULL; vIt = vIt->succ_)
      vIt->index_ = idx++;

    idx = 0;
    for (eIt = elements[i].begin; eIt!=NULL; eIt = eIt->succ_)
      eIt->index_ = idx++;

  }

  return changedGrid;

}


template <int dim, int dimworld>
void Dune::OneDGrid<dim,dimworld>::globalRefine(int refCount)
{
  // mark all entities for grid refinement
  typename Traits::template codim<0>::LevelIterator iIt    = lbegin<0>(maxlevel());
  typename Traits::template codim<0>::LevelIterator iEndIt = lend<0>(maxlevel());
  //OneDGridLevelIterator<0,All_Partition,OneDGrid<1,1> > iIt    = lbegin<0>(maxlevel());
  //OneDGridLevelIterator<0,All_Partition,OneDGrid<1,1> > iEndIt = lend<0>(maxlevel());

  for (; iIt!=iEndIt; ++iIt)
    mark(1, iIt);

  this->preAdapt();
  adapt();
  this->postAdapt();
}
