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

    return UG_NS<dimworld>::NbElem(center_, neighborCount_);
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
           && neighborCount_ < UG_NS<dimworld>::Sides_Of_Elem(center_);
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
    return UG_NS<3>::NbElem(center_, neighborCount_) == NULL;
  }
#endif

#ifdef _2
  template<>
  inline bool
  UGGridIntersectionIterator < 2,2 >::boundary()
  {
    return UG_NS<2>::NbElem(center_, neighborCount_) == NULL;
  }
#endif

  /** \todo Test this for locally refined grids! */
  template<int dim, int dimworld>
  inline bool
  UGGridIntersectionIterator<dim, dimworld>::neighbor()
  {
    //std::cout << "neighbor not implemented yet!" << std::endl;
    return !boundary();
  }

  template<>
  inline FieldVector<UGCtype, 3>&
  UGGridIntersectionIterator < 3,3 >::unit_outer_normal ()
  {
#ifdef _3
    // Get the first three vertices of this side.  Since quadrilateral faces
    // are plane in UG, the normal doesn't depend on the fourth vertex
#define CORNER_OF_SIDE(p, s, c)   (UG3d::element_descriptors[UG_NS<3>::Tag(p)]->corner_of_side[(s)][(c)])
    UG3d::VERTEX* a = UG_NS<3>::Corner(center_,CORNER_OF_SIDE(center_, neighborCount_, 0))->myvertex;
    UG3d::VERTEX* b = UG_NS<3>::Corner(center_,CORNER_OF_SIDE(center_, neighborCount_, 1))->myvertex;
    UG3d::VERTEX* c = UG_NS<3>::Corner(center_,CORNER_OF_SIDE(center_, neighborCount_, 2))->myvertex;
#undef CORNER_OF_SIDE

    FieldVector<UGCtype, 3> aPos, bPos, cPos;

#define CVECT(p)   ((p)->iv.x)
#define V3_COPY(A,C)    {(C)[0] = (A)[0];   (C)[1] = (A)[1];   (C)[2] = (A)[2];\
}
    V3_COPY(CVECT(a), aPos);
    V3_COPY(CVECT(b), bPos);
    V3_COPY(CVECT(c), cPos);
#undef CVECT
#undef V3_COPY

    FieldVector<UGCtype, 3> ba = bPos - aPos;
    FieldVector<UGCtype, 3> ca = cPos - aPos;

    //     std::cout << "aPos: " << aPos << std::endl;
    //     std::cout << "bPos: " << bPos << std::endl;
    //     std::cout << "cPos: " << cPos << std::endl;

#define V3_VECTOR_PRODUCT(A,B,C) {(C)[0] = (A)[1]*(B)[2] - (A)[2]*(B)[1];\
                                  (C)[1] = (A)[2]*(B)[0] - (A)[0]*(B)[2];\
                                  (C)[2] = (A)[0]*(B)[1] - (A)[1]*(B)[0];}

    V3_VECTOR_PRODUCT(ba, ca, outerNormal_);
#undef V3_VECTOR_PRODUCT

    // normalize
    outerNormal_ *= (1/outerNormal_.two_norm());
#endif
    return outerNormal_;

  }

  template<>
  inline FieldVector<UGCtype, 2>&
  UGGridIntersectionIterator < 2,2 >::unit_outer_normal ()
  {
#ifdef _2
    // Get the vertices of this side.
#define CORNER_OF_SIDE(p, s, c)   (UG2d::element_descriptors[UG_NS<2>::Tag(p)]->corner_of_side[(s)][(c)])
    UGCtype* aPos = UG_NS<2>::Corner(center_,CORNER_OF_SIDE(center_, neighborCount_, 0))->myvertex->iv.x;
    UGCtype* bPos = UG_NS<2>::Corner(center_,CORNER_OF_SIDE(center_, neighborCount_, 1))->myvertex->iv.x;
#undef CORNER_OF_SIDE

    // compute normal
    outerNormal_[0] = bPos[1] - aPos[1];
    outerNormal_[1] = aPos[0] - bPos[0];

    // normalize
    outerNormal_ *= (1/outerNormal_.two_norm());
#endif
    return outerNormal_;
  }

  template<int dim, int dimworld>
  inline FieldVector<UGCtype, dimworld>&
  UGGridIntersectionIterator < dim,dimworld >::
  unit_outer_normal (const FieldVector<UGCtype, dim-1>& local)
  {
    return unit_outer_normal();
  }

  template< int dim, int dimworld>
  inline UGGridElement< dim-1, dim >&
  UGGridIntersectionIterator<dim,dimworld>::
  intersection_self_local()
  {
    std::cout << "\nintersection_self_local not implemented yet!\n";
    return fakeNeigh_;
  }

  template< int dim, int dimworld>
  inline UGGridElement< dim-1, dimworld >&
  UGGridIntersectionIterator<dim,dimworld>::
  intersection_self_global()
  {

    //#define CORNERS_OF_SIDE(p, i)   (UG2d::element_descriptors[UG_NS<dimworld>::Tag(p)]->corners_of_side[(i)])
    int numCornersOfSide = UG_NS<dimworld>::Corners_Of_Side(center_, neighborCount_);
    //#undef CORNERS_OF_SIDE

    //#define         CORNER_OF_SIDE(p, s, c)   (UG2d::element_descriptors[UG_NS<dimworld>::Tag(p)]->corner_of_side[(s)][(c)])


    //std::cout << "Element side has " << numCornersOfSide << " corners" << std::endl;

    for (int i=0; i<numCornersOfSide; i++) {

      int cornerIdx = UG_NS<dimworld>::Corner_Of_Side(center_, neighborCount_, i);
      typename TargetType<dim,dim>::T* node = UG_NS<dimworld>::Corner(center_, cornerIdx);
      for (int j=0; j<dimworld; j++)
        neighGlob_.coord_[i][j] = node->myvertex->iv.x[j];

    }

    //#undef CORNER_OF_SIDE

    return neighGlob_;
  }

  template< int dim, int dimworld>
  inline UGGridElement< dim-1, dim >&
  UGGridIntersectionIterator<dim,dimworld>::
  intersection_neighbor_local()
  {
    std::cout << "\nintersection_neighbor_local not implemented yet!\n";
    return fakeNeigh_;
  }

  template< int dim, int dimworld>
  inline UGGridElement< dim-1, dimworld >&
  UGGridIntersectionIterator<dim,dimworld>::
  intersection_neighbor_global()
  {
    // According to Peter this method is supposed to do precisely the
    // same as intersection_self_global()
    return intersection_self_global();
  }

  template< int dim, int dimworld>
  inline int UGGridIntersectionIterator<dim,dimworld>::
  number_in_self ()
  {
    /** \todo Muﬂ ich die Seitennummer wirklich umrechnen? */
    const int nSides = UG_NS<dimworld>::Sides_Of_Elem(center_);

    return (neighborCount_ + nSides -1)%nSides;
  }

  template< int dim, int dimworld>
  inline int UGGridIntersectionIterator<dim,dimworld>::
  number_in_neighbor ()
  {
    const typename TargetType<0,dimworld>::T* other = target();

    /** \todo Muﬂ ich die Seitennummer wirklich umrechnen? */
    const int nSides = UG_NS<dimworld>::Sides_Of_Elem(other);

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
