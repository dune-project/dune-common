// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALBERTAGRID_CC
#define DUNE_ALBERTAGRID_CC

//************************************************************************
//
//  implementation of AlbertaGrid
//
//  namespace Dune
//
//************************************************************************
namespace Dune
{

  static ALBERTA EL_INFO statElInfo[DIM+1];

  //****************************************************************
  //
  // --AlbertaGridGeometry
  // --Geometry
  //
  //****************************************************************
  // default, do nothing
  template <int mydim, int cdim, class GridImp>
  inline int AlbertaGridGeometry<mydim,cdim,GridImp>::mapVertices (int i) const
  {
    // there is a specialisation for each combination of mydim and coorddim
    return ALBERTA AlbertHelp :: MapVertices<mydim,cdim>::mapVertices(i,face_,edge_,vertex_);
  }

  template <int mydim, int cdim, class GridImp>
  inline AlbertaGridGeometry<mydim,cdim,GridImp>::
  AlbertaGridGeometry() : myGeomType_(GeometryType::simplex,mydim)
  {
    // make empty element
    initGeom();
  }

  template <int mydim, int cdim, class GridImp>
  inline AlbertaGridGeometry<mydim,cdim,GridImp>::
  AlbertaGridGeometry(const int child) : myGeomType_(GeometryType::simplex,mydim)
  {
    // make empty element
    buildGeomInFather(child);
  }

  template <int mydim, int cdim, class GridImp>
  inline ALBERTA EL_INFO * AlbertaGridGeometry<mydim,cdim,GridImp>::
  makeEmptyElInfo()
  {
    ALBERTA EL_INFO * elInfo = &statElInfo[mydim];

    elInfo->mesh = 0;
    elInfo->el = 0;
    elInfo->parent = 0;
    elInfo->macro_el = 0;
    elInfo->level = 0;
#if DIM > 2
    elInfo->orientation = 0;
    elInfo->el_type = 0;
#endif

    for(int i =0; i<mydim+1; i++)
    {
      for(int j =0; j< cdim; j++)
      {
        elInfo->coord[i][j] = 0.0;
        elInfo->opp_coord[i][j] = 0.0;
      }
      elInfo->bound[i] = 0;
    }
    return elInfo;
  }

  template <int mydim, int cdim, class GridImp>
  inline void AlbertaGridGeometry<mydim,cdim,GridImp>::
  initGeom()
  {
    elInfo_ = 0;
    face_ = 0;
    edge_ = 0;
    vertex_ = 0;
    builtinverse_ = false;
    builtElMat_   = false;
    calcedDet_    = false;
  }

  // print the GeometryInformation
  template <int mydim, int cdim, class GridImp>
  inline void AlbertaGridGeometry<mydim,cdim,GridImp>::print (std::ostream& ss) const
  {
    ss << "AlbertaGridGeometry<" << mydim << "," << cdim << "> = { \n";
    for(int i=0; i<corners(); i++)
    {
      ss << " corner " << i << " = ";
      ss << ((*this)[i]); ss << "\n";
    }
    ss << "} \n";
  }

  template <int mydim, int cdim, class GridImp>
  inline const GeometryType & AlbertaGridGeometry<mydim,cdim,GridImp>::type() const
  {
    return myGeomType_;
  }

  template <int mydim, int cdim, class GridImp>
  inline int AlbertaGridGeometry<mydim,cdim,GridImp>::corners() const
  {
    return (mydim+1);
  }

  ///////////////////////////////////////////////////////////////////////
  template <int mydim, int cdim, class GridImp>
  inline const FieldVector<albertCtype, cdim>& AlbertaGridGeometry<mydim,cdim,GridImp>::
  operator [](int i) const
  {
    return coord_[i];
  }

  ///////////////////////////////////////////////////////////////////////
  template <int mydim, int cdim, class GridImp>
  inline FieldVector<albertCtype, cdim>& AlbertaGridGeometry<mydim,cdim,GridImp>::
  getCoordVec (int i)
  {
    assert( i >= 0 );
    assert( i < mydim+1 );

    // if global, or jacobianInverse is called then
    // matrix has to be calculated again , because coord might have changed
    builtinverse_ = false;
    builtElMat_   = false;
    calcedDet_    = false;

    return coord_[i];
  }

  template <class GridImp, int mydim, int cdim>
  struct CalcElementMatrix
  {
    enum { matdim = (mydim > 0) ? mydim : 1 };
    static bool calcElMatrix(const FieldMatrix<albertCtype,mydim+1,cdim> & coord,
                             FieldMatrix<albertCtype,matdim,matdim> & elMat)
    {
      std::string text;
      text += "AlbertaGridGeometry<";
      char fake[128];
      sprintf(fake,"%d",mydim);
      text += fake; text += ",";
      sprintf(fake,"%d",cdim); text += ">::calcElMatrix: No default implementation!";
      DUNE_THROW(AlbertaError, text);
      return false;
    }
  };

  template <class GridImp>
  struct CalcElementMatrix<GridImp,1,2>
  {
    enum { mydim  = 1 };
    enum { cdim   = 2 };
    static bool calcElMatrix(const FieldMatrix<albertCtype,mydim+1,cdim> & coord,
                             FieldMatrix<albertCtype,cdim,mydim> & elMat)
    {
      //      column 0
      // A = ( P1 - P0 )
      for (int i=0; i<cdim; ++i)
      {
        elMat[i][0] = coord[1][i] - coord[0][i];
      }
      return true;
    }
  };

  template <class GridImp>
  struct CalcElementMatrix<GridImp,2,2>
  {
    enum { mydim  = 2 };
    enum { cdim   = 2 };
    enum { matdim = 2 };
    static bool calcElMatrix(const FieldMatrix<albertCtype,mydim+1,cdim> & coord,
                             FieldMatrix<albertCtype,matdim,matdim> & elMat)
    {
      //       column 0 , column 1
      // A = ( P1 - P0  , P2 - P0 )
      for (int i=0; i<cdim; ++i)
      {
        elMat[i][0] = coord[1][i] - coord[0][i];
        elMat[i][1] = coord[2][i] - coord[0][i];
      }
      return true;
    }
  };

  template <class GridImp>
  struct CalcElementMatrix<GridImp,2,3>
  {
    enum { mydim  = 2 };
    enum { cdim   = 3 };
    static bool calcElMatrix(const FieldMatrix<albertCtype,mydim+1,cdim> & coord,
                             FieldMatrix<albertCtype,cdim,mydim> & elMat)
    {
      //       column 0 , column 1
      // A = ( P1 - P0  , P2 - P0 )
      for (int i=0; i<cdim; ++i)
      {
        elMat[i][0] = coord[1][i] - coord[0][i];
        elMat[i][1] = coord[2][i] - coord[0][i];
      }
      return true;
    }
  };

  template <class GridImp>
  struct CalcElementMatrix<GridImp,3,3>
  {
    enum { mydim  = 3 };
    enum { cdim   = 3 };
    enum { matdim = 3 };
    static bool calcElMatrix(const FieldMatrix<albertCtype,mydim+1,cdim> & coord,
                             FieldMatrix<albertCtype,matdim,matdim> & elMat)
    {
      const FieldVector<albertCtype, cdim> & coord0 = coord[0];
      for(int i=0 ; i<cdim; ++i)
      {
        elMat[i][0] = coord[1][i] - coord0[i];
        elMat[i][1] = coord[2][i] - coord0[i];
        elMat[i][2] = coord[3][i] - coord0[i];
      }
      return true;
    }
  };

  template <int mydim, int cdim, class GridImp>
  inline void AlbertaGridGeometry<mydim,cdim,GridImp>::calcElMatrix () const
  {
    if(!builtElMat_)
    {
      // build mapping from reference element to actual element
      builtElMat_ = CalcElementMatrix<GridImp,mydim,cdim>::calcElMatrix(coord_,elMat_);
    }
  }

  template <int mydim, int cdim, class GridImp>
  inline FieldVector<albertCtype, cdim> AlbertaGridGeometry<mydim,cdim,GridImp>::
  global(const FieldVector<albertCtype, mydim>& local) const
  {
    calcElMatrix();

    globalCoord_ = coord_[0];
    elMat_.umv(local,globalCoord_);
    return globalCoord_;
  }

  //local implementation for mydim < cdim
  template <int mydim, int cdim, class GridImp>
  inline FieldVector<albertCtype, mydim> AlbertaGridGeometry<mydim,cdim,GridImp>::
  local(const FieldVector<albertCtype, cdim>& global) const
  {
    if(!builtinverse_)
      buildJacobianInverseTransposed();

    globalCoord_  = global;
    globalCoord_ -= coord_[0];

    AT_x_ = FMatrixHelp::multTransposed(elMat_,globalCoord_);
    localCoord_ = FMatrixHelp::mult(Jinv_,AT_x_);
    return localCoord_;
  }

  template <>
  inline FieldVector<albertCtype, 2> AlbertaGridGeometry<2,2,const AlbertaGrid<2,2> >::
  local(const FieldVector<albertCtype, 2>& global) const
  {
    if(!builtinverse_)
      buildJacobianInverseTransposed();

    globalCoord_  = global;
    globalCoord_ -= coord_[0];
    FMatrixHelp::multAssignTransposed(Jinv_,globalCoord_,localCoord_);

    return localCoord_;
  }

  template <>
  inline FieldVector<albertCtype, 3> AlbertaGridGeometry<3,3,const AlbertaGrid<3,3> >::
  local(const FieldVector<albertCtype, 3>& global) const
  {
    if(!builtinverse_)
      buildJacobianInverseTransposed();

    globalCoord_  = global;
    globalCoord_ -= coord_[0];
    FMatrixHelp::multAssignTransposed(Jinv_,globalCoord_,localCoord_);

    return localCoord_;
  }

  // determinant of one Geometry, here line
  template <>
  inline albertCtype AlbertaGridGeometry<1,2,const AlbertaGrid<2,2> >::elDeterminant () const
  {
    // volume is length of edge
    tmpZ_ = coord_[0] - coord_[1];
    return std::abs ( tmpZ_.two_norm() );
  }

  // determinant of one Geometry, here line
  template <>
  inline albertCtype AlbertaGridGeometry<1,3,const AlbertaGrid<3,3> >::elDeterminant () const
  {
    // volume is length of edge
    tmpZ_ = coord_[0] - coord_[1];
    return std::abs ( tmpZ_.two_norm() );
  }

  // determinant of one Geometry, here triangle
  template <>
  inline albertCtype AlbertaGridGeometry<2,2,const AlbertaGrid<2,2> >::elDeterminant () const
  {
    calcElMatrix();
    return std::abs ( elMat_.determinant () );
  }

  // determinant of one Geometry, here triangle in 3d
  template <>
  inline albertCtype AlbertaGridGeometry<2,3,const AlbertaGrid<3,3> >::elDeterminant () const
  {
    enum { dim = 3 };

    // create vectors of face
    tmpV_ = coord_[1] - coord_[0];
    tmpU_ = coord_[2] - coord_[1];

    // calculate scaled outer normal
    for(int i=0; i<dim; i++)
    {
      tmpZ_[i] = (  tmpU_[(i+1)%dim] * tmpV_[(i+2)%dim]
                    - tmpU_[(i+2)%dim] * tmpV_[(i+1)%dim] );
    }

    // tmpZ is the same as 2.0 * the outer normal
    return std::abs( tmpZ_.two_norm() );
  }

  // volume of one Geometry, here therahedron
  template <>
  inline albertCtype AlbertaGridGeometry<3,3,const AlbertaGrid<3,3> >::elDeterminant () const
  {
    calcElMatrix();
    return std::abs ( elMat_.determinant () );
  }

  // volume of one Geometry, here point
  template <>
  inline albertCtype AlbertaGridGeometry<0,2,const AlbertaGrid<2,2> >::elDeterminant () const
  {
    return 1.0;
  }
  // volume of one Geometry, here point
  template <>
  inline albertCtype AlbertaGridGeometry<0,3,const AlbertaGrid<3,3> >::elDeterminant () const
  {
    return 1.0;
  }

  // this method is for (dim==dimworld) = 2 and 3
  template <>
  inline void AlbertaGridGeometry<1,2,const AlbertaGrid<2,2> >::
  buildJacobianInverseTransposed() const
  {
    // calc A and stores it in elMat_
    calcElMatrix();
    assert( builtElMat_ == true );

    // calc ret = A^T*A
    FMatrixHelp::multTransposedMatrix(elMat_,elMatT_elMat_);

    // calc Jinv_ = (A^T*A)^-1
    std::abs( FMatrixHelp::invertMatrix(elMatT_elMat_,Jinv_) );
    builtinverse_ = true;
    return;
  }

  // this method is for (dim==dimworld) = 2 and 3
  template <>
  inline void AlbertaGridGeometry<2,3,const AlbertaGrid<3,3> >::
  buildJacobianInverseTransposed() const
  {
    // calc A and stores it in elMat_
    calcElMatrix();
    assert( builtElMat_ == true );

    // calc ret = A^T*A
    FMatrixHelp::multTransposedMatrix(elMat_,elMatT_elMat_);

    // calc Jinv_ = (A^T*A)^-1
    std::abs( FMatrixHelp::invertMatrix(elMatT_elMat_,Jinv_) );
    builtinverse_ = true;
    return;
  }

  // this method is for (dim==dimworld) = 2 and 3
  template <int mydim, int cdim, class GridImp>
  inline void AlbertaGridGeometry<mydim,cdim,GridImp>::
  buildJacobianInverseTransposed() const
  {
    //******************************************************
    //
    //  the mapping is:
    //
    //  F(T) = D where T is the reference element
    //  and D the actual element
    //
    //  F(x) = A * x + b    with   A := ( P_0 , P_1 )
    //
    //  A consist of the column vectors P_0 and P_1 and
    //  is calculated by the method calcElMatrix
    //
    //******************************************************

    // calc A and stores it in elMat_
    calcElMatrix();

    // Jinv = A^-1^T
    assert( builtElMat_ == true );
    // here the transposed jacobian inverse is calculated
    elDet_ = std::abs( FMatrixHelp::invertMatrix_retTransposed(elMat_,Jinv_) );

    assert(elDet_ > 1.0E-25 );

    calcedDet_ = true;
    builtinverse_ = true;
    return;
  }

  template <int mydim, int cdim, class GridImp>
  inline albertCtype AlbertaGridGeometry<mydim,cdim,GridImp>::
  integrationElement (const FieldVector<albertCtype, mydim>& local) const
  {
    assert( calcedDet_ );
    return elDet_;
    // if inverse was built, volume was calced already
    /*
       if(calcedDet_)
       return elDet_;

       elDet_ = elDeterminant();
       calcedDet_ = true;
       return elDet_;
     */
  }

  template <int mydim, int cdim, class GridImp>
  inline const FieldMatrix<albertCtype,mydim,mydim>& AlbertaGridGeometry<mydim,cdim,GridImp>::
  jacobianInverseTransposed (const FieldVector<albertCtype, mydim>& local) const
  {
    if(builtinverse_)
      return Jinv_;

    // builds the jacobian inverse and calculates the volume
    buildJacobianInverseTransposed();
    return Jinv_;
  }

  template <int mydim, int cdim, class GridImp>
  inline bool AlbertaGridGeometry<mydim,cdim,GridImp>::
  checkInside(const FieldVector<albertCtype, mydim> &local) const
  {
    albertCtype sum = 0.0;

    for(int i=0; i<mydim; i++)
    {
      sum += local[i];
      if(local[i] < 0.0)
      {
        if(std::abs(local[i]) > 1e-13)
        {
          return false;
        }
      }
    }

    if( sum > 1.0 )
    {
      if(sum > (1.0 + 1e-13))
        return false;
    }

    return true;
  }

  // built Geometry
  template <int mydim, int cdim, class GridImp>
  inline bool AlbertaGridGeometry<mydim,cdim,GridImp>::
  builtGeom(ALBERTA EL_INFO *elInfo, int face,
            int edge, int vertex)
  {
    elInfo_ = elInfo;
    face_ = face;
    edge_ = edge;
    vertex_ = vertex;
    builtinverse_ = false;
    builtElMat_   = false;

    if(elInfo_)
    {
      // copy coordinates
      for(int i=0; i<mydim+1; ++i)
      {
        const ALBERTA REAL_D & elcoord = elInfo_->coord[mapVertices(i)];
        // copy coordinates
        for(int j=0; j<cdim; ++j) coord_[i][j] = elcoord[j];
      }

      elDet_     = elDeterminant();
      calcedDet_ = true;
      // geometry built
      return true;
    }
    else
    {
      elDet_     = 0.0;
      calcedDet_ = false;
    }
    // geometry not built
    return false;
  }


  // specialization yields speed up, because vertex_ .. is not copied
  template <>
  inline bool AlbertaGridGeometry<2,2,const AlbertaGrid<2,2> >::
  builtGeom(ALBERTA EL_INFO *elInfo, int face,
            int edge, int vertex)
  {
    typedef AlbertaGrid<2,2> GridImp;
    enum { dim = 2 };
    enum { dimworld = 2 };

    elInfo_ = elInfo;
    builtinverse_ = false;
    builtElMat_   = false;

    if(elInfo_)
    {
      // copy coordinates
      for(int i=0; i<dim+1; ++i)
      {
        const ALBERTA REAL_D & elcoord = elInfo_->coord[mapVertices(i)];
        for(int j=0; j<dimworld; ++j)
          coord_[i][j] = elcoord[j];
      }

      const ALBERTA EL * el = elInfo_->el;
      assert( el );
      // if leaf element, get determinant from leaf data
      if(el->child[0] == 0)
      {
        typedef GridImp :: LeafDataType::Data LeafDataType;
        LeafDataType * ldata = (LeafDataType *) el->child[1];
        assert( ldata );
        elDet_ = ldata->determinant;
        calcedDet_ = true;
      }
      else
      {
        elDet_     = elDeterminant();
        calcedDet_ = true;
      }

      // geometry built
      return true;
    }
    else
    {
      elDet_     = 0.0;
      calcedDet_ = false;
    }
    // geometry not built
    return false;
  }

  template <>
  inline bool AlbertaGridGeometry<3,3,const AlbertaGrid<3,3> >::
  builtGeom(ALBERTA EL_INFO *elInfo, int face,
            int edge, int vertex)
  {
    typedef AlbertaGrid<3,3> GridImp;
    enum { dim = 3 };
    enum { dimworld = 3 };

    elInfo_ = elInfo;
    builtinverse_ = false;
    builtElMat_   = false;

    if(elInfo_)
    {
      // copy coordinates
      for(int i=0; i<dim+1; ++i)
      {
        const ALBERTA REAL_D & elcoord = elInfo_->coord[mapVertices(i)];
        for(int j=0; j<dimworld; ++j)
          coord_[i][j] = elcoord[j];
      }

      const ALBERTA EL * el = elInfo_->el;
      assert( el );
      // if leaf element, get determinant from leaf data
      if(el->child[0] == 0)
      {
        typedef GridImp :: LeafDataType::Data LeafDataType;
        LeafDataType * ldata = (LeafDataType *) el->child[1];
        assert( ldata );
        elDet_ = ldata->determinant;
        calcedDet_ = true;
      }
      else
      {
        elDet_     = elDeterminant();
        calcedDet_ = true;
      }

      // geometry built
      return true;
    }
    else
    {
      elDet_     = 0.0;
      calcedDet_ = false;
    }
    // geometry not built
    return false;
  }

  // built Geometry
  template <int mydim, int cdim, class GridImp>
  template <class GeometryType, class LocalGeometryType >
  inline bool AlbertaGridGeometry<mydim,cdim,GridImp>::
  builtLocalGeom(const GeometryType &geo, const LocalGeometryType & localGeom,
                 ALBERTA EL_INFO *elInfo,int face)
  {
    elInfo_ = elInfo;
    face_ = face;
    edge_   = 0;
    vertex_ = 0;
    builtinverse_ = false;
    builtElMat_   = false;

    if(elInfo_)
    {
      // just map the point of the global intersection to the local
      // coordinates , this is the default procedure
      // for simplices this is not so bad
      for(int i=0; i<mydim+1; i++)
      {
        coord_[i] = geo.local( localGeom[i] );
      }

      elDet_     = elDeterminant();
      calcedDet_ = true;

      // geometry built
      return true;
    }
    else
    {
      elDet_     = 0.0;
      calcedDet_ = false;
    }

    // geometry not built
    return false;
  }

  // built Geometry
  template <int mydim, int cdim, class GridImp>
  inline void AlbertaGridGeometry<mydim,cdim,GridImp>::
  buildGeomInFather(const int child)
  {
    initGeom();
    // its a child of the reference element ==> det = 0.5
    elDet_ = 0.5;
    calcedDet_ = true;

    assert( (child == 0) || (child == 1) );
    if(mydim == 2)
    {
      coord_ = 0.0;
      //////////////////////////////////////////////
      //
      //             (0.0,1.0)
      //                /|\
      //               /0|1\
      //              /  |  \
      //             /   |   \
      //            /    |    \
      //           /     |     \
      //          /      |      \
      //         / ch 0  | ch 1  \
      //        /1      2|2      0\
      //        -------------------
      //   (0.0,0.0)              (1.0, 0.0)
      //
      //
      ///////////////////////////////////////////

      if( child == 0 )
      {
        coord_[0][0] = 0.0; coord_[0][1] = 1.0;
        coord_[1][0] = 0.0; coord_[1][1] = 0.0;
        coord_[2][0] = 0.0; coord_[2][1] = 0.5;
      }
      if( child == 1 )
      {
        coord_[0][0] = 1.0; coord_[0][1] = 0.0;
        coord_[1][0] = 0.0; coord_[1][1] = 1.0;
        coord_[2][0] = 0.0; coord_[2][1] = 0.5;
      }
      return ;
    }

    if(mydim == 3)
    {
      assert(false);
      coord_ = 0.0;

      //////////////////////////////////////////////
      //
      //             (0.0,1.0)
      //                /|\
      //               /0|1\
      //              /  |  \
      //             /   |   \
      //            /    |    \
      //           /     |     \
      //          /      |      \
      //         / ch 0  | ch 1  \
      //        /1      2|2      0\
      //        -------------------
      //   (0.0,0.0)              (1.0, 0.0)
      //
      //
      ///////////////////////////////////////////
      if( child == 0 )
      {
        coord_[0] = 0.0; // (0,0,0)
        coord_[1] = 0.0; coord_[1][1] = 1.0; // (0,1,0)
        coord_[2] = 0.0; coord_[2][2] = 1.0; // (0,0,1)
        coord_[3] = 0.0; coord_[3][0] = 0.5; // (0.5,0,0)
      }
      if( child == 1 )
      {
        coord_[0] = 0.0; coord_[0][0] = 1.0; // (1,0,0)
        coord_[1] = 0.0; coord_[1][1] = 1.0; // (0,1,0)
        coord_[2] = 0.0; coord_[2][2] = 1.0; // (0,0,1)
        coord_[3] = 0.0; coord_[3][0] = 0.5; // (0.5,0,0)
      }
      return ;
    }

    DUNE_THROW(NotImplemented,"wrong dimension given!");
  }

  //*************************************************************************
  //
  //  --AlbertaGridEntity
  //  --Entity
  //
  //*************************************************************************
  //
  //  codim > 0
  //
  // The Geometry is prescribed by the EL_INFO struct of ALBERTA MESH
  // the pointer to this struct is set and get by setElInfo and
  // getElInfo.
  //*********************************************************************8
  template<int codim, int dim, class GridImp>
  inline AlbertaGridEntity<codim,dim,GridImp>::
  AlbertaGridEntity(const GridImp &grid, int level,
                    ALBERTA TRAVERSE_STACK * travStack)
    : grid_(grid)
      , elInfo_(0)
      , element_(0)
      , travStack_(travStack)
      , level_ ( level )
      , geo_ (GeometryImp())
      , geoImp_( grid_.getRealImplementation(geo_) )
      , builtgeometry_    (false)
      , localFatherCoords_()
      , localFCoordCalced_(false)
      , face_             (-1)
      , edge_             (-1)
      , vertex_           (-1)
  {}


  template<int codim, int dim, class GridImp>
  inline AlbertaGridEntity<codim,dim,GridImp>::
  AlbertaGridEntity(const AlbertaGridEntity<codim,dim,GridImp> & org)
    : grid_     (org.grid_)
      , elInfo_   (org.elInfo_)
      , element_  ( (elInfo_) ? (elInfo_->el) : 0 )
      , travStack_(org.travStack_)
      , level_    (org.level_ )
      , geo_      (org.geo_)
      , geoImp_   ( grid_.getRealImplementation(geo_) )
      , builtgeometry_    (false)
      , localFatherCoords_()
      , localFCoordCalced_(false)
      , face_             (org.face_)
      , edge_             (org.edge_)
      , vertex_           (org.vertex_)
  {}


  template<int codim, int dim, class GridImp>
  inline void AlbertaGridEntity<codim,dim,GridImp>::
  setTraverseStack(ALBERTA TRAVERSE_STACK * travStack)
  {
    travStack_ = travStack;
  }

  template<int codim, int dim, class GridImp>
  inline AlbertaGridEntity<codim,dim,GridImp>::
  AlbertaGridEntity(const GridImp &grid, int level, bool)
    : grid_(grid)
      , elInfo_(0)
      , element_(0)
      , travStack_(0)
      , level_ (level)
      , geo_ (GeometryImp())
      , geoImp_( grid_.getRealImplementation(geo_) )
      , builtgeometry_(false)
      , localFatherCoords_()
      , localFCoordCalced_(false)
      , face_             (-1)
      , edge_             (-1)
      , vertex_           (-1)
  {}

  template<int codim, int dim, class GridImp>
  inline PartitionType AlbertaGridEntity <codim,dim,GridImp>::
  partitionType () const
  {
    return InteriorEntity;
  }

  template<int codim, int dim, class GridImp>
  inline bool AlbertaGridEntity<codim,dim,GridImp>::
  equals (const AlbertaGridEntity<codim,dim,GridImp> & i) const
  {
    const ALBERTA EL * e2 = i.getElement();

    // if both element null then they are equal
    if( (!e2) && (!element_) ) return true;

    return ((element_ == e2) && (getFEVnum() == i.getFEVnum()));
  }

  template<int codim, int dim, class GridImp>
  inline ALBERTA EL_INFO* AlbertaGridEntity<codim,dim,GridImp>::
  getElInfo() const
  {
    return elInfo_;
  }

  template<int codim, int dim, class GridImp>
  inline ALBERTA EL * AlbertaGridEntity<codim,dim,GridImp>::
  getElement() const
  {
    return element_;
  }

  template<int codim, int dim, class GridImp>
  inline void AlbertaGridEntity<codim,dim,GridImp>::
  removeElInfo()
  {
    elInfo_  = 0;
    element_ = 0;
    builtgeometry_ = false;
  }

  template<int codim, int dim, class GridImp>
  inline void AlbertaGridEntity<codim,dim,GridImp>::
  setElInfo(ALBERTA EL_INFO * elInfo, int face,
            int edge, int vertex )
  {
    face_ = face;
    edge_ = edge;
    vertex_ = vertex;
    elInfo_ = elInfo;
    if(elInfo_)
      element_ = elInfo_->el;
    else
      element_ = 0;
    builtgeometry_ = geoImp_.builtGeom(elInfo_,face,edge,vertex);
    localFCoordCalced_ = false;
  }

  template<int codim, int dim, class GridImp>
  inline void AlbertaGridEntity<codim,dim,GridImp>::
  setEntity(const AlbertaGridEntity<codim,dim,GridImp> & org)
  {
    setElInfo(org.elInfo_,org.face_,org.edge_,org.vertex_);
    setLevel(org.level_);
  }

  template<int codim, int dim, class GridImp>
  inline void AlbertaGridEntity<codim,dim,GridImp>::
  setLevel(int level)
  {
    level_  = level;
  }

  template<int codim, int dim, class GridImp>
  inline int AlbertaGridEntity<codim,dim,GridImp>::
  level() const
  {
    return level_;
  }

  // default
  template <class GridImp, int codim, int cdim>
  struct AlbertaGridBoundaryId
  {
    static int boundaryId (const ALBERTA EL_INFO * elInfo, int face, int edge, int vertex)
    {
      return 0;
    }
  };

  // faces in 2d and 3d
  template <class GridImp>
  struct AlbertaGridBoundaryId<GridImp,1,3>
  {
    static int boundaryId (const ALBERTA EL_INFO * elInfo, int face, int edge, int vertex)
    {
      return elInfo->boundary[face]->bound;
    }
  };

  template <class GridImp>
  struct AlbertaGridBoundaryId<GridImp,1,2>
  {
    static int boundaryId (const ALBERTA EL_INFO * elInfo, int face, int edge, int vertex)
    {
      return elInfo->boundary[face]->bound;
    }
  };

  // vertices in 2d and 3d
  template <class GridImp, int dim>
  struct AlbertaGridBoundaryId<GridImp,dim,dim>
  {
    static int boundaryId (const ALBERTA EL_INFO * elInfo, int face, int edge, int vertex)
    {
      return elInfo->bound[vertex];
    }
  };

  template <int codim, int dim, class GridImp>
  inline int AlbertaGridEntity<codim,dim,GridImp>::boundaryId() const
  {
    return AlbertaGridBoundaryId<GridImp,codim,dim>::boundaryId(elInfo_,face_,edge_,vertex_);
  }


  // vertices in 2d and 3d
  template <class GridImp, int codim, int cdim>
  struct AlbertaGridFEVnum
  {
    static int getFEVnum (int face, int edge, int vertex)
    {
      return face;
    }
  };

  template <class GridImp, int cdim>
  struct AlbertaGridFEVnum<GridImp,1,cdim>
  {
    static int getFEVnum (int face, int edge, int vertex)
    {
      return face;
    }
  };

  template <class GridImp, int cdim>
  struct AlbertaGridFEVnum<GridImp,cdim,cdim>
  {
    static int getFEVnum (int face, int edge, int vertex)
    {
      return vertex;
    }
  };

  template <class GridImp>
  struct AlbertaGridFEVnum<GridImp,2,3>
  {
    static int getFEVnum (int face, int edge, int vertex)
    {
      return edge;
    }
  };

  template<int codim, int dim, class GridImp>
  inline int AlbertaGridEntity<codim,dim,GridImp>::
  getFEVnum() const
  {
    return AlbertaGridFEVnum<GridImp,codim,GridImp::dimensionworld>::getFEVnum(face_,edge_,vertex_);
  }

  template<int cd, int dim, class GridImp>
  inline const typename AlbertaGridEntity<cd,dim,GridImp>::Geometry &
  AlbertaGridEntity<cd,dim,GridImp>::geometry() const
  {
    assert(builtgeometry_ == true);
    return geo_;
  }

  template<int codim, int dim, class GridImp>
  inline typename AlbertaGridEntity<codim,dim,GridImp>::EntityPointer
  AlbertaGridEntity<codim,dim,GridImp>::ownersFather () const
  {
    ALBERTA EL_INFO * fatherInfo = ALBERTA AlbertHelp::getFatherInfo(travStack_,elInfo_,level_);
    int fatherLevel = (level_ > 0) ? (level_-1) : 0;

    assert( fatherLevel == fatherInfo->level );

    return AlbertaGridEntityPointer<0,GridImp> (grid_,travStack_,fatherLevel,fatherInfo,0,0,0);
  }

  /*
     // coords for 2d
     static double fatherref [2][3][2] =
     {
      { {0.0,1.0},{0.0,0.0 }, {0.5,0.0 } }  ,
      { {1.0,0.0},{0.0,1.0 }, {0.5,0.0 } }
     };
   */

  template<int codim, int dim, class GridImp>
  inline const FieldVector<albertCtype, dim>&
  AlbertaGridEntity<codim,dim,GridImp>::positionInOwnersFather() const
  {
    assert( codim == dim );
    if(!localFCoordCalced_)
    {
      EntityPointer vati = this->ownersFather();
      localFatherCoords_ = (*vati).geometry().local( this->geometry()[0] );
      localFCoordCalced_ = true;
      /*
          // check
          if((level_ > 0) && (dim == 2))
          {
            ALBERTA EL_INFO * fatty = ALBERTA AlbertHelp::getFatherInfo(travStack_,elInfo_,level_);
            int child = 0;
            if(elInfo_->el == fatty->el->child[1])
            {
              child = 1;
            }

            FieldVector<double,2> tmp(0.0);
            for(int j=0; j<2; j++) tmp[j] = fatherref[child][vertex_][j];
            if( (localFatherCoords_ - tmp).two_norm() > 1.0E-10)
            {
              std::cout << localFatherCoords_ << " c|r " << tmp << " localF \n";
              assert(false);
            }
          }
       */
    }
    return localFatherCoords_;
  }


  //************************************
  //
  //  --AlbertaGridEntity codim = 0
  //  --0Entity codim = 0
  //
  template<int dim, class GridImp>
  inline AlbertaGridEntity <0,dim,GridImp>::
  AlbertaGridEntity(const GridImp &grid, int level, bool leafIt )
    : grid_(grid)
      , level_ (level)
      , travStack_ (0)
      , elInfo_ (0)
      , element_(0)
      , geoObj_( GeometryImp() )
      , geo_( grid_.getRealImplementation(geoObj_) )
      , builtgeometry_ (false)
      , leafIt_ ( leafIt )
  {}

  template<int dim, class GridImp>
  inline AlbertaGridEntity <0,dim,GridImp>::
  AlbertaGridEntity(const AlbertaGridEntity & org)
    : grid_(org.grid_)
      , level_ (org.level_)
      , travStack_ (org.travStack_)
      , elInfo_ (org.elInfo_)
      , element_( (elInfo_) ? (elInfo_->el) : 0)
      , geoObj_( org.geoObj_ )
      , geo_( grid_.getRealImplementation(geoObj_) )
      , builtgeometry_ (false)
      , leafIt_ ( org.leafIt_ )
  {}

  template<int dim, class GridImp>
  inline int AlbertaGridEntity <0,dim,GridImp>::
  boundaryId() const
  {
    // elements are always inside of our Domain
    return 0;
  }

  template<int dim, class GridImp>
  inline AdaptationState AlbertaGridEntity <0,dim,GridImp>::
  state() const
  {
    assert( element_ && elInfo_ );
    assert( element_ == elInfo_->el );
    if( element_->mark < 0 )
    {
      return COARSEN;
    }
    if( grid_.checkElNew( element_ ) )
    {
      return REFINED;
    }

    return NONE;
  }

  template<int dim, class GridImp>
  inline PartitionType AlbertaGridEntity <0,dim,GridImp>::
  partitionType () const
  {
    return grid_.partitionType( elInfo_ );
  }

  template<int dim, class GridImp>
  inline bool AlbertaGridEntity <0,dim,GridImp>::isLeaf() const
  {
    assert( element_ && elInfo_ );
    assert( element_ == elInfo_->el );

    // if no child exists, then this element is leaf element
    return (element_->child[0] == 0);
  }

  //***************************

  template<int dim, class GridImp>
  inline void AlbertaGridEntity <0,dim,GridImp>::
  makeDescription()
  {
    elInfo_  = 0;
    element_ = 0;
    builtgeometry_ = false;
  }

  template<int dim, class GridImp>
  inline bool AlbertaGridEntity<0,dim,GridImp>::
  equals (const AlbertaGridEntity<0,dim,GridImp> & i) const
  {
    // compare element pointer which are unique
    return (element_ == i.getElement());
  }

  template<int dim, class GridImp>
  inline void AlbertaGridEntity <0,dim,GridImp>::
  setTraverseStack(ALBERTA TRAVERSE_STACK * travStack)
  {
    travStack_ = travStack;
  }

  //*****************************************************************
  // count
  template <class GridImp, int dim, int cc> struct AlbertaGridCount {
    static int count () { return dim+1; }
  };

  // specialisation for codim 0
  template <class GridImp, int dim> struct AlbertaGridCount<GridImp,dim,0> {
    static int count () { return 1; }
  };

  // specialisation for edges in 3d
  template <class GridImp> struct AlbertaGridCount<GridImp,3,2> {
    static int count () { return 6; }
  };

  template<int dim, class GridImp> template <int cc>
  inline int AlbertaGridEntity <0,dim,GridImp>::count () const
  {
    return AlbertaGridCount<GridImp,dim,cc>::count();
  }

  //*****************************************************************
  template <class GridImp, int dim, int cd> struct SubEntity;

  // specialisation for elements
  template <class GridImp, int dim>
  struct SubEntity<GridImp,dim,0>
  {
    typedef typename AlbertaGridEntity <0,dim,GridImp>::template Codim<0>::EntityPointer EntityPointerType;
    static EntityPointerType entity(GridImp & grid, ALBERTA TRAVERSE_STACK * stack,
                                    int level, ALBERTA EL_INFO * elInfo, int i )
    {
      return AlbertaGridEntityPointer<0,GridImp> (grid, stack , level ,elInfo, 0,0,0);
    }
  };


  // specialisation for faces
  template <class GridImp, int dim>
  struct SubEntity<GridImp,dim,1>
  {
    typedef typename AlbertaGridEntity <0,dim,GridImp>::template Codim<1>::EntityPointer EntityPointerType;
    static EntityPointerType entity(GridImp & grid, ALBERTA TRAVERSE_STACK * stack,
                                    int level, ALBERTA EL_INFO * elInfo, int i )
    {
      return AlbertaGridEntityPointer<1,GridImp> (grid, stack , level ,elInfo, i,0,0);
    }
  };

  // specialisation for edges , only when dim == 3
  template <class GridImp>
  struct SubEntity<GridImp,3,2>
  {
    enum { dim = 3 };
    typedef typename AlbertaGridEntity <0,dim,GridImp>::template Codim<2>::EntityPointer EntityPointerType;
    static EntityPointerType entity(GridImp & grid, ALBERTA TRAVERSE_STACK * stack,
                                    int level, ALBERTA EL_INFO * elInfo, int edge )
    {
      return AlbertaGridEntityPointer<2,GridImp> (grid, stack , level ,elInfo, 0, edge ,0);
    }
  };

  // specialisation for vertices
  template <class GridImp, int dim>
  struct SubEntity<GridImp,dim,dim>
  {
    typedef typename AlbertaGridEntity <0,dim,GridImp>::template Codim<dim>::EntityPointer EntityPointerType;
    static EntityPointerType entity(GridImp & grid, ALBERTA TRAVERSE_STACK * stack,
                                    int level, ALBERTA EL_INFO * elInfo, int i )
    {
      return AlbertaGridEntityPointer<dim,GridImp> (grid, stack , level ,elInfo, 0,0,i);
    }
  };

  // default is faces
  template <int dim, class GridImp>
  template <int cc>
  inline typename AlbertaGridEntity <0,dim,GridImp>::template Codim<cc>::EntityPointer
  AlbertaGridEntity <0,dim,GridImp>::entity ( int i ) const
  {
    return SubEntity<GridImp,dim,cc> :: entity(grid_,travStack_,level(),elInfo_,i);
  }

  template<int dim, class GridImp>
  inline ALBERTA EL_INFO* AlbertaGridEntity <0,dim,GridImp>::
  getElInfo() const
  {
    return elInfo_;
  }

  template<int dim, class GridImp>
  inline ALBERTA EL * AlbertaGridEntity <0,dim,GridImp>::
  getElement() const
  {
    return element_;
  }

  template<int dim, class GridImp>
  inline int AlbertaGridEntity <0,dim,GridImp>::
  level() const
  {
    return level_;
  }

  template<int dim, class GridImp>
  inline void AlbertaGridEntity <0,dim,GridImp>::
  setLevel(int actLevel)
  {
    level_  = actLevel;
    assert( level_ >= 0);
  }

  template<int dim, class GridImp>
  inline void AlbertaGridEntity <0,dim,GridImp>::
  setNewLevel(int actLevel, bool leafIt )
  {
    level_  = actLevel;
    assert( level_ >= 0);

    leafIt_ = leafIt;
  }

  template<int dim, class GridImp>
  inline void AlbertaGridEntity<0,dim,GridImp>::
  removeElInfo()
  {
    elInfo_  = 0;
    element_ = 0;
    builtgeometry_ = false;
  }

  template<int dim, class GridImp>
  inline void AlbertaGridEntity <0,dim,GridImp>::
  setElInfo(ALBERTA EL_INFO * elInfo, int , int , int )
  {
    // just set elInfo and element
    elInfo_ = elInfo;
    if(elInfo_)
      element_ = elInfo_->el;
    else
      element_ = 0;
    builtgeometry_ = false;
  }

  template<int dim, class GridImp>
  inline void AlbertaGridEntity<0,dim,GridImp>::
  setEntity(const AlbertaGridEntity<0,dim,GridImp> & org)
  {
    setElInfo(org.elInfo_);
    setTraverseStack(org.travStack_);
    setLevel(org.level());
  }

  template<int dim, class GridImp>
  inline const typename AlbertaGridEntity <0,dim,GridImp>::Geometry &
  AlbertaGridEntity <0,dim,GridImp>::geometry() const
  {
    assert( elInfo_ && element_ );
    // geometry is only build on demand
    if(!builtgeometry_) builtgeometry_ = geo_.builtGeom(elInfo_,0,0,0);

    assert(builtgeometry_ == true);
    return geoObj_;
  }


  template<int dim, class GridImp>
  inline typename AlbertaGridEntity <0,dim,GridImp>::EntityPointer
  AlbertaGridEntity <0,dim,GridImp>::father() const
  {
    ALBERTA EL_INFO * fatherInfo = ALBERTA AlbertHelp::getFatherInfo(travStack_,elInfo_,level_);
    assert( fatherInfo );

    std::cout << "Father of el[" << grid_.getElementNumber(element_) << "] is father[" << grid_.getElementNumber(fatherInfo->el) << "\n";

    int fatherLevel = (level_ > 0) ? (level_-1) : 0;

    assert( (fatherLevel == fatherInfo->level) );

    return AlbertaGridEntityPointer<0,GridImp> (grid_,travStack_,fatherLevel,fatherInfo,0,0,0);
  }

  template<int dim, class GridImp>
  inline int AlbertaGridEntity <0,dim,GridImp>::nChild() const
  {
    // get father and check which child we have
    const ALBERTA EL * father = elInfo_->parent;
    assert( father );

    int child = 0;
    if(father->child[1] == element_) child = 1;

    assert( father->child[child] == element_ );
    return child;
  }

  // singletons of geometry in father geometries
  // GeometryType schould be of type Dune::Geometry
  template <class GeometryType>
  static inline GeometryType &
  getGeometryInFather(const int child)
  {
    typedef typename GeometryType :: ImplementationType GeometryImp;
    static GeometryType child0 (GeometryImp(0)); // child 0
    static GeometryType child1 (GeometryImp(1)); // child 1

    if(child == 0) return child0;
    if(child == 1) return child1;

    DUNE_THROW(NotImplemented,"wrong number of child given!");
    return child0;
  }

  template<>
  inline const AlbertaGridEntity <0,2,const AlbertaGrid<2,2> >::Geometry &
  AlbertaGridEntity <0,2,const AlbertaGrid<2,2> >::geometryInFather() const
  {
    return getGeometryInFather<Geometry> (this->nChild());
  }

  template<>
  inline const AlbertaGridEntity <0,3,const AlbertaGrid<3,3> >::Geometry &
  AlbertaGridEntity <0,3,const AlbertaGrid<3,3> >::geometryInFather() const
  {
    return getGeometryInFather<Geometry> (this->nChild());
  }
  // end AlbertaGridEntity

  //*******************************************************************
  //
  //  --EntityPointer
  //  --EnPointer
  //
  //*******************************************************************
  template<int codim, class GridImp >
  inline AlbertaGridEntityPointer<codim,GridImp> ::
  AlbertaGridEntityPointer(const GridImp & grid,
                           int level,  ALBERTA EL_INFO *elInfo,int face,int edge,int vertex)
    : grid_(grid)
      , isLeaf_ (true)
      , entity_ ( grid_.template getNewEntity<codim> ( level , isLeaf_ ))
      , entityImp_ ( &grid_.getRealImplementation( *entity_ ))
  {
    assert( entity_ );
    entityImp().setElInfo(elInfo,face,edge,vertex);
  }

  template<int codim, class GridImp >
  inline AlbertaGridEntityPointer<codim,GridImp> ::
  AlbertaGridEntityPointer(const GridImp & grid, ALBERTA TRAVERSE_STACK * stack ,
                           int level,  ALBERTA EL_INFO *elInfo,int face,int edge,int vertex)
    : grid_(grid)
      , isLeaf_ (true)
      , entity_ ( grid_.template getNewEntity<codim> ( level , isLeaf_ ))
      , entityImp_ ( &grid_.getRealImplementation( *entity_ ))
  {
    entityImp().setElInfo(elInfo,face,edge,vertex);
    entityImp().setTraverseStack(stack);
  }

  template<int codim, class GridImp >
  inline AlbertaGridEntityPointer<codim,GridImp> ::
  AlbertaGridEntityPointer(const GridImp & grid, int level , bool isLeaf, bool end )
    : grid_(grid)
      , isLeaf_ (isLeaf)
      , entity_ ( grid_.template getNewEntity<codim> (level, isLeaf_) )
      , entityImp_ ( &grid_.getRealImplementation( *entity_ ))
  {
    if(end) this->done();
  }

  template<int codim, class GridImp >
  inline AlbertaGridEntityPointer<codim,GridImp> ::
  //AlbertaGridEntityPointer(const AlbertaGridEntityPointerType & org)
  AlbertaGridEntityPointer(const AlbertaGridEntityPointer<codim,GridImp> & org)
    : grid_(org.grid_)
      , isLeaf_ ( org.isLeaf_ )
      , entity_ ( grid_.template getNewEntity<codim> ( org.entity_->level() , isLeaf_) )
      , entityImp_ ( &grid_.getRealImplementation( *entity_ ))
  {
    entityImp().setEntity( org.entityImp() );
  }

  template<int codim, class GridImp >
  inline AlbertaGridEntityPointer<codim,GridImp>  &
  AlbertaGridEntityPointer<codim,GridImp> ::
  operator = (const AlbertaGridEntityPointerType & org)
  {
    assert( & grid_ == & org.grid_ );
    isLeaf_ = ( org.isLeaf_ );
    entityImp().setEntity( org.entityImp() );
    return *this;
  }

  template<int codim, class GridImp >
  inline typename AlbertaGridEntityPointer<codim,GridImp> :: EntityImp &
  AlbertaGridEntityPointer<codim,GridImp> :: entityImp ()
  {
    assert( entityImp_ );
    return *entityImp_;
  }

  template<int codim, class GridImp >
  inline const typename AlbertaGridEntityPointer<codim,GridImp> :: EntityImp &
  AlbertaGridEntityPointer<codim,GridImp> :: entityImp () const
  {
    assert( entityImp_ );
    return *entityImp_;
  }

  template<int codim, class GridImp >
  inline AlbertaGridEntityPointer<codim,GridImp> ::
  AlbertaGridEntityPointer(const GridImp & grid, const EntityImp  & en)
    : grid_(grid)
      , isLeaf_ ( en.leafIt() )
      , entity_ ( grid_.template getNewEntity<codim> ( en.level(), isLeaf_ ) )
      , entityImp_ ( &grid_.getRealImplementation( *entity_ ))
  {
    entityImp().setEntity( en );
  }

  template<int codim, class GridImp >
  inline AlbertaGridEntityPointer<codim,GridImp> :: ~AlbertaGridEntityPointer()
  {
    grid_.template freeEntity<codim>( entity_ );
    entityImp_ = 0;
  }

  template<int codim, class GridImp >
  inline void AlbertaGridEntityPointer<codim,GridImp>::done ()
  {
    // sets entity pointer in the status of an end iterator
    entityImp().removeElInfo();
  }

  template<int codim, class GridImp >
  inline bool AlbertaGridEntityPointer<codim,GridImp>::
  equals (const AlbertaGridEntityPointer<codim,GridImp>& i) const
  {
    // comprae entities
    return entityImp().equals( i.entityImp() );
  }

  template<int codim, class GridImp >
  inline typename AlbertaGridEntityPointer<codim,GridImp>::Entity &
  AlbertaGridEntityPointer<codim,GridImp>::dereference () const
  {
    assert(entity_);
    return (*entity_);
  }

  template<int codim, class GridImp >
  inline int AlbertaGridEntityPointer<codim,GridImp>::level () const
  {
    return entityImp().level();
  }

  //***************************************************************
  //
  //  --AlbertaGridHierarchicIterator
  //  --HierarchicIterator
  //
  //***************************************************************
  template< class GridImp >
  inline void AlbertaGridHierarchicIterator<GridImp>::
  makeIterator()
  {
    virtualEntity_.setTraverseStack(0);
    virtualEntity_.setElInfo(0,0,0,0);
  }

  template< class GridImp >
  inline AlbertaGridHierarchicIterator<GridImp>::
  AlbertaGridHierarchicIterator(const GridImp & grid,
                                int actLevel,
                                int maxLevel)
    : AlbertaGridEntityPointer<0,GridImp> (grid,actLevel,true,true)
      , level_ (actLevel)
      , maxlevel_ (maxLevel)
      , virtualEntity_( this->entityImp() )
      , end_ (true)
  {
    makeIterator();
  }

  template< class GridImp >
  inline AlbertaGridHierarchicIterator<GridImp>::
  AlbertaGridHierarchicIterator(const GridImp & grid,
                                ALBERTA TRAVERSE_STACK *travStack,int actLevel, int maxLevel, bool leafIt )
    : AlbertaGridEntityPointer<0,GridImp> (grid,actLevel,leafIt,false)
      , level_ (actLevel)
      , maxlevel_ ( maxLevel)
      , virtualEntity_( this->entityImp() )
      , end_ (false)
  {
    if(travStack)
    {
      // get new ALBERTA TRAVERSE STACK
      manageStack_.makeItNew(true);

      ALBERTA TRAVERSE_STACK *stack = manageStack_.getStack();

      // cut old traverse stack, kepp only actual element
      cutHierarchicStack(stack, travStack);

      // set new traverse level
      if(maxlevel_ < 0)
      {
        std::cout << "WARNING: maxlevel < 0 in AlbertaGridHierarchicIterator! \n";
        // this means, we go until leaf level
        stack->traverse_fill_flag = CALL_LEAF_EL | stack->traverse_fill_flag;
        // exact here has to stand Grid->maxlevel, but is ok anyway
        maxlevel_ = this->grid_.maxLevel();
      }
      // set new traverse level
      stack->traverse_level = maxlevel_;

      virtualEntity_.setTraverseStack(stack);
      // Hier kann ein beliebiges Geometry uebergeben werden,
      // da jedes AlbertGeometry einen Zeiger auf das Macroelement
      // enthaelt.
      virtualEntity_.setElInfo(recursiveTraverse(stack));

      // set new level
      virtualEntity_.setLevel(level_);
    }
    else
    {
      std::cout << "Warning: travStack == NULL in HierarchicIterator(travStack,travLevel) \n";
      makeIterator();
    }
  }

  template< class GridImp >
  inline AlbertaGridHierarchicIterator<GridImp>::
  AlbertaGridHierarchicIterator(const AlbertaGridHierarchicIterator<GridImp> & org)
    : AlbertaGridEntityPointer<0,GridImp> (org.grid_,org.level(),true, org.end_ )
      , level_ ( org.level_ )
      , maxlevel_ ( org.maxlevel_ )
      , virtualEntity_( this->entityImp() )
      , manageStack_ ( org.manageStack_ )
  {
    if( org.virtualEntity_.getElInfo() )
      virtualEntity_.setEntity( org.virtualEntity_ );
    else
      this->done();
  }

  template< class GridImp >
  inline AlbertaGridHierarchicIterator<GridImp> &
  AlbertaGridHierarchicIterator<GridImp>::
  operator = (const AlbertaGridHierarchicIterator<GridImp> & org)
  {
    level_ = ( org.level_ );
    maxlevel_ = ( org.maxlevel_ );
    manageStack_ = ( org.manageStack_ );
    if( org.virtualEntity_.getElInfo() )
      virtualEntity_.setEntity( org.virtualEntity_ );
    else
      this->done();
    return *this;
  }

  template< class GridImp >
  inline void AlbertaGridHierarchicIterator< GridImp >::increment()
  {
    ALBERTA EL_INFO * nextinfo = recursiveTraverse(manageStack_.getStack());
    if(!nextinfo)
    {
      this->done();
      return;
    }

    virtualEntity_.setElInfo( nextinfo );
    // set new actual level, calculated by recursiveTraverse
    virtualEntity_.setLevel(level_);
    return ;
  }

  template< class GridImp >
  inline ALBERTA EL_INFO *
  AlbertaGridHierarchicIterator<GridImp>::
  recursiveTraverse(ALBERTA TRAVERSE_STACK * stack)
  {
    // see function
    // static EL_INFO *traverse_leaf_el(TRAVERSE_STACK *stack)
    // Common/traverse_nr_common.cc, line 392
    ALBERTA EL * el=0;

    if(!stack->elinfo_stack)
    {
      /* somethin' wrong */
      return 0;
    }
    else
    {
      // go up until we can go down again
      el = stack->elinfo_stack[stack->stack_used].el;

      // stack->stack_used is actual element in stack
      // stack->info_stack[stack->stack_used] >= 2
      //    means the two children has been visited
      while((stack->stack_used > 0) &&
            ((stack->info_stack[stack->stack_used] >= 2)
             || (el->child[0] == 0)
             || ( stack->traverse_level <=
                  (stack->elinfo_stack+stack->stack_used)->level)) )
      {
        stack->stack_used--;
        el = stack->elinfo_stack[stack->stack_used].el;
        level_ = stack->elinfo_stack[stack->stack_used].level;
      }

      // goto next father is done by other iterator and not our problem
      if(stack->stack_used < 1)
      {
        return 0;
      }
    }

    // go down next child
    if(el->child[0] && (stack->traverse_level >
                        (stack->elinfo_stack+stack->stack_used)->level) )
    {
      if(stack->stack_used >= stack->stack_size - 1)
        ALBERTA enlargeTraverseStack(stack);

      int i = stack->info_stack[stack->stack_used];
      el = el->child[i];
      stack->info_stack[stack->stack_used]++;

      // new: go down maxlevel, but fake the elements
      level_++;
      this->grid_.fillElInfo(i, level_, stack->elinfo_stack+stack->stack_used,
                             stack->elinfo_stack+stack->stack_used+1 ,true);
      //ALBERTA fill_elinfo(i, stack->elinfo_stack + stack->stack_used,
      //  stack->elinfo_stack + (stack->stack_used + 1));

      stack->stack_used++;
      stack->info_stack[stack->stack_used] = 0;
    }
    else
    {
      return 0;
    }

    return (stack->elinfo_stack + stack->stack_used);
  } // recursive traverse over all childs

  // end AlbertaGridHierarchicIterator
  //***************************************************************
  //
  //  --AlbertaGridBoundaryEntity
  //  --BoundaryEntity
  //
  //***************************************************************
#if 0
  template< class GridImp >
  inline AlbertaGridBoundaryEntity<GridImp>::
  AlbertaGridBoundaryEntity () : _geom () , _elInfo ( 0 ),
                                 _neigh (-1) {}

  template< class GridImp >
  inline int AlbertaGridBoundaryEntity<GridImp>::id () const
  {
    assert(_elInfo->boundary[_neigh] != 0);
    return _elInfo->boundary[_neigh]->bound;
  }

  template< class GridImp >
  inline bool AlbertaGridBoundaryEntity<GridImp>::hasGeometry () const
  {
    return _geom.builtGeom(_elInfo,0,0,0);
  }

  template< class GridImp >
  inline const typename AlbertaGridBoundaryEntity<GridImp>::Geometry &
  AlbertaGridBoundaryEntity<GridImp>::geometry () const
  {
    return _geom;
  }

  template< class GridImp >
  inline void AlbertaGridBoundaryEntity<GridImp>::
  setElInfo (ALBERTA EL_INFO * elInfo, int nb)
  {
    _neigh = nb;
    if(elInfo)
      _elInfo = elInfo;
    else
      _elInfo = 0;
  }
#endif

  //***************************************************************
  //
  //  --AlbertaGridIntersectionIterator
  //  --IntersectionIterator
  //
  //***************************************************************

  template< class GridImp >
  inline AlbertaGridIntersectionIterator<GridImp>::
  AlbertaGridIntersectionIterator(const GridImp & grid,int level) :
    grid_(grid),
    level_ (level),
    neighborCount_ (dim+1),
    elInfo_ (0),
    fakeNeighObj_(LocalGeometryImp()),
    fakeSelfObj_ (LocalGeometryImp()),
    neighGlobObj_(LocalGeometryImp()),
    fakeNeigh_ (grid_.getRealImplementation(fakeNeighObj_) ),
    fakeSelf_  (grid_.getRealImplementation(fakeSelfObj_)  ),
    neighGlob_ (grid_.getRealImplementation(neighGlobObj_)),
    neighElInfo_ () ,
    done_(true)
  {}

  template< class GridImp >
  template< class EntityType >
  inline void AlbertaGridIntersectionIterator<GridImp>::
  first(const EntityType & en ,int level)
  {
    level_ = level;
    neighborCount_ = 0;
    builtNeigh_    = false;
    elInfo_        = en.getElInfo();
    done_          = false;
    this->leafIt_  = en.leafIt();
    assert( elInfo_ );
  }

  template< class GridImp >
  inline void AlbertaGridIntersectionIterator<GridImp>::
  done ()
  {
    level_ = -1;
    neighborCount_ = dim+1;
    builtNeigh_    = false;
    elInfo_        = 0;
    done_          = true;
  }

  // copy constructor
  template< class GridImp >
  inline AlbertaGridIntersectionIterator<GridImp>::AlbertaGridIntersectionIterator
    (const AlbertaGridIntersectionIterator<GridImp> & org)
    : grid_(org.grid_)
      , level_(org.level_)
      , neighborCount_(org.neighborCount_)
      , builtNeigh_ (false)
      , leafIt_( org.leafIt_ )
      , elInfo_ ( org.elInfo_ )
      , fakeNeighObj_(LocalGeometryImp())
      , fakeSelfObj_ (LocalGeometryImp())
      , neighGlobObj_(LocalGeometryImp())
      , fakeNeigh_ (grid_.getRealImplementation(fakeNeighObj_))
      , fakeSelf_  (grid_.getRealImplementation(fakeSelfObj_ ))
      , neighGlob_ (grid_.getRealImplementation(neighGlobObj_))
      , neighElInfo_()
      , done_ ( org.done_ )
  {}

  // assignment operator
  template< class GridImp >
  inline void AlbertaGridIntersectionIterator<GridImp>::
  assign (const AlbertaGridIntersectionIterator<GridImp> & org)
  {
    // only assign iterators from the same grid
    assert( &this->grid_ == &(org.grid_));
    level_ =  org.level_;
    neighborCount_ = org.neighborCount_;
    elInfo_ = org.elInfo_;
    builtNeigh_ = false;
    leafIt_ = org.leafIt_;
    done_ = org.done_;
  }


  template< class GridImp >
  inline bool AlbertaGridIntersectionIterator<GridImp>::
  equals (const AlbertaGridIntersectionIterator<GridImp> & i) const
  {
    const ALBERTA EL * e1 = (elInfo_)   ? (elInfo_->el)   : 0;
    const ALBERTA EL * e2 = (i.elInfo_) ? (i.elInfo_->el) : 0;

    return
      ( (e1 == e2) // check equality of entities which can be both zero
        && (done_ == i.done_)); /// and then also check done status
  }

  template< class GridImp >
  inline void AlbertaGridIntersectionIterator<GridImp>::increment()
  {
    builtNeigh_ = false;
    // is like go to the next neighbour
    neighborCount_++;

    // (dim+1) is neigbourCount for end iterators
    if(neighborCount_ > dim)
    {
      this->done();
      return ;
    }

    /*
       // check whether neighbor has same level
       if( (this->neighbor()) && (!this->leafIt()) )
       {
       // if levels are not equal go next
       if( !neighborHasSameLevel () ) increment ();
       }
     */

    return ;
  }

  template< class GridImp >
  inline typename AlbertaGridIntersectionIterator<GridImp>::EntityPointer
  AlbertaGridIntersectionIterator<GridImp>::outside () const
  {
    if(!builtNeigh_)
    {
      assert( elInfo_ );
      // just copy elInfo and then set some values
      std::memcpy(&neighElInfo_,elInfo_,sizeof(ALBERTA EL_INFO));

      setupVirtEn();

      assert( level_ == elInfo_->level );
      assert( (this->leafIt() ) ? (1) : (elInfo_->level == neighElInfo_.level) );
    }
    assert( builtNeigh_ );
    assert( neighElInfo_.el );
    return AlbertaGridEntityPointer<0, GridImp>( this->grid_ ,
                                                 this->grid_.getLevelOfElement(neighElInfo_.el),
                                                 &neighElInfo_,0,0,0);
  }

  template< class GridImp >
  inline typename AlbertaGridIntersectionIterator<GridImp>::EntityPointer
  AlbertaGridIntersectionIterator<GridImp>::inside () const {
    assert(elInfo_);
    return AlbertaGridEntityPointer<0, GridImp>(this->grid_,
                                                (int) elInfo_->level,
                                                elInfo_,
                                                0, 0, 0);
  }

  template< class GridImp >
  inline int
  AlbertaGridIntersectionIterator<GridImp>::boundaryId () const
  {
    // id of interior intersections is 0
    if(!boundary()) return 0;
    assert(elInfo_);
    assert(elInfo_->boundary[neighborCount_] != 0);
    return elInfo_->boundary[neighborCount_]->bound;
  }

  template< class GridImp >
  inline bool AlbertaGridIntersectionIterator<GridImp>::boundary() const
  {
    assert( elInfo_ );
    return (elInfo_->boundary[neighborCount_] != 0);
  }

  template< class GridImp >
  inline bool AlbertaGridIntersectionIterator<GridImp>::neighbor() const
  {
    // use ALBERTA macro to get neighbour
    assert( elInfo_ );
    return (NEIGH(elInfo_->el,elInfo_)[neighborCount_] != 0);
  }

  template<class GridImp>
  inline const typename AlbertaGridIntersectionIterator<GridImp>::NormalVecType &
  AlbertaGridIntersectionIterator<GridImp>::unitOuterNormal (const LocalCoordType & local) const
  {
    // calculates the outer_normal
    unitNormal_  = this->outerNormal(local);
    unitNormal_ *= (1.0/unitNormal_.two_norm());

    return unitNormal_;
  }

  template<class GridImp>
  inline const typename AlbertaGridIntersectionIterator<GridImp>::NormalVecType &
  AlbertaGridIntersectionIterator<GridImp>::integrationOuterNormal (const LocalCoordType & local) const
  {
    return this->outerNormal(local);
  }


  template< class GridImp >
  inline const typename AlbertaGridIntersectionIterator<GridImp>::NormalVecType &
  AlbertaGridIntersectionIterator<GridImp>::outerNormal(const LocalCoordType & local) const
  {
    calcOuterNormal();
    return outNormal_;
  }

  template< class GridImp >
  inline void AlbertaGridIntersectionIterator<GridImp>:: calcOuterNormal() const
  {
    std::cout << "outer_normal() not correctly implemented yet! \n";
    assert(false);
    for(int i=0; i<dimworld; i++)
      outNormal_[i] = 0.0;

    return ;
  }

  template <>
  inline void
  AlbertaGridIntersectionIterator<const AlbertaGrid<2,2> >::calcOuterNormal () const
  {
    assert( elInfo_ );
    // seems to work
    const ALBERTA REAL_D *coord = elInfo_->coord;

    outNormal_[0] = -(coord[(neighborCount_+1)%3][1] - coord[(neighborCount_+2)%3][1]);
    outNormal_[1] =   coord[(neighborCount_+1)%3][0] - coord[(neighborCount_+2)%3][0];

    return ;
  }

  template <>
  inline void AlbertaGridIntersectionIterator<const AlbertaGrid<3,3> >::
  calcOuterNormal () const
  {
    assert( elInfo_ );
    enum { dim = 3 };
    // rechne Kreuzprodukt der Vectoren aus
    const ALBERTA REAL_D *coord = elInfo_->coord;

    // in this case the orientation is negative, therefore multiply with -1
#if DIM == 3
    const albertCtype val = (elInfo_->orientation > 0) ? 0.5 : -0.5;
#else
    const albertCtype val = 0.5;
#endif

    // neighborCount_ is the local face number
    const int * localFaces = ALBERTA AlbertHelp::localAlbertaFaceNumber[neighborCount_];
    for(int i=0; i<dim; i++)
    {
      tmpV_[i] = coord[localFaces[1]][i] - coord[localFaces[0]][i];
      tmpU_[i] = coord[localFaces[2]][i] - coord[localFaces[1]][i];
    }

    // outNormal_ has length 3
    for(int i=0; i<dim; i++)
      outNormal_[i] = tmpU_[(i+1)%dim] * tmpV_[(i+2)%dim]
                      - tmpU_[(i+2)%dim] * tmpV_[(i+1)%dim];

    outNormal_ *= val;

    return ;
  }

  template< class GridImp >
  inline const typename AlbertaGridIntersectionIterator<GridImp>::LocalGeometry &
  AlbertaGridIntersectionIterator<GridImp>::
  intersectionSelfLocal () const
  {
    fakeSelf_.builtLocalGeom(inside()->geometry(),intersectionGlobal(),
                             elInfo_,neighborCount_);
    return fakeSelfObj_;
  }

  template< class GridImp >
  inline const typename AlbertaGridIntersectionIterator<GridImp>::LocalGeometry &
  AlbertaGridIntersectionIterator<GridImp>::intersectionNeighborLocal () const
  {
    assert(neighbor());

    if(fakeNeigh_.builtLocalGeom(outside()->geometry(),intersectionGlobal(),
                                 &neighElInfo_,neighborCount_)
       )
      return fakeNeighObj_;
    else
    {
      DUNE_THROW(AlbertaError, "intersection_neighbor_local: error occured!");
    }
    return fakeNeighObj_;
  }

  template< class GridImp >
  inline const typename AlbertaGridIntersectionIterator<GridImp>::Geometry &
  AlbertaGridIntersectionIterator<GridImp>::
  intersectionGlobal () const
  {
    assert( elInfo_ );

    if(neighGlob_.builtGeom(elInfo_,neighborCount_,0,0))
      return neighGlobObj_;
    else
    {
      DUNE_THROW(AlbertaError, "intersection_self_global: error occured!");
    }
    return neighGlobObj_;
  }


  template< class GridImp >
  inline int AlbertaGridIntersectionIterator<GridImp>::
  level () const
  {
    assert( level_ >= 0 );
    return level_;
  }

  template< class GridImp >
  inline int AlbertaGridIntersectionIterator<GridImp>::
  numberInSelf () const
  {
    return neighborCount_;
  }

  template< class GridImp >
  inline int AlbertaGridIntersectionIterator<GridImp>::
  numberInNeighbor () const
  {
    assert( elInfo_ );
    return elInfo_->opp_vertex[neighborCount_];
  }

  template <class GridImp>
  inline int AlbertaGridIntersectionIterator<GridImp>::
  twistInSelf() const
  {
    return 0;
  }

  template <class GridImp>
  inline int AlbertaGridIntersectionIterator<GridImp>::
  twistInNeighbor() const
  {
    return twist_;
  }

  // setup neighbor element with the information of elInfo_
  template< class GridImp >
  inline bool AlbertaGridIntersectionIterator<GridImp>::neighborHasSameLevel () const
  {
    assert( neighbor() );
    const ALBERTA EL * myEl    = elInfo_->el;
    const ALBERTA EL * neighEl = NEIGH(elInfo_->el,elInfo_)[neighborCount_];

    return this->grid_.getLevelOfElement( myEl ) ==
           this->grid_.getLevelOfElement( neighEl );
  }

  //*****************************************
  //  setup for 2d
  //*****************************************
  template <class GridImp, int dimworld , int dim >
  struct SetupVirtualNeighbour
  {
    static int setupNeighInfo(GridImp & grid, const ALBERTA EL_INFO * elInfo,
                              const int vx, const int nb, ALBERTA EL_INFO * neighInfo)
    {

      // vx is the face number in the neighbour
      const int (& neighmap)[dim] = ALBERTA AlbertHelp :: localTriangleFaceNumber[vx];
      // neighborCount is the face number in the actual element
      const int (& selfmap) [dim] = ALBERTA AlbertHelp :: localTriangleFaceNumber[nb];

      // copy the two edge vertices
      for(int i=0; i<dim; i++)
      {
        const ALBERTA REAL_D & coord = elInfo->coord[ selfmap[i] ];
        // here the twist is simple, just swap the vertices, and do this by hand
        ALBERTA REAL_D & newcoord    = neighInfo->coord[ neighmap[(dim-1) - i] ];
        for(int j=0; j<dimworld; j++) newcoord[j] = coord[j];
      }
      //****************************************
      // twist is always 1
      return 1;
    }
  };

  //******************************************
  //  setup for 3d
  //******************************************
#if DIM == 3
  template <class GridImp, int dimworld >
  struct SetupVirtualNeighbour<GridImp,dimworld,3>
  {
    static int setupNeighInfo(GridImp & grid, const ALBERTA EL_INFO * elInfo,
                              const int vx, const int nb, ALBERTA EL_INFO * neighInfo)
    {
      enum { dim = 3 };
      // the face might be twisted when look from different elements
      // default is no, and then rthe orientation is -1
      int facemap[dim]   = {0,1,2};
      bool rightOriented = false;
      {
        int myvx[dim];
        int neighvx[dim];

        const int * vxmap = ALBERTA AlbertHelp :: localAlbertaFaceNumber[vx];
        const int * nbmap = ALBERTA AlbertHelp :: localAlbertaFaceNumber[nb];

        bool allRight = true;
        for(int i=0; i<dim; i++)
        {
          myvx[i]    = grid.getVertexNumber(elInfo->el   , nbmap[i] );
          neighvx[i] = grid.getVertexNumber(neighInfo->el, vxmap[i] );
          if( myvx[i] != neighvx[i] ) allRight = false;
        }

        // note: if the vertices are equal then the face in the neighbor
        // is not oriented right, because all face are oriented math. pos when
        // one looks from the outside of the element.
        // if the vertices are the same, the face in the neighbor is therefore
        // wrong oriented
        if( !allRight )
        {
          for(int i=0; i<dim; i++)
          {
            if(myvx[i] != neighvx[i])
            {
              for(int k=1; k<dim; k++)
              {
                int newvx = (i+k) % dim;
                if( myvx[i] == neighvx[newvx] ) facemap[i] = newvx;
              }
            }
          }
          rightOriented = true;
        }
      }

      // TODO check infulence of orientation
      // is used when calculation the outerNormal
      neighInfo->orientation = ( rightOriented ) ? elInfo->orientation : -elInfo->orientation;

      // vx is the face number in the neighbour
      const int * neighmap = ALBERTA AlbertHelp :: localAlbertaFaceNumber[vx];
      // neighborCount is the face number in the actual element
      const int * selfmap  = ALBERTA AlbertHelp :: localAlbertaFaceNumber[nb];

      // copy the three face vertices
      for(int i=0; i<dim; i++)
      {
        const ALBERTA REAL_D & coord = elInfo->coord[selfmap[i]];
        // here consider possible twist
        ALBERTA REAL_D & newcoord    = neighInfo->coord[ neighmap[ facemap[i] ] ];
        for(int j=0; j<dimworld; j++) newcoord[j] = coord[j];
      }
      //****************************************
      if (facemap[1] == (facemap[0]+1)%3) {
        return facemap[0];
      }
      return facemap[1]-3;
    }
  };
#endif

  // setup neighbor element with the information of elInfo_
  template< class GridImp >
  inline void AlbertaGridIntersectionIterator<GridImp>::setupVirtEn() const
  {
    // if this assertion fails then outside was called without checking
    // neighbor first
    assert(neighbor());

    assert( neighborCount_ < dim+1 );
    // set the neighbor element as element
    // use ALBERTA macro to get neighbour
    neighElInfo_.el = NEIGH(elInfo_->el,elInfo_)[neighborCount_];

    const int vx = elInfo_->opp_vertex[neighborCount_];
    // copy the one opposite vertex
    // the same for 2d and 3d
    {
      const ALBERTA REAL_D & coord = elInfo_->opp_coord[neighborCount_];
      ALBERTA REAL_D & newcoord    = neighElInfo_.coord[vx];
      for(int j=0; j<dimworld; j++) newcoord[j] = coord[j];
    }

    // setup coordinates of neighbour elInfo
    twist_ = SetupVirtualNeighbour<GridImp,dimworld,dim>::
             setupNeighInfo(this->grid_,elInfo_,vx,neighborCount_,&neighElInfo_);

    builtNeigh_ = true;
  }
  // end IntersectionIterator


  //*******************************************************
  //
  // --AlbertaGridTreeIterator
  // --TreeIterator
  // --LevelIterator
  //
  //*******************************************************

  namespace AlbertaTreeIteratorHelp {

    // for elements
    template <class IteratorImp, int dim>
    struct GoNextEntity<IteratorImp,dim,0>
    {
      static inline ALBERTA EL_INFO *
      goNext(IteratorImp & it, ALBERTA TRAVERSE_STACK *stack , ALBERTA EL_INFO *elinfo_old)
      {
        return it.goNextElInfo(stack,elinfo_old);
      }
    };

    // for faces
    template <class IteratorImp, int dim>
    struct GoNextEntity<IteratorImp,dim,1>
    {
      static inline ALBERTA EL_INFO *
      goNext(IteratorImp & it, ALBERTA TRAVERSE_STACK *stack , ALBERTA EL_INFO *elinfo_old)
      {
        return it.goNextFace(stack,elinfo_old);
      }
    };

    // for vertices
    template <class IteratorImp, int dim>
    struct GoNextEntity<IteratorImp,dim,dim>
    {
      static inline ALBERTA EL_INFO *
      goNext(IteratorImp & it, ALBERTA TRAVERSE_STACK *stack , ALBERTA EL_INFO *elinfo_old)
      {
        return it.goNextVertex(stack,elinfo_old);
      }
    };

    // for edges in 3d
    template <class IteratorImp>
    struct GoNextEntity<IteratorImp,3,2>
    {
      static inline ALBERTA EL_INFO *
      goNext(IteratorImp & it, ALBERTA TRAVERSE_STACK *stack , ALBERTA EL_INFO *elinfo_old)
      {
        return it.goNextEdge(stack,elinfo_old);
      }
    };
  } // end namespace AlbertaTreeIteratorHelp


  //***********************************************************
  //  some template specialization of goNextEntity
  //***********************************************************
  // default implementation, go next elInfo
  template<int codim, PartitionIteratorType pitype, class GridImp>
  inline ALBERTA EL_INFO * AlbertaGridTreeIterator<codim,pitype,GridImp>::
  goNextEntity(ALBERTA TRAVERSE_STACK *stack,ALBERTA EL_INFO *elinfo_old)
  {
    return AlbertaTreeIteratorHelp :: GoNextEntity<ThisType,GridImp::dimension,codim>::
           goNext(*this,stack,elinfo_old);
  }
  //***************************************

  template<int codim, PartitionIteratorType pitype, class GridImp>
  inline void AlbertaGridTreeIterator<codim,pitype,GridImp>::
  makeIterator()
  {
    level_ = 0;
    enLevel_ = 0;
    vertex_ = -1;
    face_ = -1;
    edge_ = -1;
    vertexMarker_ = 0;

    virtualEntity_.setTraverseStack(0);
    virtualEntity_.setElInfo(0,0,0,0);
  }

  // Make LevelIterator with point to element from previous iterations
  template<int codim, PartitionIteratorType pitype, class GridImp>
  inline AlbertaGridTreeIterator<codim,pitype,GridImp>::
  AlbertaGridTreeIterator(const GridImp & grid, int travLevel,int proc, bool leafIt )
    : AlbertaGridEntityPointer<codim,GridImp> (grid,travLevel,leafIt,true) // true means end iterator
      , level_   (travLevel)
      , enLevel_ (travLevel)
      , virtualEntity_( this->entityImp() )
      , face_(-1)
      , edge_ (-1)
      , vertex_ (-1)
      , vertexMarker_(0)
      , okReturn_(false)
      , proc_(proc)
  {}

  // Make LevelIterator with point to element from previous iterations
  template<int codim, PartitionIteratorType pitype, class GridImp>
  inline AlbertaGridTreeIterator<codim,pitype,GridImp>::
  AlbertaGridTreeIterator(const AlbertaGridTreeIterator<codim,pitype,GridImp> & org)
    : AlbertaGridEntityPointer<codim,GridImp> (org.grid_,org.level_, org.leafIt() , (org.vertexMarker_) ? false : true)
      , level_   (org.level_)
      , enLevel_ (org.enLevel_)
      , virtualEntity_( this->entityImp() )
      , manageStack_ ()
      //, manageStack_ ( org.manageStack_ )
      , face_(org.face_)
      , edge_ (org.edge_)
      , vertex_ ( org.vertex_)
      , vertexMarker_(org.vertexMarker_)
      , okReturn_ (org.okReturn_ )
      , proc_(org.proc_)
  {
    if(vertexMarker_)
    {
      // if vertexMarker is not NULL then we have a real iterator
      manageStack_.makeItNew(true);
      ALBERTA TRAVERSE_STACK * stack = manageStack_.getStack();
      ALBERTA copyTraverseStack( stack , org.manageStack_.getStack() );

      virtualEntity_.setTraverseStack( stack );
      /// get the actual used enInfo
      ALBERTA EL_INFO * elInfo = stack->elinfo_stack+stack->stack_used;

      virtualEntity_.setElInfo( elInfo,face_,edge_,vertex_ );
      virtualEntity_.setLevel( enLevel_ );

      assert( this->grid_.hierarchicIndexSet().index ( *(this->entity_) )
              == this->grid_.hierarchicIndexSet().index ( *(org.entity_) ) );
    }
  }

  // Make LevelIterator with point to element from previous iterations
  template<int codim, PartitionIteratorType pitype, class GridImp>
  inline AlbertaGridTreeIterator<codim,pitype,GridImp> &
  AlbertaGridTreeIterator<codim,pitype,GridImp>::operator =
    (const AlbertaGridTreeIterator<codim,pitype,GridImp> & org)
  {
    level_ = org.level_;
    enLevel_ = org.enLevel_;
    //, manageStack_ ( org.manageStack_ )
    face_ = (org.face_);
    edge_  = (org.edge_);
    vertex_  = ( org.vertex_);
    vertexMarker_ = (org.vertexMarker_);
    okReturn_ = (org.okReturn_ );

    assert( proc_ == org.proc_ );
    if(vertexMarker_)
    {
      // if vertexMarker is not NULL then we have a real iterator
      manageStack_.makeItNew(true);
      ALBERTA TRAVERSE_STACK * stack = manageStack_.getStack();
      ALBERTA copyTraverseStack( stack , org.manageStack_.getStack() );

      virtualEntity_.setTraverseStack( stack );
      /// get the actual used enInfo
      ALBERTA EL_INFO * elInfo = stack->elinfo_stack+stack->stack_used;

      virtualEntity_.setElInfo( elInfo,face_,edge_,vertex_ );
      virtualEntity_.setLevel( enLevel_ );

      assert( this->grid_.hierarchicIndexSet().index ( *(this->entity_) )
              == this->grid_.hierarchicIndexSet().index ( *(org.entity_) ) );
    }

    return *this;
  }

  template<int codim, PartitionIteratorType pitype, class GridImp>
  inline AlbertaGridTreeIterator<codim,pitype,GridImp>::
  AlbertaGridTreeIterator(const GridImp & grid,
                          const AlbertaMarkerVector * vertexMark,
                          int travLevel, int proc, bool leafIt)
    : AlbertaGridEntityPointer<codim,GridImp> (grid,travLevel,leafIt,false)
      , level_ (travLevel) , enLevel_(travLevel)
      , virtualEntity_( this->entityImp() )
      , face_(-1)
      , edge_ (-1)
      , vertex_ (-1)
      , vertexMarker_(0)
      , okReturn_ (false)
      , proc_(proc)
  {
    ALBERTA MESH * mesh = this->grid_.getMesh();

    if( mesh && ((travLevel >= 0) && (travLevel <= this->grid_.maxLevel())) )
    {
      vertexMarker_ = vertexMark;

      ALBERTA FLAGS travFlags = FILL_ANY; //FILL_COORDS | FILL_NEIGH;

      // CALL_LEAF_EL is not used anymore
      travFlags = travFlags | CALL_LEAF_EL_LEVEL;

      // get traverse_stack
      manageStack_.makeItNew(true);

      virtualEntity_.setTraverseStack(manageStack_.getStack());

      // diese Methode muss neu geschrieben werden, da man
      // die ParentElement explizit speichern moechte.
      ALBERTA EL_INFO* elInfo =
        goFirstElement(manageStack_.getStack(), mesh, travLevel,travFlags);

      virtualEntity_.setElInfo(elInfo,face_,edge_,vertex_);
      virtualEntity_.setLevel( enLevel_ );
    }
    else
    {
      // create empty iterator
      makeIterator();
    }
  }

  // gehe zum naechsten Element, wie auch immer
  template<int codim, PartitionIteratorType pitype, class GridImp>
  inline void AlbertaGridTreeIterator<codim,pitype,GridImp>::increment()
  {
    ALBERTA EL_INFO * nextinfo = goNextEntity(manageStack_.getStack(),virtualEntity_.getElInfo());

    if(!nextinfo)
    {
      this->done();
      return ;
    }

    virtualEntity_.setElInfo( nextinfo , face_, edge_, vertex_);
    virtualEntity_.setLevel( enLevel_ );

    return ;
  }

  template<int codim, PartitionIteratorType pitype, class GridImp>
  inline ALBERTA EL_INFO * AlbertaGridTreeIterator<codim,pitype,GridImp>::
  goNextFace(ALBERTA TRAVERSE_STACK *stack, ALBERTA EL_INFO *elInfo)
  {
    // go next Element, if face_ > numberOfVertices, then go to next elInfo
    // face_ is set to -1 by constructor
    ++face_;
    if(face_ >= (dim+1)) // dim+1 Faces
    {
      // we have checked all faces on this element,
      // therefore go to next element
      elInfo = goNextElInfo(stack, elInfo);
      face_ = 0;
      if(!elInfo) return 0; // if no more Faces, return 0 which leads to end
    }

    // check elInfo pointer before we start anything
    assert(elInfo);

    if(!this->leafIt())
    {
      // go next, if Vertex is not treated on this Element
      if(vertexMarker_->faceNotOnElement(
           this->grid_.getElementNumber(elInfo->el),
           this->grid_.getFaceNumber(elInfo->el,face_)))
      {
        elInfo = goNextFace(stack,elInfo);
      }
    }
    else
    {
      // get neighbour of this element
      const ALBERTA EL * neighbour = NEIGH(elInfo->el,elInfo)[face_];
      if( neighbour )
      {
        // get element
        const ALBERTA EL * el = elInfo->el;
        assert( el );

        // if true we must go to next face
        // when element number is small then go next because now the face is
        // reached on the element with the largest number
        bool goWeida = this->grid_.getElementNumber( el ) < this->grid_.getElementNumber( neighbour ) ;

        // additional check for level iterators
        if(goWeida && ! this->leafIt() )
        {
          // if we should go weida then only go
          // if neighbours are on the same level
          goWeida = (this->grid_.getLevelOfElement( neighbour ) == level_);
        }

        // the face was reached before therefore go to next face
        if(goWeida) elInfo = goNextFace(stack,elInfo);
      }
    }

    return elInfo;
  }

  template<int codim, PartitionIteratorType pitype, class GridImp>
  inline ALBERTA EL_INFO * AlbertaGridTreeIterator<codim,pitype,GridImp>::
  goNextEdge(ALBERTA TRAVERSE_STACK *stack, ALBERTA EL_INFO *elInfo)
  {
    // go next Element, Edge 0
    // treat Edge like Faces
    // edge_ is set to -1 by constructor
    ++edge_;
    if(edge_ >= 6) // in 3d only 6 Edges
    {
      elInfo = goNextElInfo(stack, elInfo);
      edge_ = 0;
      if(!elInfo) return 0; // if no more Edges, return
    }

    assert(elInfo);

    // go next, if Vertex is not treated on this Element
    if(vertexMarker_->edgeNotOnElement(
         this->grid_.getElementNumber(elInfo->el),
         this->grid_.getEdgeNumber(elInfo->el,edge_)))
    {
      elInfo = goNextEdge(stack,elInfo);
    }

    return elInfo;
  }

  template<int codim, PartitionIteratorType pitype, class GridImp>
  inline ALBERTA EL_INFO * AlbertaGridTreeIterator<codim,pitype,GridImp>::
  goNextVertex(ALBERTA TRAVERSE_STACK *stack, ALBERTA EL_INFO *elInfo)
  {
    // go next Element, Vertex 0
    // treat Vertices like Faces
    // vertex_ is set to -1 by constructor
    ++vertex_;
    if(vertex_ >= (dim+1)) // dim+1 Vertices
    {
      elInfo = goNextElInfo(stack, elInfo);
      vertex_ = 0;
      if(!elInfo) return 0; // if no more Vertices, return
    }

    assert(elInfo);

    // go next, if Vertex is not treated on this Element
    if(vertexMarker_->vertexNotOnElement(
         this->grid_.getElementNumber(elInfo->el),
         this->grid_.getVertexNumber(elInfo->el,vertex_)))
    {
      elInfo = goNextVertex(stack,elInfo);
    }

    return elInfo;
  }

  template<int codim, PartitionIteratorType pitype, class GridImp>
  inline ALBERTA EL_INFO * AlbertaGridTreeIterator<codim,pitype,GridImp>::
  goFirstElement(ALBERTA TRAVERSE_STACK *stack,ALBERTA MESH *mesh, int level,
                 ALBERTA FLAGS fill_flag)
  {
    FUNCNAME("goFirstElement");

    if (!stack)
    {
      ALBERTA_ERROR("no traverse stack\n");
      return 0;
    }

    stack->traverse_mesh      = mesh;
    stack->traverse_level     = level;
    stack->traverse_fill_flag = fill_flag;

    if (stack->stack_size < 1)
      enlargeTraverseStack(stack);

    for (int i=0; i<stack->stack_size; i++)
      stack->elinfo_stack[i].fill_flag = fill_flag & FILL_ANY;

    stack->elinfo_stack[0].mesh = stack->elinfo_stack[1].mesh = mesh;

    if (fill_flag & CALL_LEAF_EL_LEVEL)
    {
      ALBERTA_TEST_EXIT(level >= 0) ("invalid level: %d\n",level);
    }

    stack->traverse_mel = 0;
    stack->stack_used   = 0;
    stack->el_count     = 0;

    // go to first enInfo, therefore goNextElInfo
    ALBERTA EL_INFO * elinfo = goNextElInfo(stack,0);
    // for codim 0 we are done at this point
    if( codim == 0 ) return elinfo;

    // if iterate over faces, edges or vertices
    // we have to go the normal way, because
    // is not preset that the first face lies on the first element
    // therefore face_,edge_,vertex_ == -1 at the start
    return goNextEntity(stack,elinfo);
  }


  // --travNext
  template<int codim, PartitionIteratorType pitype, class GridImp>
  inline ALBERTA EL_INFO * AlbertaGridTreeIterator<codim,pitype,GridImp>::
  goNextElInfo(ALBERTA TRAVERSE_STACK *stack, ALBERTA EL_INFO *elinfo_old)
  {
    assert( stack );
    //assert( elinfo_old );
    assert( (stack->stack_used) ?
            (elinfo_old == stack->elinfo_stack+stack->stack_used) :
            (elinfo_old == 0) );

    /*
       PartitionIteratorType pt = pitype;
       if(this->grid_.myRank() < 0) pt = All_Partition;

       switch (pt)
       {
       // Walk over all macro_elements on this grid
       case All_Partition:
     */
    {
      // overloaded traverse_leaf_el_level, is not implemened in ALBERTA yet
      ALBERTA EL_INFO  * elinfo = traverseElLevel(stack);

      // if leafIt_ == false go to elements only on desired level
      if((elinfo) && (! this->leafIt()) )
      {
        if(elinfo->level == stack->traverse_level)
          okReturn_ = true;

        while(!okReturn_)
        {
          elinfo = traverseElLevel(stack);
          if(!elinfo) okReturn_ = true;
        }
        ++(stack->el_count);
      }

      // set new level for Entity
      if((elinfo) && (this->leafIt()) ) enLevel_ = elinfo->level;

      return(elinfo);
    }
#if 0
  // walk only over macro_elements that belong to this processor
  case Interior_Partition :
  {
    // overloaded traverse_leaf_el_level, is not implemened in ALBERTA yet
    elinfo = traverseElLevelInteriorBorder(stack);

    // if leafIt_ == false go to elements only on desired level
    if((elinfo) && (! this->leafIt()) )
    {
      if(elinfo->level == stack->traverse_level)
        okReturn_ = true;

      while(!okReturn_)
      {
        elinfo = traverseElLevelInteriorBorder(stack);
        if(!elinfo) okReturn_ = true;
      }
      ++(stack->el_count);
    }

    // set new level for Entity
    if((elinfo) && (this->leafIt()) ) enLevel_ = elinfo->level;

    return(elinfo);
  }

  // walk over ghost elements, if proc == -1 then over all ghosts
  case Ghost_Partition :
  {
    // overloaded traverse_leaf_el_level, is not implemened in ALBERTA yet
    elinfo = traverseElLevelGhosts(stack);

    // if leafIt_ == false go to elements only on desired level
    if((elinfo) && (! this->leafIt()) )
    {
      if(elinfo->level == stack->traverse_level)
        okReturn_ = true;

      while(!okReturn_)
      {
        elinfo = traverseElLevelGhosts(stack);
        if(!elinfo) okReturn_ = true;
      }
      ++(stack->el_count);
    }

    // check neighbours
    if(elinfo)
    {
      // here we have the interior element, now check the neighbours
      for(int i=0; i<dim+1; i++)
      {
        const ALBERTA EL * neigh = NEIGH(elinfo->el,elinfo)[i];
        if(neigh)
        {
          if(this->grid_.getOwner(neigh) == this->grid_.myRank())
          {
            return elinfo;
          }
        }
      }
      return goNextElInfo(stack,elinfo);
    }

    // set new level for Entity
    if((elinfo) && (this->leafIt())) enLevel_ = elinfo->level;

    return(elinfo);
  }

  // walk over interior elements which have ghosts as neighbour
  case InteriorBorder_Partition :
  {
    // overloaded traverse_leaf_el_level, is not implemened in ALBERTA yet
    elinfo = traverseElLevelInteriorBorder(stack);

    // if leafIt_ == false go to elements only on desired level
    if((elinfo) && (! this->leafIt()) )
    {
      if(elinfo->level == stack->traverse_level)
        okReturn_ = true;

      while(!okReturn_)
      {
        elinfo = traverseElLevelInteriorBorder(stack);
        if(!elinfo) okReturn_ = true;
      }
      ++(stack->el_count);
    }

    if(elinfo)
    {
      // here we have the interior element, now check the neighbours
      for(int i=0; i<dim+1; i++)
      {
        ALBERTA EL * neigh = NEIGH(elinfo->el,elinfo)[i];
        if(neigh)
        {
          if(((proc_ == -1) && (this->grid_.getOwner(neigh) != this->grid_.myRank()))||
             ((proc_ != -1) && (this->grid_.getOwner(neigh) == proc_) ))
          {
            return elinfo;
          }
        }
      }
      // we found not the one, so go next
      return goNextElInfo(stack,elinfo);
    }

    // set new level for Entity
    if((elinfo) && (this->leafIt()) ) enLevel_ = elinfo->level;

    return elinfo;
  }
  // default iterator type no supported
  default :
  {
    DUNE_THROW(AlbertaError, "AlbertaGridTreeIterator::goNextEntity: Unsupported IteratorType!");
    return 0;
  }
  } // end switch
#endif
  }

  template<int codim, PartitionIteratorType pitype, class GridImp>
  inline ALBERTA EL_INFO * AlbertaGridTreeIterator<codim,pitype,GridImp>::
  traverseElLevel(ALBERTA TRAVERSE_STACK *stack)
  {
    FUNCNAME("traverseElLevel");
    ALBERTA EL *el;
    int i;
    okReturn_ = false;

    if (stack->stack_used == 0) /* first call */
    {
      if(proc_ >= 0)
      {
        ALBERTA MACRO_EL * mel = stack->traverse_mesh->first_macro_el;
        while((this->grid_.getOwner(mel->el) != this->grid_.myRank()
               && this->grid_.isNoElement(mel)))
        {
          mel = mel->next;
          if(!mel) break;
        }
        stack->traverse_mel = mel;
      }
      else
      {
        stack->traverse_mel = stack->traverse_mesh->first_macro_el;
      }
      if (stack->traverse_mel == nil) return(nil);

      stack->stack_used = 1;

      ALBERTA fillMacroInfo(stack, stack->traverse_mel,
                            stack->elinfo_stack+stack->stack_used,level_);

      stack->info_stack[stack->stack_used] = 0;

      el = stack->elinfo_stack[stack->stack_used].el;
      if ((el == nil) || (el->child[0] == nil)) {
        return(stack->elinfo_stack+stack->stack_used);
      }
    }
    else
    {
      el = stack->elinfo_stack[stack->stack_used].el;

      /* go up in tree until we can go down again */
      while((stack->stack_used > 0) &&
            ((stack->info_stack[stack->stack_used] >= 2) || (el->child[0]==nil)
             || ( stack->traverse_level <=
                  (stack->elinfo_stack+stack->stack_used)->level)) )
      {
        stack->stack_used--;
        el = stack->elinfo_stack[stack->stack_used].el;
      }
      /* goto next macro element */
      if (stack->stack_used < 1)
      {
        ALBERTA MACRO_EL * mel = stack->traverse_mel->next;
        if(mel && (proc_ >= 0))
        {
          while((this->grid_.getOwner(mel->el) != this->grid_.myRank()
                 && this->grid_.isNoElement(mel)))
          {
            mel = mel->next;
            if(!mel) break;
          }
          stack->traverse_mel = mel;
        }
        stack->traverse_mel = mel;
        if (stack->traverse_mel == nil) return(nil);

        stack->stack_used = 1;

        ALBERTA fillMacroInfo(stack, stack->traverse_mel,
                              stack->elinfo_stack+stack->stack_used,level_);

        stack->info_stack[stack->stack_used] = 0;

        el = stack->elinfo_stack[stack->stack_used].el;
        if ((el == nil) || (el->child[0] == nil))
        {
          return(stack->elinfo_stack+stack->stack_used);
        }
      }
    }

    /* go down tree until leaf oder level*/
    while (el->child[0] &&
           ( stack->traverse_level > (stack->elinfo_stack+stack->stack_used)->level))
    {
      if(stack->stack_used >= stack->stack_size-1)
        enlargeTraverseStack(stack);

      i = stack->info_stack[stack->stack_used];
      el = el->child[i];
      stack->info_stack[stack->stack_used]++;

      this->grid_.fillElInfo(i, level_, stack->elinfo_stack+stack->stack_used,
                             stack->elinfo_stack+stack->stack_used+1, false , this->leafIt() );

      stack->stack_used++;

      ALBERTA_TEST_EXIT(stack->stack_used < stack->stack_size)
        ("stack_size=%d too small, level=(%d,%d)\n",
        stack->stack_size, stack->elinfo_stack[stack->stack_used].level);

      stack->info_stack[stack->stack_used] = 0;

      if(stack->traverse_level == (stack->elinfo_stack+stack->stack_used)->level)
      {
        okReturn_ = true;
      }
    }

    return(stack->elinfo_stack+stack->stack_used);
  }

  // iterate over interior elements
  template<int codim, PartitionIteratorType pitype, class GridImp>
  inline ALBERTA EL_INFO * AlbertaGridTreeIterator<codim,pitype,GridImp>::
  traverseElLevelInteriorBorder(ALBERTA TRAVERSE_STACK *stack)
  {
    FUNCNAME("traverseElLevelInteriorBorder");
    ALBERTA EL *el;
    int i;
    okReturn_ = false;

    if (stack->stack_used == 0) /* first call */
    {
      ALBERTA MACRO_EL * mel = stack->traverse_mesh->first_macro_el;
      while(this->grid_.getOwner(mel->el) != this->grid_.myRank())
      {
        mel = mel->next;
        if(!mel) break;
      }
      stack->traverse_mel = mel;
      if (stack->traverse_mel == nil) return(nil);

      stack->stack_used = 1;

      ALBERTA fillMacroInfo(stack, stack->traverse_mel,
                            stack->elinfo_stack+stack->stack_used, level_ );

      stack->info_stack[stack->stack_used] = 0;

      el = stack->elinfo_stack[stack->stack_used].el;
      if ((el == nil) || (el->child[0] == nil)) {
        return(stack->elinfo_stack+stack->stack_used);
      }
    }
    else
    {
      el = stack->elinfo_stack[stack->stack_used].el;

      /* go up in tree until we can go down again */
      while((stack->stack_used > 0) &&
            ((stack->info_stack[stack->stack_used] >= 2) || (el->child[0]==nil)
             || ( stack->traverse_level <=
                  (stack->elinfo_stack+stack->stack_used)->level)) )
      {
        stack->stack_used--;
        el = stack->elinfo_stack[stack->stack_used].el;
      }
      /* goto next macro element */
      if (stack->stack_used < 1)
      {
        ALBERTA MACRO_EL * mel = stack->traverse_mel->next;
        if(mel)
        {
          while(this->grid_.getOwner(mel->el) != this->grid_.myRank())
          {
            mel = mel->next;
            if(!mel) break;
          }
        }
        stack->traverse_mel = mel;
        if (stack->traverse_mel == nil) return(nil);

        stack->stack_used = 1;

        ALBERTA fillMacroInfo(stack, stack->traverse_mel,
                              stack->elinfo_stack+stack->stack_used,level_);

        stack->info_stack[stack->stack_used] = 0;

        el = stack->elinfo_stack[stack->stack_used].el;
        if ((el == nil) || (el->child[0] == nil))
        {
          return(stack->elinfo_stack+stack->stack_used);
        }
      }
    }

    /* go down tree until leaf or level reached */
    while (el->child[0] &&
           ( stack->traverse_level > (stack->elinfo_stack+stack->stack_used)->level))
    {
      if(stack->stack_used >= stack->stack_size-1)
        enlargeTraverseStack(stack);

      i = stack->info_stack[stack->stack_used];
      el = el->child[i];
      stack->info_stack[stack->stack_used]++;

      this->grid_.fillElInfo(i, level_, stack->elinfo_stack+stack->stack_used,
                             stack->elinfo_stack+stack->stack_used+1, false, this->leafIt() );

      stack->stack_used++;

      ALBERTA_TEST_EXIT(stack->stack_used < stack->stack_size)
        ("stack_size=%d too small, level=(%d,%d)\n",
        stack->stack_size, stack->elinfo_stack[stack->stack_used].level);

      stack->info_stack[stack->stack_used] = 0;
      if(stack->traverse_level == (stack->elinfo_stack+stack->stack_used)->level)
      {
        okReturn_ = true;
      }
    }

    return(stack->elinfo_stack+stack->stack_used);

  }

  template<int codim, PartitionIteratorType pitype, class GridImp>
  inline ALBERTA MACRO_EL * AlbertaGridTreeIterator<codim,pitype,GridImp>::
  nextGhostMacro(ALBERTA MACRO_EL * oldmel)
  {
    ALBERTA MACRO_EL * mel = oldmel;
    if(mel)
    {
      while( (!this->grid_.isGhost(mel)) )
      {
        mel = mel->next;
        if(!mel) break;
      }
    }
    return mel;
  }

  template<int codim, PartitionIteratorType pitype, class GridImp>
  inline ALBERTA EL_INFO * AlbertaGridTreeIterator<codim,pitype,GridImp>::
  traverseElLevelGhosts(ALBERTA TRAVERSE_STACK *stack)
  {
    FUNCNAME("traverseElLevelGhosts");
    ALBERTA EL *el;
    int i;
    okReturn_ = false;

    if (stack->stack_used == 0) /* first call */
    {
      stack->traverse_mel = nextGhostMacro(stack->traverse_mesh->first_macro_el);
      if (stack->traverse_mel == nil) return(nil);

      stack->stack_used = 1;

      ALBERTA fillMacroInfo(stack, stack->traverse_mel,
                            stack->elinfo_stack+stack->stack_used,level_);

      stack->info_stack[stack->stack_used] = 0;

      el = stack->elinfo_stack[stack->stack_used].el;
      if ((el == nil) || (el->child[0] == nil)) {
        return(stack->elinfo_stack+stack->stack_used);
      }
    }
    else
    {
      el = stack->elinfo_stack[stack->stack_used].el;

      /* go up in tree until we can go down again */
      while((stack->stack_used > 0) &&
            ((stack->info_stack[stack->stack_used] >= 2) || (el->child[0]==nil)
             || ( stack->traverse_level <=
                  (stack->elinfo_stack+stack->stack_used)->level)) )
      // Aenderung hier
      {
        stack->stack_used--;
        el = stack->elinfo_stack[stack->stack_used].el;
      }
      /* goto next macro element */
      if (stack->stack_used < 1) {

        ALBERTA MACRO_EL * mel = nextGhostMacro(stack->traverse_mel->next);
        if(!mel) return 0;

        stack->traverse_mel = mel;

        stack->stack_used = 1;

        ALBERTA fillMacroInfo(stack, stack->traverse_mel,
                              stack->elinfo_stack+stack->stack_used,level_);

        stack->info_stack[stack->stack_used] = 0;

        el = stack->elinfo_stack[stack->stack_used].el;
        if ((el == nil) || (el->child[0] == nil))
        {
          return(stack->elinfo_stack+stack->stack_used);
        }
      }
    }

    /* go down tree until leaf oder level*/
    while (el->child[0] && (this->grid_.getOwner(el) >= 0) &&
           ( stack->traverse_level > (stack->elinfo_stack+stack->stack_used)->level))
    {
      if(stack->stack_used >= stack->stack_size-1)
        enlargeTraverseStack(stack);

      i = stack->info_stack[stack->stack_used];
      el = el->child[i];

      stack->info_stack[stack->stack_used]++;

      // go next possible element, if not ghost
      if( this->grid_.getOwner(el) < 0)
        return traverseElLevelGhosts(stack);

      this->grid_.fillElInfo(i, level_, stack->elinfo_stack+stack->stack_used,
                             stack->elinfo_stack+stack->stack_used+1, false);

      stack->stack_used++;

      ALBERTA_TEST_EXIT(stack->stack_used < stack->stack_size)
        ("stack_size=%d too small, level=(%d,%d)\n",
        stack->stack_size, stack->elinfo_stack[stack->stack_used].level);

      stack->info_stack[stack->stack_used] = 0;

      if(stack->traverse_level == (stack->elinfo_stack+stack->stack_used)->level)
      {
        okReturn_ = true;
      }
    }

    return(stack->elinfo_stack+stack->stack_used);

  }

  //*************************************************************************
  //  end AlbertaGridTreeIterator
  //*************************************************************************

  template <int dim, class GridImp>
  inline AlbertaGridHierarchicIterator<GridImp>
  AlbertaGridEntity <0,dim,GridImp>::hbegin(int maxlevel) const
  {
    // Kopiere alle Eintraege des stack, da man im Stack weiterlaeuft und
    // sich deshalb die Werte anedern koennen, der elinfo_stack bleibt jedoch
    // der gleiche, deshalb kann man auch nur nach unten, d.h. zu den Kindern
    // laufen
    return AlbertaGridHierarchicIterator<GridImp> (grid_,travStack_,level(),maxlevel, this->leafIt() );
  }

  template <int dim, class GridImp>
  inline AlbertaGridHierarchicIterator<GridImp>
  AlbertaGridEntity <0,dim,GridImp>::hend(int maxlevel) const
  {
    AlbertaGridHierarchicIterator<GridImp> it(grid_,level(),maxlevel);
    return it;
  }

  template <int dim, class GridImp>
  inline typename AlbertaGridEntity <0,dim,GridImp>::AlbertaGridIntersectionIteratorType
  AlbertaGridEntity <0,dim,GridImp>::ibegin() const
  {
    //return AlbertaGridIntersectionIteratorType(grid_,level(),elInfo_, this->leafIt() );
    return AlbertaGridIntersectionIteratorType(grid_,*this,level(),false);
  }

  template <int dim, class GridImp>
  inline typename AlbertaGridEntity <0,dim,GridImp>::AlbertaGridIntersectionIteratorType
  AlbertaGridEntity <0,dim,GridImp>::iend() const
  {
    //return AlbertaGridIntersectionIteratorType(grid_,level());
    return AlbertaGridIntersectionIteratorType(grid_,*this,level(),true);
  }

  //*********************************************************************
  //
  //  AlbertMarkerVertex
  //
  //*********************************************************************
  inline bool AlbertaMarkerVector::
  vertexNotOnElement(const int elIndex, const int vertex) const
  {
    return (vec_[ vertex ] != elIndex);
  }

  inline bool AlbertaMarkerVector::
  edgeNotOnElement(const int elIndex, const int edge) const
  {
    return (edgevec_[ edge ] != elIndex);
  }

  inline bool AlbertaMarkerVector::
  faceNotOnElement(const int elIndex, const int face) const
  {
    assert( facevec_.size () > 0 );
    return (facevec_[ face ] != elIndex);
  }

  namespace AlbertaMarkerVectorHelp {

    // only for 3d calc edges markers
    template <class GridType>
    struct MarkFaces
    {
      inline static void mark(GridType & grid , Array<int> & vec, const ALBERTA EL * el, int elindex)
      {
        enum { dim = GridType :: dimension };
        // we have dim+1 faces
        for(int i=0; i<dim+1; ++i)
        {
          // the 1 is the difference between dim=3 and codim=2
          int num = grid.getFaceNumber(el ,i);
          if( vec[num] == -1 ) vec[num] = elindex;
        }
      }
    };

    template <class GridType, int dim>
    struct MarkEdges
    {
      inline static void mark(GridType & grid , Array<int> & vec, const ALBERTA EL * el, int elindex)
      {}
    };

    // only for 3d calc edges markers
    template <class GridType>
    struct MarkEdges<GridType,3>
    {
      inline static void mark(GridType & grid , Array<int> & vec, const ALBERTA EL * el, int elindex)
      {
        // in 3d 6 edges
        for(int i=0; i<6; ++i)
        {
          // the 1 is the difference between dim=3 and codim=2
          int num = grid.getEdgeNumber(el ,i);
          if( vec[num] == -1 ) vec[num] = elindex;
        }
      }
    };

  } // end namespace AlbertaMarkerVectorHelp

  template <class GridType>
  inline void AlbertaMarkerVector::markNewVertices(GridType &grid, int level)
  {
    assert( meLevel_ == true );
    enum { dim      = GridType::dimension };
    enum { dimworld = GridType::dimensionworld };

    typedef typename GridType :: HierarchicIndexSet HIndexSet;

    const HIndexSet & hset = grid.hierarchicIndexSet();
    int nvx = hset.size(dim);
    int fce = hset.size(1);

    {
      Array<int> & vec     = vec_;
      if(vec.size() < nvx) vec.resize( nvx + vxBufferSize_ );
      for(int i=0; i<vec.size(); i++) vec[i] = -1;

      Array<int> & facevec     = facevec_;
      if(facevec.size() < fce) facevec.resize( fce + vxBufferSize_ );
      for(int i=0; i<facevec.size(); i++) facevec[i] = -1;

      Array<int> & edgevec = edgevec_;
      if(dim > 2)
      {
        int edg = hset.size(dim-1);
        if(edgevec.size() < edg) edgevec.resize( edg + vxBufferSize_ );
        for(int i=0; i<edgevec.size(); i++) edgevec[i] = -1;
      }

      typedef typename GridType::template Codim<0>::LevelIterator LevelIteratorType;
      LevelIteratorType endit = grid.template lend<0> (level);
      for(LevelIteratorType it = grid.template lbegin<0> (level); it != endit; ++it)
      {
        const ALBERTA EL * el =
          (grid.getRealImplementation(*it)).getElInfo()->el;

        int elindex = grid.getElementNumber(el);
        for(int local=0; local<dim+1; local++)
        {
          int num = grid.getVertexNumber(el,local); // vertex num
          if( vec[num] == -1 ) vec[num] = elindex;
        }

        // mark edges for this element
        // in 3d 6 edges
        AlbertaMarkerVectorHelp::MarkFaces<GridType>::mark(grid,facevec, el,elindex );

        // mark edges for this element
        // in 3d 6 edges
        AlbertaMarkerVectorHelp::MarkEdges<GridType,dim>::mark(grid,edgevec, el,elindex );
      }
      // remember the number of entity on level and codim = 0
    }
    up2Date_ = true;
  }

  // mark vertices and edges using leaf iterator
  template <class GridType>
  inline void AlbertaMarkerVector::markNewLeafVertices(GridType &grid)
  {
    assert( meLevel_ == false );
    enum { dim      = GridType::dimension };
    enum { dimworld = GridType::dimensionworld };

    int nvx = grid.hierarchicIndexSet().size(dim);

    {
      Array<int> & vec = vec_;
      if(vec.size() < nvx) vec.resize( nvx + vxBufferSize_ );

      // the edge marking is only needed in 3d
      Array<int> & edgevec = edgevec_;
      if( dim > 2 )
      {
        int edg = grid.hierarchicIndexSet().size(dim-1);
        if(edgevec.size() < edg) edgevec.resize( edg + vxBufferSize_ );
        for(int i=0; i<edgevec.size(); i++) edgevec[i] = -1;
      }

      for(int i=0; i<vec.size(); i++) vec[i] = -1;

      typedef typename GridType::template Codim<0>::LeafIterator IteratorType;
      IteratorType endit = grid.template leafend<0> ();
      for(IteratorType it = grid.template leafbegin<0> (); it != endit; ++it)
      {
        const ALBERTA EL * el =
          (grid.getRealImplementation(*it)).getElInfo()->el;

        int elindex = grid.hierarchicIndexSet().index(*it);
        for(int local=0; local<dim+1; local++)
        {
          int num = el->dof[local][0]; // vertex num
          if( vec[num] == -1 ) vec[num] = elindex;
        }

        // mark edges for this element
        AlbertaMarkerVectorHelp::MarkEdges<GridType,dim>::mark(grid,edgevec,el,elindex);
      }
      // remember the number of entity on leaf level and codim = 0
    }
    up2Date_ = true;
  }

  inline void AlbertaMarkerVector::print() const
  {
    {
      if(vec_.size() > 0)
      {
        printf("\nEntries %d \n",vec_.size());
        for(int i=0; i<vec_.size(); i++)
          printf("Vx %d visited on Element %d \n",i,vec_[i]);
      }
    }
  }

  //***********************************************************************
  //
  // --AlbertaGrid
  // --Grid
  //
  //***********************************************************************
  template < int dim, int dimworld >
  inline AlbertaGrid < dim, dimworld >::AlbertaGrid() :
    mesh_ (0), maxlevel_ (0) , wasChanged_ (false)
    , isMarked_ (false)
    , vertexMarkerLeaf_(false) // creates LeafMarkerVector
    , nv_ (dim+1) , dof_ (0) , myRank_ (0)
    , hIndexSet_(*this)
    , globalIdSet_(*this)
    , levelIndexVec_(MAXL,0)
    , leafIndexSet_ (0)
    , geomTypes_(dim+1,1)
    , sizeCache_ (0)
  {
    // stored is the dim, where is the codim
    for(int i=dim; i>= 0; i--)
      geomTypes_[dim-i][0] = GeometryType(GeometryType::simplex,i);

    for(int i=0; i<AlbertHelp::numOfElNumVec; i++) dofvecs_.elNumbers[i] = 0;
    dofvecs_.elNewCheck = 0;
    dofvecs_.owner      = 0;
  }

  template < int dim, int dimworld >
  inline void AlbertaGrid < dim, dimworld >::initGrid(int proc)
  {
    ALBERTA AlbertHelp::getDofVecs(&dofvecs_);
    ALBERTA AlbertHelp::setDofVec ( dofvecs_.owner, -1 );

    // dont delete dofs on higher levels
    // needed for element numbering
    mesh_->preserve_coarse_dofs = 1;

    calcExtras();

    wasChanged_ = true;
    isMarked_ = false;

    macroVertices_.resize( mesh_->n_vertices );

    LeafDataType::initLeafDataValues(mesh_,proc);

    calcExtras();
  }

  template < int dim, int dimworld >
  inline AlbertaGrid < dim, dimworld >::AlbertaGrid(const std::string macroTriangFilename) :
    mesh_ (0), maxlevel_ (0) , wasChanged_ (false)
    , isMarked_ (false)
    , vertexMarkerLeaf_(false) // creates LeafMarkerVector
    , nv_ (dim+1) , dof_ (0) , myRank_ (-1)
    , hIndexSet_(*this)
    , globalIdSet_( *this )
    , levelIndexVec_(MAXL,0)
    , leafIndexSet_ (0)
    , geomTypes_(dim+1,1)
    , sizeCache_ (0)
  {
    // stored is the dim, where is the codim
    for(int i=dim; i>= 0; i--)
      geomTypes_[dim-i][0] = GeometryType(GeometryType::simplex,i);

    assert(dimworld == DIM_OF_WORLD);
    assert(dim      == DIM);

    const char * MacroTriangFilename = macroTriangFilename.c_str();
    assert( MacroTriangFilename );

    bool makeNew = true;
    {
      std::fstream file (MacroTriangFilename,std::ios::in);
      if(!file) DUNE_THROW(AlbertaIOError,"could not open grid file " << MacroTriangFilename);

      std::basic_string <char> str,str1;
      file >> str1; str = str1.assign(str1,0,3);
      // With that Albert MacroTriang starts DIM or DIM_OF_WORLD
      if (str != "DIM") makeNew = false;
      file.close();
    }

    ALBERTA AlbertHelp::initIndexManager_elmem_cc(indexStack_);

    if(makeNew)
    {
      ALBERTA AlbertHelp :: initBndStack( &bndStack_ );
      mesh_ = ALBERTA get_mesh("AlbertaGrid",
                               ALBERTA AlbertHelp::initDofAdmin<dim>,
                               LeafDataType::initLeafData);
      ALBERTA read_macro(mesh_, MacroTriangFilename, ALBERTA AlbertHelp::initBoundary);
      ALBERTA AlbertHelp :: removeBndStack ();

      initGrid(0);
    }
    else
    {
      double time = 0.0;
      GrapeDataIO < AlbertaGrid <dim,dimworld> > dataIO;
      dataIO.readGrid ( *this, MacroTriangFilename,time,0);
    }
    std::cout << "AlbertaGrid<"<<dim<<","<<dimworld<<"> created from macro grid file '" << macroTriangFilename << "'. \n\n";
  }

  template < int dim, int dimworld >
  inline AlbertaGrid < dim, dimworld >::
  AlbertaGrid(AlbertaGrid<dim,dimworld> & oldGrid, int proc) :
    mesh_ (0), maxlevel_ (0) , wasChanged_ (false)
    , isMarked_ (false)
    , vertexMarkerLeaf_(false) // creates LeafMarkerVector
    , nv_ (dim+1) , dof_ (0), myRank_ (proc)
    , hIndexSet_(*this)
    , globalIdSet_( *this )
    , levelIndexVec_(MAXL,0)
    , leafIndexSet_ (0)
    , geomTypes_(dim+1,1)
    , sizeCache_ (0)
  {
    for(int i=dim; i>= 0; i--)
      geomTypes_[dim-i][0] = GeometryType(GeometryType::simplex,i);

    assert(dimworld == DIM_OF_WORLD);
    assert(dim      == DIM);

    //ALBERTA MESH * oldMesh = oldGrid.getMesh();
    //ALBERTA AlbertHelp::initIndexManager_elmem_cc(indexStack_);

    DUNE_THROW(AlbertaError,"To be revised!");
  }

  template < int dim, int dimworld >
  inline AlbertaGrid < dim, dimworld >::
  AlbertaGrid(const AlbertaGrid<dim,dimworld> & copy )
  {
    DUNE_THROW(AlbertaError,"do not use grid copy constructor! ");
  }

  template < int dim, int dimworld >
  inline void AlbertaGrid < dim, dimworld >::removeMesh()
  {
    for(unsigned int i=0; i<levelIndexVec_.size(); i++)
      if(levelIndexVec_[i]) delete levelIndexVec_[i];

    if(leafIndexSet_) delete leafIndexSet_;

    for(int i=0; i<AlbertHelp::numOfElNumVec; i++)
      if(dofvecs_.elNumbers[i]) ALBERTA free_dof_int_vec(dofvecs_.elNumbers[i]);

    if(dofvecs_.elNewCheck) ALBERTA free_dof_int_vec(dofvecs_.elNewCheck);
    if(dofvecs_.owner ) ALBERTA free_dof_int_vec(dofvecs_.owner);

    if(sizeCache_) delete sizeCache_;sizeCache_ = 0;

#if DIM == 3
    if(mesh_)
    {
      // because of bug in Alberta 1.2 , here until bug fixed
      ALBERTA RC_LIST_EL * rclist = ALBERTA get_rc_list(mesh_);
      rclist = 0;
    }
#endif
    if(mesh_) ALBERTA free_mesh(mesh_);

    // delete all created boundary structures
    while ( !bndStack_.empty() )
    {
      ALBERTA BOUNDARY * obj = bndStack_.top();
      //std::cout << "Delete obj with bound = " << (int) obj->bound << "\n";
      bndStack_.pop();
      if( obj ) delete obj;
    }
  }

  // Desctructor
  template < int dim, int dimworld >
  inline AlbertaGrid < dim, dimworld >::~AlbertaGrid()
  {
    removeMesh();
  }

  template < int dim, int dimworld >
  template<int codim, PartitionIteratorType pitype>
  inline typename AlbertaGrid<dim, dimworld>::Traits::template Codim<codim>::template Partition<pitype>::LevelIterator
  //AlbertaGrid < dim, dimworld >::lbegin (int level, int proc) const
  AlbertaGrid < dim, dimworld >::lbegin (int level) const
  {
    assert( level >= 0 );
    // if we dont have this level return empty iterator
    if(level > maxlevel_) return this->template lend<codim,pitype> (level);

    if( codim > 0 ) //(dim == codim) || ((dim == 3) && (codim == 2)) )
    {
      if( ! (vertexMarkerLevel_[level].up2Date() ) )
        vertexMarkerLevel_[level].markNewVertices(*this,level);
    }
    return AlbertaGridLevelIterator<codim,pitype,const MyType> (*this,&vertexMarkerLevel_[level],level,-1);
  }

  template < int dim, int dimworld > template<int codim, PartitionIteratorType pitype>
  inline typename AlbertaGrid<dim, dimworld>::Traits::template Codim<codim>::template Partition<pitype>::LevelIterator
  //AlbertaGrid < dim, dimworld >::lend (int level, int proc ) const
  AlbertaGrid < dim, dimworld >::lend (int level) const
  {
    return AlbertaGridLevelIterator<codim,pitype,const MyType> ((*this),level,-1);
  }

  template < int dim, int dimworld > template<int codim>
  inline typename AlbertaGrid<dim, dimworld>::Traits::template Codim<codim>::template Partition<All_Partition>::LevelIterator
  AlbertaGrid < dim, dimworld >::lbegin (int level) const
  {
    return this->template lbegin<codim,All_Partition> (level);
  }

  template < int dim, int dimworld > template<int codim>
  inline typename AlbertaGrid<dim, dimworld>::Traits::template Codim<codim>::template Partition<All_Partition>::LevelIterator
  AlbertaGrid < dim, dimworld >::lend (int level) const
  {
    return this->template lend<codim,All_Partition> (level);
  }


  template < int dim, int dimworld >
  template<int codim, PartitionIteratorType pitype>
  inline typename AlbertaGrid<dim,dimworld>::Traits::template Codim<codim>::template Partition<pitype>::LeafIterator
  AlbertaGrid < dim, dimworld >::leafbegin (int level, int proc ) const
  {
    if((dim == codim) || ((dim == 3) && (codim == 2)) )
    {
      if( ! (vertexMarkerLeaf_.up2Date()) ) vertexMarkerLeaf_.markNewLeafVertices(*this);
    }
    return AlbertaGridLeafIterator<codim, pitype, const MyType> (*this,&vertexMarkerLeaf_,level,proc);
  }

  template < int dim, int dimworld >
  template<int codim>
  inline typename AlbertaGrid<dim,dimworld>::Traits::template Codim<codim>::LeafIterator
  AlbertaGrid < dim, dimworld >::leafbegin (int level, int proc ) const {
    return leafbegin<codim, All_Partition>(level, proc);
  }


  template < int dim, int dimworld >
  template<int codim, PartitionIteratorType pitype>
  inline typename AlbertaGrid<dim,dimworld>::Traits::template Codim<codim>::template Partition<pitype>::LeafIterator
  AlbertaGrid < dim, dimworld >::leafbegin () const {
    return leafbegin<codim, pitype>(maxlevel_, -1);
  }

  template < int dim, int dimworld >
  template<int codim>
  inline typename AlbertaGrid<dim,dimworld>::Traits::template Codim<codim>::LeafIterator
  AlbertaGrid < dim, dimworld >::leafbegin () const {
    return leafbegin<codim, All_Partition>(maxlevel_, -1);
  }


  template < int dim, int dimworld >
  template<int codim, PartitionIteratorType pitype>
  inline typename AlbertaGrid<dim,dimworld>::Traits::template Codim<codim>::template Partition<pitype>::LeafIterator
  AlbertaGrid < dim, dimworld >::leafend (int level, int proc ) const
  {
    return AlbertaGridLeafIterator<codim, pitype, const MyType> (*this,level,proc);
  }

  template < int dim, int dimworld >
  template<int codim>
  inline typename AlbertaGrid<dim,dimworld>::Traits::template Codim<codim>::LeafIterator
  AlbertaGrid < dim, dimworld >::leafend (int level, int proc ) const {
    return leafend<codim, All_Partition>(level, proc);
  }

  template < int dim, int dimworld >
  template<int codim, PartitionIteratorType pitype>
  inline typename AlbertaGrid<dim,dimworld>::Traits::template Codim<codim>::template Partition<pitype>::LeafIterator
  AlbertaGrid < dim, dimworld >::leafend () const {
    return leafend<codim, pitype>(maxlevel_, -1);
  }

  template < int dim, int dimworld >
  template<int codim>
  inline typename AlbertaGrid<dim,dimworld>::Traits::template Codim<codim>::LeafIterator
  AlbertaGrid < dim, dimworld >::leafend () const {
    return leafend<codim, All_Partition>(maxlevel_, -1);
  }

  template < int dim, int dimworld >
  inline typename AlbertaGrid<dim,dimworld>::LeafIterator
  AlbertaGrid < dim, dimworld >::leafbegin (int level, int proc ) const
  {
    return leafbegin<0, All_Partition> (level,proc);
  }

  template < int dim, int dimworld >
  inline typename AlbertaGrid<dim,dimworld>::LeafIterator
  AlbertaGrid < dim, dimworld >::leafbegin () const {
    return leafbegin<0, All_Partition>(maxlevel_, -1);
  }

  template < int dim, int dimworld >
  inline typename AlbertaGrid<dim,dimworld>::LeafIterator
  AlbertaGrid < dim, dimworld >::leafend (int level, int proc ) const
  {
    return leafend<0,All_Partition> (level,proc);
  }

  template < int dim, int dimworld >
  inline typename AlbertaGrid<dim,dimworld>::LeafIterator
  AlbertaGrid < dim, dimworld >::leafend () const {
    return leafend<0, All_Partition>(maxlevel_, -1);
  }

  //****************************************
  // getNewEntity methods
  //***************************************

  // default implementation used new and delete
  template <class GridImp, class EntityProvider, int dim , int codim >
  struct GetNewEntity
  {
    typedef typename SelectEntityImp<codim,dim,GridImp>::EntityObject EntityObject;
    typedef typename SelectEntityImp<codim,dim,GridImp>::EntityImp EntityImp;
    static EntityObject *
    getNewEntity(GridImp & grid, EntityProvider &enp , int level, bool leafIt )
    {
      return new EntityObject (EntityImp(grid,level,leafIt));
    }

    static void freeEntity (EntityProvider &enp , EntityObject * en)
    {
      if(en) delete en;
    }
  };

  // specialisation for codim 0 uses stack
  template <class GridImp, class EntityProvider, int dim>
  struct GetNewEntity<GridImp,EntityProvider,dim,0>
  {
    typedef typename SelectEntityImp<0,dim,GridImp>::EntityObject EntityObject;
    typedef typename SelectEntityImp<0,dim,GridImp>::EntityImp EntityImp;
    static EntityObject *
    getNewEntity(GridImp & grid, EntityProvider &enp , int level, bool leafIt )
    {
      // return object from stack
      return enp.getNewObjectEntity(grid,(EntityImp *)0,level,leafIt);
    }

    static void freeEntity (EntityProvider &enp , EntityObject * en)
    {
      enp.freeObjectEntity(en);
    }
  };

  template < int dim   , int dimworld >
  template < int codim >
  inline typename SelectEntityImp<codim,dim,const AlbertaGrid<dim,dimworld> >::EntityObject *
  AlbertaGrid < dim, dimworld >::getNewEntity (int level, bool leafIt ) const
  {
    return GetNewEntity<const MyType, EntityProvider, dim, codim > :: getNewEntity(*this,entityProvider_,level,leafIt);
  }

  template < int dim   , int dimworld >
  template < int codim >
  inline void AlbertaGrid < dim, dimworld >::
  freeEntity (typename SelectEntityImp<codim,dim,const MyType>::EntityObject * en) const
  {
    GetNewEntity<const MyType, EntityProvider, dim, codim > :: freeEntity(entityProvider_,en);
  }

  //**************************************
  //  refine and coarsen methods
  //**************************************
  //  --Adaptation
  template < int dim, int dimworld >
  inline bool AlbertaGrid < dim, dimworld >::
  globalRefine(int refCount)
  {
    // only MAXL level allowed
    assert( (refCount + maxlevel_) < MAXL );

    typedef LeafIterator LeafIt;
    LeafIt endit = this->leafend(this->maxLevel());

    assert(refCount >= 0);
    for(int i=0; i<refCount; i++)
    {
      // mark all interior elements
      for(LeafIt it = this->leafbegin(this->maxLevel()); it != endit; ++it)
      {
        this->mark(1,it);
      }

      // mark all ghosts
      for(LeafIt it = leafbegin(this->maxLevel(),Ghost_Partition); it != endit; ++it)
      {
        this->mark(1,it);
      }

      this->adapt();
      this->postAdapt();
    }

    //std::cout << "Grid was global refined !\n";
    return wasChanged_;
  }

  template < int dim, int dimworld >
  inline bool AlbertaGrid < dim, dimworld >::preAdapt()
  {
    return isMarked_;
  }

  template < int dim, int dimworld >
  inline bool AlbertaGrid < dim, dimworld >::postAdapt()
  {
    isMarked_ = false;
    if(leafIndexSet_)
    {
      leafIndexSet_->compress();
      // the number of leaf elements is store in mesh
      // check that they are the same
      assert( mesh_->n_elements == leafIndexSet_->size(0) );
    }

    return wasChanged_;
  }


  template < int dim, int dimworld >
  template <class EntityType>
  inline int AlbertaGrid < dim, dimworld >::owner(const EntityType & en) const
  {
    return this->getOwner( (this->getRealImplementation(en)).getElInfo()->el );
  }

  template < int dim, int dimworld >
  inline int AlbertaGrid < dim, dimworld >::getOwner (const ALBERTA EL *el) const
  {
    // if element is new then entry in dofVec is 1
    return ownerVec_ [el->dof[dof_][nv_]];
  }

  template < int dim, int dimworld >
  inline bool AlbertaGrid < dim, dimworld >::isGhost(const ALBERTA MACRO_EL *mel) const
  {
    assert((mel->index >= 0) && (mel->index < ghostFlag_.size()));
    return ghostFlag_[mel->index] == 1;
  }

  template < int dim, int dimworld >
  inline bool AlbertaGrid < dim, dimworld >::isNoElement(const ALBERTA MACRO_EL *mel) const
  {
    if(myRank() < 0) return false;
    assert((mel->index >= 0) && (mel->index < ghostFlag_.size()));
    return ghostFlag_[mel->index] == -1;
  }

  template < int dim, int dimworld >
  inline void AlbertaGrid < dim, dimworld >::createGhosts()
  {
    assert(myRank_ >= 0);
    if(ghostFlag_.size() < mesh_->n_macro_el) ghostFlag_.resize(mesh_->n_macro_el);
    for(ALBERTA MACRO_EL * mel = mesh_->first_macro_el; mel; mel = mel->next)
    {
      int own = getOwner(mel->el);
      //assert(own >= 0);

      int gh = 0;
      if(own != myRank_)
      {
        gh = -1;
        for(int i=0; i<dim+1; i++)
        {
          ALBERTA MACRO_EL * neigh = mel->neigh[i];
          if(neigh)
          {
            if(getOwner(neigh->el) == myRank_)
            {
              gh = 1;
              //std::cout << "Mark el " << mel->index << " as Ghost\n";
            }
          }
        }
      }
      assert((mel->index >= 0) && (mel->index < ghostFlag_.size()));
      ghostFlag_[mel->index] = gh;
    }

    /*
       std::cout << "CreateGhost of proc " << myRank_ << "\n";
       for(ALBERTA MACRO_EL * mel = mesh_->first_macro_el; mel; mel = mel->next)
       {
       std::cout << "Owner = " << getOwner(mel->el) << " flag = " << ghostFlag_[mel->index] << "\n";
       }
     */

    calcExtras ();
    //std::cout << "ende CreateGhost of proc " << myRank_ << "\n";
  }


  template < int dim, int dimworld >
  inline void AlbertaGrid < dim, dimworld >::
  unpackAll( ObjectStreamType & os )
  {
    //         global index, map to levels , for each level OS
    std:: map < int , ObjectStreamType > elmap;

    int buff;
    int newmxl = 0;
    os.readObject( buff );
    //std::cout << buff << " Read buff \n";
    if(buff == ENDOFSTREAM ) return ;
    else
    {
      assert(buff == BEGINELEMENT );
      while( buff == BEGINELEMENT )
      {
        os.readObject( buff ); // read elnum
        int elnum = buff;
        //std::cout << "Unpack el = " << elnum << "\n";
        os.readObject(buff); // read refine info
        if(buff == BEGINELEMENT ) continue;
        if(buff == ENDOFSTREAM  ) break;
        if(buff == 1) // means that macro element has children
        {
          //std::cout << "Read level info = " << buff << "\n";
          if(elmap.find(elnum) == elmap.end())
          {
            ObjectStreamType elstr;
            elmap[elnum] = elstr;
          }
          ObjectStreamType & elstr = elmap[elnum];

          os.readObject(buff); // read level
          while((buff != ENDOFSTREAM) && (buff != BEGINELEMENT ))
          {
            if(buff < 0) newmxl = std::max( newmxl, std::abs( buff ));
            elstr.writeObject( buff );
            os.readObject( buff );
          }
        }
      }
    }

    const HierarchicIndexSet & hset = this->hierarchicIndexSet();

    std:: map < int , std::map < int , int > > elmap2;
    typedef std :: map < int , int > HierMap ;
    {
      {
        // now refine grid
        typedef typename Traits::template Codim<0>::LevelIterator LevelIteratorType;
        LevelIteratorType endit = this->template lend<0> (0);
        for(LevelIteratorType it = this->template lbegin<0> (0);
            it != endit ; ++it )
        {
          int id = hset.index (*it);
          if(elmap.find(id) != elmap.end())
          {
            std::map < int , int > hiertree;
            elmap2[id] = hiertree;
            if(it->isLeaf()) this->mark(1,(*it));
          }
        }
      }

      this->preAdapt();
      this->adapt();
      this->postAdapt();

      typedef std :: map < int , int > HierMap ;

      for(int l=1; l<=newmxl; l++)
      {
        //std::cout << "Begin on Level l = " << l << "\n";
        // now refine grid
        typedef typename Traits::template Codim<0>::LevelIterator LevelIteratorType;
        LevelIteratorType endit  = this->template lend<0>   (0);
        for(LevelIteratorType it = this->template lbegin<0> (0);
            it != endit ; ++it )
        {
          int id = hset.index (*it);
          //std::cout << "Begin LevelIter it = " << id << "\n";
          if(elmap.find(id) != elmap.end())
          {
            int buff;
            // build a new hier tree
            ObjectStreamType & levstr = elmap[id];
            try {
              levstr.readObject( buff );
            }
            catch (ObjectStreamType :: EOFException)
            {
              continue;
            }
            assert( buff < 0);
            assert( std::abs( buff ) == l );

            HierMap  & hiertree = elmap2[id];
            typedef typename Traits:: template Codim<0> :: Entity :: HierarchicIterator HierIt;

            // Hier muss eine ineinandergeschateltes HierarchiIt kommen.

            typedef typename Traits:: template Codim<0> :: Entity EntityType;
            typedef typename Traits:: template Codim<0> :: EntityPointer EntityPointer;

            hiertree[id] = 1;

            HierIt hendit = it->hend(l);
            for(HierIt hit = it->hbegin(l); hit != hendit; ++hit)
            {
              if(hit->level() != l) continue;
              // if father isnt in tree then we dont do anything here
              EntityPointer vati = hit->father();

              if( hiertree.find( hset.index (*vati) ) == hiertree.end()) continue;

              int mark;
              //try {
              levstr.readObject ( mark );
              //}
              //catch (ObjectStreamType :: EOFException) {}
              if(mark == 1)
              {
                hiertree[ hset.index ( *hit ) ] = mark;
                if(hit->isLeaf()) this->mark(1,(*hit));
              }
            }
            //std::cout << "Hier it done \n";
          }
        }

        //std::cout << "Begin Adapt \n";
        this->preAdapt();
        this->adapt();
        this->postAdapt();
      }
    }


  }

  template < int dim, int dimworld >
  template <class EntityType>
  inline void AlbertaGrid < dim, dimworld >::
  packAll( ObjectStreamType & os, EntityType & en  )
  {
    assert( en.level() == 0 ); // call only on macro elements
    const HierarchicIndexSet & hset = hierarchicIndexSet();

    os.writeObject( BEGINELEMENT );
    os.writeObject( hset.index ( en ) );
    //std::cout << "Pack el = " << hset.index ( en ) << "\n";
    //std::cout << isLeaf() << " children? \n";

    if(! (en.isLeaf()) )
    {
      int mxl = this->maxLevel();
      os.writeObject( 1 ); // this element should be refined

      for(int l=1; l<mxl; l++)
      {
        os.writeObject( -l ); // store level in negative form
        // to distinguish between mark and level
        // walk only over desired level
        typedef typename EntityType :: HierarchicIterator HierIt;
        HierIt endit  = en.hend(l);
        for(HierIt it = en.hbegin(l); it != endit; ++it)
        {
          if(it->level() != l) continue;
          os.writeObject( (it->isLeaf()) ? STOPHERE : REFINEEL );

          // mark element for coarsening
          this->mark( -1, (*it) ) ;
        }
      }
    }
    //std::cout << "Done with element !\n";
    return ;
  }

  template < int dim, int dimworld >
  template <class EntityType>
  inline void AlbertaGrid < dim, dimworld >::
  packBorder ( ObjectStreamType & os, EntityType & en  )
  {
    assert( en.level() == 0 ); // call only on macro elements
    const HierarchicIndexSet & hset = hierarchicIndexSet();

    os.writeObject( BEGINELEMENT );
    os.writeObject( hset.index ( en ) );

    if(! (en.isLeaf()) )
    {
      int mxl = this->maxLevel();
      os.writeObject( 1 ); // this element should be refined

      for(int l=1; l<mxl; l++)
      {
        os.writeObject( -l ); // store level in negative form
        // to distinguish between mark and level
        typedef typename EntityType :: HierarchicIterator HierIt;
        HierIt endit  = en.hend(l);
        for(HierIt it = en.hbegin(l); it != endit; ++it)
        {
          if(it->level() != l) continue;
          if((en.partitionType() != BorderEntity) || (it->isLeaf()))
          {
            os.writeObject( STOPHERE );
          }
          else
            os.writeObject( 1 );
        }
      }
    }
    return ;
  }

  template<int dim, int dimworld>
  inline bool AlbertaGrid < dim, dimworld >::
  mark( int refCount , const typename Traits::template Codim<0>::EntityPointer & ep ) const
  {
    return this->mark(refCount,*ep);
  }

  template <int dim, int dimworld>
  template <class EntityType>
  inline int AlbertaGrid < dim, dimworld >::
  getMark( const EntityType & ep ) const
  {
    return (this->getRealImplementation(ep)).getElInfo()->el->mark;
  }

  //--mark
  template<int dim, int dimworld>
  inline bool AlbertaGrid < dim, dimworld >::
  mark( int refCount , const typename Traits::template Codim<0>::Entity & ep ) const
  {
    ALBERTA EL_INFO * elInfo = (this->getRealImplementation(ep)).getElInfo();
    if(!elInfo) return false;
    assert(elInfo);

    if( ep.isLeaf() )
    {
      // we can not mark for coarsening if already marked for refinement
      if((refCount < 0) && (elInfo->el->mark > 0))
      {
        //dverb << "WARNING:  AlbertaGrid::mark: Could not mark element for coarsening, it was marked for refinement before! in: " << __FILE__ << "  line: " << __LINE__ << "\n";
        return false;
      }

      if( refCount > 0)
      {
        elInfo->el->mark = refCount;
        return true;
      }
      if( refCount < 0)
      {
        this->setMark ( true );
        elInfo->el->mark = refCount;
        return true;
      }
    }
    //dwarn << "WARNING: in AlbertaGrid<"<<dim<<","<<dimworld<<">::mark("<<refCount<<",EP &) : called on non LeafEntity! in: " << __FILE__ << " line: "<< __LINE__ << "\n";
    elInfo->el->mark = 0;
    return false;
  }

  // --adapt
  template < int dim, int dimworld >
  inline bool AlbertaGrid < dim, dimworld >::adapt()
  {
    unsigned char flag;
    bool refined = false;
    wasChanged_ = false;

    // set global pointer to index manager in elmem.cc
    ALBERTA AlbertHelp::initIndexManager_elmem_cc( indexStack_ );

    // set all values of elNewCheck positive which means old
    ALBERTA AlbertHelp::set2positive ( dofvecs_.elNewCheck );

    flag = ALBERTA AlbertRefine ( mesh_ );
    refined = (flag == 0) ? false : true;

    if(isMarked_) // true if a least on element is marked for coarseing
      flag = ALBERTA AlbertCoarsen( mesh_ );

    if(!refined)
    {
      wasChanged_ = (flag == 0) ? false : true;
    }
    else
      wasChanged_ = true;

    if(wasChanged_)
    {
      calcExtras();
      isMarked_ = false;
    }

    ALBERTA AlbertHelp::setElOwnerNew(mesh_, dofvecs_.owner);

    // remove global pointer in elmem.cc
    ALBERTA AlbertHelp::removeIndexManager_elmem_cc(AlbertHelp::numOfElNumVec);

    return refined;
  }

  template < int dim, int dimworld >
  template <class DofManagerType, class RestrictProlongOperatorType>
  inline bool AlbertaGrid < dim, dimworld >::
  adapt(DofManagerType &, RestrictProlongOperatorType &, bool verbose)
  {
    bool refined = false;
    wasChanged_ = false;

    std::cerr << "Method adapt 2 not implemented! in: " << __FILE__ << " line: " << __LINE__ << "\n";
    abort();
    return refined;
  }

  template < int dim, int dimworld >
  inline void AlbertaGrid < dim, dimworld >::setMark (bool isMarked) const
  {
    isMarked_ = isMarked;
  }

  template < int dim, int dimworld >
  inline bool AlbertaGrid < dim, dimworld >::checkElNew (const ALBERTA EL *el) const
  {
    // if element is new then entry in dofVec is 1
    return (elNewVec_[el->dof[dof_][nv_]] < 0);
  }

  template < int dim, int dimworld >
  template <class EntityType>
  inline bool AlbertaGrid < dim, dimworld >::
  partition( int proc , EntityType & en )
  {
    return this->setOwner( (this->getRealImplementation(en)).getElInfo()->el , proc );
  }

  template < int dim, int dimworld >
  inline bool AlbertaGrid < dim, dimworld >::setOwner (const ALBERTA EL *el, int proc)
  {
    // if element is new then entry in dofVec is 1
    int dof = el->dof[dof_][nv_];
    if(ownerVec_ [dof] < 0)
    {
      ownerVec_ [dof] = proc;
      return true;
    }
    return false;
  }

  template < int dim, int dimworld >
  inline PartitionType AlbertaGrid < dim, dimworld >::
  partitionType (ALBERTA EL_INFO *elinfo) const
  {
    int owner = getOwner(elinfo->el);

    // if processor number == myRank ==> InteriorEntity or BorderEntity
    if(owner == myRank())
    {
      for(int i=0; i<dim+1; i++)
      {
        const ALBERTA EL * neigh = NEIGH(elinfo->el,elinfo)[i];
        if(neigh)
        {
          if(getOwner(neigh) != myRank())
            return BorderEntity;
        }
      }

      // if no GhostNeighbor, then we have real InteriorEntity
      return InteriorEntity;
    }

    // if processor number != myProcossor ==> GhostEntity
    if((owner >= 0) && (owner != myRank())) return GhostEntity;

    DUNE_THROW(AlbertaError, "Unsupported PartitionType");

    return OverlapEntity;
  }

  template < int dim, int dimworld >
  inline int AlbertaGrid < dim, dimworld >::maxLevel() const
  {
    return maxlevel_;
  }

  template < int dim, int dimworld >
  inline int AlbertaGrid < dim, dimworld >::global_size (int codim) const
  {
    if(codim == dim) return mesh_->n_vertices;
    // for higher codims we have the index stack
    return indexStack_[codim].size();
  }

  // --size
  template < int dim, int dimworld >
  inline int AlbertaGrid < dim, dimworld >::size (int level, int codim) const
  {
    if( (level > maxlevel_) || (level < 0) ) return 0;
    assert( this->levelIndexSet(level).size(GeometryType(GeometryType::simplex,dim-codim) )
            == sizeCache_->size(level,codim) );
    assert( sizeCache_ );
    return sizeCache_->size(level,codim);
  }


  template < int dim, int dimworld >
  inline int AlbertaGrid < dim, dimworld >::size (int level, GeometryType type) const
  {
    return ((type.isSimplex()) ? this->size(level,dim-type.dim()) : 0);
  }

  template < int dim, int dimworld >
  inline int AlbertaGrid < dim, dimworld >::size (GeometryType type) const
  {
    return ((type.isSimplex()) ? this->size(dim-type.dim()) : 0);
  }

  template < int dim, int dimworld >
  inline int AlbertaGrid < dim, dimworld >::size (int codim) const
  {
    assert( this->leafIndexSet().size(GeometryType(GeometryType::simplex,dim-codim) )
            == sizeCache_->size(codim) );
    assert( sizeCache_ );
    return sizeCache_->size(codim);
  }

  template < int dim, int dimworld >
  inline const typename AlbertaGrid < dim, dimworld > :: Traits :: LevelIndexSet &
  AlbertaGrid < dim, dimworld > :: levelIndexSet (int level) const
  {
    // assert that given level is in range
    assert( level >= 0 );
    assert( level < (int) levelIndexVec_.size() );

    if(!levelIndexVec_[level]) levelIndexVec_[level] = new LevelIndexSetImp (*this,level);
    return *(levelIndexVec_[level]);
  }

  template < int dim, int dimworld >
  inline const typename AlbertaGrid < dim, dimworld > :: Traits :: LeafIndexSet &
  AlbertaGrid < dim, dimworld > :: leafIndexSet () const
  {
    if(!leafIndexSet_) leafIndexSet_ = new LeafIndexSet (*this);
    return *leafIndexSet_;
  }


  template < int dim, int dimworld >
  inline void AlbertaGrid < dim, dimworld >::arrangeDofVec()
  {
    hIndexSet_.updatePointers(dofvecs_);

    elNewVec_ = (dofvecs_.elNewCheck)->vec;  assert(elNewVec_);
    ownerVec_ = (dofvecs_.owner)->vec;       assert(ownerVec_);
    elAdmin_ = dofvecs_.elNumbers[0]->fe_space->admin;

    // see Albert Doc. , should stay the same
    const_cast<int &> (nv_)  = elAdmin_->n0_dof[CENTER];
    const_cast<int &> (dof_) = elAdmin_->mesh->node[CENTER];
  }


  template < int dim, int dimworld >
  inline int AlbertaGrid < dim, dimworld >::getLevelOfElement (const ALBERTA EL *el) const
  {
    assert( el );
    // return the elements level which is the absolute value of the entry
    return std::abs( elNewVec_ [el->dof[dof_][nv_]] );
  }

  template < int dim, int dimworld >
  inline int AlbertaGrid < dim, dimworld >::getElementNumber ( const ALBERTA EL * el ) const
  {
    return hIndexSet_.getIndex(el,0,Int2Type<dim>());
  };

  template < int dim, int dimworld >
  inline int AlbertaGrid < dim, dimworld >::getFaceNumber ( const ALBERTA EL * el , int face ) const
  {
    // codim of faces is 2 therefore dim-1
    assert( face >= 0 );
    assert( face < dim+1 );
    return hIndexSet_.getIndex(el,face,Int2Type<dim-1>());
  };

  template < int dim, int dimworld >
  inline int AlbertaGrid < dim, dimworld >::getEdgeNumber ( const ALBERTA EL * el , int edge ) const
  {
    assert(dim == 3);
    // codim of edges is 2 therefore dim-2
    return hIndexSet_.getIndex(el,edge,Int2Type<dim-2>());
  };

  template < int dim, int dimworld >
  inline int AlbertaGrid < dim, dimworld >::getVertexNumber ( const ALBERTA EL * el , int vx ) const
  {
    return hIndexSet_.getIndex(el,vx,Int2Type<0>());
  };

  template < int dim, int dimworld >
  inline void AlbertaGrid < dim, dimworld >::calcExtras ()
  {
    // store pointer to numbering vectors and so on
    arrangeDofVec ();

    // determine new maxlevel
    maxlevel_ = ALBERTA AlbertHelp::calcMaxAbsoluteValueOfVector( dofvecs_.elNewCheck );
    assert( maxlevel_ >= 0);
    assert( maxlevel_ < MAXL);

#ifndef NDEBUG
    int mlvl = ALBERTA AlbertHelp::calcMaxLevel(mesh_,dofvecs_.elNewCheck);
    assert( mlvl == maxlevel_ );
#endif

    // unset up2Dat status, if lbegin is called then this status is updated
    for(int l=0; l<MAXL; l++) vertexMarkerLevel_[l].unsetUp2Date();

    // unset up2Dat status, if leafbegin is called then this status is updated
    vertexMarkerLeaf_.unsetUp2Date();

    // if levelIndexSet exists, then update now
    for(unsigned int i=0; i<levelIndexVec_.size(); i++)
      if(levelIndexVec_[i]) (*levelIndexVec_[i]).calcNewIndex();

    if( leafIndexSet_ ) (*leafIndexSet_).resize();

    // this is done in postAdapt
    //if( leafIndexSet_ ) (*leafIndexSet_).compress();

    if(sizeCache_) delete sizeCache_;
    // first bool says we have simplex, second not cube, third, worryabout
    sizeCache_ = new SizeCacheType (*this,true,false,true);

    // we have a new grid
    wasChanged_ = true;
  }

  template < int dim, int dimworld >  template <GrapeIOFileFormatType ftype>
  inline bool AlbertaGrid < dim, dimworld >::
  writeGrid (const std::basic_string<char> filename, albertCtype time ) const
  {
    // use only with xdr as filetype
    assert(ftype == xdr);
    return writeGridXdr (filename , time );
  }

  template < int dim, int dimworld >  template <GrapeIOFileFormatType ftype>
  inline bool AlbertaGrid < dim, dimworld >::
  readGrid (const std::basic_string<char> filename, albertCtype &time )
  {
    switch(ftype)
    {
    case xdr   : return readGridXdr   (filename , time );
    case ascii : return readGridAscii (filename , time );
    default : {
      DUNE_THROW(AlbertaError,"wrong FileType in AlbertaGrid::readGrid!");
      return false;
    }
    }
  }

  template < int dim, int dimworld >
  inline bool AlbertaGrid < dim, dimworld >::
  writeGridXdr (const std::basic_string<char> filename, albertCtype time ) const
  {
    std::string ownerfile(filename);
    if(filename.size() > 0)
    {
      ownerfile += "_own";
    }
    else
      DUNE_THROW(AlbertaIOError, "no filename given in writeGridXdr ");

    // strore element numbering to file
    for(int i=0; i<AlbertHelp::numOfElNumVec; i++)
    {
      std::string elnumfile(filename);
      elnumfile += "_num_c";
      char tmpchar[16]; sprintf(tmpchar,"%d",i);
      elnumfile += tmpchar;
      ALBERTA write_dof_int_vec_xdr(dofvecs_.elNumbers[i],elnumfile.c_str());
    }

    if(myRank() >= 0)
    {
      int val = -1;
      int entry = ALBERTA AlbertHelp::saveMyProcNum(dofvecs_.owner,myRank(),val);

      ALBERTA write_dof_int_vec_xdr(dofvecs_.owner,ownerfile.c_str());

      // set old value of owner vec
      dofvecs_.owner->vec[entry] = val;
    }

    const char * fn = filename.c_str();
    int flag = ALBERTA write_mesh_xdr (mesh_ , fn , time);
    return (flag == 1) ? true : false;
  }

  template < int dim, int dimworld >
  inline bool AlbertaGrid < dim, dimworld >::
  readGridXdr (const std::basic_string<char> filename, albertCtype & time )
  {
    const char * fn = filename.c_str();

    ALBERTA AlbertHelp :: initBndStack( &bndStack_ );
    mesh_ = (ALBERTA read_mesh_xdr (fn , &time ,
                                    LeafDataType::initLeafData,
                                    ALBERTA AlbertHelp::initBoundary) );
    ALBERTA AlbertHelp :: removeBndStack ();

    if (mesh_ == 0)
      DUNE_THROW(AlbertaIOError, "could not open grid file " << filename);

    // read element numbering from file
    std::string ownerfile (filename);
    if(filename.size() > 0)
    {
      ownerfile += "_own";
    }
    else
      return false;

    for(int i=0; i<AlbertHelp::numOfElNumVec; i++)
    {
      std::string elnumfile( filename );
      char tmpchar[16]; sprintf(tmpchar,"%d",i);
      elnumfile += "_num_c"; elnumfile += tmpchar;
      const char * elnumfn = elnumfile.c_str();
      dofvecs_.elNumbers[i] = ALBERTA read_dof_int_vec_xdr(elnumfn, mesh_ , 0 );
    }

    // if owner file exists, read it
    {
      //dofvecs_.owner = 0;

      FILE * file=0;
      const char * ownfile = ownerfile.c_str();

      file = fopen(ownfile,"r");
      if(file)
      {
        fclose(file);
        dofvecs_.owner = ALBERTA read_dof_int_vec_xdr(ownfile, mesh_ , 0 );
        const_cast<int &> (myRank_) = ALBERTA AlbertHelp::restoreMyProcNum(dofvecs_.owner);
      }
    }

    // make the rest of the dofvecs
    ALBERTA AlbertHelp::makeTheRest(&dofvecs_);

    // restore level information for each element by traversing the mesh
    ALBERTA AlbertHelp::restoreElNewCheck( mesh_ , dofvecs_.elNewCheck );

    arrangeDofVec();

    // calc maxlevel and indexOnLevel and so on
    calcExtras();

    // set el_index of index manager to max element index
    for(int i=0; i<ALBERTA AlbertHelp::numOfElNumVec; i++)
    {
      int maxIdx = ALBERTA AlbertHelp::calcMaxIndex( dofvecs_.elNumbers[i] );
      indexStack_[i].setMaxIndex(maxIdx);
    }

    leafIndexSet();

    return true;
  }

  template < int dim, int dimworld >
  inline bool AlbertaGrid < dim, dimworld >::readGridAscii
    (const std::basic_string<char> filename, albertCtype & time )
  {
    removeMesh(); // delete all objects

    mesh_ = ALBERTA get_mesh("AlbertaGrid",
                             ALBERTA AlbertHelp::initDofAdmin<dim>,
                             LeafDataType::initLeafData
                             );

    ALBERTA AlbertHelp :: initBndStack( &bndStack_ );
    ALBERTA read_macro(mesh_, filename.c_str(), ALBERTA AlbertHelp::initBoundary);
    ALBERTA AlbertHelp :: removeBndStack ();

    if( !readParameter(filename,"Time",time) )
      time = 0.0;

    // unset up2Dat status, if lbegin is called then this status is updated
    for(int l=0; l<MAXL; l++) vertexMarkerLevel_[l].unsetUp2Date();

    // unset up2Dat status, if leafbegin is called then this status is updated
    vertexMarkerLeaf_.unsetUp2Date();

    ALBERTA AlbertHelp::initIndexManager_elmem_cc(indexStack_);

    initGrid(myRank_);
    return true;
  }

  // if defined some debugging test were made that reduce the performance
  // so they were switch off normaly

  //#define DEBUG_FILLELINFO
  //*********************************************************************
  //  fillElInfo 2D
  //*********************************************************************
#define CALC_COORD
  typedef U_CHAR ALBERTA_CHAR;

  template<int dim, int dimworld>
  inline void AlbertaGrid<dim,dimworld >::
  firstNeigh(const int ichild, const ALBERTA EL_INFO *elinfo_old,
             ALBERTA EL_INFO *elinfo, const bool leafLevel) const
  {
    // old stuff
    const ALBERTA REAL_D * old_opp_coord  = elinfo_old->opp_coord;
    const ALBERTA REAL_D * old_coord      = elinfo_old->coord;

    // new stuff
    ALBERTA ALBERTA_CHAR * opp_vertex     = elinfo->opp_vertex;
    ALBERTA EL ** neigh = NEIGH(el,elinfo);
    ALBERTA REAL_D * opp_coord = elinfo->opp_coord;

    assert(neigh != 0);
    assert(neigh == NEIGH(el,elinfo));

    const int onechi = 1-ichild;

    // first nb 0 of new elinfo
    {
      ALBERTA EL * nb = NEIGH(elinfo_old->el,elinfo_old)[2];
      if(nb)
      {
        // if NULL then nonconforme refinement
        assert(nb->child[0] != 0);
        ALBERTA EL * nextNb = nb->child[onechi];
        opp_vertex[ichild] = onechi;
#ifdef CALC_COORD
        for(int i=0; i<dimworld; i++)
          opp_coord[ichild][i]  = old_opp_coord[2][i];
#endif
        // if we have children, we could go down
        if(nextNb->child[0])
        {
          // if level of neighbour to small, do down once more
          // but only one level down because of conformity
          if( leafLevel )
          {
            nextNb = nextNb->child[ichild];
            opp_vertex[ichild] = 2;
#ifdef CALC_COORD
            for(int i=0; i<dimworld; i++)
              opp_coord[ichild][i] += old_coord[ichild][i];
            for(int i=0; i<dimworld; i++)
              opp_coord[ichild][i] *= 0.5;
#endif
          }
        }
        neigh[ichild] = nextNb;
      }
      else
      {
        // if no neighbour then children have no neighbour
        neigh[ichild] = 0;
      }
    }
  }

  template<int dim, int dimworld>
  inline void AlbertaGrid<dim,dimworld >::
  secondNeigh(const int ichild, const ALBERTA EL_INFO *elinfo_old,
              ALBERTA EL_INFO *elinfo, const bool leafLevel) const
  {
    // old stuff
    const ALBERTA REAL_D * old_coord      = elinfo_old->coord;

    // new stuff
    ALBERTA ALBERTA_CHAR * opp_vertex     = elinfo->opp_vertex;
    ALBERTA EL ** neigh = NEIGH(el,elinfo);
    ALBERTA REAL_D * opp_coord = elinfo->opp_coord;

    assert(neigh != 0);
    assert(neigh == NEIGH(el,elinfo));

    const int onechi = 1-ichild;
    // nb 1 of new elinfo, always the same
    {
      ALBERTA EL * nextNb = elinfo_old->el->child[onechi];
      opp_vertex[onechi] = ichild;
#ifdef CALC_COORD
      for(int i=0; i<dimworld; i++)
        opp_coord[onechi][i]  = old_coord[onechi][i];
#endif
      // check if children exists
      if(nextNb->child[0] )
      {
        if( leafLevel )
        {
          nextNb = nextNb->child[onechi];
          opp_vertex[onechi] = 2;
#ifdef CALC_COORD
          for(int i=0; i<dimworld; i++)
            opp_coord[onechi][i] += old_coord[2][i];
          for(int i=0; i<dimworld; i++)
            opp_coord[onechi][i] *= 0.5;
#endif
        }
      }
      neigh[onechi] = nextNb;
    }
  }

  template<int dim, int dimworld>
  inline void AlbertaGrid<dim,dimworld >::
  thirdNeigh(const int ichild, const ALBERTA EL_INFO *elinfo_old,
             ALBERTA EL_INFO *elinfo, const bool leafLevel) const
  {
    // old stuff
    const ALBERTA ALBERTA_CHAR * old_opp_vertex = elinfo_old->opp_vertex;
    const ALBERTA REAL_D * old_opp_coord  = elinfo_old->opp_coord;
    const ALBERTA REAL_D * old_coord      = elinfo_old->coord;

    // new stuff
    ALBERTA ALBERTA_CHAR * opp_vertex     = elinfo->opp_vertex;
    ALBERTA EL ** neigh = NEIGH(el,elinfo);
    ALBERTA REAL_D * opp_coord = elinfo->opp_coord;

    assert(neigh != 0);
    assert(neigh == NEIGH(el,elinfo));

    const int onechi = 1-ichild;
    // nb 2 of new elinfo
    {
      ALBERTA EL * nb = NEIGH(elinfo_old->el,elinfo_old)[onechi];
      if(nb)
      {
        const int vx = old_opp_vertex[onechi];
        opp_vertex[2] = vx;
#ifdef CALC_COORD
        for(int i=0; i<dimworld; i++)
          opp_coord[2][i] = old_opp_coord[onechi][i];
#endif
        if((vx == 2) || (nb->child[0] == 0))
        {
          // means the neighbour has the same refinement edge like our child
          neigh[2] = nb;
        }
        else
        {
          assert(nb->child[0] != 0);
          neigh[2] = nb->child[1-vx];
          opp_vertex[2] = 2;
#ifdef CALC_COORD
          const int vxind = (vx == ichild) ? ichild : 2;
          for(int i=0; i<dimworld; i++)
            opp_coord[2][i] += old_coord[vxind][i];
          for(int i=0; i<dimworld; i++)
            opp_coord[2][i] *= 0.5;
#endif
        }
      }
      else
      {
        // if no neighbour then children have no neighbour
        neigh[2] = 0;
      }
    }

  }

  template<int dim, int dimworld>
  inline void AlbertaGrid<dim,dimworld >::
  fillElInfo(int ichild, int actLevel , const ALBERTA EL_INFO *elinfo_old,
             ALBERTA EL_INFO *elinfo, bool hierarchical, bool leaf) const
  {
#if 0
    // the alberta version of filling an EL_INFO structure
    ALBERTA fill_elinfo(ichild,elinfo_old,elinfo);
#else

    // old stuff
    ALBERTA EL * el_old = elinfo_old->el;
    assert(el_old != 0);
    assert(el_old->child[0] != 0);

    // new stuff
    // set new element
    ALBERTA EL * el  = el_old->child[ichild];
    elinfo->el = el;

    ALBERTA FLAGS fill_flag = elinfo_old->fill_flag;

    elinfo->macro_el  = elinfo_old->macro_el;
    elinfo->fill_flag = fill_flag;
    elinfo->mesh      = elinfo_old->mesh;
    elinfo->parent    = el_old;
    elinfo->level     = elinfo_old->level + 1;

    // calculate the coordinates
    if (fill_flag & FILL_COORDS)
    {
      if (el_old->new_coord)
      {
        for (int j = 0; j < dimworld; j++)
          elinfo->coord[2][j] = el_old->new_coord[j];
      }
      else
      {
        for (int j = 0; j < dimworld; j++)
          elinfo->coord[2][j] =
            0.5 * (elinfo_old->coord[0][j] + elinfo_old->coord[1][j]);
      }

      // set the other coord
      for (int j = 0; j < dimworld; j++)
      {
        elinfo->coord[ichild  ][j] = elinfo_old->coord[2][j];
        elinfo->coord[1-ichild][j] = elinfo_old->coord[ichild][j];
      }
    }

    // calculate neighbours
    if(fill_flag & FILL_NEIGH)
    {
      // allow to go down on neighbour more than once
      // if the following condition is satisfied
      //const bool leafLevel = ((el->child[0] == 0) || (elinfo->level < actLevel));
      const bool leafLevel = (leaf) ? true : ((el->child[0] == 0) && (elinfo->level < actLevel));

      firstNeigh (ichild,elinfo_old,elinfo,leafLevel);
      secondNeigh(ichild,elinfo_old,elinfo,leafLevel);
      thirdNeigh (ichild,elinfo_old,elinfo,leafLevel);
    }

    // boundary calculation
    if (fill_flag & FILL_BOUND)
    {
      if (elinfo_old->boundary[2])
        elinfo->bound[2] = elinfo_old->boundary[2]->bound;
      else
        elinfo->bound[2] = INTERIOR;

      elinfo->bound[ichild]   = elinfo_old->bound[2];
      elinfo->bound[1-ichild] = elinfo_old->bound[ichild];
      elinfo->boundary[ichild] = elinfo_old->boundary[2];
      elinfo->boundary[1-ichild] = nil;
      elinfo->boundary[2] = elinfo_old->boundary[1-ichild];
    }

#endif
  } // end Grid::fillElInfo 2D


  //***********************************************************************
  // fillElInfo 3D
  //***********************************************************************
#if DIM == 3
  template <>
  inline void AlbertaGrid<3,3>::
  fillElInfo(int ichild, int actLevel , const ALBERTA EL_INFO *elinfo_old,
             ALBERTA EL_INFO *elinfo, bool hierarchical, bool leaf) const
  {
    enum { dim = 3 };
    enum { dimworld = 3 };

#if 0
    ALBERTA fill_elinfo(ichild,elinfo_old,elinfo);
#else
    static S_CHAR child_orientation[3][2] = {{1,1}, {1,-1}, {1,-1}};

    int k;
    int el_type=0;                                   /* el_type in {0,1,2} */
    int ochild=0;                       /* index of other child = 1-ichild */
    int     *cv=nil;                   /* cv = child_vertex[el_type][ichild] */
    int     (*cvg)[4]=nil;                    /* cvg = child_vertex[el_type] */
    int     *ce;                     /* ce = child_edge[el_type][ichild] */
    int iedge;
    EL      *nb, *nbk, **neigh_old;
    EL      *el_old = elinfo_old->el;
    FLAGS fill_flag = elinfo_old->fill_flag;
    DOF    *dof;
#if !NEIGH_IN_EL
    int ov;
    EL      **neigh;
    FLAGS fill_opp_coords;
    U_CHAR  *opp_vertex;
#endif

    TEST_EXIT(el_old->child[0]) ("missing child?\n"); /* Kuni 22.08.96 */

    elinfo->el        = el_old->child[ichild];
    elinfo->macro_el  = elinfo_old->macro_el;
    elinfo->fill_flag = fill_flag;
    elinfo->mesh      = elinfo_old->mesh;
    elinfo->parent    = el_old;
    elinfo->level     = elinfo_old->level + 1;
#if !NEIGH_IN_EL
    elinfo->el_type   = (elinfo_old->el_type + 1) % 3;
#endif

    REAL_D * opp_coord = elinfo->opp_coord;
    REAL_D * coord = elinfo->coord;

    const REAL_D * old_coord = elinfo_old->coord;
    const REAL_D * oldopp_coord = elinfo_old->opp_coord;


    TEST_EXIT(elinfo->el) ("missing child %d?\n", ichild);

    if (fill_flag) {
      el_type = EL_TYPE(elinfo_old->el, elinfo_old);
      cvg = child_vertex[el_type];
      cv = cvg[ichild];
      ochild = 1-ichild;
    }

    if (fill_flag & FILL_COORDS)
    {
      for (int i=0; i<3; i++) {
        for (int j = 0; j < dimworld; j++) {
          coord[i][j] = old_coord[cv[i]][j];
        }
      }
      if (el_old->new_coord)
        for (int j = 0; j < dimworld; j++)
          coord[3][j] = el_old->new_coord[j];
      else
        for (int j = 0; j < dimworld; j++)
          coord[3][j] = 0.5*
                        (old_coord[0][j] + old_coord[1][j]);
    }


#if NEIGH_IN_EL
    if (fill_flag & FILL_OPP_COORDS)
    {
      neigh_old = el_old->neigh;

      /*----- nb[0] is other child -------------------------------------------*/

      /*    if (nb = el_old->child[ochild]) {        old version    */
      if (el_old->child[0]  &&  (nb = el_old->child[ochild])) /* Kuni 22.08.96*/
      {
        if (nb->child[0]) {   /* go down one level for direct neighbour */
          k = cvg[ochild][1];
          if (nb->new_coord)
            for (int j = 0; j < dimworld; j++)
              opp_coord[0][j] = nb->new_coord[j];
          else
            for (int j = 0; j < dimworld; j++)
              opp_coord[0][j] = 0.5*
                                (old_coord[ochild][j] + old_coord[k][j]);
        }
        else {
          for (int j = 0; j < dimworld; j++) {
            opp_coord[0][j] = old_coord[ochild][j];
          }
        }
      }
      else {
        ERROR_EXIT("no other child");
      }


      /*----- nb[1],nb[2] are childs of old neighbours nb_old[cv[i]] ----------*/

      for (int i=1; i<3; i++)
      {
        if (nb = neigh_old[cv[i]])
        {
          TEST_EXIT(nb->child[0]) ("nonconforming triangulation\n");

          for (k=0; k<2; k++) { /* look at both children of old neighbour */

            nbk = nb->child[k];
            if (nbk->dof[0] == el_old->dof[ichild]) {
              dof = nb->dof[el_old->opp_vertex[cv[i]]]; /* opp. vertex */
              if (dof == nbk->dof[1]) {
                if (nbk->child[0]) {
                  if (nbk->new_coord)
                    for (int j = 0; j < dimworld; j++)
                      opp_coord[i][j] = nbk->new_coord[j];
                  else
                    for (int j = 0; j < dimworld; j++)
                      opp_coord[i][j] = 0.5*
                                        (oldopp_coord[cv[i]][j] + old_coord[ichild][j]);
                  break;
                }
              }
              else {
                TEST_EXIT(dof == nbk->dof[2]) ("opp_vertex not found\n");
              }

              for (int j = 0; j < dimworld; j++) {
                opp_coord[i][j] = oldopp_coord[cv[i]][j];
              }
              break;
            }

          } /* end for k */
          TEST_EXIT(k<2) ("child not found with vertex\n");

        }
      } /* end for i */


      /*----- nb[3] is old neighbour neigh_old[ochild] ------------------------*/

      if (neigh_old[ochild]) {
        for (int j = 0; j < dimworld; j++) {
          opp_coord[3][j] = oldopp_coord[ochild][j];
        }
      }

    }

#else  /* ! NEIGH_IN_EL */

    if (fill_flag & (FILL_NEIGH | FILL_OPP_COORDS))
    {
      neigh      = elinfo->neigh;
      neigh_old  = (EL **) elinfo_old->neigh;
      opp_vertex = (U_CHAR *) &(elinfo->opp_vertex);
      fill_opp_coords = (fill_flag & FILL_OPP_COORDS);

      /*----- nb[0] is other child --------------------------------------------*/

      /*    if (nb = el_old->child[ochild])   old version  */
      if (el_old->child[0]  &&  (nb = el_old->child[ochild])) /*Kuni 22.08.96*/
      {
        if (nb->child[0])
        {   /* go down one level for direct neighbour */
          if (fill_opp_coords)
          {
            if (nb->new_coord)
            {
              for (int j = 0; j < dimworld; j++)
                opp_coord[0][j] = nb->new_coord[j];
            }
            else
            {
              k = cvg[ochild][1];
              for (int j = 0; j < dimworld; j++)
                opp_coord[0][j] = 0.5*
                                  (old_coord[ochild][j] + old_coord[k][j]);
            }
          }
          neigh[0]      = nb->child[1];
          opp_vertex[0] = 3;
        }
        else {
          if (fill_opp_coords) {
            for (int j = 0; j < dimworld; j++) {
              opp_coord[0][j] = old_coord[ochild][j];
            }
          }
          neigh[0]      = nb;
          opp_vertex[0] = 0;
        }
      }
      else {
        ERROR_EXIT("no other child");
        neigh[0] = nil;
      }


      /*----- nb[1],nb[2] are childs of old neighbours nb_old[cv[i]] ----------*/

      for (int i=1; i<3; i++)
      {
        if ((nb = neigh_old[cv[i]]))
        {
          TEST_EXIT(nb->child[0]) ("nonconforming triangulation\n");

          for (k=0; k<2; k++) /* look at both childs of old neighbour */
          {
            nbk = nb->child[k];
            if (nbk->dof[0] == el_old->dof[ichild]) {
              dof = nb->dof[elinfo_old->opp_vertex[cv[i]]]; /* opp. vertex */
              if (dof == nbk->dof[1])
              {
                ov = 1;
                if (nbk->child[0])
                {
                  if (fill_opp_coords)
                  {
                    if (nbk->new_coord)
                      for (int j = 0; j < dimworld; j++)
                        opp_coord[i][j] = nbk->new_coord[j];
                    else
                      for (int j = 0; j < dimworld; j++)
                        opp_coord[i][j] = 0.5*
                                          (oldopp_coord[cv[i]][j]
                                           + old_coord[ichild][j]);
                  }
                  neigh[i]      = nbk->child[0];
                  opp_vertex[i] = 3;
                  break;
                }
              }
              else
              {
                TEST_EXIT(dof == nbk->dof[2]) ("opp_vertex not found\n");
                ov = 2;
              }

              if (fill_opp_coords)
              {
                for (int j = 0; j < dimworld; j++)
                {
                  opp_coord[i][j] = oldopp_coord[cv[i]][j];
                }
              }
              neigh[i]      = nbk;
              opp_vertex[i] = ov;
              break;
            }

          } /* end for k */
          TEST_EXIT(k<2) ("child not found with vertex\n");
        }
        else
        {
          neigh[i] = 0;
        }
      } /* end for i */


      /*----- nb[3] is old neighbour neigh_old[ochild] ------------------------*/

      if ((neigh[3] = neigh_old[ochild]))
      {
        opp_vertex[3] = elinfo_old->opp_vertex[ochild];
        if (fill_opp_coords) {
          for (int j = 0; j < dimworld; j++) {
            opp_coord[3][j] = oldopp_coord[ochild][j];
          }
        }
      }
    }
#endif

    if (fill_flag & FILL_BOUND)
    {
      for (int i = 0; i < 3; i++)
      {
        elinfo->bound[i] = elinfo_old->bound[cv[i]];
      }
      elinfo->bound[3] = GET_BOUND(elinfo_old->boundary[N_FACES+0]);

      elinfo->boundary[0] = nil;
      elinfo->boundary[1] = elinfo_old->boundary[cv[1]];
      elinfo->boundary[2] = elinfo_old->boundary[cv[2]];
      elinfo->boundary[3] = elinfo_old->boundary[ochild];

      ce = child_edge[el_type][ichild];
      for (iedge=0; iedge<4; iedge++) {
        elinfo->boundary[N_FACES+iedge] =
          elinfo_old->boundary[N_FACES+ce[iedge]];
      }
      for (iedge=4; iedge<6; iedge++) {
        int i = 5 - cv[iedge-3];              /* old vertex opposite new edge */
        elinfo->boundary[N_FACES+iedge] = elinfo_old->boundary[i];
      }
    }


    if (elinfo->fill_flag & FILL_ORIENTATION) {
      elinfo->orientation =
        elinfo_old->orientation * child_orientation[el_type][ichild];
    }
#endif
  }
#endif

  template < int dim, int dimworld >
  inline void AlbertaGrid < dim, dimworld >::setNewCoords
    (const FieldVector<albertCtype, dimworld> & trans, const albertCtype scalar)
  {
    static FieldVector<albertCtype, dimworld> trans_(0.0);
    static albertCtype scalar_ (1.0);

    for(int i=0; i<macroVertices_.size(); i++)
      macroVertices_[i] = 0;

    for(ALBERTA MACRO_EL * mel = mesh_->first_macro_el; mel; mel=mel->next)
    {
      for(int i=0; i<dim+1; i++)
      {
        int dof = mel->el->dof[i][0];
        // visit each coord only once
        if( macroVertices_[dof] != 1)
        {
          macroVertices_[dof] = 1;
          for(int j=0; j<dimworld; j++)
          {
            mel->coord[i][j] -= trans_[j];
            mel->coord[i][j] /= scalar_;

            mel->coord[i][j] *= scalar;
            mel->coord[i][j] += trans[j];
          }
        }
      }
    }

    for (int i=0; i<dimworld; i++)
      trans_[i] = trans[i];

    scalar_ = scalar;
  }

} // namespace Dune

#undef CALC_COORD
#undef ALBERTA_CHAR
#endif
