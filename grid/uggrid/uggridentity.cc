// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include <dune/grid/uggrid.hh>
#include <dune/grid/uggrid/uggridentity.hh>

//*************************************************************************
//
//  --UGGridEntity
//  --Entity
//
//*************************************************************************


//
//  codim > 0
//
//*********************************************************************


//*****************************************************************8
// count
template <int codim, int dim, class GridImp>
template <int cc>
inline int Dune::UGGridEntity<codim,dim,GridImp>::count () const
{
  DUNE_THROW(GridError, "UGGridEntity<" << codim << ", " << dim
                                        << ">::count() not implemented yet!");
  return -1;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//     Specializations for codim == 0                                     //
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

template< int dim, class GridImp>
inline Dune::AdaptationState Dune::UGGridEntity < 0, dim ,GridImp >::state() const
{
  if (UG_NS<dim>::ReadCW(target_, UG_NS<dim>::NEWEL_CE))
    return REFINED;

  if (UG_NS<dim>::ReadCW(target_, UG_NS<dim>::COARSEN_CE))
    return COARSEN;

  return NONE;
}


//*****************************************************************8
// count
template <int dim, class GridImp>
template <int cc>
inline int Dune::UGGridEntity<0,dim,GridImp>::count() const
{
  if (dim==3) {

    switch (cc) {
    case 0 :
      return 1;
    case 1 :
      return UG_NS<dim>::Sides_Of_Elem(target_);
    case 2 :
      return UG_NS<dim>::Edges_Of_Elem(target_);
    case 3 :
      return UG_NS<dim>::Corners_Of_Elem(target_);
    }

  } else {

    switch (cc) {
    case 0 :
      return 1;
    case 1 :
      return UG_NS<dim>::Edges_Of_Elem(target_);
    case 2 :
      return UG_NS<dim>::Corners_Of_Elem(target_);
    }

  }
  DUNE_THROW(GridError, "You can't call UGGridEntity<0,dim>::count<codim> "
             << "with dim==" << dim << " and codim==" << cc << "!");
}

template <int dim, class GridImp>
inline int Dune::UGGridEntity<0, dim, GridImp>::renumberVertex(int i) const {

  if (geometry().type().isCube()) {

    // Dune numbers the vertices of a hexahedron and quadrilaterals differently than UG.
    // The following two lines do the transformation
    // The renumbering scheme is {0,1,3,2} for quadrilaterals, therefore, the
    // following code works for 2d and 3d.
    // It also works in both directions UG->DUNE, DUNE->UG !
    const int renumbering[8] = {0, 1, 3, 2, 4, 5, 7, 6};
    return renumbering[i];

  } else
    return i;

}
template <int dim, class GridImp>
inline int Dune::UGGridEntity<0, dim, GridImp>::renumberFace(int i) const {

  if (geometry().type().isCube()) {

    // Dune numbers the vertices of a hexahedron and quadrilaterals differently than UG.
    // The following two lines do the transformation
    // The renumbering scheme is {0,1,3,2} for quadrilaterals, therefore, the
    // following code works for 2d and 3d.
    const int renumbering[6] = {4, 2, 1, 3, 0, 5};
    return renumbering[i];

  }
  if (geometry().type().isSimplex()) {

    /** \todo Check this */
    // Dune numbers the vertices of a hexahedron and quadrilaterals differently than UG.
    // The following two lines do the transformation
    // The renumbering scheme is {0,1,3,2} for quadrilaterals, therefore, the
    // following code works for 2d and 3d.
    const int renumbering[4] = {1, 2, 0, 3};
    return renumbering[i];
  }
  return i;
}

template <int dim, class GridImp>
template <int cc>
inline typename GridImp::template Codim<cc>::EntityPointer
Dune::UGGridEntity<0,dim,GridImp>::entity ( int i ) const
{
  assert(i>=0 && i<count<cc>());

  if (cc==dim) {
    typename TargetType<cc,dim>::T* subEntity = UGGridSubEntityFactory<cc,dim>::get(target_,renumberVertex(i));
    return UGGridLevelIterator<cc,All_Partition,GridImp>(subEntity, level_);
  } else if (cc==0) {
    // The following cast is here to make the code compile for all cc.
    // When it gets actually called, cc==0, and the cast is nonexisting.
    typename TargetType<cc,dim>::T* myself = (typename TargetType<cc,dim>::T*)target_;
    return UGGridLevelIterator<cc,All_Partition,GridImp>(myself, level_);
  } else
    DUNE_THROW(GridError, "UGGrid<" << dim << "," << dim << ">::entity isn't implemented for cc==" << cc );
}

template<int dim, class GridImp>
inline void Dune::UGGridEntity < 0, dim ,GridImp >::
setToTarget(typename TargetType<0,dim>::T* target, int level)
{
  target_ = target;
  level_  = level;
  geo_.setToTarget(target);
}

template<int dim, class GridImp>
inline void Dune::UGGridEntity < 0, dim ,GridImp >::
setToTarget(typename TargetType<0,dim>::T* target)
{
  target_ = target;
  geo_.setToTarget(target);
}

template<int dim, class GridImp>
inline Dune::UGGridIntersectionIterator<GridImp>
Dune::UGGridEntity < 0, dim ,GridImp >::ibegin() const
{
  return UGGridIntersectionIterator<GridImp>(target_, 0, level());
}


template< int dim, class GridImp>
inline Dune::UGGridIntersectionIterator<GridImp>
Dune::UGGridEntity < 0, dim ,GridImp >::iend() const
{
  return UGGridIntersectionIterator<GridImp>(target_, -1, -1);
}

template<int dim, class GridImp>
inline Dune::UGGridHierarchicIterator<GridImp>
Dune::UGGridEntity < 0, dim ,GridImp >::hbegin(int maxlevel) const
{
  UGGridHierarchicIterator<GridImp> it(maxlevel);

  if (level()<=maxlevel) {

    // Put myself on the stack
    typename UGGridHierarchicIterator<GridImp>::StackEntry se;
    se.element = target_;
    se.level   = level();
    it.elemStack.push(se);

    // Set intersection iterator to myself
    it.virtualEntity_.setToTarget(target_, level());

    /** \todo Directly put all sons onto the stack */
    it.increment();

  } else {
    it.virtualEntity_.setToTarget(0);
  }

  return it;
}


template< int dim, class GridImp>
inline Dune::UGGridHierarchicIterator<GridImp>
Dune::UGGridEntity < 0, dim ,GridImp >::hend(int maxlevel) const
{
  return UGGridHierarchicIterator<GridImp>(maxlevel);
}


template<int dim, class GridImp>
inline int Dune::UGGridEntity < 0, dim ,GridImp >::
level() const
{
  return level_;
}

template< int dim, class GridImp>
inline const typename Dune::UGGridEntity<0,dim,GridImp>::Geometry&
Dune::UGGridEntity < 0, dim ,GridImp >::
geometry() const
{
  return geo_;
}


template<int dim, class GridImp>
inline Dune::UGGridLevelIterator<0,Dune::All_Partition,GridImp>
Dune::UGGridEntity < 0, dim, GridImp>::father() const
{
  UGGridLevelIterator<0,All_Partition,GridImp> it(level()-1);
  it.setToTarget(UG_NS<dim>::EFather(target_));
  return it;
}

template<int dim, class GridImp>
inline const typename Dune::UGGridEntity<0,dim,GridImp>::Geometry&
Dune::UGGridEntity < 0, dim, GridImp>::geometryInFather () const
{
  // we need to have a father element
  typename TargetType<0,dim>::T* fatherElement = UG_NS<dim>::EFather(target_);
  if (!fatherElement)
    DUNE_THROW(GridError, "Called geometryInFather() for an entity which doesn't have a father!");

  geometryInFather_.coordmode(); // put in the new mode
  geometryInFather_.setToTarget(target_);

  // The task is to find out the positions of the vertices of this element
  // in the local coordinate system of the father.

  // Get the 'context' of the father element.  In UG-speak, the context is
  // the set of all nodes of an elements' sons.  They appear in a fixed
  // order, therefore we can infer the local positions in the father element.
  const int MAX_CORNERS_OF_ELEM = 8;  // this is two much in 2d, but UG is that way
  const int MAX_NEW_CORNERS_DIM = (dim==2) ? 5 : 19;
  const typename TargetType<dim,dim>::T* context[MAX_CORNERS_OF_ELEM + MAX_NEW_CORNERS_DIM];
  UG_NS<dim>::GetNodeContext(fatherElement, context);

  // loop through all corner nodes
  for (int i=0; i<UG_NS<dim>::Corners_Of_Elem(target_); i++) {

    // get corner node pointer
    const typename TargetType<dim,dim>::T* fnode = UG_NS<dim>::Corner(target_,i);

    // Find out where in the father's context this node is
    int idx = -1;
    /** \todo Speed this up by only looping over the relevant part of the context */
    for (int j=0; j<MAX_CORNERS_OF_ELEM + MAX_NEW_CORNERS_DIM; j++)
      if (context[j] == fnode) {
        idx = j;
        break;
      }

    // Node has not been found.  There must be a programming error somewhere
    assert(idx!=-1);

    if (dim==2) {
      switch (UG_NS<dim>::Tag(fatherElement)) {

      case UG2d::TRIANGLE : {

        const double coords[6][2] = {
          // The corners
          {0,0}, {1,0}, {0,1},
          // The edge midpoints
          {0.5,0}, {0.5,0.5}, {0,0.5}
        };
        geometryInFather_.setCoords(i,coords[idx]);
        break;
      }
      case UG2d::QUADRILATERAL : {

        const double coords[9][2] = {
          // The corners
          {0,0}, {1,0}, {1,1}, {0,1},
          // The edge midpoints
          {0.5,0}, {1,0.5}, {0.5,1}, {0,0.5},
          // The element midpoint
          {0.5,0.5}
        };
        geometryInFather_.setCoords(i,coords[idx]);
        break;
      }

      }

    } else {
      switch (UG_NS<dim>::Tag(fatherElement)) {
      case UG3d::TETRAHEDRON : {

        const double coords[10][3] = {
          // The corners
          {0,0,0}, {1,0,0}, {0,1,0}, {0,0,1},
          // The edge midpoints
          {0.5,0,0}, {0.5,0.5,0}, {0,0.5,0},
          {0,0,0.5}, {0.5,0,0.5}, {0,0.5,0.5}
        };
        geometryInFather_.setCoords(i,coords[idx]);
        break;
      }
      case UG3d::PYRAMID : {

        const double coords[14][3] = {
          // The corners
          {0,0,0}, {1,0,0}, {1,1,0}, {0,1,0}, {0,0,1},
          // The edge midpoints
          {0.5,0,0}, {1,0.5,0}, {0.5,1,0}, {0,0.5,0},
          {0,0,0.5}, {0.5,0,0.5}, {0.5,0.5,0.5}, {0,0.5,0.5},
          // The bottom face midpoint
          {0.5,0.5,0}
        };
        geometryInFather_.setCoords(i,coords[idx]);
        break;
      }
      case UG3d::PRISM : {
        // 19 is one too many but we have to include a dummy
        const double coords[19][3] = {
          // The corners
          {0,0,0}, {1,0,0}, {0,1,0}, {0,0,1}, {1,0,1}, {0,1,1},
          // The edge midpoints
          {0.5,0,0}, {0.5,0.5,0}, {0,0.5,0},
          {0,0,0.5}, {1,0,0.5}, {0,1,0.5},
          {0.5,0,1}, {0.5,0.5,1}, {0,0.5,1},
          // A dummy for a midpoint of one of the two triangular faces,
          // which doesn't exist, because triangular faces have no midnodes
          {0,0,0},
          // The midnodes of the three quadrilateral faces
          {0.5,0,0.5}, {0.5,0.5,0.5}, {0,0.5,0.5}
        };
        geometryInFather_.setCoords(i,coords[idx]);
        break;
      }
      case UG3d::HEXAHEDRON : {

        const double coords[27][3] = {
          // The corners
          {0,0,0}, {1,0,0}, {1,1,0}, {0,1,0}, {0,0,1}, {1,0,1}, {1,1,1}, {0,1,1},
          // The edge midpoints
          {0.5,0,0}, {1,0.5,0}, {0.5,1,0}, {0,0.5,0},
          {0,0,0.5}, {1,0,0.5}, {1,1,0.5}, {0,1,0.5},
          {0.5,0,1}, {1,0.5,1}, {0.5,1,1}, {0,0.5,1},
          // The face midpoints
          {0.5,0.5,0}, {0.5,0,0.5}, {1,0.5,0.5}, {0.5,1,0.5}, {0,0.5,0.5}, {0.5,0.5,1},
          // The element midpoint
          {0.5,0.5,0.5}
        };
        geometryInFather_.setCoords(i,coords[idx]);
        break;
      }
      }
    }

  }

  return geometryInFather_;
}

template class Dune::UGGridEntity<2,2, const Dune::UGGrid<2,2> >;
template class Dune::UGGridEntity<3,3, const Dune::UGGrid<3,3> >;

template class Dune::UGGridEntity<0,2, const Dune::UGGrid<2,2> >;
template class Dune::UGGridEntity<0,3, const Dune::UGGrid<3,3> >;

template int Dune::UGGridEntity<0, 2, const Dune::UGGrid<2, 2> >::count<0>() const;
template int Dune::UGGridEntity<0, 2, const Dune::UGGrid<2, 2> >::count<1>() const;
template int Dune::UGGridEntity<0, 2, const Dune::UGGrid<2, 2> >::count<2>() const;

template int Dune::UGGridEntity<0, 3, const Dune::UGGrid<3, 3> >::count<0>() const;
template int Dune::UGGridEntity<0, 3, const Dune::UGGrid<3, 3> >::count<1>() const;
template int Dune::UGGridEntity<0, 3, const Dune::UGGrid<3, 3> >::count<2>() const;
template int Dune::UGGridEntity<0, 3, const Dune::UGGrid<3, 3> >::count<3>() const;


template Dune::Grid<2, 2, double, Dune::UGGridFamily<2, 2> >::Codim<0>::EntityPointer
Dune::UGGridEntity<0, 2, const Dune::UGGrid<2, 2> >::entity<0>(int) const;

template Dune::Grid<2, 2, double, Dune::UGGridFamily<2, 2> >::Codim<1>::EntityPointer
Dune::UGGridEntity<0, 2, const Dune::UGGrid<2, 2> >::entity<1>(int) const;

template Dune::Grid<2, 2, double, Dune::UGGridFamily<2, 2> >::Codim<2>::EntityPointer
Dune::UGGridEntity<0, 2, const Dune::UGGrid<2, 2> >::entity<2>(int) const;


template Dune::Grid<3, 3, double, Dune::UGGridFamily<3, 3> >::Codim<0>::EntityPointer
Dune::UGGridEntity<0, 3, const Dune::UGGrid<3, 3> >::entity<0>(int) const;

template Dune::Grid<3, 3, double, Dune::UGGridFamily<3, 3> >::Codim<1>::EntityPointer
Dune::UGGridEntity<0, 3, const Dune::UGGrid<3, 3> >::entity<1>(int) const;

template Dune::Grid<3, 3, double, Dune::UGGridFamily<3, 3> >::Codim<2>::EntityPointer
Dune::UGGridEntity<0, 3, const Dune::UGGrid<3, 3> >::entity<2>(int) const;

template Dune::Grid<3, 3, double, Dune::UGGridFamily<3, 3> >::Codim<3>::EntityPointer
Dune::UGGridEntity<0, 3, const Dune::UGGrid<3, 3> >::entity<3>(int) const;
