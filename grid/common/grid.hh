// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_HH
#define DUNE_GRID_HH

#include <string>
#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>
#include <dune/common/helpertemplates.hh>
#include <dune/common/typetraits.hh>

namespace Dune {

  /** @defgroup GridCommon Grid Interface

     The Dune Grid module defines a general interface to a hierarchical finite element mesh.
     The interface is independent of dimension and element type (GeometryType). Various implementations
     of this interface exits:

     - Structured Grid (SGrid) : A structured mesh in d dimensions consisting of "cubes". The number
     of elements per dimension is variable.

     - Albert Grid (AlbertGrid) : Provides the simplicial meshes of the finite element tool box ALBERT
     written by Kunibert Siebert and Alfred Schmidt.

     - UG Grid (UGGrid) : Provides the meshes of the finite element toolbox UG.

     - Yet Another Structured Parallel Grid (YaspGrid) : Provides a distributed structured mesh.

     This Module contains only the description of compounds that are common to all implementations
     of the grid interface.

     For a detailed description of the interface itself please see the documentation
     of the "Structured Grid Module". Since Dune uses the Engine concept there is no abstract definition
     of the interface. As with STL containers, all implementations must implement the
     same classes with exactly the same members to be used in generic algorithms.

     \sa AlbertGrid UGGrid SPGrid SGrid YaspGrid


     @{
   */

  //************************************************************************
  /*!
      Enum that declares identifiers for different geometry types. This
      list can be extended in the future. Not all meshes need to implement
      all geometry types.
   */

  enum GeometryType {vertex=0,line=1, triangle=2, quadrilateral=3, tetrahedron=4,
                     pyramid=5, prism=6, hexahedron=7,
                     iso_triangle=8, iso_quadrilateral=9, unknown=127};

  /*!
     Specify the format to store grid and vector data
   */
  enum FileFormatType { ascii , //!< store data in a human readable form
                        xdr ,   //!< store data in SUN's library routines
                                //!< for external data representation (xdr)
                        pgm };  //!< store data in portable graymap file format (pgm)

  enum AdaptationState {
    NONE ,   //!< notin' to do and notin' was done
    COARSEN, //!< entity could be coarsend in adaptation step
    REFINED  //!< enity was refined in adaptation step
  };


  /*! Parameter to be used for the communication functions
   */
  enum InterfaceType {
    InteriorBorder_InteriorBorder_Interface=0,
    InteriorBorder_All_Interface=1,
    Overlap_OverlapFront_Interface=2,
    Overlap_All_Interface=3,
    All_All_Interface=4
  };

  /*! Parameter to be used for the parallel level iterators
   */
  enum PartitionIteratorType {
    Interior_Partition=0,
    InteriorBorder_Partition=1,
    Overlap_Partition=2,
    OverlapFront_Partition=3,
    All_Partition=4,
    Ghost_Partition=5
  };


  /*! Define a type for communication direction parameter
   */
  enum CommunicationDirection {
    ForwardCommunication,
    BackwardCommunication
  };

  /*! Attributes used in the generic overlap model
   */
  enum PartitionType {
    InteriorEntity=0,     //!< all interior entities
    BorderEntity=1  ,     //!< on boundary between interior and overlap
    OverlapEntity=2 ,     //!< all entites lying in the overlap zone
    FrontEntity=3  ,      //!< on boundary between overlap and ghost
    GhostEntity=4         //!< ghost entities
  };

  //! provide names for the partition types
  inline std::string PartitionName(PartitionType type)
  {
    switch(type) {
    case InteriorEntity :
      return "interior";
    case BorderEntity :
      return "border";
    case OverlapEntity :
      return "overlap";
    case FrontEntity :
      return "front";
    case GhostEntity :
      return "ghost";
    default :
      return "unknown";
    }
  }

  /*! GridIndexType specifies which Index of the Entities of the grid
        should be used, i.e. globalIndex() or index()
   */
  enum GridIndexType { GlobalIndex , //!< use globalIndex() of entity
                       LevelIndex    //!< use index() of entity
  };

  //************************************************************************
  // G R I D E R R O R
  //************************************************************************

  /*!
     exceptions in Dune grid modules.
   */

  class GridError : public Exception {};

  //************************************************************************
  // R E F E R E N C E T O P O L O G Y
  //************************************************************************

  /*!
     The reference topology defines the numbering of all entities of an
     element as well as their position in the reference element.
   */
  template<int dim, class ct>
  class ReferenceTopology {
  public:
    //! default constructor will build up all the internal data
    ReferenceTopology ();

    //! return local coordinates of center in reference element
    FieldVector<ct, dim>& center_codim0_local (int elemtype);

    //! return local coordinates of center of ith codim 1 subentity
    FieldVector<ct, dim-1>& center_codim1_local (int elemtype, int i);
  };

  // Specialization dim=1
  template<class ct>
  class ReferenceTopology<1,ct> {
  public:
    //! default constructor will build up all the internal data
    ReferenceTopology ();

    //! return local coordinates of center in reference element
    FieldVector<ct, 1>& center_codim0_local (int elemtype);

    //! return local coordinates of center of ith codim 1 subentity
    FieldVector<ct, 0>& center_codim1_local (int elemtype, int i);

  private:
    FieldVector<ct, 1> center0_local[1];    // GeometryType
    FieldVector<ct, 0> center1_local[1];    // GeometryType x faces
  };

  // Specialization dim=2
  template<class ct>
  class ReferenceTopology<2,ct> {
  public:
    //! default constructor will build up all the internal data
    ReferenceTopology ();

    //! return local coordinates of center in reference element
    FieldVector<ct, 2>& center_codim0_local (int elemtype);

    //! return local coordinates of center of ith codim 1 subentity
    FieldVector<ct, 1>& center_codim1_local (int elemtype, int i);

  private:
    FieldVector<ct, 2> center0_local[2];    // GeometryType
    FieldVector<ct, 1> center1_local[2];    // GeometryType x faces
  };


  // Specialization dim=3
  template<class ct>
  class ReferenceTopology<3,ct> {
  public:
    //! default constructor will build up all the internal data
    ReferenceTopology ();

    //! return local coordinates of center in reference element
    FieldVector<ct, 3>& center_codim0_local (int elemtype);

    //! return local coordinates of center of ith codim 1 subentity
    FieldVector<ct, 2>& center_codim1_local (int elemtype, int i);

  private:
    FieldVector<ct, 3> center0_local[4];      // GeometryType
    FieldVector<ct, 2> center1_local[4][6];   // GeometryType x faces
  };

  // Forward Declarations
  template<int mydim, int cdim, class GridImp,template<int,int,class> class GeometryImp> class Geometry;
  // dim is necessary because Entity will be specialized for codim==0 _and_ codim==dim
  // EntityImp gets GridImp as 3rd template parameter to distinguish between const and mutable grid
  template<int codim, int dim, class GridImp,template<int,int,class> class EntityImp> class Entity;
  template<class GridImp, template<class> class BoundaryEntityImp> class BoundaryEntity;
  template<int codim, PartitionIteratorType pitype, class GridImp,
      template<int,PartitionIteratorType,class> class LevelIteratorImp> class LevelIterator;
  template<class GridImp, template<class> class IntersectionIteratorImp> class IntersectionIterator;
  template<class GridImp, template<class> class HierarchicIteratorImp> class HierarchicIterator;
  template<class GridImp, template<class> class LeafIteratorImp> class LeafIterator;
  template<class GridImp> class GenericLeafIterator;

  //************************************************************************
  // G R I D
  //************************************************************************

  /*!
     A Grid is a container of grid entities. Given a dimension dim these entities have a
     codimension codim with 0 <= codim <= dim.

     The Grid is assumed to be hierachically refined and nested. It enables iteration over
     entities of a given level and codimension.

     The grid can be non-matching.

     All information is provided to allocate degrees of freedom in appropriate vector
     data structures (which are not part of this module).

     Template class Grid defines a "base class" for all grids.
   */
  template< int dim, int dimworld, class ct, template<int,int> class GridImp>
  class Grid {
  public:
    template <int cd>
    struct codim
    {
      // IMPORTANT: codim<codim>::Geometry == Geometry<dim-codim,dimworld>
      typedef typename GridImp<dim,dimworld>::Traits::template codim<cd>::Geometry Geometry;
      typedef typename GridImp<dim,dimworld>::Traits::template codim<cd>::LocalGeometry LocalGeometry;
      // do we need this?
      typedef typename GridImp<dim,dimworld>::Traits::template codim<cd>::ReferenceGeometry ReferenceGeometry;

      typedef typename GridImp<dim,dimworld>::Traits::template codim<cd>::Entity Entity;

      typedef typename GridImp<dim,dimworld>::Traits::template codim<cd>::LevelIterator LevelIterator;

      typedef typename GridImp<dim,dimworld>::Traits::template codim<cd>::EntityPointer EntityPointer;

      template <PartitionIteratorType pitype>
      struct partition
      {
        typedef typename GridImp<dim,dimworld>::Traits::template codim<cd>::template partition<pitype>::LevelIterator LevelIterator;
      };

      typedef typename GridImp<dim,dimworld>::Traits::HierarchicIterator HierarchicIterator;

      typedef typename GridImp<dim,dimworld>::Traits::IntersectionIterator IntersectionIterator;

      typedef typename GridImp<dim,dimworld>::Traits::LeafIterator LeafIterator;

      typedef typename GridImp<dim,dimworld>::Traits::BoundaryEntity BoundaryEntity;
    };

    //! A grid exports its dimension
    enum { dimension=dim };

    //! A grid knowns the dimension of the world
    enum { dimensionworld=dimworld };

    //! Define type used for coordinates in grid module
    typedef ct ctype;

    /*! Return maximum level defined in this grid. Levels are numbered
       0 ... maxlevel with 0 the coarsest level.
     */
    int maxlevel() const
    {
      return asImp().maxlevel();
    }

    //! Return number of grid entities of a given codim on a given level
    int size (int level, int codim) const
    {
      return asImp().size(level,codim);
    }

    //! return size (= distance in graph) of overlap region
    int overlapSize (int level, int codim) const
    {
      return asImp().overlapSize(level,codim);
    }

    //! return size (= distance in graph) of ghost region
    int ghostSize (int level, int codim) const
    {
      return asImp().ghostSize(level,codim);
    }

    //! Iterator to first entity of given codim on level
    template<int cd, PartitionIteratorType pitype>
    typename codim<cd>::template partition<pitype>::LevelIterator lbegin (int level) const
    {
      return asImp().template lbegin<cd,pitype>(level);
    }

    //! one past the end on this level
    template<int cd, PartitionIteratorType pitype>
    typename codim<cd>::template partition<pitype>::LevelIterator lend (int level) const
    {
      return asImp().template lend<cd,pitype>(level);
    }

    //! Iterator to first entity of given codim on level
    template<int cd>
    typename codim<cd>::template partition<All_Partition>::LevelIterator lbegin (int level) const
    {
      return asImp().template lbegin<cd,All_Partition>(level);
    }

    //! one past the end on this level
    template<int cd>
    typename codim<cd>::template partition<All_Partition>::LevelIterator lend (int level) const
    {
      return asImp().template lend<cd,All_Partition>(level);
    }

    //! the generic communication function
    template<class T, template<class> class P, int codim>
    void communicate (T& t, InterfaceType iftype, CommunicationDirection dir, int level) const
    {
      asImp().template communicate<T,P,codim>(t,iftype,dir,level);
    }

  private:
    //!  Barton-Nackman trick
    GridImp<dim,dimworld>& asImp () {return static_cast<GridImp<dim,dimworld>&>(*this);}
    //!  Barton-Nackman trick
    const GridImp<dim,dimworld>& asImp () const {return static_cast<const GridImp<dim,dimworld>&>(*this);}
  };


  //************************************************************************
  //
  //  Default Methods of Grid
  //
  //************************************************************************
  //

  /** \brief The Default Methods of Grid
   *
   * \todo Please doc me!
   */
  template<int dim,
      int dimworld,
      class ct,
      template<int,int> class GridImp>
  class GridDefault : public Grid <dim,dimworld,ct,GridImp>
  {
  public:
    typedef typename Dune::LeafIterator<const GridImp<dim,dimworld>, Dune::GenericLeafIterator> LeafIterator;

    //! return LeafIterator which points to the first entity in maxLevel
    LeafIterator leafbegin(int maxLevel) const
    {
      return GenericLeafIterator< const GridImp<dim,dimworld> >(asImp(),maxLevel,false);
    };

    //! return LeafIterator which points behind the last entity in maxLevel
    LeafIterator leafend(int maxLevel) const
    {
      return GenericLeafIterator< const GridImp<dim,dimworld> >(asImp(),maxLevel,true);
    }

    //***************************************************************
    //  Interface for Adaptation
    //***************************************************************
    /*! \brief marks an entity for refCount refines.
     * If refCount is negative the entity is coarsened -refCount times
     * mark returns true if entity was marked, otherwise false
     *  - Note:
     *    default implementation is: return false; for grids with no
     *    adaptation.
     *  - for the grid programmer:
     *    this method is implemented as a template method, because the
     *    Entity type is not defined when the class is instantiated
     *
     *    You wont need this trick in the implementation.
     *    In your implementation you should use it as
     *    \code
     *    bool mark( int refCount,
     *               typename Traits::template codim<0>::EntityPointer & e ).
     *    \endcode
     *    This template method will vanish due to the inheritance
     *    rules.
     */
    template <class T>
    bool mark( int refCount, T & e )
    {
      IsTrue<Conversion<T, typename Grid<dim,dimworld,ct,GridImp>::template codim<0>::Entity>::sameType >::yes();
      return false;
    }

    /** \brief Refine all positive marked leaf entities
        coarsen all negative marked entities if possible
        \return true if a least one entity was refined

        - Note: this default implementation always returns false
          so grid with no adaptation doesn't need to implement these methods
     */
    bool adapt ()    { return false; }

    //! returns true, if at least one entity is marked for adaption
    bool preAdapt () { return false; }

    //! clean up some markers
    void postAdapt() {}

  protected:
    //! Barton-Nackman trick
    GridImp<dim,dimworld>& asImp () {return static_cast<GridImp<dim,dimworld>&>(*this);}
    const GridImp<dim,dimworld>& asImp () const {return static_cast<const GridImp<dim,dimworld>&>(*this);}
  };

  /** @} */

  template <int dim, int dimw, template<int,int> class GridImp,
      template<int,int,class> class GeometryImp,
      template<int,int,class> class EntityImp,
      template<class> class BoundaryEntityImp,
      template<int,PartitionIteratorType,class> class LevelIteratorImp,
      template<class> class IntersectionIteratorImp,
      template<class> class HierarchicIteratorImp,
      template<class> class LeafIteratorImp = Dune::GenericLeafIterator>
  struct GridTraits
  {
    typedef Dune::IntersectionIterator<const GridImp<dim,dimw>, IntersectionIteratorImp> IntersectionIterator;

    typedef Dune::HierarchicIterator<const GridImp<dim,dimw>, HierarchicIteratorImp> HierarchicIterator;

    typedef Dune::LeafIterator<const GridImp<dim,dimw>, LeafIteratorImp> LeafIterator;

    typedef Dune::BoundaryEntity<const GridImp<dim,dimw>, BoundaryEntityImp> BoundaryEntity;

    template <int cd>
    struct codim
    {
      // IMPORTANT: codim<codim>::Geometry == Geometry<dim-codim,dimw>
      typedef Dune::Geometry<dim-cd, dimw, const GridImp<dim,dimw>, GeometryImp> Geometry;
      typedef Dune::Geometry<dim-cd, dim, const GridImp<dim,dimw>, GeometryImp> LocalGeometry;
      // do we need this?
      typedef Dune::Geometry<dim-cd, dim-cd, const GridImp<dim,dimw>, GeometryImp> ReferenceGeometry;
      // we could - if needed - introduce an other struct for dimglobal of Geometry
      typedef Dune::Entity<cd, dim, const GridImp<dim,dimw>, EntityImp> Entity;

      typedef Dune::LevelIterator<cd,All_Partition,const GridImp<dim,dimw>,LevelIteratorImp> LevelIterator;

      // The wrapper class should be adjusted in future
      typedef const Dune::LevelIterator<cd,All_Partition,const GridImp<dim,dimw>,LevelIteratorImp> EntityPointer;

      //! Please doc me!
      typedef Dune::LevelIterator<cd,Interior_Partition,const GridImp<dim,dimw>,LevelIteratorImp>        InteriorLevelIterator;

      //! Please doc me!
      typedef Dune::LevelIterator<cd,InteriorBorder_Partition,const GridImp<dim,dimw>,LevelIteratorImp>  InteriorBorderLevelIterator;

      //! Please doc me!
      typedef Dune::LevelIterator<cd,Overlap_Partition,const GridImp<dim,dimw>,LevelIteratorImp>         OverlapLevelIterator;

      //! Please doc me!
      typedef Dune::LevelIterator<cd,OverlapFront_Partition,const GridImp<dim,dimw>,LevelIteratorImp>    OverlapFrontLevelIterator;

      //! Please doc me!
      typedef Dune::LevelIterator<cd,Ghost_Partition,const GridImp<dim,dimw>,LevelIteratorImp>           GhostLevelIterator;

      template <PartitionIteratorType pitype>
      struct partition
      {
        typedef Dune::LevelIterator<cd,pitype,const GridImp<dim,dimw>,LevelIteratorImp> LevelIterator;
      };

    };
  };

  /*! \internal
        Used for grid I/O
   */
  enum GridIdentifier { SGrid_Id, AlbertaGrid_Id , SimpleGrid_Id, UGGrid_Id,
                        YaspGrid_Id , BSGrid_Id, OneDGrid_Id};

  //! provide names for the different grid types
  inline std::string transformToGridName(GridIdentifier type)
  {
    switch(type) {
    case SGrid_Id :
      return "SGrid";
    case AlbertaGrid_Id :
      return "AlbertaGrid";
    case SimpleGrid_Id :
      return "SimpleGrid";
    case UGGrid_Id :
      return "UGGrid";
    case YaspGrid_Id :
      return "YaspGrid";
    case BSGrid_Id :
      return "BSGrid";
    case OneDGrid_Id :
      return "OneDGrid";
    default :
      return "unknown";
    }
  }

}

#include "geometry.hh"
#include "entity.hh"
#include "boundary.hh"
#include "leveliterator.hh"
#include "intersectioniterator.hh"
#include "hierarchiciterator.hh"
#include "leafiterator.hh"

#include "grid.cc"

#endif
