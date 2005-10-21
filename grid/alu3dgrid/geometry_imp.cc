// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "grid.hh"
#include "mappings.hh"

namespace Dune {
  // --Geometry

  //- Tetra specialization
  template<int mydim, int cdim>
  inline ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> >::
  ALU3dGridGeometry()
    : builtinverse_ (false) , builtA_ (false) , builtDetDF_ (false)
  {}

  //   B U I L T G E O M   - - -

  template<int mydim, int cdim>
  inline void ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> >::
  calcElMatrix () const
  {
    if(!builtA_)
    {
      // creat Matrix A (=Df)               INDIZES: row/col
      // Mapping: R^dim -> R^3,  F(x) = A x + p_0
      // columns:    p_1 - p_0  |  p_2 - p_0  |  p_3 - p_0

      for (int i=0; i<mydim; i++) {
        AT_[i] = coord_[i+1] - coord_[0];
      }
      builtA_ = true;
    }
  }

  //dim = dimworld = 3
  template<int mydim, int cdim>
  inline void ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> > :: buildJacobianInverseTransposed() const
  {
    if(!builtinverse_)
    {
      calcElMatrix();

      // DetDf = integrationElement, invert transposed Matrix
      detDF_ = std::abs( FMatrixHelp::invertMatrix(AT_,Jinv_) );
      builtinverse_ = builtDetDF_ = true;
    }
  }

  template<> //dim = 2 , dimworld = 3
  inline void ALU3dGridGeometry<2,3, const ALU3dGrid<3,3,tetra> > :: buildJacobianInverseTransposed() const
  {
    if(!builtinverse_)
    {
      enum { dim = 3 };

      //derr << "WARNING: ALU3dGridGeometry::buildJacobianInverseTransposed not tested yet! " << __LINE__ <<"\n";
      // create vectors of face
      tmpV_ = coord_[1] - coord_[0];
      tmpU_ = coord_[2] - coord_[1];

      // calculate scaled outer normal
      for(int i=0; i<dim; i++)
      {
        globalCoord_[i] = (  tmpU_[(i+1)%dim] * tmpV_[(i+2)%dim]
                             - tmpU_[(i+2)%dim] * tmpV_[(i+1)%dim] );
      }

      detDF_ = std::abs ( globalCoord_.two_norm() );
      builtinverse_ = builtDetDF_ = true;
    }
  }

  template<> //dim = 1 , dimworld = 3
  inline void ALU3dGridGeometry<1,3, const ALU3dGrid<3,3,tetra> > :: buildJacobianInverseTransposed() const
  {
    if(!builtinverse_)
    {
      enum { dim = 3 };
      //derr << "WARNING: ALU3dGridGeometry::buildJacobianInverseTransposed not tested yet! " << __LINE__ <<"\n";
      // create vectors of face
      globalCoord_ = coord_[1] - coord_[0];
      detDF_ = std::abs ( globalCoord_.two_norm() );
      builtinverse_ = builtDetDF_ = true;
    }
  }

  template<> //dim = 1 , dimworld = 3
  inline void ALU3dGridGeometry<0,3, const ALU3dGrid<3,3,tetra> > :: buildJacobianInverseTransposed() const
  {
    if(!builtinverse_)
    {
      enum { dim = 3 };
      detDF_ = 1.0;
      builtinverse_ = builtDetDF_ = true;
    }
  }

  template <>
  inline bool ALU3dGridGeometry<3,3, const ALU3dGrid<3,3,tetra> > ::
  buildGeom(const IMPLElementType & item, int, int)
  {
    enum { dim = 3 };
    enum { dimworld = 3};

    builtinverse_ = builtA_ = builtDetDF_ = false;

    for (int i=0; i<(dim+1); i++)
    {
      const double (&p)[3] =
        item.myvertex(ElementTopo::dune2aluVertex(i))->Point();
      for (int j=0; j<dimworld; j++)
      {
        coord_[i][j] = p[j];
      }
    }
    return true;
  }

  template <>
  inline bool ALU3dGridGeometry<3,3, const ALU3dGrid<3,3,tetra> > ::
  buildGhost(const PLLBndFaceType & ghost)
  {
    enum { dim = 3 };
    enum { dimworld = 3};

    builtinverse_ = builtA_ = builtDetDF_ = false;

    GEOFaceType & face = dynamic_cast<GEOFaceType &> (*(ghost.myhface3(0)));

    // here apply the negative twist, because the twist is from the
    // neighbouring element's point of view which is outside of the ghost
    // element
    const int map[3] = { (ghost.twist(0) < 0) ? 2 : 0 , 1 , (ghost.twist(0) < 0) ? 0 : 2 };

    for (int i=0; i<dim; i++) // col is the point vector
    {
      const double (&p)[3] = face.myvertex(map[i])->Point();
      for (int j=0; j<dimworld; j++) // row is the coordinate of the point
      {
        coord_[i][j] = p[j];
      }
    }

    {
      const double (&p)[3] = ghost.oppositeVertex(0);
      for (int j=0; j<dimworld; j++)
      {
        coord_[3][j] = p[j];
      }
    }

    return true;
  }

  template <>
  inline bool ALU3dGridGeometry<2,3, const ALU3dGrid<3,3,tetra> > ::
  buildGeom(const ALU3DSPACE HFaceType & item, int twist, int)
  {
    enum { dim = 2 };
    enum { dimworld = 3};

    builtinverse_ = builtA_ = builtDetDF_ = false;

    for (int i=0; i<(dim+1); i++)
    {
      // Transform Dune index to ALU index and apply twist
      int localALUIndex = FaceTopo::dune2aluVertex(i);
      int rotatedALUIndex = FaceTopo::twist(localALUIndex, twist);

      const double (&p)[3] =
        static_cast<const GEOFaceType &>(item).myvertex(rotatedALUIndex)->Point();
      for (int j=0; j<dimworld; j++)
      {
        coord_[i][j] = p[j];
      }
    }

    buildJacobianInverseTransposed();
    return true;
  }

  template <>
  inline bool ALU3dGridGeometry<2, 3, const ALU3dGrid<3,3,tetra> > ::
  buildGeom(const FaceCoordinatesType& coords) {
    enum { dim = 2 };
    enum { dimworld = 3};

    builtinverse_ = builtA_ = builtDetDF_ = false;

    for (int i = 0; i < (dim+1); ++i) {
      coord_[i] = coords[i];
    }

    buildJacobianInverseTransposed();
    return true;
  }

  template <> // for edges
  inline bool ALU3dGridGeometry<1,3, const ALU3dGrid<3,3,tetra> > ::
  buildGeom(const ALU3DSPACE HEdgeType & item, int twist, int)
  {
    enum { dim = 1 };
    enum { dimworld = 3};

    builtinverse_ = builtA_ = builtDetDF_ = false;

    for (int i=0; i<(dim+1); i++)
    {
      const double (&p)[3] =
        static_cast<const GEOEdgeType &> (item).myvertex((i+twist)%2)->Point();
      for (int j=0; j<dimworld; j++)
      {
        coord_[i][j] = p[j];
      }
    }

    buildJacobianInverseTransposed();
    return true;
  }

  template <> // for Vertices ,i.e. Points (note that twist is a dummy parameter here, needed for consistency)
  inline bool ALU3dGridGeometry<0,3, const ALU3dGrid<3,3,tetra> > ::
  buildGeom(const ALU3DSPACE VertexType & item, int, int)
  {
    enum { dim = 0 };
    enum { dimworld = 3};

    builtinverse_ = builtA_ = builtDetDF_ = false;

    const double (&p)[3] = static_cast<const GEOVertexType &> (item).Point();
    for (int j=0; j<dimworld; j++) coord_[0][j] = p[j];

    buildJacobianInverseTransposed();
    return true;
  }


  /* Comment in for adaptation to new GeometryType */
  template <int mydim, int cdim>
  inline GeometryType
  ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> > ::type () const {
    return simplex;
  }

  template <int mydim, int cdim>
  inline GeometryType
  ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, hexa> > ::type () const {
    return cube;
  }

  template<int mydim, int cdim>
  inline int ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> > ::corners () const
  {
    return dimbary;
  }

  template<int mydim, int cdim>
  inline const FieldVector<alu3d_ctype, cdim>&
  ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> > :: operator[] (int i) const
  {
    assert((i>=0) && (i < mydim+1));
    return coord_[i];
  }

  template<int mydim, int cdim>
  inline FieldVector<alu3d_ctype, cdim>&
  ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> > :: getCoordVec (int i)
  {
    assert((i>=0) && (i < mydim+1));
    return coord_[i];
  }

  //   G L O B A L   - - -

  // dim = 1,2,3 dimworld = 3
  template<int mydim, int cdim>
  inline FieldVector<alu3d_ctype, cdim>
  ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> >::
  global(const FieldVector<alu3d_ctype, mydim>& local) const
  {
    calcElMatrix();

    globalCoord_ = coord_[0];
    // multiply with transposed because AT is also transposed
    AT_.umtv(local,globalCoord_);
    return globalCoord_;
  }

  template<> // dim = dimworld = 3
  inline FieldVector<alu3d_ctype, 3>
  ALU3dGridGeometry<3,3,const ALU3dGrid<3,3,tetra> > ::
  local(const FieldVector<alu3d_ctype, 3>& global) const
  {
    if (!builtinverse_) buildJacobianInverseTransposed();

    globalCoord_ = global - coord_[0];

    // multiply with transposed because Jinv_ is already transposed
    localCoord_ = FMatrixHelp:: multTransposed(Jinv_,globalCoord_);
    return localCoord_;
  }

  template<int mydim, int cdim>
  inline bool ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> > ::
  checkInside(const FieldVector<alu3d_ctype, mydim>& local) const
  {
    alu3d_ctype sum = 0.0;

    for(int i=0; i<mydim; i++)
    {
      sum += local[i];
      if(local[i] < 0.0)
      {
        if(std::abs(local[i]) > ALUnumericEpsilon)
        {
          return false;
        }
      }
    }

    if( sum > 1.0 )
    {
      if(sum > (1.0 + ALUnumericEpsilon))
        return false;
    }
    return true;
  }

  template<int mydim, int cdim>
  inline alu3d_ctype
  ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> > ::integrationElement (const FieldVector<alu3d_ctype, mydim>& local) const
  {
    if(builtDetDF_)
      return detDF_;

    calcElMatrix();

    detDF_ = AT_.determinant();

    assert(detDF_ > 0.0);

    builtDetDF_ = true;
    return detDF_;
  }

  //  J A C O B I A N _ I N V E R S E  - - -

  template<> // dim = dimworld = 3
  inline const FieldMatrix<alu3d_ctype,3,3> &
  ALU3dGridGeometry<3,3, const ALU3dGrid<3,3,tetra> >::
  jacobianInverseTransposed (const FieldVector<alu3d_ctype, 3>& local) const
  {
    if (!builtinverse_) buildJacobianInverseTransposed();
    return Jinv_;
  }

  // print the ElementInformation
  template<int mydim, int cdim>
  inline void ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> >::print (std::ostream& ss) const
  {
    ss << "ALU3dGridGeometry<" << mydim << "," << cdim << ", tetra> = {\n";
    for(int i=0; i<corners(); i++)
    {
      ss << " corner " << i << " ";
      ss << "{" << ((*this)[i]) << "}"; ss << std::endl;
    }
    ss << "} \n";
  }

  //- Hexahedron specialization
  template <int mydim, int cdim>
  inline ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> >::
  ALU3dGridGeometry() :
    coord_(0.0),
    triMap_(0),
    biMap_(0)
  {}

  template <>
  inline ALU3dGridGeometry<3, 3, const ALU3dGrid<3, 3, hexa> >::
  ALU3dGridGeometry() :
    coord_(0.0),
    triMap_(0),
    biMap_(0)
  {}

  template <>
  inline ALU3dGridGeometry<2, 3, const ALU3dGrid<3, 3, hexa> >::
  ALU3dGridGeometry()
    : coord_(0.0),
      triMap_(0),
      biMap_(0)
  {}

  template <>
  inline ALU3dGridGeometry<2, 2, const ALU3dGrid<3, 3, hexa> >::
  ALU3dGridGeometry()
    : coord_(0.0),
      triMap_(0),
      biMap_(0)
  {}


  template <int mydim, int cdim>
  ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> >::
  ~ALU3dGridGeometry() {
    delete triMap_;
    delete biMap_;
  }

  template <int mydim, int cdim>
  inline int
  ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> >::corners() const {
    return Power_m_p<2,mydim>::power;
  }

  template<int mydim, int cdim>
  inline FieldVector<alu3d_ctype, cdim>&
  ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, hexa> > :: getCoordVec (int i)
  {
    assert((i>=0) && (i < corners()));
    return coord_[i];
  }

  template <int mydim, int cdim>
  const FieldVector<alu3d_ctype, cdim>&
  ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> >::
  operator[] (int i) const {
    assert((i >= 0) && (i < corners()));
    return coord_[i];
  }

  template <>
  inline FieldVector<alu3d_ctype, 3>
  ALU3dGridGeometry<3, 3, const ALU3dGrid<3, 3, hexa> >::
  global (const FieldVector<alu3d_ctype, 3>& local) const {
    assert(triMap_);
    triMap_->map2world(local, tmp2_);
    return tmp2_;
  }

  template <>
  inline FieldVector<alu3d_ctype, 3>
  ALU3dGridGeometry<2, 3, const ALU3dGrid<3, 3, hexa> >::
  global (const FieldVector<alu3d_ctype, 2>& local) const {
    assert(biMap_);
    biMap_->map2world(local, tmp2_);
    return tmp2_;
  }

  template <>
  inline FieldVector<alu3d_ctype, 3>
  ALU3dGridGeometry<3, 3, const ALU3dGrid<3, 3, hexa> >::
  local (const FieldVector<alu3d_ctype, 3>& global) const {
    assert(triMap_);
    triMap_->world2map(global, tmp2_);
    return tmp2_;
  }

  template <>
  inline FieldVector<alu3d_ctype, 2>
  ALU3dGridGeometry<2, 3, const ALU3dGrid<3, 3, hexa> >::
  local (const FieldVector<alu3d_ctype, 3>& global) const {
    assert(false); // could be a bit tricky, eh?
    //biMap_->world2map(global, tmp1_);
    return FieldVector<alu3d_ctype, 2>();
  }

  template <int mydim, int cdim>
  bool
  ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> >::
  checkInside(const FieldVector<alu3d_ctype, mydim>& local) const {
    bool result = true;
    for (int i = 0; i < mydim; i++ ) {
      result &= (local[i] >= 0.0 && local[i] <= 1.0);
    }
    return result;
  }

  template<>
  inline alu3d_ctype
  ALU3dGridGeometry<3, 3, const ALU3dGrid<3, 3, hexa> >::
  integrationElement (const FieldVector<alu3d_ctype, 3>& local) const {
    assert(triMap_);
    return triMap_->det(local);
  }

  template<>
  inline alu3d_ctype
  ALU3dGridGeometry<2, 3, const ALU3dGrid<3, 3, hexa> >::
  integrationElement (const FieldVector<alu3d_ctype, 2>& local) const {
    assert(biMap_);
    biMap_->normal(local, tmp2_);
    return tmp2_.two_norm();
  }

  template <>
  inline const FieldMatrix<alu3d_ctype, 3, 3>&
  ALU3dGridGeometry<3, 3, const ALU3dGrid<3, 3, hexa> >::
  jacobianInverseTransposed (const FieldVector<alu3d_ctype, 3>& local) const {
    assert(triMap_);
    jInv_ = triMap_->jacobianInverse(local);
    return jInv_;
  }

  template <int mydim, int cdim>
  void
  ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> >::
  print (std::ostream& ss) const {
    ss << "ALU3dGridGeometry<" << mydim << "," << cdim << ", hexa> = {\n";
    for(int i=0; i<corners(); i++)
    {
      ss << " corner " << i << " ";
      ss << "{" << ((*this)[i]) << "}"; ss << std::endl;
    }
    ss << "} \n";
  }

  template <>
  inline bool
  ALU3dGridGeometry<3, 3, const ALU3dGrid<3, 3, hexa> >::
  buildGeom(const IMPLElementType& item, int , int ) {
    enum { dim = 3 };
    enum { dimworld = 3 };

    for (int i = 0; i < corners(); ++i) {
      const double (&p)[3] =
        item.myvertex(ElementTopo::dune2aluVertex(i))->Point();
      for (int j = 0; j < dimworld; ++j) {
        coord_[i][j] = p[j];
      }
    }

    if(triMap_) delete triMap_;
    triMap_ = new TrilinearMapping(coord_[0], coord_[1], coord_[2], coord_[3],
                                   coord_[4], coord_[5], coord_[6], coord_[7]);

    return true;
  }

  template <>
  inline bool
  ALU3dGridGeometry<3,3, const ALU3dGrid<3,3,hexa> >::
  buildGhost(const PLLBndFaceType & ghost) {
    enum { dim = 3 };
    enum { dimworld = 3 };

    GEOFaceType & face = dynamic_cast<GEOFaceType &> (*(ghost.myhface4(0)));

    // The ghost element can be oriented to your liking. The convention here is
    // the following: the 0th vertex of the face is mapped to the 0th vertex of
    // the ghost entity. mapFront takes into account the different numbering
    // conventions of dune and alugrid and the twist of the face. (Take into
    // account that a twist is defined with regard to the inner entity, so it is
    // actually the opposite of the twist with respect to the ghost...
    //
    //  (dune)   4 ------ 5     neg. twist: (alu)     pos. twist: (alu)
    //           /|     /|            .      .              .      .
    //          / |    / |           .      .              .      .
    //        0 ------ 1 |         0 ------ 3            0 ------ 1
    //        .| 6 --.|-- 7         |      |              |      |
    //       . | /  . | /           | .    | .            | .    | .
    //      .  |/  .  |/            |.     |.             |.     |.
    //        2 ------ 3           1 ------ 2            3 ------ 2
    //       .       .
    //      .       .
    //
    // mapFront: i \in reference hexahedron vertex index dune -> l \in reference
    // quad face vertex index alu + twist
    // Note: due to the vertex numbering for dune hexahedrons, mapFront can also
    // be used to map the back plane. The entries {0, 1, 2, 3} refer to the local
    // vertex numbers {4, 5, 6, 7} of the (dune) reference hexahedron then
    bool negativeTwist = ghost.twist(0) < 0;
    const int mapFront[4] = { 0,
                              negativeTwist ? 3 : 1,
                              negativeTwist ? 1 : 3,
                              2 };

    // Store the coords of the ghost element incident with the boundary face
    // 4 is the number of vertices of the boundary faces for hexas
    for (int i = 0; i < 4; ++i) {
      const double (&p)[3] = face.myvertex(mapFront[i])->Point();
      for (int j = 0; j < dimworld; ++j) {
        coord_[i][j] = p[j];
      }
    }

    // 4 is the number of vertices of the face opposite the boundary
    for (int i = 0; i < 4; ++i) {
      const double (&p)[3] = ghost.oppositeVertex(mapFront[i]);
      for (int j = 0; j < dimworld; ++j) {
        coord_[4+i][j] = p[j];
      }
    }

    return true;
  }

  template <>
  inline bool
  ALU3dGridGeometry<2,3, const ALU3dGrid<3, 3, hexa> > ::
  buildGeom(const ALU3DSPACE HFaceType & item, int twist, int faceNum ) {
    enum { dim = 2 };
    enum { dimworld = 3 };

    const GEOFaceType& face = static_cast<const GEOFaceType&> (item);

    for (int i = 0; i < 4; ++i)
    {
      // Transform Dune index to ALU index and apply twist
      int localALUIndex = ElementTopo::dune2aluFaceVertex(faceNum,i);
      int rotatedALUIndex = FaceTopo::twist(localALUIndex, twist);

      const double (&p)[3] =
        face.myvertex(rotatedALUIndex)->Point();
      for (int j = 0; j < dimworld; ++j)
      {
        coord_[i][j] = p[j];
      }
    }

    if(biMap_) delete biMap_;
    biMap_ = new BilinearSurfaceMapping(coord_[0], coord_[1],
                                        coord_[2], coord_[3]);

    return true;
  }

  template <>
  inline bool
  ALU3dGridGeometry<2,3, const ALU3dGrid<3, 3, hexa> > ::
  buildGeom(const FaceCoordinatesType& coords) {
    enum { dim = 2 };
    enum { dimworld = 3 };

    for (int i = 0; i < 4; ++i) {
      coord_[i] = coords[i];
    }

    if(biMap_) delete biMap_;
    biMap_ = new BilinearSurfaceMapping(coord_[0], coord_[1],
                                        coord_[2], coord_[3]);

    return true;
  }

  template <> // for edges
  inline bool
  ALU3dGridGeometry<1,3, const ALU3dGrid<3, 3, hexa> >::
  buildGeom(const ALU3DSPACE HEdgeType & item, int twist, int) {
    enum { dim = 1 };
    enum { dimworld = 3 };

    for (int i = 0; i < 2; ++i) {
      const double (&p)[3] =
        static_cast<const GEOEdgeType &> (item).myvertex((i+twist)%2)->Point();
      for (int j = 0; j < dimworld; ++j) {
        coord_[i][j] = p[j];
      }
    }

    return true;
  }

  template <> // for Vertices ,i.e. Points
  inline bool
  ALU3dGridGeometry<0,3, const ALU3dGrid<3,3,hexa> >::
  buildGeom(const ALU3DSPACE VertexType & item, int twist, int) {
    enum { dim = 0 };
    enum { dimworld = 3};

    const double (&p)[3] = static_cast<const GEOVertexType &> (item).Point();
    for (int j=0; j<dimworld; j++) coord_[0][j] = p[j];

    return true;
  }


} // end namespace Dune
