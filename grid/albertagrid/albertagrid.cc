// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
//************************************************************************
//
//  implementation of AlbertaGrid
//
//  namespace Dune
//
//************************************************************************

#include <algorithm>
#include <dune/io/file/grapedataio.hh>

namespace Dune
{

  // because of gcc bug 14479
#ifdef HAVE_ICC
#define TEMPPARAM2
#endif

  static ALBERTA EL_INFO statElInfo[DIM+1];

  // singleton holding reference elements
  template<int dim>
  struct AlbertaGridReferenceElement
  {
    enum { dimension = dim };
    AlbertaGridElement<dim,dim> refelem;

    AlbertaGridReferenceElement () : refelem (true) {};
  };

  // initialize static variable with bool constructor
  // (which makes reference element)
  // this sucks but for gcc we do a lot
  static AlbertaGridReferenceElement<3> refelem_3;
  static AlbertaGridReferenceElement<2> refelem_2;
  static AlbertaGridReferenceElement<1> refelem_1;

  //****************************************************************
  //
  // --AlbertaGridElement
  // --Element
  //
  //****************************************************************

  //****************************************************************
  //
  //  specialization of mapVertices
  //
  //****************************************************************
  //
  // default, do nothing
  template< int dim, int dimworld>
  inline int AlbertaGridElement<dim,dimworld>::mapVertices (int i) const
  {
    return i;
  }

  // specialication for tetrhedrons
  // the local numbering in Albert is diffrent to Dune
  // see Albert Doc page 12
  //static const int mapVerts_3d[4] = {0,3,2,1};
  template <>
  inline int AlbertaGridElement<3,3>::mapVertices (int i) const
  {
    return i;
  }

  // specialication for codim == 1, faces
  template <>
  inline int AlbertaGridElement<1,2>::mapVertices (int i) const
  {
    int vert = ((face_ + 1 + i) % (N_VERTICES));
    return vert;
  }

  template <>
  inline int AlbertaGridElement<2,3>::mapVertices (int i) const
  {
    return ALBERTA AlbertHelp::localTetraFaceNumber[face_][i];
  }

  // specialization for codim == 2, edges
  template <>
  inline int AlbertaGridElement<1,3>::mapVertices (int i) const
  {
    return ((face_+1)+ (edge_+1) +i)% (N_VERTICES);
  }

  template <>
  inline int AlbertaGridElement<0,2>::mapVertices (int i) const
  {
    return ((face_+1)+ (vertex_+1) +i)% (N_VERTICES);
  }

  template <>
  inline int AlbertaGridElement<0,3>::mapVertices (int i) const
  {
    return ((face_+1)+ (edge_+1) +(vertex_+1) +i)% (N_VERTICES);
  }

  template< int dim, int dimworld>
  inline AlbertaGridElement<dim,dimworld>::
  AlbertaGridElement(bool makeRefElement)
  {
    // make empty element
    initGeom();

    // make coords for reference elements, spezial for different dim
    if(makeRefElement)
      makeRefElemCoords();
  }

  template< int dim, int dimworld>
  inline ALBERTA EL_INFO * AlbertaGridElement<dim,dimworld>::
  makeEmptyElInfo()
  {
    ALBERTA EL_INFO * elInfo = &statElInfo[dim];

    elInfo->mesh = 0;
    elInfo->el = 0;
    elInfo->parent = 0;
    elInfo->macro_el = 0;
    elInfo->level = 0;
#if DIM > 2
    elInfo->orientation = 0;
    elInfo->el_type = 0;
#endif

    for(int i =0; i<dim+1; i++)
    {
      for(int j =0; j< dimworld; j++)
      {
        elInfo->coord[i][j] = 0.0;
        elInfo->opp_coord[i][j] = 0.0;
      }
      elInfo->bound[i] = 0;
    }
    return elInfo;
  }


  template <>
  inline void AlbertaGridElement<2,2>::
  makeRefElemCoords()
  {
    // make empty elInfo
    elInfo_ = makeEmptyElInfo();

    //*****************************************************************
    //!
    /*!
       Dune reference element triangles (2d)

        (0,1)
         2|\    coordinates and local node numbers
     | \
     |  \
         1|   \0
     |    \
     |     \
         0|______\1
        (0,0) 2  (1,0)
     */
    //*****************************************************************

    // set reference coordinates
    coord_ = 0.0;

    // vertex 1
    coord_(0,1) = 1.0;

    // vertex 2
    coord_(1,2) = 1.0;
  }

  template <>
  inline void AlbertaGridElement<3,3>::
  makeRefElemCoords()
  {
    //! make ReferenzElement as default
    elInfo_ = makeEmptyElInfo();
    //*****************************************************************
    //
    //! reference element 3d
    //!        z
    //!        |     y
    //        3|    /
    // (0,0,1) |   /2 (0,1,0)
    //         |  /
    //         | /
    //         |/      (1,0,0)
    //       0 ------------x
    //      (0,0,0)     1
    //!
    //
    //*****************************************************************

    // set coordinates
    coord_ = 0.0;

    // vertex 1
    coord_(0,1) = 1.0;

    // vertex 2
    coord_(1,2) = 1.0;

    // vertex 3
    coord_(2,3) = 1.0;
  }
  template <>
  inline void AlbertaGridElement<1,1>::
  makeRefElemCoords()
  {
    //! make  Referenz Element as default
    elInfo_ = makeEmptyElInfo();

    // set reference coordinates
    coord_ = 0.0;

    // vertex 1
    coord_(1) = 1.0;
  }

  template <int dim, int dimworld>
  inline void AlbertaGridElement<dim,dimworld>::
  makeRefElemCoords()
  {
    DUNE_THROW(AlbertaError, "No default implementation for this AlbertaGridElement!");
  }

  template< int dim, int dimworld>
  inline void AlbertaGridElement<dim,dimworld>::
  initGeom()
  {
    elInfo_ = 0;
    face_ = 0;
    edge_ = 0;
    vertex_ = 0;
    builtinverse_ = false;
    builtElMat_   = false;
  }

  // built Geometry
  template< int dim, int dimworld>
  inline bool AlbertaGridElement<dim,dimworld>::
  builtGeom(ALBERTA EL_INFO *elInfo, int face,
            int edge, int vertex)
  {
    elInfo_ = elInfo;
    face_ = face;
    edge_ = edge;
    vertex_ = vertex;
    elDet_ = 0.0;
    builtinverse_ = false;
    builtElMat_   = false;

    if(elInfo_)
    {
      for(int i=0; i<dim+1; i++)
      {
        const ALBERTA REAL_D & elcoord = elInfo_->coord[mapVertices(i)];
        for(int j=0; j<dimworld; j++)
          coord_[i][j] = elcoord[j];
      }
      // geometry built
      return true;
    }
    // geometry not built
    return false;
  }


  // specialization yields speed up, because vertex_ .. is not copied
  template <>
  inline bool AlbertaGridElement<2,2>::
  builtGeom(ALBERTA EL_INFO *elInfo, int face,
            int edge, int vertex)
  {
    enum { dim = 2 };
    enum { dimworld = 2 };

    elInfo_ = elInfo;
    elDet_ = 0.0;
    builtinverse_ = false;
    builtElMat_   = false;

    if(elInfo_)
    {
      for(int i=0; i<dim+1; i++)
      {
        const ALBERTA REAL_D & elcoord = elInfo_->coord[mapVertices(i)];
        for(int j=0; j<dimworld; j++)
          coord_[i][j] = elcoord[j];
      }
      // geometry built
      return true;
    }
    // geometry not built
    return false;
  }

  template <>
  inline bool AlbertaGridElement<3,3>::
  builtGeom(ALBERTA EL_INFO *elInfo, int face,
            int edge, int vertex)
  {
    enum { dim = 3 };
    enum { dimworld = 3 };

    elInfo_ = elInfo;
    elDet_ = 0.0;
    builtinverse_ = false;
    builtElMat_   = false;

    if(elInfo_)
    {
      for(int i=0; i<dim+1; i++)
      {
        const ALBERTA REAL_D & elcoord = elInfo_->coord[mapVertices(i)];
        for(int j=0; j<dimworld; j++)
          coord_[i][j] = elcoord[j];
      }
      // geometry built
      return true;
    }
    // geometry not built
    return false;
  }


  // print the ElementInformation
  template<int dim, int dimworld>
  inline void AlbertaGridElement<dim,dimworld>::print (std::ostream& ss, int indent) const
  {
    ss << "AlbertaGridElement<" << dim << "," << dimworld << "> = {\n";
    for(int i=0; i<corners(); i++)
    {
      ss << " corner " << i;
      ss << ((*this)[i]); ss << "\n";
    }
    ss << "} \n";
  }

  template< int dim, int dimworld>
  inline ElementType AlbertaGridElement<dim,dimworld>::type() const
  {
    switch (dim)
    {
    case 1 : return line;
    case 2 : return triangle;
    case 3 : return tetrahedron;

    default : return unknown;
    }
  }

  template <>
  inline ElementType AlbertaGridElement<2,2>::type() const
  {
    return triangle;
  }

  template <>
  inline ElementType AlbertaGridElement<3,3>::type() const
  {
    return tetrahedron;
  }

  template< int dim, int dimworld>
  inline int AlbertaGridElement<dim,dimworld>::corners() const
  {
    return (dim+1);
  }

  ///////////////////////////////////////////////////////////////////////
  template< int dim, int dimworld>
  inline FieldVector<albertCtype, dimworld>& AlbertaGridElement<dim,dimworld>::
  operator [](int i) const
  {
    return coord_(i);
  }

  template<>
  inline AlbertaGridElement<3,3>& AlbertaGridElement<3,3>::
  refelem() const
  {
    return refelem_3.refelem;
  }

  template<>
  inline AlbertaGridElement<2,2>& AlbertaGridElement<2,3>::
  refelem() const
  {
    return refelem_2.refelem;
  }

  template<>
  inline AlbertaGridElement<2,2>& AlbertaGridElement<2,2>::
  refelem() const
  {
    return refelem_2.refelem;
  }

  template<>
  inline AlbertaGridElement<1,1>& AlbertaGridElement<1,3>::
  refelem() const
  {
    return refelem_1.refelem;
  }

  template<>
  inline AlbertaGridElement<1,1>& AlbertaGridElement<1,2>::
  refelem() const
  {
    return refelem_1.refelem;
  }

  template<>
  inline AlbertaGridElement<1,1>& AlbertaGridElement<1,1>::
  refelem() const
  {
    return refelem_1.refelem;
  }

  template< int dim, int dimworld>
  inline FieldVector<albertCtype, dimworld> AlbertaGridElement<dim,dimworld>::
  global(const FieldVector<albertCtype, dim>& local) const
  {
    // 1hecked, works

    // we calculate interal in barycentric coordinates
    // fake the third local coordinate via localFake
    albertCtype c = local[0];
    albertCtype localFake=1.0-c;

    // the initialize
    // note that we have to swap the j and i
    // in coord(j,i) means coord_(i)(j)
    for(int j=0; j<dimworld; j++)
      globalCoord_[j] = c * coord_(j,1);

    // for all local coords
    for (int i = 1; i < dim; i++)
    {
      c = local[i];
      localFake -= c;
      for(int j=0; j<dimworld; j++)
        globalCoord_[j] += c * coord_(j,i+1);
    }

    // for the last barycentric coord
    for(int j=0; j<dimworld; j++)
      globalCoord_[j] += localFake * coord_(j,0);

    return globalCoord_;
  }

  template <int dim, int dimworld>
  inline void AlbertaGridElement<dim,dimworld>::calcElMatrix () const
  {
    builtElMat_ = false;
    char text [1024];
    sprintf(text,"AlbertaGridElement<%d,%d>::calcElMatrix: No default implementation",dim,dimworld);
    DUNE_THROW(AlbertaError, text);
  }
  // calc A for triangles
  template <>
  inline void AlbertaGridElement<2,2>::calcElMatrix () const
  {
    if( !builtElMat_ )
    {
      // A = ( P1 - P0 , P2 - P0 )
      for (int i=0; i<2; i++)
      {
        elMat_(i,0) = coord_(i,1) - coord_(i,0);
        elMat_(i,1) = coord_(i,2) - coord_(i,0);
      }
      builtElMat_ = true;
    }
  }

  // calc A for tetrahedra
  template <>
  inline void AlbertaGridElement<3,3>::calcElMatrix () const
  {
    enum { dimworld = 3 };
    if( !builtElMat_)
    {
      FieldVector<albertCtype, dimworld> & coord0 = coord_(0);
      for(int i=0 ; i<dimworld; i++)
      {
        elMat_(i,0) = coord_(i,1) - coord0[i];
        elMat_(i,1) = coord_(i,2) - coord0[i];
        elMat_(i,2) = coord_(i,3) - coord0[i];
      }
      builtElMat_ = true;
    }

  }

  // uses the element matrix, because faster
  template<>
  inline FieldVector<albertCtype, 2> AlbertaGridElement<2,2>::
  global(const FieldVector<albertCtype, 2>& local) const
  {
    calcElMatrix();
    globalCoord_  = elMat_ * local;
    globalCoord_ += coord_(0);
    return globalCoord_;
  }

  template<>
  inline FieldVector<albertCtype, 3> AlbertaGridElement<3,3>::
  global(const FieldVector<albertCtype, 3>& local) const
  {
    calcElMatrix();
    globalCoord_  = elMat_ * local;
    globalCoord_ += coord_(0);
    return globalCoord_;
  }


  template< int dim, int dimworld>
  inline FieldVector<albertCtype, dim> AlbertaGridElement<dim,dimworld>::
  local(const FieldVector<albertCtype, dimworld>& global) const
  {
    char text [1024];
    sprintf(text,"AlbertaGridElement<%d,%d>::local: dim != dimworld not implemented!",dim,dimworld);
    DUNE_THROW(AlbertaError, text);

    localCoord_ = 0.0;
    return localCoord_;
  }

  template <>
  inline FieldVector<albertCtype, 2> AlbertaGridElement<2,2>::
  local(const FieldVector<albertCtype, 2>& global) const
  {
    if(!builtinverse_)
      buildJacobianInverse();

    localCoord_ = Jinv_ * ( global - coord_(0));
    return localCoord_;
  }

  template <>
  inline FieldVector<albertCtype, 3> AlbertaGridElement<3,3>::
  local(const FieldVector<albertCtype, 3>& global) const
  {
    if(!builtinverse_)
      buildJacobianInverse();

    localCoord_ = Jinv_ * ( global - coord_(0));
    return localCoord_;
  }



  // this method is for (dim==dimworld) = 2 and 3
  template <int dim, int dimworld>
  inline void AlbertaGridElement<dim,dimworld>::
  buildJacobianInverse() const
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

    // Jinv = A^-1
    assert( builtElMat_ == true );
    elDet_ = std::abs( elMat_.invert(Jinv_) );

    assert(elDet_ > 1.0E-25);
    builtinverse_ = true;
    return;
  }

  // calc volume of face of tetrahedron
  template <>
  inline void AlbertaGridElement<2,3>::
  buildJacobianInverse() const
  {
    enum { dim = 2 };
    enum { dimworld = 3 };

    // is faster than the lower method
    DUNE_THROW(AlbertaError,"buildJacobianInverse<2,3> not correctly implemented!");
    elDet_ = 0.1;
    builtinverse_ = true;
  }

  template <>
  inline void AlbertaGridElement<1,2>::
  buildJacobianInverse() const
  {
    // volume is length of edge
    FieldVector<albertCtype, 2> vec = coord_(0) - coord_(1);
    elDet_ = vec.two_norm();

    builtinverse_ = true;
  }
  // default implementation calls ALBERTA routine
  template< int dim, int dimworld>
  inline albertCtype AlbertaGridElement<dim,dimworld>::elDeterminant () const
  {
    return ALBERTA el_det(elInfo_);
  }

  // volume of one Element, here triangle
  template <>
  inline albertCtype AlbertaGridElement<2,2>::elDeterminant () const
  {
    calcElMatrix();
    return std::abs ( elMat_.determinant () );
  }

  // volume of one Element, here therahedron
  template <>
  inline albertCtype AlbertaGridElement<3,3>::elDeterminant () const
  {
    calcElMatrix();
    return std::abs ( elMat_.determinant () );
  }

  template< int dim, int dimworld>
  inline albertCtype AlbertaGridElement<dim,dimworld>::
  integration_element (const FieldVector<albertCtype, dim>& local) const
  {
    // if inverse was built, volume was calced already
    if(builtinverse_)
      return elDet_;

    elDet_ = elDeterminant();
    return elDet_;
  }

  template <>
  inline Mat<1,1>& AlbertaGridElement<1,2>::
  Jacobian_inverse (const FieldVector<albertCtype, 1>& global) const
  {
    DUNE_THROW(AlbertaError,"Jaconbian_inverse for dim=1,dimworld=2 not implemented yet!");
    return Jinv_;
  }

  template< int dim, int dimworld>
  inline Mat<dim,dim>& AlbertaGridElement<dim,dimworld>::
  Jacobian_inverse (const FieldVector<albertCtype, dim>& global) const
  {
    if(builtinverse_)
      return Jinv_;

    // builds the jacobian inverse and calculates the volume
    buildJacobianInverse();
    return Jinv_;
  }

  //************************************************************************
  //  checkMapping and checkInverseMapping are for checks of Jinv_
  //************************************************************************
  template <int dim, int dimworld>
  inline bool AlbertaGridElement<dim,dimworld>::checkInverseMapping (int loc) const
  {
    DUNE_THROW(AlbertaError,"AlbertaGridElement::checkInverseMapping: no default implemantation!");
    return false;
  }

  template <>
  inline bool AlbertaGridElement<2,2>::checkInverseMapping (int loc) const
  {
    // checks if F^-1 (x_i) == xref_i
    enum { dim =2 };

    FieldVector<albertCtype, dim> & coord    = coord_(loc);
    FieldVector<albertCtype, dim> & refcoord = refelem()[loc];
    buildJacobianInverse();

    FieldVector<albertCtype, dim> tmp2 = coord - coord_(0);
    tmp2 = Jinv_ * tmp2;

    for(int j=0; j<dim; j++)
      if(std::abs(tmp2[j] - refcoord[j]) > 1e-15)
      {
        std::cout << "AlbertaGridElement<2,2>::checkInverseMapping: Mapping of coord " << loc << " incorrect! \n";
        return false;
      }
    return true;
  }

  template <>
  inline bool AlbertaGridElement<3,3>::checkInverseMapping (int loc) const
  {
    // checks if F^-1 (x_i) == xref_i
    enum { dim = 3 };

    FieldVector<albertCtype, dim> & coord    = coord_(loc);
    FieldVector<albertCtype, dim> & refcoord = refelem()[loc];
    buildJacobianInverse();

    FieldVector<albertCtype, dim> tmp2 = coord - coord_(0);
    tmp2 = Jinv_ * tmp2;

    for(int j=0; j<dim; j++)
      if(std::abs(tmp2[j] - refcoord[j]) > 1e-15)
      {
        std::cout << "AlbertaGridElement<3,3>::checkInverseMapping: Mapping of coord " << loc << " incorrect! \n";
        return false;
      }
    return true;
  }


  template <int dim, int dimworld>
  inline bool AlbertaGridElement<dim,dimworld>::checkMapping (int loc) const
  {
    DUNE_THROW(AlbertaError,"AlbertaGridElement::checkMapping: no default implemantation!");
    return false;
  }

  template <>
  inline bool AlbertaGridElement<2,2>::checkMapping (int loc) const
  {
    // checks the mapping
    // F = Ax + P_0
    enum { dim =2 };

    calcElMatrix ();

    FieldVector<albertCtype, dim> & coord    = coord_(loc);
    FieldVector<albertCtype, dim> & refcoord = refelem()[loc];

    FieldVector<albertCtype, dim> tmp2 =  elMat_ * refcoord;
    tmp2 += coord_(0);

    for(int j=0; j<dim; j++)
      if(tmp2[j] != coord[j])
      {
        std::cout << coord; std::cout << tmp2; std::cout << "\n";
        std::cout << "AlbertaGridElement<2,2>::checkMapping: Mapping of coord " << loc << " incorrect! \n";
        return false;
      }
    return true;
  }

  template <>
  inline bool AlbertaGridElement<3,3>::checkMapping (int loc) const
  {
    // checks the mapping
    // F = Ax + P_0

    enum { dim = 3 };

    calcElMatrix ();

    FieldVector<albertCtype, dim> & coord    = coord_(loc);
    FieldVector<albertCtype, dim> & refcoord = refelem()[loc];

    FieldVector<albertCtype, dim> tmp2 =  elMat_ * refcoord;
    tmp2 += coord_(0);

    for(int j=0; j<dim; j++)
    {
      if(std::abs(tmp2[j] - coord[j]) > 1e-15)
      {
        std::cout << "Checking of " << loc << " not ok!\n";
        std::cout << coord; std::cout << refcoord;
        std::cout << tmp2; std::cout << "\n";
        std::cout << "AlbertaGridElement<3,3>::checkMapping: Mapping of coord " << loc << " incorrect! \n";
        return false;
      }
    }
    return true;
  }


  template<int dim, int dimworld>
  inline bool AlbertaGridElement <dim ,dimworld >::
  checkInside(const FieldVector<albertCtype, dim> &local) const
  {
    albertCtype sum = 0.0;

    for(int i=0; i<dim; i++)
    {
      sum += local[i];
      if(local[i] < 0.0)
      {
        if(std::abs(local[i]) > 1e-15)
        {
          return false;
        }
      }
    }

    if( sum > 1.0 )
    {
      if(sum > (1.0 + 1e-15))
        return false;
    }

    return true;
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
  // The Element is prescribed by the EL_INFO struct of ALBERTA MESH
  // the pointer to this struct is set and get by setElInfo and
  // getElInfo.
  //*********************************************************************8
  template<int codim, int dim, int dimworld>
  inline void AlbertaGridEntity < codim, dim ,dimworld >::
  makeDescription()
  {
    elInfo_ = 0;
    builtgeometry_ = false;

    // to slow and not needed
    //geo_.initGeom();
  }

  template<int codim, int dim, int dimworld>
  inline AlbertaGridEntity < codim, dim ,dimworld >::
  AlbertaGridEntity(const AlbertaGrid<dim,dimworld> &grid, int level,
                    ALBERTA TRAVERSE_STACK * travStack) : grid_(grid)
                                                          , level_ ( level )
                                                          , geo_ (false)
  {
    travStack_ = travStack;
    makeDescription();
  }


  template<int codim, int dim, int dimworld>
  inline void AlbertaGridEntity < codim, dim ,dimworld >::
  setTraverseStack(ALBERTA TRAVERSE_STACK * travStack)
  {
    travStack_ = travStack;
  }

  template<int codim, int dim, int dimworld>
  inline AlbertaGridEntity < codim, dim ,dimworld >::
  AlbertaGridEntity(const AlbertaGrid<dim,dimworld> &grid, int level) :
    grid_(grid)
    , level_ (level)
    , geo_ ( false )
  {
    travStack_ = 0;
    makeDescription();
  }

  template<int codim, int dim, int dimworld>
  inline ALBERTA EL_INFO* AlbertaGridEntity < codim, dim ,dimworld >::
  getElInfo() const
  {
    return elInfo_;
  }

  template<int codim, int dim, int dimworld>
  inline void AlbertaGridEntity < codim, dim ,dimworld >::
  setElInfo(ALBERTA EL_INFO * elInfo, int elNum, int face,
            int edge, int vertex )
  {
    elNum_ = elNum;
    face_ = face;
    edge_ = edge;
    vertex_ = vertex;
    elInfo_ = elInfo;
    builtgeometry_ = geo_.builtGeom(elInfo_,face,edge,vertex);
  }

  template<int codim, int dim, int dimworld>
  inline int AlbertaGridEntity < codim, dim ,dimworld >::
  level() const
  {
    return level_;
  }

  template<int codim, int dim, int dimworld>
  inline int AlbertaGridEntity < codim, dim ,dimworld >::
  index() const
  {
    assert(elNum_ >= 0);
    return elNum_;
  }

  template<int codim, int dim, int dimworld>
  inline int AlbertaGridEntity < codim, dim ,dimworld >::
  boundaryId() const
  {
    return (0);
  }


  template <>
  inline int AlbertaGridEntity<2,2,2>::boundaryId() const
  {
    return elInfo_->bound[vertex_];
  }

  template <>
  inline int AlbertaGridEntity<3,3,3>::boundaryId() const
  {
    return elInfo_->bound[vertex_];
  }

  template <>
  inline int AlbertaGridEntity<1,2,2>::boundaryId() const
  {
    return elInfo_->boundary[face_]->bound;
  }


  template <>
  inline int AlbertaGridEntity<1,3,3>::boundaryId() const
  {
    return elInfo_->boundary[face_]->bound;
  }


  template<int dim, int dimworld>
  inline int AlbertaGridEntity < 0, dim ,dimworld >::
  boundaryId() const
  {
    return (0);

  }

  template<int codim, int dim, int dimworld>
  inline int AlbertaGridEntity < codim, dim ,dimworld >::
  el_index() const
  {
    assert(codim == dim);
    return elInfo_->el->dof[vertex_][0];
  }

  template<int codim, int dim, int dimworld>
  inline int AlbertaGridEntity < codim, dim ,dimworld >::
  global_index() const
  {
    return el_index();
  }

  template< int codim, int dim, int dimworld>
  inline AlbertaGridElement<dim-codim,dimworld>&
  AlbertaGridEntity < codim, dim ,dimworld >::geometry() const
  {
    return geo_;
  }

  template<int codim, int dim, int dimworld>
  inline FieldVector<albertCtype, dim>&
  AlbertaGridEntity < codim, dim ,dimworld >::local() const
  {
    return localFatherCoords_;
  }

  template<int codim, int dim, int dimworld>
  inline AlbertaGridLevelIterator<0,dim,dimworld,All_Partition>
  AlbertaGridEntity < codim, dim ,dimworld >::father() const
  {
    ALBERTA TRAVERSE_STACK travStack;
    ALBERTA initTraverseStack(&travStack);

    travStack = (*travStack_);

    travStack.stack_used--;

    AlbertaGridLevelIterator <0,dim,dimworld,All_Partition>
    it(grid_,level(),travStack.elinfo_stack+travStack.stack_used,0,0,0,0);
    return it;
  }


  //************************************
  //
  //  --AlbertaGridEntity codim = 0
  //  --0Entity codim = 0
  //
  //************************************
  template< int dim, int dimworld>
  inline bool AlbertaGridEntity < 0, dim ,dimworld >::
  mark( int refCount )
  {
    if(! hasChildren() )
    {
      // we can not mark for coarsening if already marked for refinement
      if((refCount < 0) && (elInfo_->el->mark > 0))
      {
        return true;
      }

      if( refCount > 0)
      {
        elInfo_->el->mark = 1;
        return true;
      }
      if( refCount < 0)
      {
        grid_.setMark ( true );
        elInfo_->el->mark = -1;
        return true;
      }
    }
    // only for debugging
    else
    {
      fprintf(stderr,"ERROR in AlbertaGridEntity<0,%d,%d>::mark(%d) : called on non LeafEntity! \n",dim,dimworld,refCount);
      abort();
    }
    elInfo_->el->mark = 0;
    return false;
  }

  template< int dim, int dimworld>
  inline AdaptationState AlbertaGridEntity < 0, dim ,dimworld >::
  state() const
  {
    if( elInfo_->el->mark < 0 )
    {
      return COARSEN;
    }
    if( grid_.checkElNew( elInfo_->el ) )
    {
      return REFINED;
    }

    return NONE;
  }

  template< int dim, int dimworld>
  inline bool AlbertaGridEntity < 0, dim ,dimworld >::
  partition( int proc )
  {
    return grid_.setOwner( elInfo_->el , proc );
  }

  template< int dim, int dimworld>
  inline PartitionType AlbertaGridEntity < 0, dim ,dimworld >::
  partitionType () const
  {
    return grid_.partitionType( elInfo_ );
  }

  template< int dim, int dimworld>
  inline bool AlbertaGridEntity < 0, dim ,dimworld >::
  master() const
  {
    return (owner() == grid_.myProcessor());
  }


  template< int dim, int dimworld>
  inline int AlbertaGridEntity < 0, dim ,dimworld >::
  owner() const
  {
    return grid_.getOwner( elInfo_->el );
  }

  template< int dim, int dimworld>
  inline bool AlbertaGridEntity < 0, dim ,dimworld >::hasChildren() const
  {
    return !(this->isLeaf());
  }

  template< int dim, int dimworld>
  inline bool AlbertaGridEntity < 0, dim ,dimworld >::isLeaf() const
  {
    if(elInfo_)
      return (elInfo_->el->child[0] == 0);
    return false;
  }

  //***************************

  template< int dim, int dimworld>
  inline void AlbertaGridEntity < 0, dim ,dimworld >::
  makeDescription()
  {
    elInfo_ = 0;
    builtgeometry_ = false;
  }

  template<int dim, int dimworld>
  inline void AlbertaGridEntity < 0, dim ,dimworld >::
  setTraverseStack(ALBERTA TRAVERSE_STACK * travStack)
  {
    travStack_ = travStack;
  }

  template< int dim, int dimworld>
  inline AlbertaGridEntity < 0, dim ,dimworld >::
  AlbertaGridEntity(AlbertaGrid<dim,dimworld> &grid, int level)
    : grid_(grid)
      , level_ (level)
      , vxEntity_ ( grid_ , -1, 0, 0, 0, 0, 0)
      , travStack_ (0) , elInfo_ (0)
      , geo_(false)
      , builtgeometry_ (false)
  {}

  //*****************************************************************8
  // count
  template <int dim, int dimworld> template <int cc>
  inline int AlbertaGridEntity<0,dim,dimworld>::count () const
  {
    return (dim+1);
  }
  //! specialization only for codim == 2 , edges,
  //! a tetrahedron has always 6 edges
  template <>
#ifdef TEMPPARAM2
  template <>
#endif
  inline int AlbertaGridEntity<0,3,3>::count<2> () const
  {
    return 6;
  }

  // subIndex
  template <int dim, int dimworld> template <int cc>
  inline int AlbertaGridEntity<0,dim,dimworld>::subIndex ( int i ) const
  {
    return entity<cc>(i)->index();
  }


  // subIndex
  template <>
#ifdef TEMPPARAM2
  template <>
#endif
  inline int AlbertaGridEntity<0,2,2>::subIndex<2> ( int i ) const
  {
    //return grid_.indexOnLevel<2>(elInfo_->el->dof[i][0],level_);
    return (elInfo_->el->dof[i][0]);
  }

  // subIndex
  template <>
#ifdef TEMPPARAM2
  template <>
#endif
  inline int AlbertaGridEntity<0,2,3>::subIndex<2> ( int i ) const
  {
    return grid_.indexOnLevel<2>(elInfo_->el->dof[i][0],level_);
  }

  // subIndex
  template <>
#ifdef TEMPPARAM2
  template <>
#endif
  inline int AlbertaGridEntity<0,3,3>::subIndex<3> ( int i ) const
  {
    //return grid_.indexOnLevel<3>(elInfo_->el->dof[i][0],level_);
    return (elInfo_->el->dof[i][0]);
  }


  // default is faces
  template <int dim, int dimworld> template <int cc>
  inline AlbertaGridLevelIterator<cc,dim,dimworld,All_Partition>
  AlbertaGridEntity<0,dim,dimworld>::entity ( int i ) const
  {
    AlbertaGridLevelIterator<cc,dim,dimworld,All_Partition> tmp (grid_, level() ,elInfo_,
                                                                 grid_. template indexOnLevel<cc>( el_index() ,level()),i,0,0);
    return tmp;
  }

  template <>
#ifdef TEMPPARAM2
  template <>
#endif
  inline AlbertaGridLevelIterator<2,3,3,All_Partition>
  AlbertaGridEntity<0,3,3>::entity<2> ( int i ) const
  {
    //enum { cc = 2 };
    int num = grid_.indexOnLevel<2>(el_index() ,level_);
    if(i < 3)
    { // 0,1,2
      AlbertaGridLevelIterator<2,3,3,All_Partition> tmp (grid_,level_,elInfo_,num, 0,i,0);
      return tmp;
    }
    else
    { // 3,4,5
      AlbertaGridLevelIterator<2,3,3,All_Partition> tmp (grid_,level_,elInfo_,num, i-2,1,0);
      return tmp;
    }
  }

  // specialization for vertices
  template <>
#ifdef TEMPPARAM2
  template <>
#endif
  inline AlbertaGridLevelIterator<2,2,2,All_Partition>
  AlbertaGridEntity<0,2,2>::entity<2> ( int i ) const
  {
    // we are looking at vertices
    //enum { cc = dimension };
    enum { cc = 2 };
    AlbertaGridLevelIterator<cc,2,2,All_Partition>
    tmp (grid_,level_,elInfo_, grid_.indexOnLevel<cc>( elInfo_->el->dof[i][0],level_),
         0,0,i);
    return tmp;
  }
  // specialization for vertices
  template <>
#ifdef TEMPPARAM2
  template <>
#endif
  inline AlbertaGridLevelIterator<2,2,3,All_Partition>
  AlbertaGridEntity<0,2,3>::entity<2> ( int i ) const
  {
    // we are looking at vertices
    //enum { cc = dimension };
    enum { cc = 2 };
    AlbertaGridLevelIterator<cc,2,3,All_Partition>
    tmp (grid_,level_,elInfo_, grid_.indexOnLevel<cc>( elInfo_->el->dof[i][0],level_),
         0,0,i);
    return tmp;
  }
  // specialization for vertices
  template <>
#ifdef TEMPPARAM2
  template <>
#endif
  inline AlbertaGridLevelIterator<3,3,3,All_Partition>
  AlbertaGridEntity<0,3,3>::entity<3> ( int i ) const
  {
    // we are looking at vertices
    enum { cc = 3 };
    //enum { cc = dimension };
    AlbertaGridLevelIterator<cc,3,3,All_Partition>
    tmp (grid_,level_,elInfo_, grid_.indexOnLevel<cc>( elInfo_->el->dof[i][0],level_),
         0,0,i);
    return tmp;
  }
  //***************************

  template<int dim, int dimworld>
  inline ALBERTA EL_INFO* AlbertaGridEntity < 0 , dim ,dimworld >::
  getElInfo() const
  {
    return elInfo_;
  }

  template<int dim, int dimworld>
  inline int AlbertaGridEntity < 0, dim ,dimworld >::
  level() const
  {
    return level_;
  }

  template<int dim, int dimworld>
  inline int AlbertaGridEntity < 0, dim ,dimworld >::
  index() const
  {
    return grid_.template indexOnLevel<0>( el_index() , level_ );
  }

  // --el_index
  template<int dim, int dimworld>
  inline int AlbertaGridEntity < 0, dim ,dimworld >::
  el_index() const
  {
    return grid_.getElementNumber( elInfo_->el );
  }

  template<int dim, int dimworld>
  inline int AlbertaGridEntity < 0, dim ,dimworld >::
  global_index() const
  {
    return el_index();
  }

  template< int dim, int dimworld>
  inline void AlbertaGridEntity < 0, dim ,dimworld >::
  setLevel(int actLevel)
  {
    level_ = actLevel;
  }

  template< int dim, int dimworld>
  inline void AlbertaGridEntity < 0, dim ,dimworld >::
  setElInfo(ALBERTA EL_INFO * elInfo, int elNum,  int face,
            int edge, int vertex )
  {
    // in this case the face, edge and vertex information is not used,
    // because we are in the element case
    elInfo_ = elInfo;
    builtgeometry_ = geo_.builtGeom(elInfo_,face,edge,vertex);
  }

  template< int dim, int dimworld>
  inline AlbertaGridElement<dim,dimworld>&
  AlbertaGridEntity < 0, dim ,dimworld >::geometry() const
  {
    assert(builtgeometry_ == true);
    return geo_;
  }


  template< int dim, int dimworld>
  inline AlbertaGridLevelIterator<0,dim,dimworld,All_Partition>
  AlbertaGridEntity < 0, dim ,dimworld >::father() const
  {
    ALBERTA EL_INFO * fatherInfo = 0;
    int fatherLevel = level_-1;
    // if this level > 0 return father = elInfoStack -1,
    // else return father = this
    assert(travStack_ != 0);

    if(level_ > 0)
      fatherInfo = & (travStack_->elinfo_stack)[travStack_->stack_used-1];
    else
    {
      std::cout << "No father on macro level! \n";
      fatherInfo = elInfo_;
      fatherLevel = 0;
    }

    int fatherIndex = grid_.template indexOnLevel<0>(grid_.getElementNumber(fatherInfo->el),fatherLevel);
    // new LevelIterator with EL_INFO one level above
    AlbertaGridLevelIterator <0,dim,dimworld,All_Partition> it(grid_,fatherLevel,fatherInfo,fatherIndex,0,0,0);
    return it;
  }

  template< int dim, int dimworld>
  inline AlbertaGridEntity < 0, dim ,dimworld >
  AlbertaGridEntity < 0, dim ,dimworld >::
  newEntity()
  {
    AlbertaGridEntity < 0, dim ,dimworld > tmp ( grid_ , level_ );
    return tmp;
  }

  template< int dim, int dimworld>
  inline void
  AlbertaGridEntity < 0, dim ,dimworld >::
  father(AlbertaGridEntity < 0, dim ,dimworld >& vati ) const
  {
    ALBERTA EL_INFO * fatherInfo = 0;
    int fatherLevel = level_-1;
    // if this level > 0 return father = elInfoStack -1,
    // else return father = this
    assert(travStack_ != 0);

    if(level_ > 0)
      fatherInfo = & (travStack_->elinfo_stack)[travStack_->stack_used-1];
    else
    {
      std::cout << "No father on macro level! \n";
      fatherInfo = elInfo_;
      fatherLevel = 0;
    }

    vati.setElInfo( fatherInfo );
    vati.setLevel ( fatherLevel );
  }

  template< int dim, int dimworld>
  inline AlbertaGridElement<dim,dim>&
  AlbertaGridEntity < 0, dim ,dimworld >::father_relative_local() const
  {
    //AlbertaGridLevelIterator<0,dim,dimworld> daddy = father();
    AlbertaGridElement<dim,dimworld> daddy = (*father()).geometry();

    fatherReLocal_.initGeom();
    // compute the local coordinates in father refelem
    for(int i=0; i<fatherReLocal_.corners(); i++)
      fatherReLocal_[i] = daddy.local(geometry()[i]);

    return fatherReLocal_;
  }
  // end AlbertaGridEntity


  //***************************************************************
  //
  //  --AlbertaGridHierarchicIterator
  //  --HierarchicIterator
  //
  //***************************************************************
  template< int dim, int dimworld>
  inline void AlbertaGridHierarchicIterator<dim,dimworld>::
  makeIterator()
  {
    virtualEntity_.setTraverseStack(0);
    virtualEntity_.setElInfo(0,0,0,0,0);
  }

  template< int dim, int dimworld>
  inline AlbertaGridHierarchicIterator<dim,dimworld>::
  AlbertaGridHierarchicIterator(AlbertaGrid<dim,dimworld> &grid,int actLevel,
                                int maxLevel) : grid_(grid), level_ (actLevel)
                                                , maxlevel_ (maxLevel) , virtualEntity_(grid,level_)
  {
    makeIterator();
  }

  template< int dim, int dimworld>
  inline AlbertaGridHierarchicIterator<dim,dimworld>::
  AlbertaGridHierarchicIterator(AlbertaGrid<dim,dimworld> &grid,
                                ALBERTA TRAVERSE_STACK *travStack,int actLevel, int maxLevel) :
    grid_(grid), level_ (actLevel),
    maxlevel_ ( maxLevel), virtualEntity_(grid,level_)
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
        maxlevel_ = grid_.maxlevel(); //123456789;
      }
      // set new traverse level
      stack->traverse_level = maxlevel_;

      virtualEntity_.setTraverseStack(stack);
      // Hier kann ein beliebiges Element uebergeben werden,
      // da jedes AlbertElement einen Zeiger auf das Macroelement
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

  template< int dim, int dimworld>
  inline AlbertaGridHierarchicIterator<dim,dimworld>&
  AlbertaGridHierarchicIterator< dim,dimworld >::operator ++()
  {
    virtualEntity_.setElInfo(
      recursiveTraverse(manageStack_.getStack())
      );
    // set new actual level, calculated by recursiveTraverse
    virtualEntity_.setLevel(level_);
    return (*this);
  }

  template< int dim, int dimworld>
  inline bool AlbertaGridHierarchicIterator<dim,dimworld>::
  operator ==(const AlbertaGridHierarchicIterator& I) const
  {
    return (virtualEntity_.getElInfo() == I.virtualEntity_.getElInfo());
  }

  template< int dim, int dimworld>
  inline bool AlbertaGridHierarchicIterator<dim,dimworld>::
  operator !=(const AlbertaGridHierarchicIterator& I) const
  {
    return !((*this) == I);
  }

  template< int dim, int dimworld>
  inline AlbertaGridEntity < 0, dim ,dimworld >&
  AlbertaGridHierarchicIterator<dim,dimworld>::
  operator *()
  {
    return virtualEntity_;
  }

  template< int dim, int dimworld>
  inline AlbertaGridEntity < 0, dim ,dimworld >*
  AlbertaGridHierarchicIterator<dim,dimworld>::
  operator ->()
  {
    return &virtualEntity_;
  }

  template< int dim, int dimworld>
  inline ALBERTA EL_INFO *
  AlbertaGridHierarchicIterator<dim,dimworld>::
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
      grid_.fillElInfo(i, level_, stack->elinfo_stack+stack->stack_used,
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

  template<int dim, int dimworld>
  inline AlbertaGridBoundaryEntity<dim,dimworld>::
  AlbertaGridBoundaryEntity () : _geom (false) , _elInfo ( 0 ),
                                 _neigh (-1) {}

  template<int dim, int dimworld>
  inline int AlbertaGridBoundaryEntity<dim,dimworld>::id () const
  {
    assert(_elInfo->boundary[_neigh] != 0);
    return _elInfo->boundary[_neigh]->bound;
  }

  template<int dim, int dimworld>
  inline bool AlbertaGridBoundaryEntity<dim,dimworld>::hasGeometry () const
  {
    return _geom.builtGeom(_elInfo,0,0,0);
  }

  template<int dim, int dimworld>
  inline AlbertaGridElement<dim,dimworld>&
  AlbertaGridBoundaryEntity<dim,dimworld>::geometry () const
  {
    return _geom;
  }

  template<int dim, int dimworld>
  inline void AlbertaGridBoundaryEntity<dim,dimworld>::
  setElInfo (ALBERTA EL_INFO * elInfo, int nb)
  {
    _neigh = nb;
    if(elInfo)
      _elInfo = elInfo;
    else
      _elInfo = 0;
  }

  //***************************************************************
  //
  //  --AlbertaGridIntersectionIterator
  //  --IntersectionIterator
  //
  //***************************************************************

  // these object should be generated with new by Entity, because
  // for a LevelIterator we only need one virtualNeighbour Entity, which is
  // given to the Neighbour Iterator, we need a list of Neighbor Entitys
  template< int dim, int dimworld>
  inline void AlbertaGridIntersectionIterator<dim,dimworld>::freeObjects ()
  {
    if(manageObj_)
      manageObj_ = grid_->entityProvider_.freeObjectEntity(manageObj_);

    if(manageInterEl_)
      manageInterEl_ = grid_->interSelfProvider_.freeObjectEntity(manageInterEl_);

    if(manageNeighEl_)
      manageNeighEl_ = grid_->interNeighProvider_.freeObjectEntity(manageNeighEl_);

    if(boundaryEntity_)
    {
      delete boundaryEntity_;
      boundaryEntity_ = 0;
    }

    if(manageNeighInfo_)
      manageNeighInfo_ = elinfoProvider.freeObjectEntity(manageNeighInfo_);
  }

  template< int dim, int dimworld>
  inline AlbertaGridIntersectionIterator<dim,dimworld>::~AlbertaGridIntersectionIterator ()
  {
    freeObjects();
  }

  template< int dim, int dimworld>
  inline AlbertaGridIntersectionIterator<dim,dimworld>::
  AlbertaGridIntersectionIterator(AlbertaGrid<dim,dimworld> &grid, int level) :
    grid_( &grid ), level_ (level) , neighborCount_ (dim+1), virtualEntity_ (0)
    , elInfo_ (0)
    , manageObj_ (0)
    , manageInterEl_ (0)
    , manageNeighEl_ (0)
    , fakeNeigh_ (0)
    , neighGlob_ (0)
    , boundaryEntity_ (0)
    , manageNeighInfo_ (0) , neighElInfo_ (0) {}


  template< int dim, int dimworld>
  inline AlbertaGridIntersectionIterator<dim,dimworld>::AlbertaGridIntersectionIterator
    (AlbertaGrid<dim,dimworld> & grid, int level, ALBERTA EL_INFO *elInfo ) :
    grid_( &grid ) , level_ (level), neighborCount_ (0)
    , builtNeigh_ (false)
    , virtualEntity_ (0)
    , elInfo_ ( elInfo )
    , manageObj_ (0)
    , manageInterEl_ (0)
    , manageNeighEl_ (0)
    , fakeNeigh_ (0) , neighGlob_ (0)
    , boundaryEntity_ (0)
  {
    manageNeighInfo_ = elinfoProvider.getNewObjectEntity();
    neighElInfo_ = manageNeighInfo_->item;
  }

  // empty constructor
  template< int dim, int dimworld>
  inline AlbertaGridIntersectionIterator<dim,dimworld>::
  AlbertaGridIntersectionIterator ( ) :
    grid_( 0 ) , level_ ( -1 ), neighborCount_ ( -1 )
    , builtNeigh_ (false)
    , virtualEntity_ (0)
    , elInfo_ ( 0 )
    , manageObj_ (0)
    , manageInterEl_ (0)
    , manageNeighEl_ (0)
    , fakeNeigh_ (0) , neighGlob_ (0)
    , boundaryEntity_ (0) {}

  template< int dim, int dimworld>
  inline void AlbertaGridIntersectionIterator<dim,dimworld>::makeBegin
    (AlbertaGrid<dim,dimworld> &grid, int level, ALBERTA EL_INFO *elInfo )
  {
    grid_ = &grid;
    level_ = level;
    elInfo_ = elInfo;
    neighborCount_ = 0;
    builtNeigh_ = false;

    // remove old objects
    freeObjects();

    manageNeighInfo_ = elinfoProvider.getNewObjectEntity();
    neighElInfo_ = manageNeighInfo_->item;
  }

  template< int dim, int dimworld>
  inline void AlbertaGridIntersectionIterator<dim,dimworld>::makeEnd
    (AlbertaGrid<dim,dimworld> &grid, int level )
  {
    grid_ = &grid;
    level_ = level;
    elInfo_ = 0;
    neighborCount_ = dim+1;
    builtNeigh_ = false;

    // remove old objects
    freeObjects();
  }

  template< int dim, int dimworld>
  inline AlbertaGridIntersectionIterator<dim,dimworld>&
  AlbertaGridIntersectionIterator<dim,dimworld>::
  operator ++()
  {
    builtNeigh_ = false;
    // is like go to the next neighbour
    neighborCount_++;
    return (*this);
  }

  template< int dim, int dimworld>
  inline bool AlbertaGridIntersectionIterator<dim,dimworld>::operator ==
    (const AlbertaGridIntersectionIterator& I) const
  {
    return (neighborCount_ == I.neighborCount_);
  }

  template< int dim, int dimworld>
  inline bool AlbertaGridIntersectionIterator<dim,dimworld>::
  operator !=(const AlbertaGridIntersectionIterator& I) const
  {
    return (neighborCount_ != I.neighborCount_);
  }

  template< int dim, int dimworld>
  inline AlbertaGridEntity < 0, dim ,dimworld >&
  AlbertaGridIntersectionIterator<dim,dimworld>::
  operator *()
  {
    if(!builtNeigh_)
    {
      if(!manageObj_)
      {
        manageObj_ = grid_->entityProvider_.getNewObjectEntity( *grid_ ,level_);
        virtualEntity_ = manageObj_->item;
        virtualEntity_->setLevel(level_);
        memcpy(neighElInfo_,elInfo_,sizeof(ALBERTA EL_INFO));
      }

      setupVirtEn();
    }
    return (*virtualEntity_);
  }

  template< int dim, int dimworld>
  inline AlbertaGridEntity < 0, dim ,dimworld >*
  AlbertaGridIntersectionIterator<dim,dimworld>::
  operator ->()
  {
    if(!builtNeigh_)
    {
      if(!manageObj_)
      {
        manageObj_ = grid_->entityProvider_.getNewObjectEntity( *grid_ ,level_);
        virtualEntity_ = manageObj_->item;
        virtualEntity_->setLevel(level_);
        memcpy(neighElInfo_,elInfo_,sizeof(ALBERTA EL_INFO));
      }

      setupVirtEn();
    }
    return virtualEntity_;
  }

  template< int dim, int dimworld>
  inline AlbertaGridBoundaryEntity<dim,dimworld>&
  AlbertaGridIntersectionIterator<dim,dimworld>::boundaryEntity () const
  {
    if(!boundaryEntity_)
    {
      boundaryEntity_ = new AlbertaGridBoundaryEntity<dim,dimworld> ();
    }
    boundaryEntity_->setElInfo(elInfo_,neighborCount_);
    return (*boundaryEntity_);
  }

  template< int dim, int dimworld>
  inline bool AlbertaGridIntersectionIterator<dim,dimworld>::boundary() const
  {
    return (elInfo_->boundary[neighborCount_] != 0);
  }

  template< int dim, int dimworld>
  inline bool AlbertaGridIntersectionIterator<dim,dimworld>::neighbor() const
  {
    return (elInfo_->neigh[neighborCount_] != 0);
  }

  template< int dim, int dimworld>
  inline FieldVector<albertCtype, dimworld>& AlbertaGridIntersectionIterator<dim,dimworld>::
  unit_outer_normal(FieldVector<albertCtype, dim-1>& local) const
  {
    // calculates the outer_normal
    FieldVector<albertCtype, dimworld>& tmp = outer_normal(local);

    double norm_1 = (1.0/tmp.two_norm());
    assert(norm_1 > 0.0);
    outNormal_ *= norm_1;

    return outNormal_;
  }

  template< int dim, int dimworld>
  inline FieldVector<albertCtype, dimworld>& AlbertaGridIntersectionIterator<dim,dimworld>::
  unit_outer_normal() const
  {
    // calculates the outer_normal
    FieldVector<albertCtype, dimworld>& tmp = outer_normal();

    double norm_1 = (1.0/tmp.two_norm());
    assert(norm_1 > 0.0);
    outNormal_ *= norm_1;

    return outNormal_;
  }

  template< int dim, int dimworld>
  inline FieldVector<albertCtype, dimworld>& AlbertaGridIntersectionIterator<dim,dimworld>::
  outer_normal(FieldVector<albertCtype, dim-1>& local) const
  {
    // we dont have curved boundary
    // therefore return outer_normal
    return outer_normal();
  }

  template< int dim, int dimworld>
  inline FieldVector<albertCtype, dimworld>& AlbertaGridIntersectionIterator<dim,dimworld>::
  outer_normal() const
  {
    std::cout << "outer_normal() not correctly implemented yet! \n";
    for(int i=0; i<dimworld; i++)
      outNormal_(i) = 0.0;

    return outNormal_;
  }

  template <>
  inline FieldVector<albertCtype, 2>& AlbertaGridIntersectionIterator<2,2>::
  outer_normal() const
  {
    // seems to work
    ALBERTA REAL_D *coord = elInfo_->coord;

    outNormal_[0] = -(coord[(neighborCount_+1)%3][1] - coord[(neighborCount_+2)%3][1]);
    outNormal_[1] =   coord[(neighborCount_+1)%3][0] - coord[(neighborCount_+2)%3][0];

    return outNormal_;
  }

  template <>
  inline FieldVector<albertCtype, 3>& AlbertaGridIntersectionIterator<3,3>::
  outer_normal() const
  {
    enum { dim = 3 };
    // rechne Kreuzprodukt der Vectoren aus
    ALBERTA REAL_D *coord = elInfo_->coord;

    // in this case the orientation is negative, therefore multiply with -1
#if DIM == 3
    const albertCtype val = (elInfo_->orientation > 0) ? 0.5 : -0.5;
#else
    const albertCtype val = 0.5;
#endif

    // neighborCount_ is the local face number
    const int * localFaces = ALBERTA AlbertHelp::localTetraFaceNumber[neighborCount_];
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
    return outNormal_;
  }

  template< int dim, int dimworld>
  inline AlbertaGridElement< dim-1, dim >&
  AlbertaGridIntersectionIterator<dim,dimworld>::
  intersection_self_local() const
  {
    std::cout << "\nintersection_self_local not checked until now! \n";
    if(!manageInterEl_)
    {
      manageInterEl_ = grid_->interSelfProvider_.getNewObjectEntity();
      fakeNeigh_ = manageInterEl_->item;
    }

    fakeNeigh_->builtGeom(elInfo_,neighborCount_,0,0);
    return (*fakeNeigh_);
  }

  template< int dim, int dimworld>
  inline AlbertaGridElement< dim-1, dimworld >&
  AlbertaGridIntersectionIterator<dim,dimworld>::
  intersection_self_global() const
  {
    if(!manageNeighEl_)
    {
      manageNeighEl_ = grid_->interNeighProvider_.getNewObjectEntity();
      neighGlob_ = manageNeighEl_->item;
    }

    if(neighGlob_->builtGeom(elInfo_,neighborCount_,0,0))
      return (*neighGlob_);
    else
    {
      DUNE_THROW(AlbertaError, "intersection_self_global: error occured!");
    }
    return (*neighGlob_);
  }

  template< int dim, int dimworld>
  inline AlbertaGridElement< dim-1, dim >&
  AlbertaGridIntersectionIterator<dim,dimworld>::
  intersection_neighbor_local() const
  {
    std::cout << "intersection_neighbor_local not checked until now! \n";
    if(!manageInterEl_)
    {
      manageInterEl_ = grid_->interSelfProvider_.getNewObjectEntity();
      fakeNeigh_ = manageInterEl_->item;
    }

    if(!builtNeigh_)
    {
      setupVirtEn();
    }

    fakeNeigh_->builtGeom(neighElInfo_,neighborCount_,0,0);
    return (*fakeNeigh_);
  }

  template< int dim, int dimworld>
  inline AlbertaGridElement< dim-1, dimworld >&
  AlbertaGridIntersectionIterator<dim,dimworld>::
  intersection_neighbor_global() const
  {
    std::cout << "intersection_neighbor_global not check until now! \n";
    if(!manageNeighEl_)
    {
      manageNeighEl_ = grid_->interNeighProvider_.getNewObjectEntity();
      neighGlob_ = manageNeighEl_->item;
    }

    // built neighGlob_ first
    if(!builtNeigh_)
    {
      setupVirtEn();
    }
    neighGlob_->builtGeom(elInfo_,neighborCount_,0,0);
    return (*neighGlob_);
  }

  template< int dim, int dimworld>
  inline int AlbertaGridIntersectionIterator<dim,dimworld>::
  number_in_self () const
  {
    return neighborCount_;
  }

  template< int dim, int dimworld>
  inline int AlbertaGridIntersectionIterator<dim,dimworld>::
  number_in_neighbor () const
  {
    return elInfo_->opp_vertex[neighborCount_];
  }

  // setup neighbor element with the information of elInfo_
  template< int dim, int dimworld>
  inline void AlbertaGridIntersectionIterator<dim,dimworld>::setupVirtEn()
  {
    // set the neighbor element as element
    neighElInfo_->el = elInfo_->neigh[neighborCount_];
#if DIM==3
    neighElInfo_->orientation = elInfo_->orientation;
#endif

    int vx = elInfo_->opp_vertex[neighborCount_];

    for(int i=0; i<dimworld; i++)
      neighElInfo_->coord[vx][i] = elInfo_->opp_coord[neighborCount_][i];

    for(int i=1; i<dim+1; i++)
    {
      int nb = (((neighborCount_-i)%(dim+1)) +dim+1)%(dim+1);
      for(int j=0; j<dimworld; j++)
        neighElInfo_->coord[(vx+i)%(dim+1)][j] = elInfo_->coord[nb][j];
    }
    /* works, tested many times */

    virtualEntity_->setElInfo(neighElInfo_);
    builtNeigh_ = true;
  }
  // end IntersectionIterator


  //*******************************************************
  //
  // --AlbertaGridLevelIterator
  // --LevelIterator
  //
  //*******************************************************

  //***********************************************************
  //  some template specialization of goNextEntity
  //***********************************************************
  // default implementation, go next elInfo
  template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
  inline ALBERTA EL_INFO * AlbertaGridLevelIterator<codim,dim,dimworld,pitype>::
  goNextEntity(ALBERTA TRAVERSE_STACK *stack,ALBERTA EL_INFO *elinfo_old)
  {
    return goNextElInfo(stack,elinfo_old);
  }

  // specializations for codim 1, go next face
  template <>
  inline ALBERTA EL_INFO * AlbertaGridLevelIterator<1,2,2,All_Partition>::
  goNextEntity(ALBERTA TRAVERSE_STACK *stack,ALBERTA EL_INFO *elinfo_old)
  {
    return goNextFace(stack,elinfo_old);
  }
  template <>
  inline ALBERTA EL_INFO * AlbertaGridLevelIterator<1,2,3,All_Partition>::
  goNextEntity(ALBERTA TRAVERSE_STACK *stack,ALBERTA EL_INFO *elinfo_old)
  {
    return goNextFace(stack,elinfo_old);
  }
  template <>
  inline ALBERTA EL_INFO * AlbertaGridLevelIterator<1,3,3,All_Partition>::
  goNextEntity(ALBERTA TRAVERSE_STACK *stack,ALBERTA EL_INFO *elinfo_old)
  {
    return goNextFace(stack,elinfo_old);
  }

  // specialization for codim 2, if dim > 2, go next edge,
  // only if dim == dimworld == 3
  template <>
  inline ALBERTA EL_INFO * AlbertaGridLevelIterator<2,3,3,All_Partition>::
  goNextEntity(ALBERTA TRAVERSE_STACK *stack,ALBERTA EL_INFO *elinfo_old)
  {
    return goNextEdge(stack,elinfo_old);
  }

  // specialization for codim == dim , go next vertex
  template <>
  inline ALBERTA EL_INFO * AlbertaGridLevelIterator<2,2,2,All_Partition>::
  goNextEntity(ALBERTA TRAVERSE_STACK *stack,ALBERTA EL_INFO *elinfo_old)
  {
    return goNextVertex(stack,elinfo_old);
  }
  template <>
  inline ALBERTA EL_INFO * AlbertaGridLevelIterator<2,2,3,All_Partition>::
  goNextEntity(ALBERTA TRAVERSE_STACK *stack,ALBERTA EL_INFO *elinfo_old)
  {
    return goNextVertex(stack,elinfo_old);
  }
  template <>
  inline ALBERTA EL_INFO * AlbertaGridLevelIterator<3,3,3,All_Partition>::
  goNextEntity(ALBERTA TRAVERSE_STACK *stack,ALBERTA EL_INFO *elinfo_old)
  {
    return goNextVertex(stack,elinfo_old);
  }
  // end specialization of goNextEntity
  //***************************************

  template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
  inline void AlbertaGridLevelIterator<codim,dim,dimworld,pitype>::
  makeIterator()
  {
    level_ = 0;
    vertex_ = 0;
    face_ = 0;
    edge_ = 0;
    vertexMarker_ = 0;

    virtualEntity_.setTraverseStack(0);
    virtualEntity_.setElInfo(0,0,0,0,0);
  }

  // Make LevelIterator with point to element from previous iterations
  template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
  inline AlbertaGridLevelIterator<codim,dim,dimworld,pitype>::
  AlbertaGridLevelIterator(AlbertaGrid<dim,dimworld> &grid, int travLevel,
                           int proc, bool leafIt ) :
    grid_(grid), level_ (travLevel) ,
    virtualEntity_(grid,travLevel)
    ,leafIt_(leafIt) , proc_(proc)
  {
    makeIterator();
  }

  // Make LevelIterator with point to element from previous iterations
  template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
  inline AlbertaGridLevelIterator<codim,dim,dimworld,pitype>::
  AlbertaGridLevelIterator(AlbertaGrid<dim,dimworld> &grid, int level,
                           ALBERTA EL_INFO *elInfo,int elNum,int face,int edge,int vertex) :
    grid_(grid), level_ (level)
    , virtualEntity_(grid,level) , elNum_ ( elNum ) , face_ ( face ) ,
    edge_ ( edge ), vertex_ ( vertex ) , leafIt_(false) ,
    proc_(-1)
  {
    vertexMarker_ = 0;
    virtualEntity_.setTraverseStack(0);

    if(elInfo)
    {
      // diese Methode muss neu geschrieben werden, da man
      // die ParentElement explizit speichern moechte.
      virtualEntity_.setElInfo(elInfo,elNum_,face_,edge_,vertex_);
    }
  }

  template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
  inline AlbertaGridLevelIterator<codim,dim,dimworld,pitype>::
  AlbertaGridLevelIterator(AlbertaGrid<dim,dimworld> &grid,
                           AlbertaMarkerVector * vertexMark,
                           int travLevel, int proc, bool leafIt)
    : grid_(grid) , level_ (travLevel), virtualEntity_(grid,travLevel)
      , leafIt_(leafIt), proc_(proc)
  {
    ALBERTA MESH * mesh = grid_.getMesh();

    if( mesh && ((travLevel >= 0) && (travLevel <= grid_.maxlevel())) )
    {
      elNum_  = 0;
      vertex_ = 0;
      face_   = 0;
      edge_   = 0;

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

      virtualEntity_.setElInfo(elInfo,elNum_,face_,edge_,vertex_);
    }
    else
    {
      // create empty iterator
      makeIterator();
    }
  };

  template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
  inline bool AlbertaGridLevelIterator<codim,dim,dimworld,pitype>::
  operator ==(const AlbertaGridLevelIterator<codim,dim,dimworld,pitype> &I) const
  {
    return (virtualEntity_.getElInfo() == I.virtualEntity_.getElInfo());
  }

  template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
  inline bool AlbertaGridLevelIterator<codim,dim,dimworld,pitype>::
  operator !=(const AlbertaGridLevelIterator<codim,dim,dimworld,pitype> & I) const
  {
    return (virtualEntity_.getElInfo() != I.virtualEntity_.getElInfo() );
  }


  // gehe zum naechsten Element, wie auch immer
  template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
  inline AlbertaGridLevelIterator<codim,dim,dimworld,pitype>&
  AlbertaGridLevelIterator<codim,dim,dimworld,pitype>::operator ++()
  {
    elNum_++;
    virtualEntity_.setElInfo(
      goNextEntity(manageStack_.getStack(),virtualEntity_.getElInfo()),
      elNum_,face_,edge_,vertex_);

    return (*this);
  }

  template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
  inline ALBERTA EL_INFO * AlbertaGridLevelIterator<codim,dim,dimworld,pitype>::
  goNextFace(ALBERTA TRAVERSE_STACK *stack, ALBERTA EL_INFO *elInfo)
  {
    // go next Element, if face_ > numberOfVertices, then go to next elInfo
    face_++;
    if(face_ >= (dim+1)) // dim+1 Faces
    {
      elInfo = goNextElInfo(stack, elInfo);
      face_ = 0;
    }

    if(!elInfo)
      return elInfo; // if no more Faces, return

    if( (elInfo->neigh[face_]) &&
        (grid_.getElementNumber(elInfo->el) > grid_.getElementNumber(elInfo->neigh[face_])))
    {
      // if reachedFace before, go next
      elInfo = goNextFace(stack,elInfo);
    }

    return elInfo;
  }

  template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
  inline ALBERTA EL_INFO * AlbertaGridLevelIterator<codim,dim,dimworld,pitype>::
  goNextEdge(ALBERTA TRAVERSE_STACK *stack, ALBERTA EL_INFO *elInfo)
  {
    DUNE_THROW(AlbertaError,"EdgeIterator not implemented for 3d!");
    return 0;
  }

  template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
  inline ALBERTA EL_INFO * AlbertaGridLevelIterator<codim,dim,dimworld,pitype>::
  goNextVertex(ALBERTA TRAVERSE_STACK *stack, ALBERTA EL_INFO *elInfo)
  {
    // go next Element, Vertex 0
    // treat Vertices like Faces
    vertex_++;
    if(vertex_ >= (dim+1)) // dim+1 Vertices
    {
      elInfo = goNextElInfo(stack, elInfo);
      vertex_ = 0;
    }

    if(!elInfo) return 0; // if no more Vertices, return

    // go next, if Vertex is not treated on this Element
    if(vertexMarker_->notOnThisElement(elInfo->el,
                                       grid_.getElementNumber(elInfo->el),level_,vertex_))
      elInfo = goNextVertex(stack,elInfo);

    return elInfo;
  }


  template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
  inline AlbertaGridEntity<codim,dim,dimworld> &
  AlbertaGridLevelIterator<codim,dim,dimworld,pitype>::operator *()
  {
    assert(virtualEntity_.getElInfo() != 0);
    return virtualEntity_;
  }

  template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
  inline AlbertaGridEntity< codim,dim,dimworld >*
  AlbertaGridLevelIterator<codim,dim,dimworld,pitype>::operator ->()
  {
    assert(virtualEntity_.getElInfo() != 0);
    return &virtualEntity_;
  }

  template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
  inline const AlbertaGridEntity<codim,dim,dimworld> &
  AlbertaGridLevelIterator<codim,dim,dimworld,pitype>::operator *() const
  {
    assert(virtualEntity_.getElInfo() != 0);
    return virtualEntity_;
  }

  template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
  inline const AlbertaGridEntity< codim,dim,dimworld >*
  AlbertaGridLevelIterator<codim,dim,dimworld,pitype>::operator ->() const
  {
    assert(virtualEntity_.getElInfo() != 0);
    return &virtualEntity_;
  }


  template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
  inline ALBERTA EL_INFO * AlbertaGridLevelIterator<codim,dim,dimworld,pitype>::
  goFirstElement(ALBERTA TRAVERSE_STACK *stack,ALBERTA MESH *mesh, int level,
                 ALBERTA FLAGS fill_flag)
  {
    FUNCNAME("goFirstElement");

    if (!stack)
    {
      ALBERTA_ERROR("no traverse stack\n");
      return(nil);
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

    // go to first enInfo, therefore goNextElInfo for all codims
    return(goNextElInfo(stack,0));
  }


  template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
  inline ALBERTA EL_INFO * AlbertaGridLevelIterator<codim,dim,dimworld,pitype>::
  goNextElInfo(ALBERTA TRAVERSE_STACK *stack, ALBERTA EL_INFO *elinfo_old)
  {
    FUNCNAME("goNextElInfo");
    ALBERTA EL_INFO       *elinfo = 0;

    if (stack->stack_used)
    {
      ALBERTA_TEST_EXIT(elinfo_old == stack->elinfo_stack+stack->stack_used)
        ("invalid old elinfo\n");
    }
    else
    {
      ALBERTA_TEST_EXIT(elinfo_old == nil) ("invalid old elinfo != nil\n");
    }

    switch (pitype)
    {
    // walk only over macro_elements that belong to this processor
    case Interior_Partition :
    {
      // overloaded traverse_leaf_el_level, is not implemened in ALBERTA yet
      elinfo = traverseElLevelInteriorBorder(stack);

      // if leafIt_ == false go to elements only on desired level
      if((elinfo) && (!leafIt_))
      {
        if(elinfo->level == stack->traverse_level)
          okReturn_ = true;

        while(!okReturn_)
        {
          elinfo = traverseElLevelInteriorBorder(stack);
          if(!elinfo) okReturn_ = true;
        }
        stack->el_count++;
      }
      return(elinfo);
    }

    // Walk over all macro_elements on this grid
    case All_Partition :
    {
      // overloaded traverse_leaf_el_level, is not implemened in ALBERTA yet
      elinfo = traverseElLevel(stack);

      // if leafIt_ == false go to elements only on desired level
      if((elinfo) && (!leafIt_))
      {
        if(elinfo->level == stack->traverse_level)
          okReturn_ = true;

        while(!okReturn_)
        {
          elinfo = traverseElLevel(stack);
          if(!elinfo) okReturn_ = true;
        }
        stack->el_count++;
      }
      return(elinfo);
    }

    // walk over ghost elements, if proc == -1 then over all ghosts
    case Ghost_Partition :
    {
      // overloaded traverse_leaf_el_level, is not implemened in ALBERTA yet
      elinfo = traverseElLevelGhosts(stack);

      // if leafIt_ == false go to elements only on desired level
      if((elinfo) && (!leafIt_))
      {
        if(elinfo->level == stack->traverse_level)
          okReturn_ = true;

        while(!okReturn_)
        {
          elinfo = traverseElLevelGhosts(stack);
          if(!elinfo) okReturn_ = true;
        }
        stack->el_count++;
      }
      return(elinfo);
    }

    // walk over interior elements which have ghosts as neighbour
    case InteriorBorder_Partition :
    {
      DUNE_THROW(AlbertaError, "AlbertaGridLevelIterator::goNextEntity: Unsupported IteratorType!");
      // overloaded traverse_leaf_el_level, is not implemened in ALBERTA yet
      elinfo = traverseElLevelInteriorBorder(stack);

      // if leafIt_ == false go to elements only on desired level
      if((elinfo) && (!leafIt_))
      {
        if(elinfo->level == stack->traverse_level)
          okReturn_ = true;

        while(!okReturn_)
        {
          elinfo = traverseElLevelInteriorBorder(stack);
          if(!elinfo) okReturn_ = true;
        }
        stack->el_count++;
      }

      if(elinfo)
      {
        // here we have the interior element, now check the neighbours
        for(int i=0; i<dim+1; i++)
        {
          ALBERTA EL * neigh = NEIGH(elinfo->el,elinfo)[i];
          if(neigh)
          {
            if(((proc_ == -1) && (grid_.getOwner(neigh) != grid_.myProcessor()))||
               ((proc_ != -1) && (grid_.getOwner(neigh) == proc_) ))
            {
              return elinfo;
            }
          }
        }
        // we found not the one, so go next
        return goNextElInfo(stack,elinfo);
      }

      return elinfo;
    }
    // default iterator type no supported
    default :
      DUNE_THROW(AlbertaError, "AlbertaGridLevelIterator::goNextEntity: Unsupported IteratorType!");
    } // end switch
  }


  template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
  inline ALBERTA EL_INFO * AlbertaGridLevelIterator<codim,dim,dimworld,pitype>::
  traverseElLevel(ALBERTA TRAVERSE_STACK *stack)
  {
    FUNCNAME("traverseElLevel");
    ALBERTA EL *el;
    int i;
    okReturn_ = false;

    if (stack->stack_used == 0) /* first call */
    {
      stack->traverse_mel = stack->traverse_mesh->first_macro_el;
      if (stack->traverse_mel == nil) return(nil);

      stack->stack_used = 1;

      ALBERTA fillMacroInfo(stack, stack->traverse_mel,
                            stack->elinfo_stack+stack->stack_used);

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

        stack->traverse_mel = stack->traverse_mel->next;
        if (stack->traverse_mel == nil) return(nil);

        stack->stack_used = 1;

        ALBERTA fillMacroInfo(stack, stack->traverse_mel,
                              stack->elinfo_stack+stack->stack_used);

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
    // Aenderung hier
    {
      if(stack->stack_used >= stack->stack_size-1)
        enlargeTraverseStack(stack);

      i = stack->info_stack[stack->stack_used];
      el = el->child[i];
      stack->info_stack[stack->stack_used]++;

      //ALBERTA fill_elinfo(i, stack->elinfo_stack+stack->stack_used,
      //               stack->elinfo_stack+stack->stack_used+1);
      grid_.fillElInfo(i, level_, stack->elinfo_stack+stack->stack_used,
                       stack->elinfo_stack+stack->stack_used+1, false , leafIt_);

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
  template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
  inline ALBERTA EL_INFO * AlbertaGridLevelIterator<codim,dim,dimworld,pitype>::
  traverseElLevelInteriorBorder(ALBERTA TRAVERSE_STACK *stack)
  {
    FUNCNAME("traverseElLevelInteriorBorder");
    ALBERTA EL *el;
    int i;
    okReturn_ = false;

    if (stack->stack_used == 0) /* first call */
    {
      ALBERTA MACRO_EL * mel = stack->traverse_mesh->first_macro_el;
      while(grid_.getOwner(mel->el) != grid_.myProcessor())
      {
        mel = mel->next;
        if(!mel) break;
      }
      stack->traverse_mel = mel;
      if (stack->traverse_mel == nil) return(nil);

      stack->stack_used = 1;

      ALBERTA fillMacroInfo(stack, stack->traverse_mel,
                            stack->elinfo_stack+stack->stack_used);

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
      if (stack->stack_used < 1)
      {
        ALBERTA MACRO_EL * mel = stack->traverse_mel->next;
        if(mel)
        {
          while(grid_.getOwner(mel->el) != grid_.myProcessor())
          {
            mel = mel->next;
            if(!mel) break;
          }
        }
        stack->traverse_mel = mel;
        if (stack->traverse_mel == nil) return(nil);

        stack->stack_used = 1;

        ALBERTA fillMacroInfo(stack, stack->traverse_mel,
                              stack->elinfo_stack+stack->stack_used);

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
    // Aenderung hier
    {
      if(stack->stack_used >= stack->stack_size-1)
        enlargeTraverseStack(stack);

      i = stack->info_stack[stack->stack_used];
      el = el->child[i];
      stack->info_stack[stack->stack_used]++;

      //ALBERTA fill_elinfo(i, stack->elinfo_stack+stack->stack_used,
      //              stack->elinfo_stack+stack->stack_used+1);
      grid_.fillElInfo(i, level_, stack->elinfo_stack+stack->stack_used,
                       stack->elinfo_stack+stack->stack_used+1, false, leafIt_);

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

  template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
  inline ALBERTA MACRO_EL * AlbertaGridLevelIterator<codim,dim,dimworld,pitype>::
  nextGhostMacro(ALBERTA MACRO_EL * oldmel)
  {
    ALBERTA MACRO_EL * mel = oldmel;
    if(mel)
    {
      int owner = grid_.getOwner(mel->el);
      while(((proc_ != -1) && (owner != proc_)) ||
            ((proc_ == -1) && (owner == grid_.myProcessor() )))
      {
        mel = mel->next;
        if(!mel) break;
        owner = grid_.getOwner(mel->el);
      }
    }
    return mel;
  }

  template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
  inline ALBERTA EL_INFO * AlbertaGridLevelIterator<codim,dim,dimworld,pitype>::
  traverseElLevelGhosts(ALBERTA TRAVERSE_STACK *stack)
  {
    FUNCNAME("traverseElLevel");
    ALBERTA EL *el;
    int i;
    okReturn_ = false;

    if (stack->stack_used == 0) /* first call */
    {
      stack->traverse_mel = nextGhostMacro(stack->traverse_mesh->first_macro_el);
      if (stack->traverse_mel == nil) return(nil);

      stack->stack_used = 1;

      ALBERTA fillMacroInfo(stack, stack->traverse_mel,
                            stack->elinfo_stack+stack->stack_used);

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
                              stack->elinfo_stack+stack->stack_used);

        stack->info_stack[stack->stack_used] = 0;

        el = stack->elinfo_stack[stack->stack_used].el;
        if ((el == nil) || (el->child[0] == nil))
        {
          return(stack->elinfo_stack+stack->stack_used);
        }
      }
    }

    /* go down tree until leaf oder level*/
    while (el->child[0] && (grid_.getOwner(el) >= 0) &&
           ( stack->traverse_level > (stack->elinfo_stack+stack->stack_used)->level))
    // Aenderung hier
    {
      if(stack->stack_used >= stack->stack_size-1)
        enlargeTraverseStack(stack);

      i = stack->info_stack[stack->stack_used];
      el = el->child[i];

      stack->info_stack[stack->stack_used]++;

      // go next possible element, if not ghost
      if( grid_.getOwner(el) < 0)
        return traverseElLevelGhosts(stack);

      //ALBERTA fill_elinfo(i, stack->elinfo_stack+stack->stack_used,
      //               stack->elinfo_stack+stack->stack_used+1);
      grid_.fillElInfo(i, level_, stack->elinfo_stack+stack->stack_used,
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

  template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
  inline int AlbertaGridLevelIterator<codim,dim,dimworld,pitype>::level() const
  {
    return (manageStack_.getStack())->stack_used;
  }

  //*************************************************************************
  //  end AlbertaGridLevelIterator
  //*************************************************************************


  template< int dim, int dimworld>
  inline AlbertaGridHierarchicIterator<dim,dimworld>
  AlbertaGridEntity < 0, dim ,dimworld >::hbegin(int maxlevel) const
  {
    // Kopiere alle Eintraege des stack, da man im Stack weiterlaeuft und
    // sich deshalb die Werte anedern koennen, der elinfo_stack bleibt jedoch
    // der gleiche, deshalb kann man auch nur nach unten, d.h. zu den Kindern
    // laufen
    AlbertaGridHierarchicIterator<dim,dimworld>
    it(grid_,travStack_,level(),maxlevel);
    return it;
  }

  template< int dim, int dimworld>
  inline AlbertaGridHierarchicIterator<dim,dimworld>
  AlbertaGridEntity < 0, dim ,dimworld >::hend(int maxlevel) const
  {
    AlbertaGridHierarchicIterator<dim,dimworld> it(grid_,level(),maxlevel);
    return it;
  }

  template< int dim, int dimworld>
  inline AlbertaGridIntersectionIterator<dim,dimworld>
  AlbertaGridEntity < 0, dim ,dimworld >::ibegin() const
  {
    AlbertaGridIntersectionIterator<dim,dimworld> it(grid_,level(),elInfo_);
    return it;
  }

  template< int dim, int dimworld>
  inline AlbertaGridIntersectionIterator<dim,dimworld>
  AlbertaGridEntity < 0, dim ,dimworld >::iend() const
  {
    AlbertaGridIntersectionIterator<dim,dimworld> it(grid_,level());
    return it;
  }

  template< int dim, int dimworld>
  inline void AlbertaGridEntity < 0, dim ,dimworld >::
  ibegin(AlbertaGridIntersectionIterator<dim,dimworld> &it) const
  {
    it.makeBegin( grid_ , level() , elInfo_ );
  }

  template< int dim, int dimworld>
  inline void AlbertaGridEntity < 0, dim ,dimworld >::
  iend(AlbertaGridIntersectionIterator<dim,dimworld> &it) const
  {
    it.makeEnd( grid_ , level() );
  }
  //*********************************************************************
  //
  //  AlbertMarkerVertex
  //
  //*********************************************************************
  inline bool AlbertaMarkerVector::
  notOnThisElement(ALBERTA EL * el, int elIndex, int level, int localNum)
  {
    return (vec_[level][ el->dof[localNum][0] ] != elIndex);
  }

  template <class GridType>
  inline void AlbertaMarkerVector::markNewVertices(GridType &grid)
  {
    enum { dim      = GridType::dimension };
    enum { dimworld = GridType::dimensionworld };

    ALBERTA MESH *mesh_ = grid.getMesh();

    int nvx = mesh_->n_vertices;
    // remember the number of vertices of the mesh
    numVertex_ = nvx;

    int maxlevel = grid.maxlevel();

    for(int level=0; level <= maxlevel; level++)
    {
      Array<int> & vec = vec_[level];

      if(vec.size() < nvx) vec.resize( nvx + vxBufferSize_ );

      for(int i=0; i<vec.size(); i++) vec[i] = -1;

      typedef typename GridType::template Traits<0>::LevelIterator LevelIteratorType;
      LevelIteratorType endit = grid.template lend<0> (level);
      for(LevelIteratorType it = grid.template lbegin<0> (level); it != endit; ++it)
      {
        for(int local=0; local<dim+1; local++)
        {
          int num = it->getElInfo()->el->dof[local][0]; // vertex num
          if( vec[num] == -1 ) vec[num] = it->global_index();
        }
      }
      // remember the number of entity on level and codim = 0
    }
  }

  inline void AlbertaMarkerVector::print()
  {
    for(int l=0; l<MAXL; l++)
    {
      if(vec_[l].size() > 0)
      {
        Array<int> & vec = vec_[l];
        printf("\nEntries %d \n",vec.size());
        for(int i=0; i<vec.size(); i++)
          printf("Vx %d visited on Element %d \n",i,vec[i]);
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
    , time_ (0.0) , hasLevelIndex_ (true)
    , nv_ (dim+1) , dof_ (0) , myProc_ (0)
  {
    vertexMarker_ = new AlbertaMarkerVector ();
    dofvecs_.elNumbers  = 0;
    dofvecs_.elNewCheck = 0;
    dofvecs_.owner      = 0;
  }

  template < int dim, int dimworld >
  inline void AlbertaGrid < dim, dimworld >::initGrid(int proc, bool swapEls )
  {
    ALBERTA AlbertHelp::getDofVecs(&dofvecs_);
    ALBERTA AlbertHelp::setDofVec ( dofvecs_.owner, -1 );

    // dont delete dofs on higher levels
    mesh_->preserve_coarse_dofs = 1;

    numberOfEntitys_[0]     = mesh_->n_hier_elements;
    numberOfEntitys_[1]     = 0;
    numberOfEntitys_[dim-1] = 0;
    numberOfEntitys_[dim]   = mesh_->n_vertices;

    // we have at least one level, level 0
    maxlevel_ = 0;
    maxHierIndex_[0] = mesh_->n_hier_elements;

    calcExtras();

    wasChanged_ = true;
    isMarked_ = false;

    macroVertices_.resize( mesh_->n_vertices );

    ALBERTA AlbertHelp::initProcessor(mesh_,proc);

    // if hasLevelIndex == true then first macro element hast the number 1
    // and not 0
    // this could be seen as a hack but is the only way i know to store
    // whether we have level index or not in the grid files , :(
    // default value of swapEls = true
    if(mesh_ && hasLevelIndex_ && swapEls )
    {
      ALBERTA MACRO_EL *mel = mesh_->first_macro_el;
      assert(mel != 0);
      ALBERTA EL * first_el = mel->el;

      mel = mel->next;
      if( !mel )
      {
        ALBERTA AlbertHelp::initIndexManager_elmem_cc(&(indexStack_[0]));
        ALBERTA AlbertHelp::swapElNum ( dofvecs_.elNumbers, first_el );
        ALBERTA AlbertHelp::removeIndexManager_elmem_cc();
      }
      else
      {
        ALBERTA EL * sec_el   = mel->el;
        assert(sec_el != 0);

        ALBERTA AlbertHelp::swapElNum ( dofvecs_.elNumbers, first_el , sec_el );
      }
      std::cerr << "AlbertaGrid: LevelIndex is used!\n";
    }
    else
    {
      hasLevelIndex_ = false;
    }
    calcExtras();
  }

  template < int dim, int dimworld >
  inline AlbertaGrid < dim, dimworld >::AlbertaGrid(const char *MacroTriangFilename, bool levInd) :
    mesh_ (0), maxlevel_ (0) , wasChanged_ (false)
    , isMarked_ (false)
    , time_ (0.0) , hasLevelIndex_ (levInd)
    , nv_ (dim+1) , dof_ (0) , myProc_ (-1)
  {
    assert(dimworld == DIM_OF_WORLD);
    assert(dim      == DIM);

    bool makeNew = true;
    {
      std::fstream file (MacroTriangFilename,std::ios::in);
      std::basic_string <char> str,str1;
      file >> str1; str = str1.assign(str1,0,3);
      // With that Albert MacroTriang starts DIM or DIM_OF_WORLD
      if (str != "DIM") makeNew = false;
      file.close();
    }

    vertexMarker_ = new AlbertaMarkerVector ();
    ALBERTA AlbertHelp::initIndexManager_elmem_cc(&(indexStack_[0]));

    if(makeNew)
    {
      mesh_ = ALBERTA get_mesh("AlbertaGrid", ALBERTA AlbertHelp::initDofAdmin,
                               ALBERTA AlbertHelp::initLeafData);
      ALBERTA read_macro(mesh_, MacroTriangFilename, ALBERTA AlbertHelp::initBoundary);

      initGrid(0);
    }
    else
    {
      GrapeDataIO < AlbertaGrid <dim,dimworld> > dataIO;
      dataIO.readGrid ( *this, MacroTriangFilename,time_,0);
    }
  }

  template < int dim, int dimworld >
  inline AlbertaGrid < dim, dimworld >::
  AlbertaGrid(AlbertaGrid<dim,dimworld> & oldGrid, int proc , bool levInd) :
    mesh_ (0), maxlevel_ (0) , wasChanged_ (false)
    , isMarked_ (false)
    , time_ (0.0) , hasLevelIndex_ (levInd)
    , nv_ (dim+1) , dof_ (0), myProc_ (proc)
  {
    assert(dimworld == DIM_OF_WORLD);
    assert(dim      == DIM);

    assert(dim == 2);

    ALBERTA MESH * oldMesh = oldGrid.getMesh();

    vertexMarker_ = new AlbertaMarkerVector ();
    ALBERTA AlbertHelp::initIndexManager_elmem_cc(&(indexStack_[0]));

    DUNE_THROW(AlbertaError,"To be revised!");
#if 0
    {
      ALBERTA MESH * fakeMesh = ALBERTA get_mesh("PartialGrid",
                                                 ALBERTA AlbertHelp::emptyDofAdmin, ALBERTA AlbertHelp::initLeafData);

      char fakename [1024];
      sprintf(fakename,"_tmp_fakemesh_%d_.macro",proc);

      int length = oldMesh->n_elements;
      int * ownvec = (int *) std::malloc(length * sizeof(int));

      {
        ALBERTA MACRO_DATA mdata;

        {
          int no = 0;
          typedef LeafIterator LeafIteratorType;

          LeafIteratorType it    = oldGrid.leafbegin( oldGrid.maxlevel () );
          LeafIteratorType endit = oldGrid.leafend  ( oldGrid.maxlevel () );

          for(; it != endit; ++it )
          {
            ownvec[no] = it->owner();
            no++;
          }
          assert(no == length);
        }

#ifdef _ALBERTA_H_
        ALBERTA write_macro(oldMesh,fakename);
        ALBERTA read_macro(fakeMesh,fakename,ALBERTA AlbertHelp::initBoundary);
#else
        ALBERTA AlbertHelp::mesh2macro_data( oldMesh , &mdata );

        if(!mdata.neigh) ALBERTA AlbertHelp::compute_neigh_fast(&mdata);

        // to be revised
        if(!mdata.boundary) ALBERTA AlbertHelp::dirichlet_boundary(&mdata);

        ALBERTA AlbertHelp::macro_data2mesh( fakeMesh , &mdata,
                                             ALBERTA AlbertHelp::initBoundary );
        ALBERTA AlbertHelp::free_macro_data ( &mdata );
#endif
      }

      ALBERTA AlbertHelp::storeMacroElements(fakeMesh);
      // make the partition, remove elements that not belong to processor proc
      ALBERTA AlbertHelp::removeMacroEls(fakeMesh, proc, ownvec);

      mesh_ = ALBERTA get_mesh("PartialGrid", ALBERTA AlbertHelp::initDofAdmin,
                               ALBERTA AlbertHelp::initLeafData);

      {

#ifdef _ALBERTAA_H_
        ALBERTA write_macro ( fakeMesh, fakename );
        ALBERTA read_macro  ( mesh_ , fakename, ALBERT AlbertHelp::initBoundary);
#else
        ALBERTA MACRO_DATA mdata;
        ALBERTA AlbertHelp::mesh2macro_data( fakeMesh , &mdata );

        if(!mdata.neigh) ALBERTA AlbertHelp::compute_neigh_fast(&mdata);

        // to be revised
        if(!mdata.boundary) ALBERTA AlbertHelp::dirichlet_boundary(&mdata);

        ALBERTA AlbertHelp::macro_data2mesh( mesh_ , &mdata,
                                             ALBERTA AlbertHelp::initBoundary );
        ALBERTA AlbertHelp::free_macro_data ( &mdata );
#endif
      }

      // restore the old mesh and free it
      ALBERTA AlbertHelp::resetMacroElements(fakeMesh);
      ALBERTA free_mesh(fakeMesh);

      // delete fake macro file
      //std::remove ( fakename );

      // setup AlbertaGrid Interface to Albert Mesh
      initGrid(proc);

      // make the original owners
      ALBERTA AlbertHelp::copyOwner(dofvecs_.owner,ownvec);
      std::free(ownvec); ownvec = 0;

    }
#endif
  }

  template < int dim, int dimworld >
  inline void AlbertaGrid < dim, dimworld >::removeMesh()
  {
    if(vertexMarker_) delete vertexMarker_;
    if(dofvecs_.elNumbers) ALBERTA free_dof_int_vec(dofvecs_.elNumbers);
    if(dofvecs_.elNewCheck) ALBERTA free_dof_int_vec(dofvecs_.elNewCheck);
    if(dofvecs_.owner ) ALBERTA free_dof_int_vec(dofvecs_.owner);

#if DIM == 3
    // because of bug in Alberta 1.2 , here until bug fixed
    if((maxlevel() <= 0) && mesh_ )
      RC_LIST_EL * rclist = ALBERTA get_rc_list(mesh_);
#endif
    if(mesh_) ALBERTA free_mesh(mesh_);
  }

  // Desctructor
  template < int dim, int dimworld >
  inline AlbertaGrid < dim, dimworld >::~AlbertaGrid()
  {
    removeMesh();
  }

  template < int dim, int dimworld > template<int codim, PartitionIteratorType pitype>
  inline AlbertaGridLevelIterator<codim,dim,dimworld,pitype>
  AlbertaGrid < dim, dimworld >::lbegin (int level, int proc)
  {
    AlbertaGridLevelIterator<codim,dim,dimworld,pitype> it(*this,vertexMarker_,level,proc);
    return it;
  }

  template < int dim, int dimworld > template<int codim, PartitionIteratorType pitype>
  inline AlbertaGridLevelIterator<codim,dim,dimworld,pitype>
  AlbertaGrid < dim, dimworld >::lend (int level, int proc )
  {
    AlbertaGridLevelIterator<codim,dim,dimworld,pitype> it((*this),level,proc);
    return it;
  }

  template < int dim, int dimworld > template<int codim>
  inline AlbertaGridLevelIterator<codim,dim,dimworld,All_Partition>
  AlbertaGrid < dim, dimworld >::lbegin (int level, int proc)
  {
    AlbertaGridLevelIterator<codim,dim,dimworld,All_Partition> it(*this,vertexMarker_,level,proc);
    return it;
  }

  template < int dim, int dimworld > template<int codim>
  inline AlbertaGridLevelIterator<codim,dim,dimworld,All_Partition>
  AlbertaGrid < dim, dimworld >::lend (int level, int proc )
  {
    AlbertaGridLevelIterator<codim,dim,dimworld,All_Partition> it((*this),level,proc);
    return it;
  }

  //**********************************************
  // the const versions of LevelIterator
  //**********************************************
  template < int dim, int dimworld > template<int codim, PartitionIteratorType pitype>
  inline typename AlbertaGrid < dim, dimworld > :: template ConstAlbertaGridLevelIterator<codim,pitype> :: IteratorType
  AlbertaGrid < dim, dimworld >::lbegin (int level, int proc) const
  {
    // const_cast ok, because constness of object is preserved via const iterator
    AlbertaGrid<dim,dimworld> & mygrid = const_cast<AlbertaGrid<dim,dimworld> &> (*this);
    AlbertaGridLevelIterator<codim,dim,dimworld,pitype> it( mygrid ,vertexMarker_,level,proc);
    typename ConstAlbertaGridLevelIterator<codim,pitype> :: IteratorType cit ( it );
    return cit;
  }

  template < int dim, int dimworld > template<int codim, PartitionIteratorType pitype>
  inline typename AlbertaGrid < dim, dimworld >::template ConstAlbertaGridLevelIterator<codim,pitype> :: IteratorType
  AlbertaGrid < dim, dimworld >::lend (int level, int proc ) const
  {
    // const_cast ok, because constness of object is preserved via const iterator
    AlbertaGrid<dim,dimworld> & mygrid = const_cast<AlbertaGrid<dim,dimworld> &> (*this);
    AlbertaGridLevelIterator<codim,dim,dimworld,pitype> it( mygrid ,level,proc);
    typename ConstAlbertaGridLevelIterator<codim,pitype> :: IteratorType cit ( it );
    return cit;
  }

  template < int dim, int dimworld > template<int codim>
  inline typename AlbertaGrid < dim, dimworld >:: template ConstAlbertaGridLevelIterator<codim,All_Partition> :: IteratorType
  AlbertaGrid < dim, dimworld >::lbegin (int level, int proc) const
  {
    // const_cast ok, because constness of object is preserved via const iterator
    AlbertaGrid<dim,dimworld> & mygrid = const_cast<AlbertaGrid<dim,dimworld> &> (*this);
    AlbertaGridLevelIterator<codim,dim,dimworld,All_Partition> it( mygrid ,vertexMarker_,level,proc);
    typename ConstAlbertaGridLevelIterator<codim,All_Partition> :: IteratorType cit ( it );
    return cit;
  }

  template < int dim, int dimworld > template<int codim>
  inline typename AlbertaGrid < dim, dimworld >:: template ConstAlbertaGridLevelIterator<codim,All_Partition> :: IteratorType
  AlbertaGrid < dim, dimworld >::lend (int level, int proc ) const
  {
    // const_cast ok, because constness of object is preserved via const iterator
    AlbertaGrid<dim,dimworld> & mygrid = const_cast<AlbertaGrid<dim,dimworld> &> (*this);
    AlbertaGridLevelIterator<codim,dim,dimworld,All_Partition> it( mygrid ,level,proc);
    typename ConstAlbertaGridLevelIterator<codim,All_Partition> :: IteratorType cit ( it );
    return cit;
  }

  //*****************************************************************
  template < int dim, int dimworld >
  inline AlbertaGridLevelIterator<0,dim,dimworld,All_Partition>
  AlbertaGrid < dim, dimworld >::leafbegin (int level, int proc )
  {
    bool leaf = true;
    AlbertaGridLevelIterator<0,dim,dimworld,All_Partition>
    it(*this,vertexMarker_,level,proc,leaf);
    return it;
  }

  template < int dim, int dimworld >
  inline AlbertaGridLevelIterator<0,dim,dimworld,All_Partition>
  AlbertaGrid < dim, dimworld >::leafend (int level, int proc )
  {
    bool leaf = true;
    AlbertaGridLevelIterator<0,dim,dimworld,All_Partition> it((*this),level,proc,leaf);
    return it;
  }


  //**************************************
  //  refine and coarsen methods
  //**************************************
  template < int dim, int dimworld >
  inline bool AlbertaGrid < dim, dimworld >::
  globalRefine(int refCount)
  {
    typedef LeafIterator LeafIt;
    LeafIt endit = leafend(maxlevel());

    for(int i=0; i<refCount; i++)
    {
      // mark all interior elements
      for(LeafIt it = leafbegin(maxlevel()); it != endit; ++it)
      {
        (*it).mark(refCount);
      }

      // mark all ghosts
      for(LeafIt it = leafbegin(maxlevel(),Ghost_Partition); it != endit; ++it)
      {
        (*it).mark(refCount);
      }

      this->adapt();
      this->postAdapt();
    }
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
    return wasChanged_;
  }

  template < int dim, int dimworld >
  inline void AlbertaGrid < dim, dimworld >::setMark (bool isMarked)
  {
    isMarked_ = isMarked;
  }

  template < int dim, int dimworld >
  inline bool AlbertaGrid < dim, dimworld >::checkElNew (ALBERTA EL *el) const
  {
    // if element is new then entry in dofVec is 1
    return (elNewVec_[el->dof[dof_][nv_]] > 0);
  }

  template < int dim, int dimworld >
  inline bool AlbertaGrid < dim, dimworld >::setOwner (ALBERTA EL *el, int proc)
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

    // if processor number == myProcessor ==> InteriorEntity or BorderEntity
    if(owner == myProcessor())
    {
      for(int i=0; i<dim+1; i++)
      {
        ALBERTA EL * neigh = NEIGH(elinfo->el,elinfo)[i];
        if(neigh)
        {
          if(getOwner(neigh) != myProcessor())
            return BorderEntity;
        }
      }

      // if no GhostNeighbor, then we have real InteriorEntity
      return InteriorEntity;
    }

    // if processor number != myProcossor ==> GhostEntity
    if((owner >= 0) && (owner != myProcessor())) return GhostEntity;

    DUNE_THROW(AlbertaError, "Unsupported PartitionType");

    return OverlapEntity;
  }

  template < int dim, int dimworld >
  inline int AlbertaGrid < dim, dimworld >::getOwner (ALBERTA EL *el) const
  {
    // if element is new then entry in dofVec is 1
    return ownerVec_ [el->dof[dof_][nv_]];
  }

  template < int dim, int dimworld >
  inline bool AlbertaGrid < dim, dimworld >::adapt()
  {
    unsigned char flag;
    bool refined = false;
    wasChanged_ = false;

    // set global pointer to index manager in elmem.cc
    ALBERTA AlbertHelp::initIndexManager_elmem_cc(&(indexStack_[0]));
    ALBERTA AlbertHelp::clearDofVec ( dofvecs_.elNewCheck );

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
    ALBERTA AlbertHelp::removeIndexManager_elmem_cc();

    return refined;
  }

  template < int dim, int dimworld >
  inline int AlbertaGrid < dim, dimworld >::maxlevel() const
  {
    return maxlevel_;
  }

  template < int dim, int dimworld >
  inline int AlbertaGrid < dim, dimworld >::global_size (int codim) const
  {
    // at this moment only for codim=0 and codim=dim
    assert((codim == dim) || (codim == 0));
    return maxHierIndex_[codim];
  }

  template < int dim, int dimworld >
  inline int AlbertaGrid < dim, dimworld >::size (int level, int codim) const
  {
    return const_cast<AlbertaGrid<dim,dimworld> &> (*this).calcLevelSize(level,codim);
  }

  template < int dim, int dimworld >
  inline int AlbertaGrid < dim, dimworld >::calcLevelSize (int level, int codim)
  {
    enum { numCodim = dim+1 };
    int ind = (level * numCodim) + codim;

    if(size_[ind] == -1)
    {
      int numberOfElements = 0;

      if(codim == 0)
      {
        AlbertaGridLevelIterator<0,dim,dimworld,All_Partition> endit = lend<0>(level);
        for(AlbertaGridLevelIterator<0,dim,dimworld,All_Partition> it = lbegin<0>(level);
            it != endit; ++it)
          numberOfElements++;
      }
      if(codim == 1)
      {
        AlbertaGridLevelIterator<1,dim,dimworld,All_Partition> endit = lend<1>(level);
        for(AlbertaGridLevelIterator<1,dim,dimworld,All_Partition> it = lbegin<1>(level);
            it != endit; ++it)
          numberOfElements++;
      }
      if(codim == 2)
      {
        AlbertaGridLevelIterator<2,dim,dimworld,All_Partition> endit = lend<2>(level);
        for(AlbertaGridLevelIterator<2,dim,dimworld,All_Partition> it = lbegin<2>(level);
            it != endit; ++it)
          numberOfElements++;
      }

      if(codim == 3)
      {
        AlbertaGridLevelIterator<3,dim,dimworld,All_Partition> endit = lend<3>(level);
        for(AlbertaGridLevelIterator<3,dim,dimworld,All_Partition> it = lbegin<3>(level);
            it != endit; ++it)
          numberOfElements++;
      }

      size_[ind] = numberOfElements;
      return numberOfElements;
    }
    else
    {
      return size_[ind];
    }
  }

  template < int dim, int dimworld >
  inline void AlbertaGrid < dim, dimworld >::arrangeDofVec()
  {
    elNumVec_ = (dofvecs_.elNumbers)->vec;   assert(elNumVec_);
    elNewVec_ = (dofvecs_.elNewCheck)->vec;  assert(elNewVec_);
    ownerVec_ = (dofvecs_.owner)->vec;       assert(ownerVec_);
    elAdmin_ = dofvecs_.elNumbers->fe_space->admin;

    // see Albert Doc. , should stay the same
    const_cast<int &> (nv_)  = elAdmin_->n0_dof[CENTER];
    const_cast<int &> (dof_) = elAdmin_->mesh->node[CENTER];
  }


  template < int dim, int dimworld >
  inline int AlbertaGrid < dim, dimworld >::getElementNumber ( ALBERTA EL * el ) const
  {
    if(el)
      return elNumVec_[el->dof[dof_][nv_]];
    else
    {
      return -1;
    }
  };

  template < int dim, int dimworld >
  inline void AlbertaGrid < dim, dimworld >::calcExtras ()
  {
    arrangeDofVec ();

    // save number of old entities
    for(int i=0; i<dim+1; i++)
      oldNumberOfEntities_[i] = numberOfEntitys_[i];

    // calc new number of entities
    numberOfEntitys_[0]     = mesh_->n_hier_elements; // elements
    numberOfEntitys_[1]     = 1;                    // faces
    numberOfEntitys_[dim-1] = 1;                    // edges
    numberOfEntitys_[dim]   = mesh_->n_vertices;    // vertices

    maxHierIndex_[1]   = mesh_->n_edges;
    maxHierIndex_[dim] = mesh_->n_vertices;

    // determine new maxlevel and mark neighbours
    maxlevel_ = ALBERTA AlbertHelp::calcMaxLevelAndMarkNeighbours
                  ( mesh_, dofvecs_.elNumbers , maxHierIndex_[0] );

    // mark vertices on elements
    vertexMarker_->markNewVertices(*this);

    // map the indices
    markNew();

    // we have a new grid
    wasChanged_ = true;
  }

  template < int dim, int dimworld >  template <FileFormatType ftype>
  inline bool AlbertaGrid < dim, dimworld >::
  writeGrid (const char * filename, albertCtype time ) const
  {
    assert(ftype == xdr);
    return writeGridXdr (filename , time );
  }

  template < int dim, int dimworld >  template <FileFormatType ftype>
  inline bool AlbertaGrid < dim, dimworld >::readGrid (const char * filename, albertCtype &time )
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
  writeGridXdr (const char * filename, albertCtype time ) const
  {
    // strore element numbering to file
    char elnumfile[2048];
    sprintf(elnumfile,"%s_num",filename);
    ALBERTA write_dof_int_vec_xdr(dofvecs_.elNumbers,elnumfile);

    if(myProcessor() >= 0)
    {
      int val = -1;
      int entry = ALBERTA AlbertHelp::saveMyProcNum(dofvecs_.owner,myProcessor(),val);

      char ownerfile[2048];
      sprintf(ownerfile,"%s_own",filename);
      ALBERTA write_dof_int_vec_xdr(dofvecs_.owner,ownerfile);

      // set old value of owner vec
      dofvecs_.owner->vec[entry] = val;
    }

    // use write_mesh_xdr, but works not correctly
    return static_cast<bool> (ALBERTA write_mesh (mesh_ , filename, time) );
  }

  template < int dim, int dimworld >
  inline bool AlbertaGrid < dim, dimworld >::readGridXdr (const char * filename, albertCtype & time )
  {
    // use read_mesh_xdr, but works not correctly
    mesh_ = (ALBERTA read_mesh (filename, &time , ALBERTA AlbertHelp::initLeafData ,
                                ALBERTA AlbertHelp::initBoundary) );
    if (mesh_ == 0)
      DUNE_THROW(AlbertaIOError, "could not open grid file " << filename);

    // read element numbering from file
    char elnumfile[2048];
    sprintf(elnumfile,"%s_num",filename);
    dofvecs_.elNumbers  = ALBERTA read_dof_int_vec_xdr(elnumfile, mesh_ , 0 );

    // if owner file exists, read it
    {
      dofvecs_.owner = 0;
      FILE * file=0;
      char ownerfile[2048];
      sprintf(ownerfile,"%s_own",filename);
      file = fopen(ownerfile,"r");
      if(file)
      {
        fclose(file);
        dofvecs_.owner = ALBERTA read_dof_int_vec_xdr(ownerfile, mesh_ , 0 );
        const_cast<int &> (myProc_) = ALBERTA AlbertHelp::restoreMyProcNum(dofvecs_.owner);
      }
    }

    // make the rest of the dofvecs
    ALBERTA AlbertHelp::makeTheRest(&dofvecs_);

    arrangeDofVec();

    // if hasLevelIndex_ then number of first element is 1 and not 0
    // see initGrid ()
    hasLevelIndex_ = (getElementNumber( mesh_->first_macro_el->el ) == 1) ? true : false;

    // calc maxlevel and indexOnLevel and so on
    calcExtras();
    // set el_index of index manager to max element index
    indexStack_[0].setMaxIndex(maxHierIndex_[0]);

    return true;
  }

  template < int dim, int dimworld >
  inline bool AlbertaGrid < dim, dimworld >::readGridAscii
    (const char * filename, albertCtype & time )
  {
    removeMesh(); // delete all objects

    mesh_ = ALBERTA get_mesh("AlbertaGrid", ALBERTA AlbertHelp::initDofAdmin,
                             ALBERTA AlbertHelp::initLeafData);
    ALBERTA read_macro(mesh_, filename, ALBERTA AlbertHelp::initBoundary);

    if( !readParameter(filename,"Time",time) )
      time = 0.0;

    vertexMarker_ = new AlbertaMarkerVector (); assert(vertexMarker_);
    ALBERTA AlbertHelp::initIndexManager_elmem_cc(&(indexStack_[0]));

    initGrid(0,false);
    return true;
  }

  //! Index Mapping
  template < int dim, int dimworld >
  inline void AlbertaGrid < dim, dimworld >::
  makeNewSize(Array<int> &a, int newNumberOfEntries)
  {
    if(newNumberOfEntries > a.size())
    {
      a.resize(newNumberOfEntries);
    }
    for(int i=0; i<a.size(); i++) a[i] = -1;
  }

  template < int dim, int dimworld > template <int codim>
  inline int AlbertaGrid < dim, dimworld >::
  indexOnLevel(int globalIndex, int level) const
  {
    assert(hasLevelIndex_ == true);
    // level = 0 is not interesting for this implementation
    // +1, because if Entity is Boundary then globalIndex == -1
    // an therefore we add 1 and get Entry 0, which schould be -1
    if (globalIndex < 0)
      return globalIndex;
    else
      return levelIndex_[codim][level][globalIndex];
  }

  // create lookup table for indices of the elements
  template < int dim, int dimworld >
  inline void AlbertaGrid < dim, dimworld >::markNew()
  {
    int nElements = maxHierIndex_[0];
    int nVertices = mesh_->n_vertices;

    // make new size and set all levels to -1 ==> new calc
    if((maxlevel_+1)*(numCodim) > size_.size())
      makeNewSize(size_, 2*((maxlevel_+1)*numCodim));

    for(int l=0; l<=maxlevel_; l++)
    {
      if(nElements > levelIndex_[0][l].size())
        makeNewSize(levelIndex_[0][l], nElements);
    }

    // if hasLevelIndex_ == false then the LevelIndex should not be generated
    if(!hasLevelIndex_) return;

    // the easiest way, in Albert all elements have unique global element
    // numbers, therefore we make one big array from which we get with the
    // global unique number the local level number
    for(int level=0; level <= maxlevel_; level++)
    {
      typedef AlbertaGridLevelIterator<0,dim,dimworld,All_Partition> LevelIterator;
      int num = 0;
      LevelIterator endit = lend<0>(level);
      for(LevelIterator it = lbegin<0> (level); it != endit; ++it)
      {
        int no = it->el_index();

        levelIndex_[0][level][no] = num;
        num++;
      }
      // remember the number of entity on level and codim = 0
      size_[level*numCodim /* +0 */] = num;
    };

    for(int l=0; l<=maxlevel_; l++)
    {
      if(nVertices > levelIndex_[dim][l].size())
      {
        makeNewSize(levelIndex_[dim][l], nVertices);
      }
    }

    for(int level=0; level <= maxlevel_; level++)
    {
      typedef AlbertaGridLevelIterator<dim,dim,dimworld,All_Partition> LevelIterator;
      int num = 0;
      LevelIterator endit = lend<dim> (level);
      for(LevelIterator it = lbegin<dim> (level); it != endit; ++it)
      {
        int no = it->global_index();
        levelIndex_[dim][level][no] = num;
        num++;
      }
      // remember the number of entity on level and codim = 0
      size_[level*numCodim + dim] = num;
    };
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
    //************************************************************
    // calc the normal elinfo
#if 0
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
      const bool leafLevel = ((el->child[0] == 0) && (elinfo->level < actLevel));
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

#if 1
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
          neigh[i] = nil;
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
            mel->coord[i][j] -= trans_(j);
            mel->coord[i][j] /= scalar_;

            mel->coord[i][j] *= scalar;
            mel->coord[i][j] += trans(j);
          }
        }
      }
    }

    for (int i=0; i<dimworld; i++)
      trans_(i) = trans(i);

    scalar_ = scalar;
  }

} // namespace Dune
