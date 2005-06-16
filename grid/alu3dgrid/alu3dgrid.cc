// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DGRID_CC
#define DUNE_ALU3DGRID_CC

//#include "alu3dmappings.hh"

namespace Dune {

  //- Trilinear mapping (from alu3dmappings.hh)
  inline TrilinearMapping ::
  TrilinearMapping (const coord_t& x0, const coord_t& x1,
                    const coord_t& x2, const coord_t& x3,
                    const coord_t& x4, const coord_t& x5,
                    const coord_t& x6, const coord_t& x7)
    : p0(x0), p1(x1), p2(x2), p3(x3), p4(x4), p5(x5), p6(x6), p7(x7) {
    a [0][0] = p0 [0] ;
    a [0][1] = p0 [1] ;
    a [0][2] = p0 [2] ;
    a [1][0] = p1 [0] - p0 [0] ;
    a [1][1] = p1 [1] - p0 [1] ;
    a [1][2] = p1 [2] - p0 [2] ;
    a [2][0] = p2 [0] - p0 [0] ;
    a [2][1] = p2 [1] - p0 [1] ;
    a [2][2] = p2 [2] - p0 [2] ;
    a [3][0] = p4 [0] - p0 [0] ;
    a [3][1] = p4 [1] - p0 [1] ;
    a [3][2] = p4 [2] - p0 [2] ;
    a [4][0] = p3 [0] - p2 [0] - a [1][0] ;
    a [4][1] = p3 [1] - p2 [1] - a [1][1] ;
    a [4][2] = p3 [2] - p2 [2] - a [1][2] ;
    a [5][0] = p6 [0] - p4 [0] - a [2][0] ;
    a [5][1] = p6 [1] - p4 [1] - a [2][1] ;
    a [5][2] = p6 [2] - p4 [2] - a [2][2] ;
    a [6][0] = p5 [0] - p1 [0] - a [3][0] ;
    a [6][1] = p5 [1] - p1 [1] - a [3][1] ;
    a [6][2] = p5 [2] - p1 [2] - a [3][2] ;
    a [7][0] = p7 [0] - p5 [0] + p4 [0] - p6 [0] - p3 [0] + p1 [0] + a [2][0] ;
    a [7][1] = p7 [1] - p5 [1] + p4 [1] - p6 [1] - p3 [1] + p1 [1] + a [2][1] ;
    a [7][2] = p7 [2] - p5 [2] + p4 [2] - p6 [2] - p3 [2] + p1 [2] + a [2][2] ;
    return ;
  }

  inline TrilinearMapping :: TrilinearMapping (const TrilinearMapping & map)
    : p0(map.p0), p1(map.p1), p2(map.p2), p3(map.p3),
      p4(map.p4), p5(map.p5), p6(map.p6), p7(map.p7) {
    for (int i = 0 ; i < 8 ; i ++)
      for (int j = 0 ; j < 3 ; j ++)
        a [i][j] = map.a [i][j] ;
    return ;
  }

  inline FieldMatrix<double, 3, 3>
  TrilinearMapping::jacobianInverse(const coord_t& p) {
    inverse (p);
    return Dfi;
  }

  inline void TrilinearMapping ::
  map2world(const coord_t& p, coord_t& world) const {
    double x = p [0];
    double y = p [1];
    double z = p [2];
    double t3 = y * z ;
    double t8 = x * z ;
    double t13 = x * y ;
    double t123 = x * t3 ;
    world [0] = a [0][0] + a [1][0] * x + a [2][0] * y + a [3][0] * z + a [4][0] * t13 + a [5][0] * t3 + a [6][0] * t8 + a [7][0] * t123 ;
    world [1] = a [0][1] + a [1][1] * x + a [2][1] * y + a [3][1] * z + a [4][1] * t13 + a [5][1] * t3 + a [6][1] * t8 + a [7][1] * t123 ;
    world [2] = a [0][2] + a [1][2] * x + a [2][2] * y + a [3][2] * z + a [4][2] * t13 + a [5][2] * t3 + a [6][2] * t8 + a [7][2] * t123 ;
    return ;
  }

  inline void TrilinearMapping ::
  map2world(const double x1, const double x2,
            const double x3, coord_t& world ) const {
    coord_t map ;
    map [0] = x1 ;
    map [1] = x2 ;
    map [2] = x3 ;
    map2world (map, world) ;
    return ;
  }

  inline void TrilinearMapping :: linear(const coord_t& p ) {
    double x = p[0];
    double y = p[1];
    double z = p[2];
    // ? get rid of that  double t0 = .5 ;
    double t3 = y * z ;
    double t8 = x * z ;
    double t13 = x * y ;
    Df[2][0] = a[1][2] + y * a[4][2] + z * a[6][2] + t3 * a[7][2] ;
    Df[2][1] = a[2][2] + x * a[4][2] + z * a[5][2] + t8 * a[7][2] ;
    Df[1][2] = a[3][1] + y * a[5][1] + x * a[6][1] + t13 * a[7][1] ;
    Df[2][2] = a[3][2] + y * a[5][2] + x * a[6][2] + t13 * a[7][2] ;
    Df[0][0] = a[1][0] + y * a[4][0] + z * a[6][0] + t3 * a[7][0] ;
    Df[0][2] = a[3][0] + y * a[5][0] + x * a[6][0] + t13 * a[7][0] ;
    Df[1][0] = a[1][1] + y * a[4][1] + z * a[6][1] + t3 * a[7][1] ;
    Df[0][1] = a[2][0] + x * a[4][0] + z * a[5][0] + t8 * a[7][0] ;
    Df[1][1] = a[2][1] + x * a[4][1] + z * a[5][1] + t8 * a[7][1] ;

  }

  inline double TrilinearMapping :: det(const coord_t& point ) {
    //  Determinante der Abbildung f:[-1,1]^3 -> Hexaeder im Punkt point.
    linear (point) ;
    return (DetDf = Df.determinant());

  }

  inline void TrilinearMapping :: inverse(const coord_t& p ) {
    //  Kramer - Regel, det() rechnet Df und DetDf neu aus.
    double val = 1.0 / det(p) ;
    Dfi[0][0] = ( Df[1][1] * Df[2][2] - Df[1][2] * Df[2][1] ) * val ;
    Dfi[0][1] = ( Df[0][2] * Df[2][1] - Df[0][1] * Df[2][2] ) * val ;
    Dfi[0][2] = ( Df[0][1] * Df[1][2] - Df[0][2] * Df[1][1] ) * val ;
    Dfi[1][0] = ( Df[1][2] * Df[2][0] - Df[1][0] * Df[2][2] ) * val ;
    Dfi[1][1] = ( Df[0][0] * Df[2][2] - Df[0][2] * Df[2][0] ) * val ;
    Dfi[1][2] = ( Df[0][2] * Df[1][0] - Df[0][0] * Df[1][2] ) * val ;
    Dfi[2][0] = ( Df[1][0] * Df[2][1] - Df[1][1] * Df[2][0] ) * val ;
    Dfi[2][1] = ( Df[0][1] * Df[2][0] - Df[0][0] * Df[2][1] ) * val ;
    Dfi[2][2] = ( Df[0][0] * Df[1][1] - Df[0][1] * Df[1][0] ) * val ;
    return ;
  }

  inline void TrilinearMapping::world2map (const coord_t& wld , coord_t& map )
  {
    //  Newton - Iteration zum Invertieren der Abbildung f.
    double err = 10.0 * _epsilon ;
#ifndef NDEBUG
    int count = 0 ;
#endif
    map [0] = map [1] = map [2] = .0 ;
    do {
      coord_t upd ;
      map2world (map, upd) ;
      inverse (map) ;
      double u0 = upd [0] - wld [0] ;
      double u1 = upd [1] - wld [1] ;
      double u2 = upd [2] - wld [2] ;
      double c0 = Dfi [0][0] * u0 + Dfi [0][1] * u1 + Dfi [0][2] * u2 ;
      double c1 = Dfi [1][0] * u0 + Dfi [1][1] * u1 + Dfi [1][2] * u2 ;
      double c2 = Dfi [2][0] * u0 + Dfi [2][1] * u1 + Dfi [2][2] * u2 ;
      map [0] -= c0 ;
      map [1] -= c1 ;
      map [2] -= c2 ;
      err = fabs (c0) + fabs (c1) + fabs (c2) ;
      assert (count ++ < 1000) ;
    } while (err > _epsilon) ;
    return ;
  }

  //- Bilinear surface mapping
  inline BilinearSurfaceMapping ::
  BilinearSurfaceMapping (const coord3_t& x0, const coord3_t& x1,
                          const coord3_t& x2, const coord3_t& x3)
    : _p0 (x0), _p1 (x1), _p2 (x2), _p3 (x3) {
    _b [0][0] = _p0 [0] ;
    _b [0][1] = _p0 [1] ;
    _b [0][2] = _p0 [2] ;
    _b [1][0] = _p1 [0] - _p0 [0] ;
    _b [1][1] = _p1 [1] - _p0 [1] ;
    _b [1][2] = _p1 [2] - _p0 [2] ;
    _b [2][0] = _p2 [0] - _p0 [0] ;
    _b [2][1] = _p2 [1] - _p0 [1] ;
    _b [2][2] = _p2 [2] - _p0 [2] ;
    _b [3][0] = _p3 [0] - _p2 [0] - _b [1][0] ;
    _b [3][1] = _p3 [1] - _p2 [1] - _b [1][1] ;
    _b [3][2] = _p3 [2] - _p2 [2] - _b [1][2] ;
    _n [0][0] = _b [1][1] * _b [2][2] - _b [1][2] * _b [2][1] ;
    _n [0][1] = _b [1][2] * _b [2][0] - _b [1][0] * _b [2][2] ;
    _n [0][2] = _b [1][0] * _b [2][1] - _b [1][1] * _b [2][0] ;
    _n [1][0] = _b [1][1] * _b [3][2] - _b [1][2] * _b [3][1] ;
    _n [1][1] = _b [1][2] * _b [3][0] - _b [1][0] * _b [3][2] ;
    _n [1][2] = _b [1][0] * _b [3][1] - _b [1][1] * _b [3][0] ;
    _n [2][0] = _b [3][1] * _b [2][2] - _b [3][2] * _b [2][1] ;
    _n [2][1] = _b [3][2] * _b [2][0] - _b [3][0] * _b [2][2] ;
    _n [2][2] = _b [3][0] * _b [2][1] - _b [3][1] * _b [2][0] ;
    return ;
  }

  inline BilinearSurfaceMapping ::
  BilinearSurfaceMapping (const BilinearSurfaceMapping & m)
    : _p0(m._p0), _p1(m._p1), _p2(m._p2), _p3(m._p3) {
    {for (int i = 0 ; i < 4 ; i ++)
       for (int j = 0 ; j < 3 ; j ++ )
         _b [i][j] = m._b [i][j] ;}
    {for (int i = 0 ; i < 3 ; i ++)
       for (int j = 0 ; j < 3 ; j ++ )
         _n [i][j] = m._n [i][j] ;}
    return ;
  }

  inline void BilinearSurfaceMapping ::
  map2world (const coord2_t& map, coord3_t& wld) const {
    double x = map [0];
    double y = map [1];
    double xy = x * y ;
    wld[0] = _b [0][0] + x * _b [1][0] + y * _b [2][0] + xy * _b [3][0] ;
    wld[1] = _b [0][1] + x * _b [1][1] + y * _b [2][1] + xy * _b [3][1] ;
    wld[2] = _b [0][2] + x * _b [1][2] + y * _b [2][2] + xy * _b [3][2] ;
    return ;
  }

  inline void BilinearSurfaceMapping ::
  map2world (double x, double y, coord3_t& w) const {
    coord2_t p ;
    p [0] = x ;
    p [1] = y ;
    map2world (p,w) ;
    return ;
  }

  inline void BilinearSurfaceMapping ::
  normal (const coord2_t& map, coord3_t& normal) const {
    double x = map [0];
    double y = map [1];
    normal [0] = -(_n [0][0] + _n [1][0] * x + _n [2][0] * y);
    normal [1] = -(_n [0][1] + _n [1][1] * x + _n [2][1] * y);
    normal [2] = -(_n [0][2] + _n [1][2] * x + _n [2][2] * y);
    return ;
  }

  // Converter routines
  /*
     ALU3dGridElementType
     convertGeometryType2ALU3dGridElementType(GeometryType geo) {
     switch (geo) {
     case hexahedron:
      return hexa;
     case tetrahedron:
      return tetra;
     default:
      return error;
     }
     assert(false);
     return error;
     }

     GeometryType
     convertALU3dGridElementType2GeometryType(ALU3dGridElementType alu) {
     switch(alu) {
     case hexa:
      return hexahedron;
     case tetra:
      return tetrahedron;
     case mixed:
      return unknown;
     default:
      return unknown;
     }

     assert(false);
     return unknown;
     }
   */
  // singleton holding reference elements
  template<int dim, class GridImp>
  struct ALU3dGridReferenceGeometry
  {
    ALU3dGridMakeableGeometry<dim,dim,GridImp> refelem;
    ALU3dGridReferenceGeometry () : refelem (true) {};
  };

  inline const char * elType2Name( ALU3dGridElementType elType )
  {
    switch( elType )
    {
    case tetra  : return "Tetraeder";
    case hexa   : return "Hexaeder";
    case mixed  : return "Mixed";
    default     : return "Error";
    }
  }

  inline bool checkMacroGrid ( ALU3dGridElementType elType , const std::string filename )
  {
    std::fstream file (filename.c_str(),std::ios::in);
    if( file )
    {
      std::string str;
      file >> str;

      std::string cmp ("!");
      cmp += elType2Name( elType );

      if (str != cmp)
      {
        derr << "ALU3DGrid<" << elType2Name(elType) << "> tries to read MacroGridFile with < " << str
             << " >. Identifier should be < " << cmp << " >!\n";
        //abort();
      }

      file.close();
      return true;
    }
    else
    {
      derr << "Couldn't open macro grid file < " << filename << " > !\n";
      abort();
    }
    return false;
  }

  //--Grid
  //template <int dim, int dimworld, ALU3dGridElementType elType>
  //const ALU3dGridElementType
  //ALU3dGrid<dim, dimworld, elType>::elementType = elType;

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline ALU3dGrid<dim, dimworld, elType>::ALU3dGrid(const char* macroTriangFilename
#ifdef _ALU3DGRID_PARALLEL_
                                                     , MPI_Comm mpiComm
#endif
                                                     )
    : mygrid_ (0) , maxlevel_(0)
      , coarsenMarked_(0) , refineMarked_(0)
#ifdef _ALU3DGRID_PARALLEL_
      , mpAccess_(mpiComm) , myRank_( mpAccess_.myrank() )
#else
      ,  myRank_(-1)
#endif
      , hIndexSet_ (*this)
      , levelIndexSet_(0) , leafIndexSet_(0)
  {
    if( myRank_ <= 0 )
    {
      checkMacroGrid ( elType , macroTriangFilename );
    }

    mygrid_ = new ALU3DSPACE GitterImplType (macroTriangFilename
#ifdef _ALU3DGRID_PARALLEL_
                                             , mpAccess_
#endif
                                             );
    assert(mygrid_ != 0);

#ifdef _ALU3DGRID_PARALLEL_
    //loadBalance();
    __MyRank__ = mpAccess_.myrank();

    dverb << "************************************************\n";
    dverb << "Created grid on p=" << mpAccess_.myrank() << "\n";
    dverb << "************************************************\n";

#endif

    mygrid_->printsize();


    postAdapt();
    calcExtras();
    leafIndexSet_ = new LeafIndexSetType ( *this );
    std::cout << "Constructor of Grid finished!\n";
  }

#ifdef _ALU3DGRID_PARALLEL_
  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline ALU3dGrid<dim, dimworld, elType>::ALU3dGrid(MPI_Comm mpiComm)
    : mygrid_ (0) , maxlevel_(0)
      , coarsenMarked_(0) , refineMarked_(0)
      , mpAccess_(mpiComm) , myRank_( mpAccess_.myrank() )
      , hIndexSet_ (*this)
      , levelIndexSet_(0) , leafIndexSet_(0)
  {}
#else
  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline ALU3dGrid<dim, dimworld, elType>::ALU3dGrid(int myrank)
    : mygrid_ (0) , maxlevel_(0)
      , coarsenMarked_(0) , refineMarked_(0)
      , myRank_(myrank)
      , hIndexSet_ (*this)
      , levelIndexSet_(0) , leafIndexSet_(0)
  {}
#endif

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline ALU3dGrid<dim, dimworld, elType>::ALU3dGrid(const ALU3dGrid<dim, dimworld, elType> & g)
    : mygrid_ (0) , maxlevel_(0)
      , coarsenMarked_(0) , refineMarked_(0)
      , myRank_(-1)
      , hIndexSet_(*this) , levelIndexSet_(0), leafIndexSet_(0)
  {
    DUNE_THROW(ALU3dGridError,"Do not use copy constructor of ALU3dGrid! \n");
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline ALU3dGrid<dim, dimworld, elType> & ALU3dGrid<dim, dimworld, elType>::operator = (const ALU3dGrid<dim, dimworld, elType> & g)
  {
    DUNE_THROW(ALU3dGridError,"Do not use assignment operator of ALU3dGrid! \n");
    return (*this);
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline ALU3dGrid<dim, dimworld, elType>::~ALU3dGrid()
  {
    if(levelIndexSet_) delete levelIndexSet_;
    if(leafIndexSet_) delete leafIndexSet_;
    if(mygrid_) delete mygrid_;
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline int ALU3dGrid<dim, dimworld, elType>::size(int level, int codim) const
  {
    return levelIndexSet().size(level,codim);
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline void ALU3dGrid<dim, dimworld, elType>::updateStatus()
  {
    calcMaxlevel();
    calcExtras();
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline void ALU3dGrid<dim, dimworld, elType>::calcMaxlevel()
  {
    assert(mygrid_);
    maxlevel_ = 0;
    ALU3DSPACE BSLeafIteratorMaxLevel w (*mygrid_) ;
    for (w->first () ; ! w->done () ; w->next ())
    {
      if(w->item().level() > maxlevel_ ) maxlevel_ = w->item().level();
    }
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline void ALU3dGrid<dim, dimworld, elType>::calcExtras()
  {
    if(levelIndexSet_) (*levelIndexSet_).calcNewIndex();

    coarsenMarked_ = 0;
    refineMarked_  = 0;
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline int ALU3dGrid<dim, dimworld, elType>::global_size(int codim) const
  {
    // return actual size of hierarchical index set
    // this is always up to date
    // maxIndex is the largest index used + 1
    return (*mygrid_).indexManager(codim).getMaxIndex();
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline int ALU3dGrid<dim, dimworld, elType>::maxlevel() const
  {
    return maxlevel_;
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline ALU3DSPACE GitterImplType & ALU3dGrid<dim, dimworld, elType>::myGrid()
  {
    return *mygrid_;
  }

  // lbegin methods
  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <int cd, PartitionIteratorType pitype>
  inline typename ALU3dGrid<dim, dimworld, elType>::Traits::template codim<cd>::template partition<pitype>::LevelIterator
  ALU3dGrid<dim, dimworld, elType>::lbegin(int level) const {
    assert( level >= 0 );
    return ALU3dGridLevelIterator<cd,pitype,const MyType> (*this,level);
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <int cd, PartitionIteratorType pitype>
  inline typename ALU3dGrid<dim, dimworld, elType>::Traits::template codim<cd>::template partition<pitype>::LevelIterator
  ALU3dGrid<dim, dimworld, elType>::lend(int level) const {
    assert( level >= 0 );
    return ALU3dGridLevelIterator<cd,pitype,const MyType> (*this,level,true);
  }

  // lbegin methods
  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <int cd>
  inline typename ALU3dGrid<dim, dimworld, elType>::Traits::template codim<cd>::template partition<All_Partition>::LevelIterator
  ALU3dGrid<dim, dimworld, elType>::lbegin(int level) const {
    assert( level >= 0 );
    return ALU3dGridLevelIterator<cd,All_Partition,const MyType> (*this,level);
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <int cd>
  inline typename ALU3dGrid<dim, dimworld, elType>::Traits::template codim<cd>::template partition<All_Partition>::LevelIterator
  ALU3dGrid<dim, dimworld, elType>::lend(int level) const {
    assert( level >= 0 );
    return ALU3dGridLevelIterator<cd,All_Partition,const MyType> (*this,level,true);
  }

  // leaf methods
  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline typename ALU3dGrid<dim, dimworld, elType>::LeafIteratorType
  ALU3dGrid<dim, dimworld, elType>::leafbegin(int level, PartitionIteratorType pitype) const
  {
    assert( level >= 0 );
    return ALU3dGridLeafIterator<const MyType> ((*this),level,false,pitype);
  }
  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline typename ALU3dGrid<dim, dimworld, elType>::LeafIteratorType
  ALU3dGrid<dim, dimworld, elType>::leafend(int level, PartitionIteratorType pitype) const
  {
    assert( level >= 0 );
    return ALU3dGridLeafIterator<const MyType> ((*this),level,true,pitype);
  }

  // global refine
  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline bool ALU3dGrid<dim,dimworld, elType>::
  mark(int ref, typename Traits::template codim<0>::EntityPointer & ep )
  {
    return this->mark(ref,*ep);
  }

  // global refine
  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline bool ALU3dGrid<dim,dimworld, elType>::
  mark(int ref, const typename Traits::template codim<0>::Entity & ep )
  {
    bool marked = (this->template getRealEntity<0> (ep)).mark(ref);
    if(marked)
    {
      if(ref > 0) refineMarked_ ++ ;
      if(ref < 0) coarsenMarked_ ++ ;
    }
    return marked;
  }

  // global refine
  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline bool ALU3dGrid<dim, dimworld, elType>::globalRefine(int anzahl)
  {
    bool ref = false;
    for (; anzahl>0; anzahl--)
    {
      LeafIteratorType endit  = leafend   ( maxlevel() );
      for(LeafIteratorType it = leafbegin ( maxlevel() ); it != endit; ++it)
      {
        this->mark(1, (*it) );
      }
      ref = this->adapt();
      if(ref) this->postAdapt();
    }

    // important that loadbalance is called on each processor
    this->loadBalance();

    return ref;
  }

  // preprocess grid
  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline bool ALU3dGrid<dim, dimworld, elType>::preAdapt()
  {
    return (coarsenMarked_ > 0);
  }

  // adapt grid
  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline bool ALU3dGrid<dim, dimworld, elType>::adapt()
  {
#ifdef _ALU3DGRID_PARALLEL_
    bool ref = myGrid().dAdapt(); // adapt grid
#else
    bool ref = myGrid().adapt(); // adapt grid
#endif
    if(ref)
    {
      //maxlevel_++;
      calcMaxlevel();           // calculate new maxlevel
      calcExtras();               // reset size and things
    }
    return ref;
  }

  // adapt grid
  // --adapt
  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <class DofManagerType, class RestrictProlongOperatorType>
  inline bool ALU3dGrid<dim, dimworld, elType>::
  adapt(DofManagerType & dm, RestrictProlongOperatorType & rpo, bool verbose )
  {
    assert( ((verbose) ? (dverb << "ALU3dGrid :: adapt() new method called!\n", 1) : 1 ) );

    EntityImp f ( *this, this->maxlevel() );
    EntityImp s ( *this, this->maxlevel() );

    typedef AdaptiveLeafIdSetRestrictProlong < LeafIndexSetType > AdLeafIndexRPOpType;
    AdLeafIndexRPOpType adlfop ( *leafIndexSet_ );

    typedef typename DofManagerType :: IndexSetRestrictProlongType IndexSetRPType;
    typedef CombinedAdaptProlongRestrict < IndexSetRPType,RestrictProlongOperatorType > COType;
    COType tmprpop ( dm.indexSetRPop() , rpo );

    int defaultChunk = newElementsChunk_;
    int actChunk     = refineEstimate_ * refineMarked_;

    // guess how many new elements we get
    int newElements = std::max( actChunk , defaultChunk );
    ALU3DSPACE AdaptRestrictProlongImpl<ALU3dGrid<dim, dimworld, elType>, EntityImp, COType >
    rp(*this,f,s,tmprpop);

    dm.resizeMem( newElements );
    bool ref = myGrid().duneAdapt(rp); // adapt grid

    // if new maxlevel was claculated
    if(rp.maxlevel() >= 0) maxlevel_ = rp.maxlevel();
    assert( ((verbose) ? (dverb << "maxlevel = " << maxlevel_ << "!\n", 1) : 1 ) );

    if(ref)
    {
      calcMaxlevel();
      calcExtras(); // reset size and things
    }

    // check whether we have balance
    loadBalance(dm);
    dm.dofCompress();
    communicate(dm);

    postAdapt();
    assert( ((verbose) ? (dverb << "ALU3dGrid :: adapt() new method finished!\n", 1) : 1 ) );
    return ref;
  }


  // post process grid
  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline void ALU3dGrid<dim, dimworld, elType>::postAdapt()
  {
#ifndef _ALU3DGRID_PARALLEL_
    //  if(mpAccess_.nlinks() < 1)
    //#endif
    {
      maxlevel_ = 0;
      ALU3DSPACE BSLeafIteratorMaxLevel w ( myGrid() ) ;
      for (w->first () ; ! w->done () ; w->next ())
      {
        if(w->item().level() > maxlevel_ ) maxlevel_ = w->item().level();
        w->item ().resetRefinedTag();

        // note, resetRefinementRequest sets the request to coarsen
        w->item ().resetRefinementRequest();
      }
    }
    //#ifdef _ALU3DGRID_PARALLEL_
#else
    //  else
    {
      // we have to walk over all hierarchcy because during loadBalance
      // we get newly refined elements, which have to be cleared
      int fakeLevel = maxlevel_;
      maxlevel_ = 0;
      for(int l=0; l<= fakeLevel; l++)
      {
        {
          ALU3DSPACE ALU3dGridLevelIteratorWrapper<0> w ( *this, l ) ;
          for (w.first () ; ! w.done () ; w.next ())
          {
            if(w.item().level() > maxlevel_ ) maxlevel_ = w.item().level();
            w.item ().resetRefinedTag();
          }
        }
      }

      ALU3DSPACE BSLeafIteratorMaxLevel w ( myGrid() ) ;
      for (w->first () ; ! w->done () ; w->next ())
      {
        if(w->item().level() > maxlevel_ ) maxlevel_ = w->item().level();
        w->item ().resetRefinedTag();

        // note, resetRefinementRequest sets the request to coarsen
        w->item ().resetRefinementRequest();
      }
    }
#endif
  }

  template <int dim, int dimworld, ALU3dGridElementType elType> template <class T>
  inline T ALU3dGrid<dim, dimworld, elType>::globalMin(T val) const
  {
#ifdef _ALU3DGRID_PARALLEL_
    T ret = mpAccess_.gmin(val);
    return ret;
#else
    return val;
#endif
  }
  template <int dim, int dimworld, ALU3dGridElementType elType> template <class T>
  inline T ALU3dGrid<dim, dimworld, elType>::globalMax(T val) const
  {
#ifdef _ALU3DGRID_PARALLEL_
    T ret = mpAccess_.gmax(val);
    return ret;
#else
    return val;
#endif
  }
  template <int dim, int dimworld, ALU3dGridElementType elType> template <class T>
  inline T ALU3dGrid<dim, dimworld, elType>::globalSum(T val) const
  {
#ifdef _ALU3DGRID_PARALLEL_
    T sum = mpAccess_.gsum(val);
    return sum;
#else
    return val;
#endif
  }
  template <int dim, int dimworld, ALU3dGridElementType elType> template <class T>
  inline void ALU3dGrid<dim, dimworld, elType>::globalSum(T * send, int s , T * recv) const
  {
#ifdef _ALU3DGRID_PARALLEL_
    mpAccess_.gsum(send,s,recv);
    return ;
#else
    std::memcpy(recv,send,s*sizeof(T));
    return ;
#endif
  }


  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline bool ALU3dGrid<dim, dimworld, elType>::loadBalance()
  {
#ifdef _ALU3DGRID_PARALLEL_
    bool changed = myGrid().duneLoadBalance();
    if(changed)
    {
      dverb << "Grid was balanced on p = " << myRank() << std::endl;
      calcMaxlevel();             // calculate new maxlevel
      calcExtras();               // reset size and things
    }
    return changed;
#else
    return false;
#endif
  }

  // adapt grid
  template <int dim, int dimworld, ALU3dGridElementType elType> template <class DataCollectorType>
  inline bool ALU3dGrid<dim, dimworld, elType>::loadBalance(DataCollectorType & dc)
  {
#ifdef _ALU3DGRID_PARALLEL_
    EntityImp en ( *this, this->maxlevel() );

    ALU3DSPACE GatherScatterImpl< ALU3dGrid<dim, dimworld, elType>, EntityImp, DataCollectorType >
    gs(*this,en,dc);

    bool changed = myGrid().duneLoadBalance(gs);

    if(changed)
    {
      dverb << "Grid was balanced on p = " << myRank() << std::endl;
      calcMaxlevel();             // calculate new maxlevel
      calcExtras();               // reset size and things
    }
    return changed;
#else
    return false;
#endif
  }

  // adapt grid
  template <int dim, int dimworld, ALU3dGridElementType elType> template <class DataCollectorType>
  inline bool ALU3dGrid<dim, dimworld, elType>::communicate(DataCollectorType & dc)
  {
#ifdef _ALU3DGRID_PARALLEL_
    EntityImp en ( *this, this->maxlevel() );

    ALU3DSPACE GatherScatterImpl< ALU3dGrid<dim, dimworld, elType> , EntityImp ,
        DataCollectorType > gs(*this,en,dc);

    myGrid().duneExchangeData(gs);
    return true;
#else
    return false;
#endif
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <GrapeIOFileFormatType ftype>
  inline bool ALU3dGrid<dim, dimworld, elType>::
  writeGrid(const std::string filename, alu3d_ctype time ) const
  {
    ALU3DSPACE GitterImplType & mygrd = const_cast<ALU3dGrid<dim, dimworld, elType> &> (*this).myGrid();
    mygrd.duneBackup(filename.c_str());

    // write time and maxlevel
    {
      std::string extraName(filename);
      extraName += ".extra";
      std::ofstream out (extraName.c_str());
      if(out)
      {
        out.precision (16);
        out << time << " ";
        out << maxlevel_ << " ";
        out.close();
      }
      else
      {
        derr << "ALU3dGrid::writeGrid: couldn't open <" << extraName << ">! \n";
      }
    }
    return true;
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <GrapeIOFileFormatType ftype>
  inline bool ALU3dGrid<dim,dimworld, elType>::
  readGrid( const std::string filename, alu3d_ctype & time )
  {
    {
      typedef std::ostringstream StreamType;
      std::string mName(filename);
      mName += ".macro";
      const char * macroName = mName.c_str();

      { //check if file exists
        std::ifstream check ( macroName );
        if( !check )
          DUNE_THROW(ALU3dGridError,"cannot read file " << macroName << "\n");
        check.close();
      }

      mygrid_ = new ALU3DSPACE GitterImplType (macroName
#ifdef _ALU3DGRID_PARALLEL_
                                               , mpAccess_
#endif
                                               );
    }

    assert(mygrid_ != 0);
    myGrid().duneRestore(filename.c_str());

    {
      typedef std::ostringstream StreamType;
      StreamType eName;

      eName << filename;
      eName << ".extra";
      const char * extraName = eName.str().c_str();
      std::ifstream in (extraName);
      if(in)
      {
        in.precision (16);
        in  >> time;
        in  >> maxlevel_;
        in.close();
      }
      else
      {
        derr << "ALU3dGrid::readGrid: couldn't open <" << extraName << ">! \n";
      }
    }

    calcMaxlevel(); // calculate new maxlevel
    calcExtras();  // calculate indices

    return true;
  }

  // return Grid type
  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline GridIdentifier ALU3dGrid<dim, dimworld, elType>::type () const
  {
    return ALU3dGrid_Id;
  }


  template <class GridImp, int codim>
  struct ALU3dGridEntityFactory
  {
    typedef ALU3dGridMakeableEntity<codim,GridImp::dimension,const GridImp> EntityImp;
    template <class EntityProviderType>
    static ALU3dGridMakeableEntity<codim,GridImp::dimension,const GridImp> *
    getNewEntity (const GridImp & grid, EntityProviderType & ep, int level)
    {
      return new EntityImp( grid, level );
    }

    template <class EntityProviderType>
    static void freeEntity( EntityProviderType & ep, EntityImp * e )
    {
      if( e ) delete e;
    }
  };

  template <class GridImp>
  struct ALU3dGridEntityFactory<GridImp,0>
  {
    typedef ALU3dGridMakeableEntity<0,GridImp::dimension,const GridImp> EntityImp;
    template <class EntityProviderType>
    static ALU3dGridMakeableEntity<0,GridImp::dimension,const GridImp> *
    getNewEntity (const GridImp & grid, EntityProviderType & ep, int level)
    {
      return ep.getNewObjectEntity( grid, level);
    }

    template <class EntityProviderType>
    static void freeEntity( EntityProviderType & ep, EntityImp * e )
    {
      ep.freeObjectEntity( e );
    }
  };

  // return Grid type
  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <int cd>
  inline ALU3dGridMakeableEntity<cd,dim,const ALU3dGrid<dim,dimworld,elType> > *
  ALU3dGrid<dim,dimworld,elType>::getNewEntity (int level) const
  {
    return ALU3dGridEntityFactory<MyType,cd>::getNewEntity(*this,entityProvider_,level);
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <int cd>
  inline void ALU3dGrid<dim,dimworld,elType>::
  freeEntity (ALU3dGridMakeableEntity<cd,dim,const ALU3dGrid<dim,dimworld,elType> > * e) const
  {
    return ALU3dGridEntityFactory<MyType,cd>::freeEntity(entityProvider_, e);
  }

  /*************************************************************************
  #       ######  #    #  ######  #          #     #####  ######  #####
  #       #       #    #  #       #          #       #    #       #    #
  #       #####   #    #  #####   #          #       #    #####   #    #
  #       #       #    #  #       #          #       #    #       #####
  #       #        #  #   #       #          #       #    #       #   #
  ######  ######    ##    ######  ######     #       #    ######  #    #
  *************************************************************************/
  //--LevelIterator
  template<int codim, PartitionIteratorType pitype, class GridImp >
  inline ALU3dGridLevelIterator<codim,pitype,GridImp> ::
  ALU3dGridLevelIterator(const GridImp & grid, int level,bool end)
    : ALU3dGridEntityPointer<codim,GridImp> ( grid ,level,end)
      , index_(-1)
      , level_(level)
  {
    if(!end)
    {
      IteratorType * it = new IteratorType ( this->grid_ , level_ );
      iter_.store( it );

      (*iter_).first();
      if(!(*iter_).done())
      {
        assert((*iter_).size() > 0);
        index_=0;
        myEntity().reset( level_ );
        myEntity().setElement( (*iter_).item());
      }
    }
    else
      this->done();
  }

  template<int codim, PartitionIteratorType pitype, class GridImp >
  inline ALU3dGridLevelIterator<codim,pitype,GridImp> ::
  ALU3dGridLevelIterator(const ALU3dGridLevelIterator<codim,pitype,GridImp> & org )
    : ALU3dGridEntityPointer<codim,GridImp> ( org.grid_,org.level_,(org.index_ < 0) ? true : false )
      , index_( org.index_ )
      , level_( org.level_ )
      , iter_ ( org.iter_ )
  {
    if(index_ >= 0)
    {
      myEntity().reset( level_ );
      myEntity().setElement( (*iter_).item());
    }
  }

  template<int codim, PartitionIteratorType pitype, class GridImp >
  inline void ALU3dGridLevelIterator<codim,pitype,GridImp> :: increment ()
  {
    // if assertion is thrown then end iterator was forgotten or didnt stop
    assert(index_ >= 0);

    (*iter_).next();
    index_++;
    if ((*iter_).done())
    {
      index_ = -1;
      this->done();
      return ;
    }

    myEntity().setElement( (*iter_).item());
    return ;
  }

  //*******************************************************************
  //
  //  LEAFITERATOR
  //
  //--LeafIterator
  //*******************************************************************
  template<class GridImp>
  inline ALU3dGridLeafIterator<GridImp> ::
  ALU3dGridLeafIterator(const GridImp &grid, int level,
                        bool end, PartitionIteratorType pitype)
    : ALU3dGridEntityPointer <0,GridImp> ( grid,level,end)
      , index_(-1)
      , level_(level)
      , pitype_ (pitype)
  {
    if(!end)
    {
      IteratorType * it = new IteratorType ( this->grid_ , level_ );
      iter_.store( it );

      (*iter_).first();
      if(!(*iter_).done()) // else iterator empty
      {
        assert((*iter_).size() > 0);
        index_=0;
        myEntity().reset( level_ );
        myEntity().setElement( (*iter_).item() );
      }
    }
    else
      this->done();
  }

  template<class GridImp>
  inline ALU3dGridLeafIterator<GridImp> ::
  ALU3dGridLeafIterator(const ALU3dGridLeafIterator<GridImp> &org)
    : ALU3dGridEntityPointer <0,GridImp> ( org.grid_,org.level_,(org.index_ < 0) ? true : false )
      , index_(org.index_)
      , level_(org.level_)
      , iter_ ( org.iter_ )
      , pitype_(org.pitype_)
  {
    if(index_ >= 0)
    {
      myEntity().reset( level_ );
      myEntity().setElement( iter_->item() );
    }
  }

  template<class GridImp>
  inline void ALU3dGridLeafIterator<GridImp> :: increment ()
  {
    // if assertion is thrown then end iterator was forgotten or didnt stop
    assert(index_  >= 0);

    (*iter_).next();
    index_++;

    if((*iter_).done())
    {
      index_ = -1;
      this->done();
      return ;
    }

    myEntity().setElement( (*iter_).item());
    return ;
  }

  //*******************************************************************
  //
  //  --EntityPointer
  //  --EnPointer
  //
  //*******************************************************************
  template<int codim, class GridImp >
  inline ALU3dGridEntityPointer<codim,GridImp> ::
  ALU3dGridEntityPointer(const GridImp & grid, const MyHElementType &item)
    : grid_(grid)
      , entity_ ( grid_.template getNewEntity<codim> ( item.level() ) )
      , done_ (false)
  {
    assert( entity_ );
    (*entity_).setElement( const_cast<MyHElementType &> (item) );
  }

  template<int codim, class GridImp >
  inline ALU3dGridEntityPointer<codim,GridImp> ::
  ALU3dGridEntityPointer(const GridImp & grid, int level , bool done )
    : grid_(grid)
      , entity_ ( grid_.template getNewEntity<codim> (level) )
      , done_ (done)
  {
    (*entity_).reset( level );
  }

  template<int codim, class GridImp >
  inline ALU3dGridEntityPointer<codim,GridImp> ::
  ALU3dGridEntityPointer(const ALU3dGridEntityPointerType & org)
    : grid_(org.grid_)
      , entity_ ( grid_.template getNewEntity<codim> ( org.entity_->level() ) )
  {
    (*entity_).setEntity( *(org.entity_) );
  }

  template<int codim, class GridImp >
  inline ALU3dGridEntityPointer<codim,GridImp> ::
  ~ALU3dGridEntityPointer()
  {
    grid_.freeEntity( entity_ );
  }

  template<int codim, class GridImp >
  inline void ALU3dGridEntityPointer<codim,GridImp>::done ()
  {
    // sets entity pointer in the status of an end iterator
    (*entity_).removeElement();
    done_ = true;
  }

  template<int codim, class GridImp >
  inline bool ALU3dGridEntityPointer<codim,GridImp>::
  equals (const ALU3dGridEntityPointer<codim,GridImp>& i) const
  {
    return (((*entity_).equals(*(i.entity_))) && (done_ == i.done_));
  }

  template<int codim, class GridImp >
  inline typename ALU3dGridEntityPointer<codim,GridImp>::Entity &
  ALU3dGridEntityPointer<codim,GridImp>::dereference () const
  {
    assert(entity_);
    return (*entity_);
  }

  template<int codim, class GridImp >
  inline int ALU3dGridEntityPointer<codim,GridImp>::level () const
  {
    assert(entity_);
    return (*entity_).level();
  }

  /************************************************************************************
  #     #
  #     #     #    ######  #####      #     #####  ######  #####
  #     #     #    #       #    #     #       #    #       #    #
  #######     #    #####   #    #     #       #    #####   #    #
  #     #     #    #       #####      #       #    #       #####
  #     #     #    #       #   #      #       #    #       #   #
  #     #     #    ######  #    #     #       #    ######  #    #
  ************************************************************************************/
  // --HierarchicIterator
  template <class GridImp>
  inline ALU3dGridHierarchicIterator<GridImp> ::
  ALU3dGridHierarchicIterator(const GridImp & grid ,
                              const ALU3DSPACE HElementType & elem, int maxlevel ,bool end)
    : ALU3dGridEntityPointer<0,GridImp> ( grid, elem )
      , elem_(elem) , item_(0) , maxlevel_(maxlevel)
  {
    if (!end)
    {
      item_ = const_cast<ALU3DSPACE HElementType *> (this->elem_.down());
      if(item_)
      {
        // we have children and they lie in the disired level range
        if(item_->level() <= maxlevel_)
        {
          myEntity().reset( maxlevel_ );
          myEntity().setElement(*item_);
        }
        else
        { // otherwise do nothing
          item_ = 0;
          this->done();
        }
      }
      else
      {
        this->done();
      }
    }
  }

  template <class GridImp>
  inline ALU3dGridHierarchicIterator<GridImp> ::
  ALU3dGridHierarchicIterator(const ALU3dGridHierarchicIterator<GridImp> & org)
    : ALU3dGridEntityPointer<0,GridImp> (org.grid_,org.elem_)
      , elem_ (org.elem_) , item_(org.item_) , maxlevel_(org.maxlevel_)
  {
    if(item_)
    {
      myEntity().reset( maxlevel_ );
      myEntity().setElement(*item_);
    }
    else
      this->done();
  }

  template <class GridImp>
  inline ALU3DSPACE HElementType * ALU3dGridHierarchicIterator<GridImp>::
  goNextElement(ALU3DSPACE HElementType * oldelem )
  {
    // strategy is:
    // - go down as far as possible and then over all children
    // - then go to father and next and down again

    ALU3DSPACE HElementType * nextelem = oldelem->down();
    if(nextelem)
    {
      if(nextelem->level() <= maxlevel_)
        return nextelem;
    }

    nextelem = oldelem->next();
    if(nextelem)
    {
      if(nextelem->level() <= maxlevel_)
        return nextelem;
    }

    nextelem = oldelem->up();
    if(nextelem == &elem_) return 0;

    while( !nextelem->next() )
    {
      nextelem = nextelem->up();
      if(nextelem == &elem_) return 0;
    }

    if(nextelem) nextelem = nextelem->next();

    return nextelem;
  }

  template <class GridImp>
  inline void ALU3dGridHierarchicIterator<GridImp> :: increment ()
  {
    assert(item_   != 0);

    item_ = goNextElement( item_ );
    if(!item_)
    {
      this->done();
      return ;
    }

    myEntity().setElement(*item_);
    return ;
  }

  //************************************************************************
  //
  //  --ALU3dGridBoundaryEntity
  //  --BoundaryEntity
  //
  //************************************************************************
  template <class GridImp>
  inline ALU3dGridBoundaryEntity<GridImp>::ALU3dGridBoundaryEntity () :
    _geom (false) , _id(-1) {}

  template <class GridImp>
  inline int ALU3dGridBoundaryEntity<GridImp>::id () const
  {
    return _id;
  }

  template <class GridImp>
  inline bool ALU3dGridBoundaryEntity<GridImp>::hasGeometry () const
  {
    return false;
  }

  template <class GridImp>
  inline const typename ALU3dGridBoundaryEntity<GridImp>::Geometry &
  ALU3dGridBoundaryEntity<GridImp>::geometry () const
  {
    assert(hasGeometry());
    return _geom;
  }

  template <class GridImp>
  inline void ALU3dGridBoundaryEntity<GridImp>::setId ( int id )
  {
    _id = id;
  }

  /************************************************************************************
  ###
  #     #    #   #####  ######  #####    ####   ######   ####      #     #####
  #     ##   #     #    #       #    #  #       #       #    #     #       #
  #     # #  #     #    #####   #    #   ####   #####   #          #       #
  #     #  # #     #    #       #####        #  #       #          #       #
  #     #   ##     #    #       #   #   #    #  #       #    #     #       #
  ###    #    #     #    ######  #    #   ####   ######   ####      #       #
  ************************************************************************************/
  // --IntersectionIterator
  template<class GridImp>
  inline ALU3dGridIntersectionIterator<GridImp> ::
  ALU3dGridIntersectionIterator(const GridImp & grid,
                                ALU3DSPACE HElementType *el,
                                int wLevel,bool end)
    : ALU3dGridEntityPointer<0,GridImp> ( grid , wLevel, end ),
      nFaces_(el ? el->nFaces() : 0),
      walkLevel_ ( wLevel ),
      twist_(false)
  {
    if( !end )
    {
      numberInNeigh_ = -1;
      interSelfGlobal_ =
        this->grid_.geometryProvider_.getNewObjectEntity( this->grid_ ,wLevel );
      interSelfLocal_ =
        this->grid_.geometryProvider_.getNewObjectEntity(this->grid_, wLevel);
      interNeighLocal_ =
        this->grid_.geometryProvider_.getNewObjectEntity(this->grid_, wLevel);
      bndEntity_ =
        this->grid_.bndProvider_.getNewObjectEntity( this->grid_ , walkLevel_ );
      first(*el,wLevel);
    }
    else
    {
      last();
    }
  }

  template<class GridImp>
  inline void ALU3dGridIntersectionIterator<GridImp> :: resetBools () const
  {
    needSetup_   = true;
    initInterGl_ = false;
    initInterLocal_ = false;
  }

  template<class GridImp>
  inline void ALU3dGridIntersectionIterator<GridImp> ::
  first (ALU3DSPACE HElementType & elem, int wLevel)
  {
    item_  = static_cast<GEOElementType *> (&elem);
    index_ = 0;
    neigh_ = 0;
    ghost_ = 0;

    neighpair_.first  = 0;
    neighpair_.second = 0;

    // if needed more than once we spare the virtual funtion call
    isBoundary_ = getNeighPair(index_).first->isboundary();
    checkGhost();

    theSituation_ = ( (item_->level() < wLevel ) && item_->leaf() );
    daOtherSituation_ = false;

    resetBools();
  }

  template<class GridImp>
  inline void ALU3dGridIntersectionIterator<GridImp> :: last ()
  {
    // reset entity pointer for equality
    this->done();

    interSelfGlobal_ = 0; // * Resource leak ?
    interNeighLocal_ = 0;
    interSelfLocal_  = 0;
    bndEntity_ = 0;
    item_      = 0;
    index_     = nFaces_;
  }

  // copy constructor
  template<class GridImp>
  inline ALU3dGridIntersectionIterator<GridImp> ::
  ALU3dGridIntersectionIterator(const ALU3dGridIntersectionIterator<GridImp> & org)
    : ALU3dGridEntityPointer<0,GridImp> (org.grid_ , org.walkLevel_ , (org.item_) ? false : true ) ,
      nFaces_ (org.nFaces_)

  {
    if(org.item_) // else its a end iterator
    {
      walkLevel_      = org.walkLevel_;
      item_           = org.item_;
      neigh_          = org.neigh_;
      ghost_          = org.ghost_;
      index_          = org.index_;
      numberInNeigh_  = org.numberInNeigh_;
      theSituation_   = org.theSituation_;
      daOtherSituation_ = org.daOtherSituation_;
      isBoundary_     = org.isBoundary_; // isBoundary_ == true means no neighbour
      isGhost_        = org.isGhost_;
      needSetup_      = true;
      twist_          = org.twist_;
      initInterGl_    = false;
      interSelfGlobal_  = (org.interSelfGlobal_) ? this->grid_.geometryProvider_.getNewObjectEntity( this->grid_ , walkLevel_ ) : 0;
      initInterLocal_ = false;
      interSelfLocal_ = (org.interSelfLocal_) ?
                        this->grid_.geometryProvider_.getNewObjectEntity(this->grid_, walkLevel_) : 0;
      interNeighLocal_ = (org.interNeighLocal_) ?
                         this->grid_.geometryProvider_.getNewObjectEntity(this->grid_, walkLevel_) : 0;
      bndEntity_      = (org.bndEntity_) ? this->grid_.bndProvider_.getNewObjectEntity( this->grid_ , walkLevel_ ) : 0;
    }
    else
    {
      last();
    }
  }

  template<class GridImp>
  inline ALU3dGridIntersectionIterator<GridImp> :: ~ALU3dGridIntersectionIterator()
  {
    if(interSelfGlobal_) this->grid_.geometryProvider_.freeObjectEntity( interSelfGlobal_ );
    interSelfGlobal_ = 0;

    if(bndEntity_) this->grid_.bndProvider_.freeObjectEntity( bndEntity_ );
    bndEntity_ = 0;

    if(interSelfLocal_) this->grid_.geometryProvider_.freeObjectEntity( interSelfLocal_ );
    interSelfLocal_ = 0;

    if(interNeighLocal_) this->grid_.geometryProvider_.freeObjectEntity( interNeighLocal_ );
    interNeighLocal_ = 0;
  }


  template<class GridImp>
  inline void ALU3dGridIntersectionIterator<GridImp> :: checkGhost () const
  {
#ifdef _ALU3DGRID_PARALLEL_
    isGhost_ = false;
    ghost_   = 0;
    if(isBoundary_)
    {
      PLLBndFaceType * bnd =
        dynamic_cast<PLLBndFaceType *> (getNeighPair(index_).first);
      if(bnd->bndtype() == ALU3DSPACE ProcessorBoundary_t)
      {
        isBoundary_ = false;
        isGhost_ = true;
      }
    }
#else
    isGhost_ = false;
#endif
  }

  template<class GridImp>
  inline void ALU3dGridIntersectionIterator<GridImp> :: increment ()
  {
    assert(item_);

    if( neighpair_.first && theSituation_ && daOtherSituation_ )
    {
      neighpair_.first = neighpair_.first->next();
    }
    else
    {
      neighpair_.first = 0;
    }

    if( !neighpair_.first )
    {
      ++index_;
      //twist_ = (index_ == 3 || index_ == 4) ? true : false;
      // * overkill here: can't get here without neighpair_.first == 0
      neighpair_.first = 0;
    }

    if(index_ >= nFaces_)
    {
      // set iterator to end status
      // we cannot call last here, because last only is for end iterators
      this->done();
      return ;
    }

    // if faces are wrong oriented the assertion is thrown
    assert( getNeighPair(index_).first );

    // if needed more than once we spare the virtual funtion call
    isBoundary_ = getNeighPair(index_).first->isboundary();
    checkGhost();

    resetBools();
    return ;
  }

  // set new neighbor
  template<class GridImp>
  inline void ALU3dGridIntersectionIterator<GridImp> ::
  setNeighbor () const
  {
    assert( this->neighbor() );

    if(! neighpair_.first )
    {
      // get the face(index_)  of this element
      neighpair_ = getNeighFace(index_);
      assert(neighpair_.first);

      // the "situation" describes the case we are on a leaf element but the
      // walking level is deeper than our own level. This means the neighbour
      // can have a deeper level and therefore we have on this face not one
      // neighbour, we have all children as neighbours. So we go to the face
      // and then to the children of this face which are all faces of the
      // children on this face. If we went down then we are also allowed to
      // go next otherwise we are not allowed to go next which is described as
      // "da other situation"

      GEOFaceType * dwn = neighpair_.first->down();

#ifndef NDEBUG
      if( theSituation_ )
#endif
      if( theSituation_ && dwn )
      {
        neighpair_.first  = dwn;
        daOtherSituation_ = true;
      }
      else
      {
        daOtherSituation_ = false;
      }
    }

#ifdef _ALU3DGRID_PARALLEL_
    if(isGhost_)
    {
      assert( getNeighPair(index_).first->isboundary() );

      NeighbourPairType np = (neighpair_.second < 0) ?
                             (neighpair_.first->nb.front()) : (neighpair_.first->nb.rear());

      ghost_ = static_cast<PLLBndFaceType *> (np.first);
      numberInNeigh_ =
        ALU3dImplTraits<GridImp::elementType>::alu2duneFace(np.second);

      // if our level is smaller then the level of the real ghost then go one
      // level up and set the element
      if((*ghost_).ghostLevel() != (*ghost_).level())
      {
        assert(ghost_->ghostLevel() < ghost_->level());
        assert(ghost_->up());

        if(daOtherSituation_)
        {
          neighpair_ = getNeighFace(index_);
          daOtherSituation_ = false;
        }

        ghost_ = static_cast<PLLBndFaceType *> ( ghost_->up() );
        assert(ghost_->level() == ghost_->ghostLevel());
      }

      // old set ghost method
      (*(this->entity_)).setGhost( *ghost_ );

      // new ghost not supported for a moment
      //assert( ghost_->getGhost() );
      //(*(this->entity_)).setGhost( *(ghost_->getGhost()) );

      needSetup_ = false;
      neigh_ = 0;
      return;
    }
#endif
    assert(!isGhost_);

    // same as in method myneighbour of Tetra and Hexa in gitter_sti.hh
    // neighpair_.second is the twist of the face
    NeighbourPairType np = (neighpair_.second < 0) ?
                           (neighpair_.first->nb.front()) : (neighpair_.first->nb.rear());

    neigh_ = static_cast<GEOElementType *> (np.first);
    numberInNeigh_ =
      ALU3dImplTraits<GridImp::elementType>::alu2duneFace(np.second);

    assert(neigh_ != item_);
    assert(neigh_ != 0);

    (*(this->entity_)).setElement(*neigh_);
    ghost_ = 0;
    needSetup_ = false;
  }

  template<class GridImp>
  inline typename ALU3dGridIntersectionIterator<GridImp>::Entity &
  ALU3dGridIntersectionIterator<GridImp>::dereference () const
  {
    if(needSetup_) setNeighbor();

    if( daOtherSituation_ )
    {
      if( neigh_ )
        assert( neigh_->down() == 0 );
    }

    return ALU3dGridEntityPointer<0,GridImp>::dereference();
  }

  template<class GridImp>
  inline bool ALU3dGridIntersectionIterator<GridImp> :: boundary () const
  {
    return isBoundary_;
  }

  template<class GridImp>
  inline bool ALU3dGridIntersectionIterator<GridImp>::neighbor () const
  {
    return !(this->boundary());
  }

  template<class GridImp>
  inline int ALU3dGridIntersectionIterator<GridImp>::numberInSelf () const
  {
    return index_;
  }


  template <class GridImp>
  inline const typename ALU3dGridIntersectionIterator<GridImp>::LocalGeometry &
  ALU3dGridIntersectionIterator<GridImp>::intersectionSelfLocal() const {
    initLocals();
    return *interSelfLocal_;
  }

  template <class GridImp>
  inline void ALU3dGridIntersectionIterator<GridImp>::initLocals() const {
    if (!initInterLocal_) {
      initLocal(*item_, index_, *interSelfLocal_);
      if (!boundary()) {
        initLocal(*neigh_, numberInNeigh_, *interNeighLocal_);
      } else {
#ifdef _ALU3DGRID_PARALLEL_
        // * init with ghost (how to do that?)
#else
        // * this is temporary (what else should I do here?)
        //initLocal(*item_, index_, *interNeighLocal_);
#endif
      }
      initInterLocal_ = true;
    }
    return;
  }

  template<class GridImp>
  inline int ALU3dGridIntersectionIterator<GridImp>::numberInNeighbor () const
  {
    assert(item_ != 0);

    if(needSetup_) setNeighbor();
    return numberInNeigh_;
  }

  template <class GridImp>
  inline const typename ALU3dGridIntersectionIterator<GridImp>::LocalGeometry &
  ALU3dGridIntersectionIterator<GridImp>::intersectionNeighborLocal() const {
    assert(!boundary());

    initLocals();
    return *interNeighLocal_;
  }

  template<class GridImp>
  inline typename ALU3dGridIntersectionIterator<GridImp>::NormalType
  ALU3dGridIntersectionIterator<GridImp>::
  integrationOuterNormal(const FieldVector<alu3d_ctype, dim-1>& local) const
  {
    return this->outerNormal(local);
  }

  template<class GridImp>
  inline typename ALU3dGridIntersectionIterator<GridImp>::NormalType
  ALU3dGridIntersectionIterator<GridImp>::
  outerNormal(const FieldVector<alu3d_ctype, dim-1>& local) const
  {
    assert(item_ != 0);
    {
      NormalType outNormal;

      if( boundary() ) {
        // if boundary calc normal normal ;)
        calculateNormal(local, outNormal);
        //(*item_).outerNormal(index_, &(local[0]), &(outNormal[0]) );
      } else {
        if(needSetup_) setNeighbor();

        if (!daOtherSituation_) {
          calculateNormal(local, outNormal);
          //(*item_).outerNormal(index_, &(local[0]), &(outNormal[0]) );
        } else {
          if(neigh_)
          {
            calculateNormalNeighbor(local, outNormal);
            //(*neigh_).neighOuterNormal(numberInNeigh_, &(local[0]),
            //                           &(outNormal[0]));
          }
          else
          {
            assert(ghost_);
            assert(ghost_->level() != (*item_).level());

            calculateNormal(local, outNormal);
            // ghostpair_.second stores the twist of the face
            // multiply by 0.25 because the normal is scaled with the face
            // volume and we have a nonconformity here
            outNormal *= 0.25;
          }
        }
      }
      return outNormal;
    }
    assert(false);
    DUNE_THROW(ALU3dGridError,"Error in IntersectionIterator::outerNormal()! \n");
    NormalType tmp;
    return tmp;
  }

  template <>
  inline void ALU3dGridIntersectionIterator<const ALU3dGrid<3, 3, tetra> >::
  calculateNormal(const FieldVector<alu3d_ctype, 2>& local,
                  NormalType& result) const {
    ALU3DSPACE BSGridLinearSurfaceMapping
    LSM (
      (*item_).myvertex(ALU3dImplTraits<tetra>::dune2aluFace(index_),
                        0)->Point(),
      (*item_).myvertex(ALU3dImplTraits<tetra>::dune2aluFace(index_),
                        1)->Point(),
      (*item_).myvertex(ALU3dImplTraits<tetra>::dune2aluFace(index_),
                        2)->Point()
      );
    LSM.normal(result);
  }

  template <>
  inline void ALU3dGridIntersectionIterator<const ALU3dGrid<3, 3, tetra> >::
  calculateNormalNeighbor(const FieldVector<alu3d_ctype, 2>& local,
                          NormalType& result) const {
    ALU3DSPACE BSGridLinearSurfaceMapping LSM
    (
      (*neigh_).myvertex(ALU3dImplTraits<tetra>::dune2aluFace(numberInNeigh_),
                         2)->Point(),
      (*neigh_).myvertex(ALU3dImplTraits<tetra>::dune2aluFace(numberInNeigh_),
                         1)->Point(),
      (*neigh_).myvertex(ALU3dImplTraits<tetra>::dune2aluFace(numberInNeigh_),
                         0)->Point()
    );
    LSM.normal(result);
  }

  template <>
  inline void ALU3dGridIntersectionIterator<const ALU3dGrid<3, 3, hexa> >::
  calculateNormal(const FieldVector<alu3d_ctype, 2>& local,
                  NormalType& result) const {
    if (!initInterGl_) {
      this->intersectionGlobal();
    }
    interSelfGlobal_->realGeometry.biMap_->normal(local, result);
    if (twist_) {
      result *= -1.0;
    }
  }

  template <>
  inline void ALU3dGridIntersectionIterator<const ALU3dGrid<3, 3, hexa> >::
  calculateNormalNeighbor(const FieldVector<alu3d_ctype, 2>& local, NormalType& result) const {
    calculateNormal(local, result);
  }

  template<class GridImp>
  inline typename ALU3dGridIntersectionIterator<GridImp>::NormalType
  ALU3dGridIntersectionIterator<GridImp>::
  unitOuterNormal(const FieldVector<alu3d_ctype, dim-1>& local) const
  {
    NormalType unitOuterNormal_  = this->outerNormal(local);
    unitOuterNormal_ *= (1.0/unitOuterNormal_.two_norm());
    return unitOuterNormal_;
  }

  template<class GridImp>
  inline const typename ALU3dGridIntersectionIterator<GridImp>::Geometry &
  ALU3dGridIntersectionIterator<GridImp>::intersectionGlobal () const
  {
    if(initInterGl_)
    {
      assert( interSelfGlobal_ );
      return (*interSelfGlobal_);
    }

    if( boundary() )
    {
      assert( interSelfGlobal_ );
      NeighbourFaceType face = getNeighFace(index_);
      twist_ = (face.second < 0);
      initInterGl_ =
        interSelfGlobal_->buildGeom(*face.first);
      return (*interSelfGlobal_);
    }

    // in case of neighbor
    if( needSetup_ ) setNeighbor();

    assert( interSelfGlobal_ );
    twist_ = (neighpair_.second < 0);
    initInterGl_ =
      interSelfGlobal_->buildGeom(*neighpair_.first);
    return (*interSelfGlobal_);
  }


  template <class GridImp>
  inline void
  ALU3dGridIntersectionIterator<GridImp>::
  initLocal(const GEOElementType& item, int faceIdx,
            LocalGeometryImp& geo) const {

    // * OLD CODE
    //geo.buildGeom
    //(item.twist(ALU3dImplTraits<GridImp::elementType>::dune2aluFace(faceIdx)),
    //   faceIdx);
  }

  template <class GridImp>
  inline ALU3dImplTraits<tetra>::GEOFaceType&
  ALU3dGridIntersectionIterator<GridImp>::
  getFace(int index, Int2Type<tetra>) const {
    return *(item_->myhface3(ALU3dImplTraits<tetra>::dune2aluFace(index)));
  }

  template <class GridImp>
  inline ALU3dImplTraits<hexa>::GEOFaceType&
  ALU3dGridIntersectionIterator<GridImp>::getFace(int index, Int2Type<hexa>) const {
    return *(item_->myhface4(ALU3dImplTraits<hexa>::dune2aluFace(index)));
  }

  template<class GridImp>
  inline const typename ALU3dGridIntersectionIterator<GridImp>::BoundaryEntity &
  ALU3dGridIntersectionIterator<GridImp>::boundaryEntity () const
  {
    assert(boundary());
    BNDFaceType * bnd = dynamic_cast<BNDFaceType *> (getNeighPair(index_).first);
    int id = bnd->bndtype(); // id's are positive
    (*bndEntity_).setId( -id );
    return (*bndEntity_);
  }

  template <class GridImp>
  inline typename ALU3dGridIntersectionIterator<GridImp>::NeighbourPairType
  ALU3dGridIntersectionIterator<GridImp>::
  getNeighPair (int index) const {
    return item_->myneighbour
             (ALU3dImplTraits<GridImp::elementType>::dune2aluFace(index));
  }

  template <class GridImp>
  inline typename ALU3dGridIntersectionIterator<GridImp>::NeighbourFaceType
  ALU3dGridIntersectionIterator<GridImp>::
  getNeighFace (int index) const {
    return item_->myintersection
             (ALU3dImplTraits<GridImp::elementType>::dune2aluFace(index));
  }

  /************************************************************************************
  ######  #    #   #####     #     #####   #   #
  #       ##   #     #       #       #      # #
  #####   # #  #     #       #       #       #
  #       #  # #     #       #       #       #
  #       #   ##     #       #       #       #
  ######  #    #     #       #       #       #
  ************************************************************************************/
  // --0Entity
  template<int dim, class GridImp>
  inline ALU3dGridEntity<0,dim,GridImp> ::
  ALU3dGridEntity(const GridImp  &grid, int wLevel)
    : grid_(grid)
      , item_(0)
      , ghost_(0), isGhost_(false), geo_(false) , builtgeometry_(false)
      , walkLevel_ (wLevel)
      , glIndex_(-1), level_(-1)
      , geoInFather_ (false)
  {}

  template<int dim, class GridImp>
  inline void ALU3dGridEntity<0,dim,GridImp> ::
  removeElement ()
  {
    item_  = 0;
    ghost_ = 0;
  }

  template<int dim, class GridImp>
  inline void ALU3dGridEntity<0,dim,GridImp> ::
  reset (int walkLevel )
  {
    assert( walkLevel_ >= 0 );

    item_       = 0;
    ghost_      = 0;
    isGhost_    = false;
    builtgeometry_ = false;
    walkLevel_     = walkLevel;
    glIndex_    = -1;
    level_      = -1;
  }

  // works like assignment
  template<int dim, class GridImp>
  inline void
  ALU3dGridEntity<0,dim,GridImp> :: setEntity(const ALU3dGridEntity<0,dim,GridImp> & org)
  {
    item_          = org.item_;
    isGhost_       = org.isGhost_;
    ghost_         = org.ghost_;
    builtgeometry_ = false;
    index_         = org.index_;
    level_         = org.level_;
    walkLevel_     = org.walkLevel_;
    glIndex_       = org.glIndex_;
  }

  template<int dim, class GridImp>
  inline void
  ALU3dGridEntity<0,dim,GridImp> :: setElement(ALU3DSPACE HElementType & element)
  {
    item_= static_cast<IMPLElementType *> (&element);
    isGhost_ = false;
    ghost_ = 0;
    builtgeometry_=false;
    index_   = -1;
    level_   = (*item_).level();
    glIndex_ = (*item_).getIndex();
  }

  template<int dim, class GridImp>
  inline void
  ALU3dGridEntity<0,dim,GridImp> :: setGhost(ALU3DSPACE HElementType & element)
  {
    item_= static_cast<IMPLElementType *> (&element);
    isGhost_ = true;
    ghost_ = 0;
    builtgeometry_=false;
    index_   = -1;
    level_   = (*item_).level();
    glIndex_ = (*item_).getIndex();
  }

  template<int dim, class GridImp>
  inline void
  ALU3dGridEntity<0,dim,GridImp> :: setGhost(PLLBndFaceType & ghost)
  {
    item_    = 0;
    ghost_   = &ghost;
    isGhost_ = true;
    index_   = -1;
    glIndex_ = ghost.getIndex();
    level_   = ghost.level();
    builtgeometry_ = false;
  }

  template<int dim, class GridImp>
  inline int
  ALU3dGridEntity<0,dim,GridImp> :: level() const
  {
    return level_;
  }

  template<int dim, class GridImp>
  inline bool ALU3dGridEntity<0,dim,GridImp> ::
  equals (const ALU3dGridEntity<0,dim,GridImp> &org ) const
  {
    return ( (item_ == org.item_) && (ghost_ == org.ghost_) );
  }

  template<int dim, class GridImp>
  inline const typename ALU3dGridEntity<0,dim,GridImp>::Geometry &
  ALU3dGridEntity<0,dim,GridImp> :: geometry () const
  {
    assert((ghost_ != 0) || (item_ != 0));
#ifdef _ALU3DGRID_PARALLEL_
    if(!builtgeometry_)
    {
      if(item_)
        builtgeometry_ = geo_.buildGeom(*item_);
      else
      {
        assert(ghost_);
        builtgeometry_ = geo_.buildGhost(*ghost_);
      }
    }
#else
    if(!builtgeometry_) builtgeometry_ = geo_.buildGeom(*item_);
#endif
    return geo_;
  }

  template<int dim, class GridImp>
  inline const typename ALU3dGridEntity<0,dim,GridImp>::Geometry &
  ALU3dGridEntity<0,dim,GridImp> :: geometryInFather () const
  {
    const Geometry & vati   = (*this->father()).geometry();
    const Geometry & myself = this->geometry();

    for(int i=0; i<vati.corners(); i++)
      geoInFather_.getCoordVec(i) = vati.local( myself[i] );

    return geoInFather_;
  }

  template<int dim, class GridImp>
  inline int ALU3dGridEntity<0,dim,GridImp> :: index() const
  {
    const Entity en (*this);
    return grid_.levelIndexSet().index(en);
  }

  template<int dim, class GridImp>
  inline int ALU3dGridEntity<0,dim,GridImp> :: globalIndex() const
  {
    return glIndex_;
  }

  template<int dim, class GridImp>
  inline int ALU3dGridEntity<0,dim,GridImp> :: getIndex() const
  {
    return glIndex_;
  }

  //********* begin method subIndex ********************
  // partial specialisation of subIndex
  template <class IMPLElemType, int codim> struct IndexWrapper;

  // specialisation for vertices
  template <class IMPLElemType> struct IndexWrapper<IMPLElemType,3>
  {
    static inline int subIndex(const IMPLElemType &elem, int i)
    {
      return elem.myvertex(i)->getIndex();
    }
  };

  // specialisation for faces
  template <class IMPLElemType> struct IndexWrapper<IMPLElemType,1>
  {
    static inline int subIndex(const IMPLElemType &elem, int i)
    {
      return elem.myhface3(i)->getIndex();
    }
  };

  // specialisation for faces
  template <class IMPLElemType> struct IndexWrapper<IMPLElemType,2>
  {
    static inline int subIndex(const IMPLElemType &elem, int i)
    {
      dwarn << "method not tested yet. ! in:" << __FILE__ << " line:" << __LINE__ << "\n";
      if(i<3)
        return elem.myhface3(0)->myhedge1(i)->getIndex();
      else
        return elem.myhface3(i-2)->myhedge1(i-3)->getIndex();
    }
  };

  template<int dim, class GridImp>
  template<int cc>
  inline int ALU3dGridEntity<0,dim,GridImp> :: subIndex (int i) const
  {
    assert(cc == dim);
    assert(item_ != 0);
    typedef typename  ALU3dImplTraits<GridImp::elementType>::IMPLElementType IMPLElType;
    return IndexWrapper<IMPLElType,cc>::subIndex ( *item_ ,i);
  }
  //******** end method subIndex *************

  template <class GridImp, int dim, int cc> struct ALU3dGridCount {
    static int count () { return dim+1; }
  };
  template <class GridImp> struct ALU3dGridCount<GridImp,3,2> {
    static int count () { return 6; }
  };

  template<int dim, class GridImp>
  template<int cc>
  inline int ALU3dGridEntity<0,dim,GridImp> :: count () const
  {
    return ALU3dGridCount<GridImp,dim,cc>::count();
  }

  //******** begin method entity ******************
  template <class GridImp, int dim, int cd> struct SubEntities;

  // specialisation for faces
  template <class GridImp, int dim>
  struct SubEntities<GridImp,dim,1>
  {
    static typename ALU3dGridEntity<0,dim,GridImp> :: template codim<1>:: EntityPointer
    entity (const GridImp & grid, const typename ALU3dImplTraits<GridImp::elementType>::IMPLElementType & item, int i)
    {
      return ALU3dGridEntityPointer<1,GridImp> (grid, *(item.myhface3(i)) );
    }
  };

  // specialisation for edges
  template <class GridImp, int dim>
  struct SubEntities<GridImp,dim,2>
  {
    static typename ALU3dGridEntity<0,dim,GridImp> :: template codim<2>:: EntityPointer
    entity (const GridImp & grid, const typename ALU3dImplTraits<GridImp::elementType>::IMPLElementType & item, int i)
    {
      dwarn << "method not tested yet. ! in:" << __FILE__ << " line:" << __LINE__ << "\n";
      if(i<3)
      {
        return ALU3dGridEntityPointer<2,GridImp> (grid, (*(item.myhface3(0)->myhedge1(i))) );
      }
      else
      {
        return ALU3dGridEntityPointer<2,GridImp> (grid, (*(item.myhface3(i-2)->myhedge1(i-3))) );
      }
    }
  };

  // specialisation for vertices
  template <class GridImp, int dim>
  struct SubEntities<GridImp,dim,3>
  {
    static typename ALU3dGridEntity<0,dim,GridImp> :: template codim<3>:: EntityPointer
    entity (const GridImp & grid, const typename ALU3dImplTraits<GridImp::elementType>::IMPLElementType & item, int i)
    {
      return ALU3dGridEntityPointer<3,GridImp> (grid, (*(item.myvertex(i))) );
    }
  };

  template<int dim, class GridImp>
  template<int cc>
  inline typename ALU3dGridEntity<0,dim,GridImp> :: template codim<cc>:: EntityPointer
  ALU3dGridEntity<0,dim,GridImp> :: entity (int i) const
  {
    return SubEntities<GridImp,dim,cc>::entity(grid_,*item_,i);
  }

  //**** end method entity *********

  template<int dim, class GridImp>
  inline PartitionType ALU3dGridEntity<0,dim,GridImp> ::
  partitionType () const
  {
    return ((isGhost_) ?  GhostEntity : InteriorEntity);
  }

  template<int dim, class GridImp>
  inline bool ALU3dGridEntity<0,dim,GridImp> :: isLeaf() const
  {
    assert(item_ != 0);
    return (item_->down() == 0);
  }

  template<int dim, class GridImp>
  inline ALU3dGridHierarchicIterator<GridImp> ALU3dGridEntity<0,dim,GridImp> :: hbegin (int maxlevel) const
  {
    assert(item_ != 0);
    return ALU3dGridHierarchicIterator<GridImp>(grid_,*item_,maxlevel);
  }

  template<int dim, class GridImp>
  inline ALU3dGridHierarchicIterator<GridImp> ALU3dGridEntity<0,dim,GridImp> :: hend (int maxlevel) const
  {
    assert(item_ != 0);
    return ALU3dGridHierarchicIterator<GridImp> (grid_,*item_,maxlevel,true);
  }

  template<int dim, class GridImp>
  inline ALU3dGridIntersectionIterator<GridImp> ALU3dGridEntity<0,dim,GridImp> :: ibegin () const
  {
    assert(item_ != 0);
    return ALU3dGridIntersectionIterator<GridImp> (grid_,item_,walkLevel_);
  }

  template<int dim, class GridImp>
  inline ALU3dGridIntersectionIterator<GridImp> ALU3dGridEntity<0,dim,GridImp> :: iend () const
  {
    assert(item_ != 0);
    return ALU3dGridIntersectionIterator<GridImp> (grid_, 0 ,walkLevel_,true);
  }

  template<int dim, class GridImp>
  inline typename ALU3dGridEntity<0,dim,GridImp> :: EntityPointer
  ALU3dGridEntity<0,dim,GridImp> :: father() const
  {
    if(! item_->up() )
    {
      std::cerr << "ALU3dGridEntity<0," << dim << "," << dimworld << "> :: father() : no father of entity globalid = " << globalIndex() << "\n";
      return ALU3dGridEntityPointer<0,GridImp> (grid_, static_cast<ALU3DSPACE HElementType &> (*item_));
    }
    return ALU3dGridEntityPointer<0,GridImp> (grid_, static_cast<ALU3DSPACE HElementType &> (*(item_->up())));
  }

  // Adaptation methods
  template<int dim, class GridImp>
  inline bool ALU3dGridEntity<0,dim,GridImp> :: mark (int ref) const
  {
    // refine_element_t and coarse_element_t are defined in bsinclude.hh
    if(ghost_) return false;

    assert(item_ != 0);

    // if this assertion is thrown then you try to mark a non leaf entity
    // which is leads to unpredictable results
    assert( isLeaf() );

    // mark for coarsening
    if(ref < 0)
    {
      if(level() <= 0) return false;
      if((*item_).requestrule() == refine_element_t)
      {
        return false;
      }

      (*item_).request(coarse_element_t);
      return true;
    }

    // mark for refinement
    if(ref > 0)
    {
      (*item_).request(refine_element_t);
      return true;
    }

    (*item_).request( nosplit_element_t );
    return false;
  }


  // Adaptation methods
  template<int dim, class GridImp>
  inline AdaptationState ALU3dGridEntity<0,dim,GridImp> :: state () const
  {
    assert(item_ != 0);
    if((*item_).requestrule() == coarse_element_t)
    {
      return COARSEN;
    }

    if(item_->hasBeenRefined())
    {
      return REFINED;
    }

    return NONE;
  }


  /************************************************************************************
  ######  #    #   #####     #     #####   #   #
  #       ##   #     #       #       #      # #
  #####   # #  #     #       #       #       #
  #       #  # #     #       #       #       #
  #       #   ##     #       #       #       #
  ######  #    #     #       #       #       #
  ************************************************************************************/
  // --Entity
  template <int cd, int dim, class GridImp>
  inline ALU3dGridEntity<cd,dim,GridImp> ::
  ALU3dGridEntity(const GridImp  &grid, int level)
    : grid_(grid), level_(0) , gIndex_(-1)
      , item_(0) , father_(0)
      , geo_(false) , builtgeometry_(false)
      , localFCoordCalced_ (false)
  {}

  template<int cd, int dim, class GridImp>
  inline void ALU3dGridEntity<cd,dim,GridImp> ::
  reset( int l )
  {
    item_  = 0;
    level_ = l;
  }

  template<int cd, int dim, class GridImp>
  inline void ALU3dGridEntity<cd,dim,GridImp> ::
  removeElement()
  {
    item_ = 0;
  }

  template<int cd, int dim, class GridImp>
  inline bool ALU3dGridEntity<cd,dim,GridImp> ::
  equals(const ALU3dGridEntity<cd,dim,GridImp> & org) const
  {
    return (item_ == org.item_);
  }

  template<int cd, int dim, class GridImp>
  inline void ALU3dGridEntity<cd,dim,GridImp> ::
  setEntity(const ALU3dGridEntity<cd,dim,GridImp> & org)
  {
    item_   = org.item_;
    gIndex_ = org.gIndex_;
    level_  = org.level_;
    father_ = org.father_;
    builtgeometry_= false;
    localFCoordCalced_ = false;
  }

  template<int cd, int dim, class GridImp>
  inline void ALU3dGridEntity<cd,dim,GridImp> :: setElement(const BSElementType & item)
  {
    item_   = static_cast<const BSIMPLElementType *> (&item);
    gIndex_ = (*item_).getIndex();
    level_  = (*item_).level();
    builtgeometry_=false;
    localFCoordCalced_ = false;
  }

  template<>
  inline void ALU3dGridEntity<3,3,const ALU3dGrid<3,3,hexa> > ::
  setElement(const ALU3DSPACE HElementType &el, const ALU3DSPACE VertexType &vx)
  {
    item_   = static_cast<const BSIMPLElementType *> (&vx);
    gIndex_ = (*item_).getIndex();
    father_ = static_cast<const ALU3DSPACE HElementType *> (&el);
    builtgeometry_=false;
    localFCoordCalced_ = false;
  }

  template<>
  inline void ALU3dGridEntity<3,3,const ALU3dGrid<3,3,tetra> > ::
  setElement(const ALU3DSPACE HElementType &el, const ALU3DSPACE VertexType &vx)
  {
    item_   = static_cast<const BSIMPLElementType *> (&vx);
    gIndex_ = (*item_).getIndex();
    level_  = (*item_).level();
    father_ = static_cast<const ALU3DSPACE HElementType *> (&el);
    builtgeometry_=false;
    localFCoordCalced_ = false;
  }

  template<int cd, int dim, class GridImp>
  inline int ALU3dGridEntity<cd,dim,GridImp> :: index () const
  {
    const Entity en (*this);
    return grid_.levelIndexSet().index(en);
  }

  template<int cd, int dim, class GridImp>
  inline int ALU3dGridEntity<cd,dim,GridImp> :: globalIndex () const
  {
    return gIndex_;
  }

  template<int cd, int dim, class GridImp>
  inline int ALU3dGridEntity<cd,dim,GridImp> :: getIndex () const
  {
    return gIndex_;
  }

  template<int cd, int dim, class GridImp>
  inline int ALU3dGridEntity<cd,dim,GridImp> :: level () const
  {
    return level_;
  }

  template<int cd, int dim, class GridImp>
  inline const typename ALU3dGridEntity<cd,dim,GridImp>::Geometry &
  ALU3dGridEntity<cd,dim,GridImp>:: geometry() const
  {
    if(!builtgeometry_) builtgeometry_ = geo_.buildGeom(*item_);
    return geo_;
  }

  template<int cd, int dim, class GridImp>
  inline typename ALU3dGridEntity<cd,dim,GridImp>::EntityPointer
  ALU3dGridEntity<cd,dim,GridImp>:: ownersFather() const
  {
    assert(cd == dim);
    assert(father_);

    ALU3dGridLevelIterator<cd,All_Partition,const GridImp> vati(grid_,(*father_));
    return vati;
  }

  template<int cd, int dim, class GridImp>
  inline FieldVector<alu3d_ctype, dim> &
  ALU3dGridEntity<cd,dim,GridImp>:: positionInOwnersFather() const
  {
    assert( cd == dim );
    if(!localFCoordCalced_)
    {
      EntityPointer vati = this->ownersFather();
      localFatherCoords_ = (*vati).geometry().local( this->geometry()[0] );
      localFCoordCalced_ = true;
    }
    return localFatherCoords_;
  }

  /***********************************************************************
  ######  #       ######  #    #  ######  #    #   #####
  #       #       #       ##  ##  #       ##   #     #
  #####   #       #####   # ## #  #####   # #  #     #
  #       #       #       #    #  #       #  # #     #
  #       #       #       #    #  #       #   ##     #
  ######  ######  ######  #    #  ######  #    #     #
  ***********************************************************************/
  // --Geometry
  //- Tetra specialization
  template<int mydim, int cdim>
  inline ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> >::
  ALU3dGridGeometry(bool makeRefElement)
    : builtinverse_ (false) , builtA_ (false) , builtDetDF_ (false)
  {
    // create reference element
    if(makeRefElement)
    {
      coord_ = 0.0;
      for(int i=1; i<mydim+1; i++)
        coord_[i][i-1] = 1.0;
    }
  }

  //   B U I L T G E O M   - - -

  template<int mydim, int cdim>
  inline void ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> >::
  calcElMatrix () const
  {
    if(!builtA_)
    {
      // creat Matrix A (=Df)               INDIZES: col/row
      // Mapping: R^dim -> R^3,  F(x) = A x + p_0
      // columns:    p_1 - p_0  |  p_2 - p_0  |  p_3 - p_0

      for (int i=0; i<mydim; i++)
      {
        //FieldVector<alu3d_ctype,cdim> & row = const_cast<FieldMatrix<alu3d_ctype,matdim,matdim> &> (A_)[i];
        //row = coord_[i+1] - coord_[0];
      }
      builtA_ = true;
    }
  }

  // matrix for mapping from reference element to current element
  template<>
  inline void ALU3dGridGeometry<3,3, const ALU3dGrid<3,3,tetra> >::
  calcElMatrix () const
  {
    if(!builtA_)
    {
      enum { mydim = 3 };
      // creat Matrix A (=Df)               INDIZES: col/row
      // Mapping: R^dim -> R^3,  F(x) = A x + p_0
      // columns:    p_1 - p_0  |  p_2 - p_0  |  p_3 - p_0

      const FieldVector<alu3d_ctype,mydim> & coord0 = coord_[0];
      for (int i=0; i<mydim; i++)
      {
        A_[i][0] = coord_[1][i] - coord0[i];
        A_[i][1] = coord_[2][i] - coord0[i];
        A_[i][2] = coord_[3][i] - coord0[i];
      }
      builtA_ = true;
    }
  }

  //dim = dimworld = 3
  template<int mydim, int cdim>
  inline void ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> > :: buildJacobianInverse() const
  {
    if(!builtinverse_)
    {
      calcElMatrix();

      // DetDf = integrationElement
      detDF_ = std::abs( FMatrixHelp::invertMatrix(A_,Jinv_) );
      builtinverse_ = builtDetDF_ = true;
    }
  }

  template<> //dim = 2 , dimworld = 3
  inline void ALU3dGridGeometry<2,3, const ALU3dGrid<3,3,tetra> > :: buildJacobianInverse() const
  {
    if(!builtinverse_)
    {
      enum { dim = 3 };

      //derr << "WARNING: ALU3dGridGeometry::buildJacobianInverse not tested yet! " << __LINE__ <<"\n";
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
  inline void ALU3dGridGeometry<1,3, const ALU3dGrid<3,3,tetra> > :: buildJacobianInverse() const
  {
    if(!builtinverse_)
    {
      enum { dim = 3 };
      //derr << "WARNING: ALU3dGridGeometry::buildJacobianInverse not tested yet! " << __LINE__ <<"\n";
      // create vectors of face
      globalCoord_ = coord_[1] - coord_[0];
      detDF_ = std::abs ( globalCoord_.two_norm() );
      builtinverse_ = builtDetDF_ = true;
    }
  }

  template<> //dim = 1 , dimworld = 3
  inline void ALU3dGridGeometry<0,3, const ALU3dGrid<3,3,tetra> > :: buildJacobianInverse() const
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
  buildGeom(const IMPLElementType & item)
  {
    enum { dim = 3 };
    enum { dimworld = 3};

    builtinverse_ = builtA_ = builtDetDF_ = false;

    for (int i=0; i<(dim+1); i++)
    {
      const double (&p)[3] = item.myvertex(i)->Point();
      for (int j=0; j<dimworld; j++)
      {
        coord_[i][j] = p[j];
      }
    }
    return true;
  }

  template <>
  inline bool ALU3dGridGeometry<3,3, const ALU3dGrid<3,3,tetra> > :: buildGhost(const PLLBndFaceType & ghost)
  {
    enum { dim = 3 };
    enum { dimworld = 3};

    builtinverse_ = builtA_ = builtDetDF_ = false;

    GEOFaceType & face = dynamic_cast<GEOFaceType &> (*(ghost.myhface3(0)));

    // here apply the negative twist, because the twist is from the
    // neighbouring elements point of view which is outside of the ghost
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
  buildGeom(const ALU3DSPACE HFaceType & item)
  {
    enum { dim = 2 };
    enum { dimworld = 3};

    builtinverse_ = builtA_ = builtDetDF_ = false;

    for (int i=0; i<(dim+1); i++)
    {
      const double (&p)[3] = static_cast<const GEOFaceType &> (item).myvertex(i)->Point();
      for (int j=0; j<dimworld; j++)
      {
        coord_[i][j] = p[j];
      }
    }

    buildJacobianInverse();
    return true;
  }

  template <>
  inline bool ALU3dGridGeometry<2, 3, const ALU3dGrid<3, 3, tetra> >::
  buildGeom(int twist, int faceIdx) {
    enum { dim = 2 };
    enum { dimworld = 3};

    const Geometry<3, 3, const ALU3dGrid<3, 3, tetra>, Dune::ALU3dGridGeometry> &
    refElem =
      ALU3dGridGeometry<3, 3, const ALU3dGrid<3, 3, tetra> >::refelem();

    for (int i = 0; i < corners(); ++i) {
      const int localVertexIdx  = invTwist(twist, i);
      const int globalVertexIdx = faceIndex(faceIdx, localVertexIdx);
      FieldVector<alu3d_ctype, dimworld> p = refElem[globalVertexIdx];
      for (int j = 0; j < dimworld; ++j) {
        coord_[i][j] = p[j];
      }
    }

    return true;
  }

  template <> // for edges
  inline bool ALU3dGridGeometry<1,3, const ALU3dGrid<3,3,tetra> > :: buildGeom(const ALU3DSPACE HEdgeType & item)
  {
    enum { dim = 1 };
    enum { dimworld = 3};

    builtinverse_ = builtA_ = builtDetDF_ = false;

    for (int i=0; i<(dim+1); i++)
    {
      const double (&p)[3] = static_cast<const GEOEdgeType &> (item).myvertex(i)->Point();
      for (int j=0; j<dimworld; j++)
      {
        coord_[i][j] = p[j];
      }
    }

    buildJacobianInverse();
    return true;
  }

  template <> // for Vertices ,i.e. Points
  inline bool ALU3dGridGeometry<0,3, const ALU3dGrid<3,3,tetra> > :: buildGeom(const ALU3DSPACE VertexType & item)
  {
    enum { dim = 0 };
    enum { dimworld = 3};

    builtinverse_ = builtA_ = builtDetDF_ = false;

    const double (&p)[3] = static_cast<const GEOVertexType &> (item).Point();
    for (int j=0; j<dimworld; j++) coord_[0][j] = p[j];

    buildJacobianInverse();
    return true;
  }

  template <int mydim, int cdim>
  int ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, tetra> >::
  faceTwist(int val, int idx) const {
    return (val < 0) ? (7 - idx + val)%3 : (val + idx)%3 ;
  }

  template <int mydim, int cdim>
  int ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, tetra> >::
  invTwist(int val, int idx) const {
    return (val < 0) ? (7 - idx + val)%3 : (3 + idx - val)%3;
  }

  template <int mydim, int cdim>
  int ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, tetra> >::
  faceIndex(int faceIdx, int vtxIdx) const {
    return faceIndex_[faceIdx][vtxIdx];
  }

  template <int mydim, int cdim>
  const int ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, tetra> >::
  faceIndex_[4][3] = {{1, 3, 2},
                      {0, 3, 1},
                      {0, 2, 3},
                      {0, 1, 2}};

  template <GeometryType eltype , int dim> struct ALU3dGridElType {
    static inline GeometryType type () { return unknown; }
  };
  template <> struct ALU3dGridElType<tetrahedron,3> {
    static inline GeometryType type () { return tetrahedron; }
  };
  template <> struct ALU3dGridElType<tetrahedron,2> {
    static inline GeometryType type () { return triangle; }
  };
  template <GeometryType eltype> struct ALU3dGridElType<eltype,1> {
    static inline GeometryType type () { return line; }
  };
  template <GeometryType eltype> struct ALU3dGridElType<eltype,0> {
    static inline GeometryType type () { return vertex; }
  };
  template <> struct ALU3dGridElType<hexahedron,3> {
    static inline GeometryType type () { return hexahedron; }
  };
  template <> struct ALU3dGridElType<hexahedron,2> {
    static inline GeometryType type () { return quadrilateral; }
  };

  template<int mydim, int cdim>
  inline GeometryType ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> > ::type () const
  {
    return ALU3dGridElType<tetrahedron,mydim>::type();
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
    A_.umv(local,globalCoord_);
    return globalCoord_;
  }

  template<>
  inline FieldVector<alu3d_ctype, 3>
  ALU3dGridGeometry<3,3, const ALU3dGrid<3,3,tetra> >::
  global(const FieldVector<alu3d_ctype, 3> & local) const
  {
    calcElMatrix();

    globalCoord_ = coord_[0];
    A_.umv(local,globalCoord_);
    return globalCoord_;
  }

  template<> // dim = dimworld = 3
  inline FieldVector<alu3d_ctype, 3>
  ALU3dGridGeometry<3,3,const ALU3dGrid<3,3,tetra> > ::
  local(const FieldVector<alu3d_ctype, 3>& global) const
  {
    if (!builtinverse_) buildJacobianInverse();
    enum { dim = 3 };
    for(int i=0; i<dim; i++)
      globalCoord_[i] = global[i] - coord_[0][i];

    FMatrixHelp::multAssign(Jinv_,globalCoord_,localCoord_);
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

  template<int mydim, int cdim>
  inline alu3d_ctype
  ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> > ::integrationElement (const FieldVector<alu3d_ctype, mydim>& local) const
  {
    if(builtDetDF_)
      return detDF_;

    calcElMatrix();

    detDF_ = A_.determinant();

    assert(detDF_ > 0.0);

    builtDetDF_ = true;
    return detDF_;
  }

  //  J A C O B I A N _ I N V E R S E  - - -

  template<> // dim = dimworld = 3
  inline const FieldMatrix<alu3d_ctype,3,3> &
  ALU3dGridGeometry<3,3, const ALU3dGrid<3,3,tetra> >:: jacobianInverse (const FieldVector<alu3d_ctype, 3>& local) const
  {
    if (!builtinverse_) buildJacobianInverse();
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

  template <class GridImp, int dim> struct ALU3dGridRefElem;
  template<int mydim, int cdim>
  inline const Dune::Geometry<mydim,mydim,const ALU3dGrid<3,3,tetra>, Dune::ALU3dGridGeometry> &
  ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3,3,tetra> >:: refelem () {
    return ALU3dGridRefElem<const ALU3dGrid<3,3,tetra>,mydim>::refelem();
  }

  //- Hexahedron specialization
  template <int mydim, int cdim>
  inline ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> >::
  ALU3dGridGeometry(bool makeRefElement) :
    coord_(0.0),
    triMap_(0),
    biMap_(0)
  {
    assert(false);
    if (makeRefElement) {
      for (int i = 1; i < mydim+1; ++i) {
        coord_[i][i-1] = 1.0;
      }
    }
  }

  template <>
  inline ALU3dGridGeometry<3, 3, const ALU3dGrid<3, 3, hexa> >::
  ALU3dGridGeometry(bool makeRefElement) :
    coord_(0.0),
    triMap_(0),
    biMap_(0)
  {
    // Dune reference element - hardwired
    if (makeRefElement) {
      coord_[1][0] = 1.0;
      coord_[2][1] = 1.0;
      coord_[3][0] = 1.0;
      coord_[3][1] = 1.0;
      coord_[4][2] = 1.0;
      coord_[5][0] = 1.0;
      coord_[5][2] = 1.0;
      coord_[6][1] = 1.0;
      coord_[6][2] = 1.0;
      coord_[7][0] = 1.0;
      coord_[7][1] = 1.0;
      coord_[7][2] = 1.0;
    }
  }

  template <>
  inline ALU3dGridGeometry<2, 3, const ALU3dGrid<3, 3, hexa> >::
  ALU3dGridGeometry(bool makeRefElement)
    : coord_(0.0),
      triMap_(0),
      biMap_(0)
  {
    if (makeRefElement) {
      coord_[1][0] = 1.0;
      coord_[2][1] = 1.0;
      coord_[3][0] = 1.0;
      coord_[3][1] = 1.0;
    }
  }

  template <int mydim, int cdim>
  ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> >::
  ~ALU3dGridGeometry() {
    delete triMap_;
    delete biMap_;
  }

  template<int mydim, int cdim>
  inline GeometryType
  ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> >::type() const {
    return ALU3dGridElType<hexahedron, mydim>::type();
  }

  template <int mydim, int cdim>
  inline int
  ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> >::corners() const {
    return POWER_M_P<2,mydim>::power;
  }

  template <int mydim, int cdim>
  const FieldVector<alu3d_ctype, cdim>&
  ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> >::
  operator[] (int i) const {
    assert((i >= 0) && (i < corners()));
    return coord_[i];
  }

  template <int mydim, int cdim>
  const Dune::Geometry<mydim, mydim, const ALU3dGrid<3,3,hexa>,
      Dune::ALU3dGridGeometry>&
  ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> >::
  refelem () {
    return ALU3dGridRefElem<const ALU3dGrid<3, 3, hexa>, mydim>::refelem();
  }

  template <>
  inline FieldVector<alu3d_ctype, 3>
  ALU3dGridGeometry<3, 3, const ALU3dGrid<3, 3, hexa> >::
  global (const FieldVector<alu3d_ctype, 3>& local) const {
    triMap_->map2world(local, tmp2_);
    return tmp2_;
  }

  template <>
  inline FieldVector<alu3d_ctype, 3>
  ALU3dGridGeometry<2, 3, const ALU3dGrid<3, 3, hexa> >::
  global (const FieldVector<alu3d_ctype, 2>& local) const {
    biMap_->map2world(local, tmp2_);
    return tmp2_;
  }

  template <>
  inline FieldVector<alu3d_ctype, 3>
  ALU3dGridGeometry<3, 3, const ALU3dGrid<3, 3, hexa> >::
  local (const FieldVector<alu3d_ctype, 3>& global) const {
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
    for (int i = 1; i < mydim; ++i) {
      result &= (local[i] >= 0.0 && local[i] <= 1.0);
    }
    return result;
  }

  template<>
  inline alu3d_ctype
  ALU3dGridGeometry<3, 3, const ALU3dGrid<3, 3, hexa> >::
  integrationElement (const FieldVector<alu3d_ctype, 3>& local) const {
    return triMap_->det(local);
  }

  template<>
  inline alu3d_ctype
  ALU3dGridGeometry<2, 3, const ALU3dGrid<3, 3, hexa> >::
  integrationElement (const FieldVector<alu3d_ctype, 2>& local) const {
    // * is this right? Seems so!
    biMap_->normal(local, tmp2_);
    return tmp2_.two_norm();
  }

  template <>
  inline const FieldMatrix<alu3d_ctype, 3, 3>&
  ALU3dGridGeometry<3, 3, const ALU3dGrid<3, 3, hexa> >::
  jacobianInverse (const FieldVector<alu3d_ctype, 3>& local) const {
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
  buildGeom(const IMPLElementType& item) {
    enum { dim = 3 };
    enum { dimworld = 3 };

    for (int i = 0; i < corners(); ++i) {
      const double (&p)[3] = item.myvertex(dune2aluVol[i])->Point();
      for (int j = 0; j < dimworld; ++j) {
        coord_[i][j] = p[j];
      }
    }

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

  /*  * old version
     template <>
     inline bool
     ALU3dGridGeometry<2,3, const ALU3dGrid<3, 3, hexa> > ::
     buildGeom(const ALU3DSPACE HFaceType & item, int twist, int faceIdx) {
     enum { dim = 2 };
     enum { dimworld = 3 };

     const GEOFaceType& face = static_cast<const GEOFaceType&> (item);
     for (int i = 0; i < 4; ++i) {
      const int idx = faceTwist(twist, dune2aluFaceVertex[faceIdx][i]);
      const double (&p)[3] =
     //static_cast<const GEOFaceType &>(item).myvertex(dune2aluQuad[i])->Point();
     //   face.myvertex(dune2aluQuad[i])->Point();
      face.myvertex(idx)->Point();
      for (int j = 0; j < dimworld; ++j) {
        coord_[i][j] = p[j];
      }
     }

     biMap_ = new BilinearSurfaceMapping(coord_[0], coord_[1],
                                        coord_[2], coord_[3]);

     return true;
     }
   */

  template <>
  inline bool
  ALU3dGridGeometry<2,3, const ALU3dGrid<3, 3, hexa> > ::
  buildGeom(const ALU3DSPACE HFaceType & item) {
    enum { dim = 2 };
    enum { dimworld = 3 };

    const GEOFaceType& face = static_cast<const GEOFaceType&> (item);
    for (int i = 0; i < 4; ++i) {
      const double (&p)[3] =
        face.myvertex(dune2aluQuad[i])->Point();
      for (int j = 0; j < dimworld; ++j) {
        coord_[i][j] = p[j];
      }
    }

    biMap_ = new BilinearSurfaceMapping(coord_[0], coord_[1],
                                        coord_[2], coord_[3]);

    return true;
  }

  template <>
  inline bool
  ALU3dGridGeometry<2,3, const ALU3dGrid<3, 3, hexa> > ::
  buildGeom(int twist, int duneFaceIdx) {
    enum { dim = 2 };
    enum { dimworld = 3 };

    const Geometry<3, 3, const ALU3dGrid<3, 3, hexa>, Dune::ALU3dGridGeometry >&
    refElem =
      ALU3dGridGeometry<3, 3, const ALU3dGrid<3, 3, hexa> >::refelem();

    const int aluFaceIdx = ALU3dImplTraits<hexa>::dune2aluFace(duneFaceIdx);

    for (int i = 0; i < corners(); ++i) {
      const int localAluVertexIdx = invTwist(twist, dune2aluQuad[i]);
      const int globalDuneVertexIdx =
        alu2duneFaceVertexGlobal[aluFaceIdx][localAluVertexIdx];
      FieldVector<alu3d_ctype, dimworld> p = refElem[globalDuneVertexIdx];
      for (int j = 0; j < dimworld; ++j) {
        coord_[i][j] = p[j];
      }
    }

    biMap_ = new BilinearSurfaceMapping(coord_[0], coord_[1],
                                        coord_[2], coord_[3]);

    return true;
  }

  template <> // for edges
  inline bool
  ALU3dGridGeometry<1,3, const ALU3dGrid<3, 3, hexa> >::
  buildGeom(const ALU3DSPACE HEdgeType & item) {
    enum { dim = 1 };
    enum { dimworld = 3 };

    // * potentially wrong, since the edges are renumbered in dune.
    for (int i = 0; i < 2; ++i) {
      const double (&p)[3] = static_cast<const GEOEdgeType &> (item).myvertex(i)->Point();
      for (int j = 0; j < dimworld; ++j) {
        coord_[i][j] = p[j];
      }
    }

    return true;
  }

  template <> // for Vertices ,i.e. Points
  inline bool
  ALU3dGridGeometry<0,3, const ALU3dGrid<3,3,hexa> >::
  buildGeom(const ALU3DSPACE VertexType & item) {
    enum { dim = 0 };
    enum { dimworld = 3};

    const double (&p)[3] = static_cast<const GEOVertexType &> (item).Point();
    for (int j=0; j<dimworld; j++) coord_[0][j] = p[j];

    return true;
  }

  template <int mydim, int cdim>
  int ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> >::
  faceTwist(int val, int idx) const {
    return (val < 0) ? (9 - idx + val)%4 : (val + idx)%4 ;
  }

  template <int mydim, int cdim>
  int ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> >::
  invTwist(int val, int idx) const {
    return (val < 0) ? (9 - idx + val)%4 : (4 + idx - val)%4;
  }

  template<int mydim, int cdim>
  const int ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, hexa> >::
  alu2duneVol[8] = {1, 3, 2, 0, 5, 7, 6, 4};

  template<int mydim, int cdim>
  const int ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, hexa> >::
  dune2aluVol[8] = {3, 0, 2, 1, 7, 4, 6, 5};

  template<int mydim, int cdim>
  const int ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, hexa> >::
  alu2duneFace[6] = {4, 5, 1, 3, 0, 2};

  template<int mydim, int cdim>
  const int ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, hexa> >::
  dune2aluFace[6] = {4, 2, 5, 3, 0, 1};

  template<int mydim, int cdim>
  const int ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, hexa> >::
  alu2duneQuad[4] = {0, 2, 3, 1};

  template<int mydim, int cdim>
  const int ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, hexa> >::
  dune2aluQuad[4] = {0, 3, 1, 2};

  template<int mydim, int cdim>
  const int ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, hexa> >::
  dune2aluFaceVertex[6][4] = {{1, 0, 2, 3},
                              {0, 1, 3, 2},
                              {3, 0, 2, 1},
                              {1, 0, 2, 3},
                              {1, 0, 2, 3},
                              {3, 0, 2, 1}};
  // dune2aluFaceVertex[6][4] = {{0, 1, 3, 2},
  //                             {0, 1, 3, 2},
  //                             {3, 0, 2, 1},
  //                             {3, 0, 2, 1},
  //                             {3, 0, 2, 1},
  //                             {3, 0, 2, 1}};

  template<int mydim, int cdim>
  const int ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, hexa> >::
  alu2duneFaceVertex[6][4] = {{1, 0, 2, 3},
                              {1, 3, 2, 0},
                              {0, 1, 3, 2},
                              {1, 0, 2, 3},
                              {1, 0, 2, 3},
                              {1, 3, 2, 0}};

  template <int mydim, int cdim>
  const int ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> >::
  alu2duneFaceVertexGlobal[6][4] = {{1, 0, 2, 3},
                                    {5, 7, 6, 4},
                                    {1, 3, 7, 5},
                                    {3, 2, 6, 7},
                                    {2, 0, 4, 6},
                                    {1, 5, 4, 0}};

  //**********************************************************
  //  Reference Element
  //**********************************************************
  template <class GridImp> struct ALU3dGridRefElem<GridImp,1> {
    static const Dune::Geometry<1,1,GridImp,Dune::ALU3dGridGeometry> & refelem ()
    {
      static ALU3dGridReferenceGeometry<1,GridImp> ref;
      return ref.refelem;
    }
  };

  template <class GridImp> struct ALU3dGridRefElem<GridImp,2> {
    static const Dune::Geometry<2,2,GridImp,Dune::ALU3dGridGeometry> & refelem ()
    {
      static ALU3dGridReferenceGeometry<2,GridImp> ref;
      return ref.refelem;
    }
  };
  template <class GridImp> struct ALU3dGridRefElem<GridImp,3> {
    static const Dune::Geometry<3,3,GridImp,Dune::ALU3dGridGeometry> & refelem ()
    {
      static ALU3dGridReferenceGeometry<3,GridImp> ref;
      return ref.refelem;
    }
  };

}

#endif
