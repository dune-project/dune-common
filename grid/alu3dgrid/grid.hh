// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DGRIDGRID_HH
#define DUNE_ALU3DGRIDGRID_HH

//- System includes
#include <vector>

//- Dune includes
#include <dune/io/file/grapedataio.hh>
#include <dune/common/capabilities.hh>
#include <dune/common/interfaces.hh>
#include <dune/common/collectivecommunication.hh>
#include <dune/common/bigunsignedint.hh>

#include "../common/grid.hh"
#include "../common/referenceelements.hh"
#include "../common/defaultindexsets.hh"
#include "../common/leafindexset.hh"
#include "../common/sizecache.hh"
#include "../common/intersectioniteratorwrapper.hh"

//- Local includes
#include "alu3dinclude.hh"
#include "indexsets.hh"
#include "memory.hh"

namespace Dune {

  // Forward declarations
  template<int cd, int dim, class GridImp>
  class ALU3dGridEntity;
  template<int cd, PartitionIteratorType pitype, class GridImp >
  class ALU3dGridLevelIterator;
  template<int cd, class GridImp >
  class ALU3dGridEntityPointerBase;
  template<int cd, class GridImp >
  class ALU3dGridEntityPointer;
  template<int mydim, int coorddim, class GridImp>
  class ALU3dGridMakeableGeometry;
  template<int mydim, int coorddim, class GridImp>
  class ALU3dGridGeometry;
  template<class GridImp>
  class ALU3dGridHierarchicIterator;
  template<class GridImp>
  class ALU3dGridIntersectionIterator;
  template<int codim, PartitionIteratorType pitype, class GridImp>
  class ALU3dGridLeafIterator;
  template <int mydim, int coorddim, class GridImp>
  class ALU3dGridMakeableEntity;
  template <class GridImp>
  class ALU3dGridFaceGeometryInfo;
  template<int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGridGlobalIdSet;
  template<int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGridLocalIdSet;
  template<int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGridHierarchicIndexSet;
  template <class EntityImp>
  class ALUMemoryProvider;
  template<int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGrid;

  //! contains list of vertices of one level
  class ALU3dGridVertexList
  {
  public:
    // level vertex iterator list
    typedef std::vector < ALU3DSPACE VertexType * > VertexListType;
    typedef VertexListType :: iterator IteratorType;

    ALU3dGridVertexList () : up2Date_(false) {}

    size_t size () const { return vertexList_.size(); }

    bool up2Date () const { return up2Date_;  }
    void unsetUp2Date ()  { up2Date_ = false; }

    // make grid walkthrough and calc global size
    template <class GridType>
    void setupVxList (const GridType & grid, int level);

    IteratorType begin () { return vertexList_.begin(); }
    IteratorType end   () { return vertexList_.end(); }
  private:
    bool up2Date_;
    VertexListType vertexList_;
  };

  //**********************************************************************
  //
  // --ALU3dGrid
  // --Grid
  //
  //**********************************************************************
  template <int dim, int dimworld, ALU3dGridElementType elType>
  struct ALU3dGridFamily
  {
    //! Type of the global id set
    typedef ALU3dGridGlobalIdSet<dim,dimworld,elType> GlobalIdSetImp;

    //! Type of the local id set
    typedef ALU3dGridLocalIdSet<dim,dimworld,elType> LocalIdSetImp;

    //! Type of the level index set
    typedef DefaultLevelIndexSet<ALU3dGrid < dim, dimworld, elType > >  LevelIndexSetImp;
    //! Type of the leaf index set
    typedef AdaptiveLeafIndexSet<ALU3dGrid < dim, dimworld, elType > >  LeafIndexSetImp;

    //! type of ALU3dGrids global id
    typedef bigunsignedint<6*32> GlobalIdType;

    //! type of ALU3dGrids local id
    typedef int LocalIdType;

    typedef ALU3dGrid<dim,dimworld,elType> GridImp;

    struct Traits
    {
      //! type of ALU3dGrids local id
      typedef int LocalIdType;

      //! type of ALU3dGrids global id
      typedef bigunsignedint<6*32> GlobalIdType;

      typedef ALU3dGrid<dim,dimworld,elType> Grid;

      typedef Dune::IntersectionIterator<const GridImp, IntersectionIteratorWrapper> IntersectionIterator;

      typedef Dune::HierarchicIterator<const GridImp, ALU3dGridHierarchicIterator> HierarchicIterator;

      template <int cd>
      struct Codim
      {
        // IMPORTANT: Codim<codim>::Geometry == Geometry<dim-codim,dimw>
        typedef Dune::Geometry<dim-cd, dimworld, const GridImp, ALU3dGridGeometry> Geometry;
        typedef Dune::Geometry<dim-cd, dim, const GridImp, ALU3dGridGeometry> LocalGeometry;
        // we could - if needed - introduce an other struct for dimglobal of Geometry
        typedef Dune::Entity<cd, dim, const GridImp, ALU3dGridEntity> Entity;

        typedef Dune::LevelIterator<cd,All_Partition,const GridImp,ALU3dGridLevelIterator> LevelIterator;

        typedef Dune::LeafIterator<cd,All_Partition,const GridImp,ALU3dGridLeafIterator> LeafIterator;

        typedef Dune::EntityPointer<const GridImp,ALU3dGridEntityPointer<cd,const GridImp> > EntityPointer;

        template <PartitionIteratorType pitype>
        struct Partition
        {
          typedef Dune::LevelIterator<cd,pitype,const GridImp,ALU3dGridLevelIterator> LevelIterator;
          typedef Dune::LeafIterator<cd,pitype,const GridImp,ALU3dGridLeafIterator> LeafIterator;
        };

      };

      typedef IndexSet<GridImp,LevelIndexSetImp,DefaultLevelIteratorTypes<GridImp> > LevelIndexSet;
      typedef LeafIndexSetImp LeafIndexSet;
      typedef IdSet<GridImp,LocalIdSetImp,LocalIdType> LocalIdSet;
#ifdef _ALU3DGRID_PARALLEL_
      typedef IdSet<GridImp,GlobalIdSetImp,GlobalIdType> GlobalIdSet;
#else
      typedef LocalIdSet GlobalIdSet;
#endif
    };
  };


  /**
     \brief [<em> provides \ref Dune::Grid </em>]
     \brief 3D grid with support for hexahedrons and tetrahedrons.
     @ingroup GridImplementations
     The ALU3dGrid implements the Dune GridInterface for 3d tetrahedral and
     hexahedral meshes. This grid can be locally adapted and used in parallel
     computations using dynamcic load balancing.

     @note
     Adaptive parallel grid supporting dynamic load balancing, written
     mainly by Bernard Schupp. This grid supports hexahedrons and tetrahedrons.

     (see ALUGrid homepage: http://www.mathematik.uni-freiburg.de/IAM/Research/alugrid/)

     Two tools are available for partitioning :
     \li Metis ( version 4.0 and higher, see http://www-users.cs.umn.edu/~karypis/metis/metis/ )
     \li Party Lib ( version 1.1 and higher, see http://wwwcs.upb.de/fachbereich/AG/monien/RESEARCH/PART/party.html)

   */
  template <int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGrid :
    public GridDefaultImplementation<dim, dimworld, alu3d_ctype, ALU3dGridFamily <dim,dimworld,elType> >,
    public HasObjectStream
  {
    CompileTimeChecker<(dim      == 3)> ALU3dGrid_only_implemented_for_3dp;
    CompileTimeChecker<(dimworld == 3)> ALU3dGrid_only_implemented_for_3dw;

    typedef ALU3dGrid<dim,dimworld,elType> MyType;

    friend class ALU3dGridEntity <0,dim,MyType>;
    friend class ALU3dGridEntity <0,dim,const MyType>;
    friend class ALU3dGridIntersectionIterator<MyType>;

    friend class ALU3dGridEntityPointerBase<0,const MyType >;
    friend class ALU3dGridEntityPointerBase<1,const MyType >;
    friend class ALU3dGridEntityPointer<1,const MyType >;
    friend class ALU3dGridEntityPointerBase<2,const MyType >;
    friend class ALU3dGridEntityPointer<2,const MyType >;
    friend class ALU3dGridEntityPointerBase<3,const MyType >;
    friend class ALU3dGridEntityPointer<3,const MyType >;

    friend class ALU3dGridIntersectionIterator<const MyType>;
    friend class ALU3dGridHierarchicIterator<const MyType>;

    friend class ALU3dGridHierarchicIndexSet<dim,dimworld,elType>;
    friend class ALU3dGridGlobalIdSet<dim,dimworld,elType>;
    friend class ALU3dGridLocalIdSet<dim,dimworld,elType>;

    //**********************************************************
    // The Interface Methods
    //**********************************************************
  public:
    static const ALU3dGridElementType elementType = elType;
    typedef ALU3DSPACE ObjectStream ObjectStreamType;

    typedef ALU3dGridFamily<dim,dimworld,elType> GridFamily;

    friend class Conversion< ALU3dGrid<dim,dimworld,elementType> , HasObjectStream > ;
    friend class Conversion< const ALU3dGrid<dim,dimworld,elementType> , HasObjectStream > ;

    //! my Traits class
    typedef typename ALU3dGridFamily < dim , dimworld , elType > :: Traits Traits;

    //! Type of the hierarchic index set
    typedef ALU3dGridHierarchicIndexSet<dim,dimworld,elType> HierarchicIndexSet;

    //! Type of the local id set
    typedef ALU3dGridLocalIdSet<dim,dimworld,elType> LocalIdSetImp;

#ifdef _ALU3DGRID_PARALLEL_
    //! Type of the global id set
    typedef ALU3dGridGlobalIdSet<dim,dimworld,elType> GlobalIdSetImp;
#else
    typedef LocalIdSetImp GlobalIdSetImp;
#endif

    //! Type of the global id set
    typedef typename Traits :: GlobalIdSet GlobalIdSet;

    //! Type of the local id set
    typedef typename Traits :: LocalIdSet LocalIdSet;

    //! Type of the level index set
    typedef DefaultLevelIndexSet<MyType>           LevelIndexSetImp;
    //! Type of the leaf index set
    typedef AdaptiveLeafIndexSet<MyType>           LeafIndexSetImp;

    typedef typename SelectType<elType == tetra,
        ReferenceSimplex<alu3d_ctype, dim>,
        ReferenceCube   <alu3d_ctype, dim> >::Type ReferenceElementType;

    //! a standard leaf iterator
    typedef ALU3dGridLeafIterator<0, All_Partition, MyType> LeafIteratorImp;
    typedef typename Traits::template Codim<0>::LeafIterator LeafIteratorType;
    typedef typename Traits::template Codim<0>::LeafIterator LeafIterator;

    typedef ALU3dGridHierarchicIterator<MyType> HierarchicIteratorImp;


    //! maximal number of levels
    enum { MAXL = 64 };

    //! normal default number of new elements for new adapt method
    enum { newElementsChunk_ = 100 };

    //! if one element is refined then it causes apporximately not more than
    //! this number of new elements
    enum { refineEstimate_ = 40 };

    //! Constructor which reads an ALU3dGrid Macro Triang file
    //! or given GridFile
#ifdef _ALU3DGRID_PARALLEL_
    ALU3dGrid(const std::string macroTriangFilename , MPI_Comm mpiComm);
    ALU3dGrid(MPI_Comm mpiComm);
#else
    ALU3dGrid(const std::string macroTriangFilename );
    //! empty Constructor
    ALU3dGrid(int myrank = -1);
#endif

    //! Desctructor
    ~ALU3dGrid();

    //! for type identification
    GridIdentifier type  () const;

    //! Return maximum level defined in this grid. Levels are numbered
    //! 0 ... maxLevel with 0 the coarsest level.
    int maxLevel() const;

    //! Iterator to first entity of given codim on level
    template<int cd, PartitionIteratorType pitype>
    typename Traits::template Codim<cd>::template Partition<pitype>::LevelIterator
    lbegin (int level) const;

    //! one past the end on this level
    template<int cd, PartitionIteratorType pitype>
    typename Traits::template Codim<cd>::template Partition<pitype>::LevelIterator
    lend (int level) const;

    //! Iterator to first entity of given codim on level
    template<int cd>
    typename Traits::template Codim<cd>::
    template Partition<All_Partition>::LevelIterator
    lbegin (int level) const;

    //! one past the end on this level
    template<int cd>
    typename Traits::template Codim<cd>::
    template Partition<All_Partition>::LevelIterator
    lend (int level) const;

    //! General definiton for a leaf iterator
    template <int codim, PartitionIteratorType pitype>
    typename Traits::template Codim<codim>::template Partition<pitype>::LeafIterator
    leafbegin(int level) const;

    //! General definition for an end iterator on leaf level
    template <int codim, PartitionIteratorType pitype>
    typename Traits::template Codim<codim>::template Partition<pitype>::LeafIterator
    leafend(int level) const;

    //! General definiton for a leaf iterator
    template <int codim>
    typename Traits::template Codim<codim>::LeafIterator
    leafbegin(int level) const;

    //! General definition for an end iterator on leaf level
    template <int codim>
    typename Traits::template Codim<codim>::LeafIterator
    leafend(int level) const;

    //! Iterator to first entity of codim 0 on leaf level (All_Partition)
    LeafIteratorType leafbegin (int level) const;

    //! one past the end on this leaf level (codim 0 and All_Partition)
    LeafIteratorType leafend (int level) const;

    //! General definiton for a leaf iterator
    template <int codim, PartitionIteratorType pitype>
    typename Traits::template Codim<codim>::template Partition<pitype>::LeafIterator
    leafbegin() const;

    //! General definition for an end iterator on leaf level
    template <int codim, PartitionIteratorType pitype>
    typename Traits::template Codim<codim>::template Partition<pitype>::LeafIterator
    leafend() const;

    //! General definiton for a leaf iterator
    template <int codim>
    typename Traits::template Codim<codim>::LeafIterator
    leafbegin() const;

    //! General definition for an end iterator on leaf level
    template <int codim>
    typename Traits::template Codim<codim>::LeafIterator
    leafend() const;

    //! Iterator to first entity of codim 0 on leaf level (All_Partition)
    LeafIteratorType leafbegin () const;

    //! one past the end on this leaf level (codim 0 and All_Partition)
    LeafIteratorType leafend () const;

    //! General definiton for a leaf iterator
    template <int codim, PartitionIteratorType pitype>
    typename Traits::template Codim<codim>::template Partition<pitype>::LeafIterator
    createLeafIteratorBegin (int level) const;

    //! General definition for an end iterator on leaf level
    template <int codim, PartitionIteratorType pitype>
    typename Traits::template Codim<codim>::template Partition<pitype>::LeafIterator
    createLeafIteratorEnd(int level) const;

    //! number of grid entities per level and codim
    int size (int level, int cd) const;

    //! number of leaf entities per codim in this process
    int size (int codim) const;

    //! number of entities per level and geometry type in this process
    int size (int level, GeometryType type) const;

    //! number of leaf entities per geometry type in this process
    int size (GeometryType type) const;

    //! number of grid entities on all levels for given codim
    int global_size (int cd) const ;

    //! number of grid entities in the entire grid for given codim
    int hierSetSize (int cd) const;

    //! get global id set of grid
    const GlobalIdSet & globalIdSet () const {
      if(!globalIdSet_) globalIdSet_ = new GlobalIdSetImp(*this);
      return *globalIdSet_;
    }

    //! get global id set of grid
    const LocalIdSet & localIdSet () const { return localIdSet_; }

    //! get hierarchic index set of the grid
    const HierarchicIndexSet & hierarchicIndexSet () const { return hIndexSet_; }

    //! get leaf index set of the grid
    const typename Traits :: LeafIndexSet & leafIndexSet () const;

    //! get level index set of the grid
    const typename Traits :: LevelIndexSet & levelIndexSet (int level) const;

    //! calculate load of each proc and repartition if neccessary
    bool loadBalance ();

    //! calculate load of each proc and repartition if neccessary
    template <class DofManagerType>
    bool loadBalance (DofManagerType & dm);

    //! calculate load of each proc and repartition if neccessary
    template <class DofManagerType>
    bool communicate (DofManagerType & dm);

    /** \brief ghostSize is zero for this grid  */
    int ghostSize (int level, int codim) const { return 0; }

    /** \brief overlapSize is zero for this grid  */
    int overlapSize (int level, int codim) const { return 0; }

    /** \brief ghostSize is zero for this grid  */
    int ghostSize (int codim) const { return 0; }

    /** \brief overlapSize is zero for this grid  */
    int overlapSize (int codim) const { return 0; }

    /** dummy communicate */
    template<class DataHandle>
    void communicate (DataHandle& data, InterfaceType iftype, CommunicationDirection dir, int level) const
    {}

    /** dummy communicate */
    template<class DataHandle>
    void communicate (DataHandle& data, InterfaceType iftype, CommunicationDirection dir) const
    {}

    /** dummy collective communication */
    const CollectiveCommunication<ALU3dGrid>& comm () const
    {
      return ccobj;
    }

  private:
    CollectiveCommunication<ALU3dGrid> ccobj;

  public:

    template <class T> T globalMin (T val) const ;
    template <class T> T globalMax (T val) const ;
    template <class T> T globalSum (T val) const ;
    template <class T> void globalSum (T *, int , T *) const ;

    //! returns if a least one entity was marked for coarsening
    bool preAdapt ( );

    //! clear all entity new markers
    void postAdapt ( );

    /**! refine all positive marked leaf entities,
       return true if a least one entity was refined */
    bool adapt ( );

    //! adapt with DofManager
    template <class DofManagerType, class RestrictProlongOperatorType>
    bool adapt (DofManagerType &, RestrictProlongOperatorType &, bool verbose=false );

    //**********************************************************
    // End of Interface Methods
    //**********************************************************
    //! uses the interface, mark on entity and refineLocal
    bool globalRefine(int refCount);

    /** \brief write Grid to file in specified FileFormatType
     */
    template <GrapeIOFileFormatType ftype>
    bool writeGrid( const std::string filename, alu3d_ctype time ) const ;

    bool writeGrid_Xdr( const std::string filename, alu3d_ctype time ) const ;
    bool writeGrid_Ascii( const std::string filename, alu3d_ctype time ) const ;

    /** \brief read Grid from file filename and store time of mesh in time
     */
    template <GrapeIOFileFormatType ftype>
    bool readGrid( const std::string filename, alu3d_ctype & time );

    //! return my rank (only parallel)
    int myRank () const { return myRank_; }

    //! no interface method, but has to be public
    void updateStatus ();

    //! mark entities for refinement or coarsening, refCount < 0 will mark
    //! the entity for one coarsen step and refCount > 0 will mark for one
    //! refinement, one refinement will create 8 children per element
    bool mark( int refCount , const typename Traits::template Codim<0>::EntityPointer & ep );
  private:
    bool mark( int refCount , const typename Traits::template Codim<0>::Entity & en );

  public:
    IntersectionIteratorWrapper<const MyType>&
    getRealIntersectionIterator(typename Traits::IntersectionIterator& it)
    {
      return this->getRealImplementation(it);
    }

    const IntersectionIteratorWrapper<const MyType>&
    getRealIntersectionIterator(const typename Traits::IntersectionIterator& it)  const
    {
      return this->getRealImplementation(it);
    }

    //! deliver all geometry types used in this grid
    const std::vector<GeometryType>& geomTypes (int codim) const { return geomTypes_[codim]; }

    //! return reference to org ALU3dGrid
    //! private method, but otherwise we have to friend class all possible
    //! types of LevelIterator ==> later
    ALU3DSPACE GitterImplType & myGrid();
    const ALU3DSPACE GitterImplType & myGrid() const;

    //! return reference to Dune reference element according to elType
    const ReferenceElementType & referenceElement() const { return referenceElement_; }

    //! check whether macro grid has the right element type
    void checkMacroGrid ();
  private:
    //! Copy constructor should not be used
    ALU3dGrid( const MyType & g );

    //! assignment operator should not be used
    ALU3dGrid<dim,dimworld,elType>&
    operator = (const MyType & g);

    // reset size and global size
    void calcExtras();

    // calculate maxlevel
    void calcMaxlevel();

    // make grid walkthrough and calc global size
    void recalcGlobalSize();

    // the real grid
    ALU3DSPACE GitterImplType * mygrid_;
#ifdef _ALU3DGRID_PARALLEL_
    ALU3DSPACE MpAccessMPI mpAccess_;
#endif
    // max level of grid
    int maxlevel_;

    // count how much elements where marked
    mutable int coarsenMarked_;
    mutable int refineMarked_;

    const int myRank_;

    // at the moment the number of different geom types is 1
    enum { numberOfGeomTypes = 1 };
    std::vector< std::vector<GeometryType> > geomTypes_;

    // create GeomTypes
    void makeGeomTypes ();

    // our hierarchic index set
    HierarchicIndexSet hIndexSet_;

    // out global id set
    mutable GlobalIdSetImp * globalIdSet_;

    // out global id set
    LocalIdSetImp localIdSet_;

    // the level index set ( default type )
    mutable std::vector < LevelIndexSetImp * > levelIndexVec_;

    // the leaf index set
    mutable LeafIndexSetImp * leafIndexSet_;

    // the entity codim 0
    typedef ALU3dGridMakeableEntity<0,dim,const MyType> EntityImp;
    typedef ALUMemoryProvider< EntityImp > EntityProvider;

    template <int codim>
    ALU3dGridMakeableEntity<codim,dim,const MyType> * getNewEntity ( int level ) const;

    template <int codim>
    void freeEntity (ALU3dGridMakeableEntity<codim,dim,const MyType> * en) const;

    mutable EntityProvider entityProvider_;

    // the reference element
    ReferenceElementType referenceElement_;

    typedef ALU3dGridVertexList VertexListType;
    mutable VertexListType vertexList_[MAXL];

    // the type of our size cache
    typedef SingleTypeSizeCache<MyType> SizeCacheType;
    SizeCacheType * sizeCache_;

    // new intersection iterator is a wrapper which get itersectioniteratoimp as pointers
  public:
    typedef ALU3dGridIntersectionIterator<const MyType>
    IntersectionIteratorImp;
    typedef ALUMemoryProvider< IntersectionIteratorImp > IntersectionIteratorProviderType;
  private:
    friend class IntersectionIteratorWrapper< const MyType > ;
    // return reference to intersectioniterator storage
    IntersectionIteratorProviderType & intersetionIteratorProvider() const { return interItProvider_; }
    mutable IntersectionIteratorProviderType interItProvider_;
  }; // end class ALU3dGrid

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
      return ep.getObject( grid, level);
    }

    template <class EntityProviderType>
    static void freeEntity( EntityProviderType & ep, EntityImp * e )
    {
      ep.freeObject( e );
    }
  };


  bool checkMacroGrid ( ALU3dGridElementType elType ,
                        const std::string filename );
  const char* elType2Name( ALU3dGridElementType elType );

  namespace Capabilities {

    template<int dim,int dimw, Dune::ALU3dGridElementType elType>
    struct hasLeafIterator< Dune::ALU3dGrid<dim, dimw, elType> >
    {
      static const bool v = true;
    };

    template<int dim, int dimw, Dune::ALU3dGridElementType elType, int cdim>
    struct hasEntity<Dune::ALU3dGrid<dim, dimw, elType>, cdim >
    {
      static const bool v = true;
    };

    template <int dim, int dimw, ALU3dGridElementType elType>
    struct isParallel<const ALU3dGrid<dim, dimw, elType> > {
      static const bool v = true;
    };

    template<int dim, int dimw, Dune::ALU3dGridElementType elType>
    struct isLevelwiseConforming< ALU3dGrid<dim,dimw,elType> >
    {
      static const bool v = true;
    };

    template<int dim, int dimw, Dune::ALU3dGridElementType elType>
    struct hasHangingNodes< ALU3dGrid<dim,dimw,elType> >
    {
      static const bool v = true;
    };
  } // end namespace Capabilities

} // end namespace Dune

#include "grid_imp.cc"

#endif
