// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GEOMETRY_TYPE_HH
#define DUNE_GEOMETRY_TYPE_HH

#include <cassert>

/** \file
    \brief A unique label for each type of element that can occur in a grid
 */

#include <dune/common/exceptions.hh>

namespace Dune {

  /** \brief Unique label for each type of entities that can occur in DUNE grids

     This class has to be extended if a grid implementation with new entity types
     is added to DUNE.

      \ingroup COGeometryType
   */
  class GeometryType
  {
  public:
    /** \brief Each entity can be tagged by one of these basic types
        plus its space dimension */
    enum BasicType {
      simplex,               //!< Simplicial element in any nonnegative dimension
      cube,                  //!< Cube element in any nonnegative dimension
      pyramid,               //!< Four sided pyramid in three dimensions
      prism,                 //!< Prism element in three dimensions
      extended,      //!< Other, more general geometry, representable as topologyId
      none                   //!< Generic element in any nonnegative dimension
    };

    /** \brief A few binary constants */
    enum Binary {
      b0001 = 1,
      b0011 = 3,
      b0101 = 5,
      b0111 = 7
    };
  private:

    /** \brief Topology Id element */
    unsigned int topologyId_;

    /** \brief Dimension of the element */
    unsigned char dim_  : 7;

    /** \brief bool if this is none-type */
    bool none_ : 1;

  public:
    /** \brief Default constructor, not initializing anything */
    GeometryType ()
      : topologyId_(0), dim_(0), none_(true)
    {}

    /** \brief Constructor */
    GeometryType(BasicType basicType, unsigned int dim)
      : topologyId_(0), dim_(dim), none_(false)
    {
      if (dim < 2)
        return;
      switch( basicType )
      {
      case GeometryType::simplex :
        makeSimplex(dim);
        break;
      case GeometryType::cube :
        makeCube(dim);
        break;
      case GeometryType::pyramid :
        if (dim == 3)
          makePyramid();
        break;
      case GeometryType::prism :
        if (dim == 3)
          makePrism();
        break;
      case GeometryType::none :
        makeNone(dim);
        break;
      default :
        DUNE_THROW( RangeError,
                    "Invalid basic geometry type: " << basicType << " for dimension " << dim << "." );
      }
    }

    /** \brief Constructor */
    GeometryType(unsigned int topologyId, unsigned int dim)
      : topologyId_(topologyId), dim_(dim), none_(false)
    {}

    /** \brief Constructor from static TopologyType class
     *
     * Constructs the GeometryType object from a static topology representation.
     *
     * \tparam TopologyType A class providing public static unsigned int members
     *                      TopologyType::dimension and TopologyType::id.
     *                      You can e.g. use the Point, Prism and Pyramid structs from
     *                      topologytypes.hh in dune-grid.
     * \param t             Any object of type TopologyType. The object t itself is ignored.
     */
    template<class TopologyType>
    GeometryType(TopologyType t)
      : topologyId_(TopologyType::id), dim_(TopologyType::dimension), none_(false)
    {}

    /** \brief Constructor for vertices and segments */
    explicit GeometryType(unsigned int dim)
      : topologyId_(0), dim_(dim), none_(false)
    {
      assert(dim < 2);
    }

    /** \brief Constructor for vertices and segments */
    explicit GeometryType(int dim)
      : topologyId_(0), dim_(dim), none_(false)
    {
      assert(dim < 2);
    }

    /** @name Setup Methods */
    /*@{*/

    /** \brief Make a vertex */
    void makeVertex() {
      none_  = false;
      dim_ = 0;
      topologyId_ = 0;
    }

    /** \brief Make a line segment */
    void makeLine() {
      none_  = false;
      dim_ = 1;
      topologyId_ = 0;
    }

    /** \brief Make a triangle */
    void makeTriangle() {
      makeSimplex(2);
    }

    /** \brief Make a quadrilateral */
    void makeQuadrilateral() {
      makeCube(2);
    }

    /** \brief Make a tetrahedron */
    void makeTetrahedron() {
      makeSimplex(3);
    }

    /** \brief Make a pyramid */
    void makePyramid() {
      none_  = false;
      dim_ = 3;
      topologyId_ = b0011;
    }

    /** \brief Make a prism */
    void makePrism() {
      none_  = false;
      dim_ = 3;
      topologyId_ = b0101;       // (1 << (dim_-1)) - 1;
    }

    /** \brief Make a hexahedron */
    void makeHexahedron() {
      makeCube(3);
    }

    /** \brief Make a simplex of given dimension */
    void makeSimplex(unsigned int dim) {
      none_  = false;
      dim_ = dim;
      topologyId_ = 0;
    }

    /** \brief Make a hypercube of given dimension */
    void makeCube(unsigned int dim) {
      none_  = false;
      dim_ = dim;
      topologyId_ = ((dim>1) ? ((1 << dim) - 1) : 0);
    }

    /** \brief Make a singular of given dimension */
    void makeNone(unsigned int dim) {
      none_ = true;
      dim_ = dim;
      topologyId_  = 0;
    }

    /*@}*/


    /** @name Query Methods */
    /*@{*/
    /** \brief Return true if entity is a vertex */
    bool isVertex() const {
      return dim_==0;
    }

    /** \brief Return true if entity is a line segment */
    bool isLine() const {
      return dim_==1;
    }

    /** \brief Return true if entity is a triangle */
    bool isTriangle() const {
      return ! none_ && dim_==2 && (topologyId_ | 1) == b0001;
    }

    /** \brief Return true if entity is a quadrilateral */
    bool isQuadrilateral() const {
      return ! none_ && dim_==2 && (topologyId_ | 1) == b0011;
    }

    /** \brief Return true if entity is a tetrahedron */
    bool isTetrahedron() const {
      return ! none_ && dim_==3 && (topologyId_ | 1) == b0001;
    }

    /** \brief Return true if entity is a pyramid */
    bool isPyramid() const {
      return ! none_ && dim_==3 && (topologyId_ | 1) == b0011;
    }

    /** \brief Return true if entity is a prism */
    bool isPrism() const {
      return ! none_ && dim_==3 && (topologyId_ | 1) == b0101;
    }

    /** \brief Return true if entity is a hexahedron */
    bool isHexahedron() const {
      return ! none_ && dim_==3 && (topologyId_ | 1) == b0111;
    }

    /** \brief Return true if entity is a simplex of any dimension */
    bool isSimplex() const {
      return ! none_ && (topologyId_ | 1) == 1;
    }

    /** \brief Return true if entity is a cube of any dimension */
    bool isCube() const {
      return ! none_ && ((topologyId_ ^ ((1 << dim_)-1)) >> 1 == 0);
    }

    /** \brief Return true if entity is a singular of any dimension */
    bool isNone() const {
      return none_;
    }

    /** \brief Return dimension of the type */
    unsigned int dim() const {
      return dim_;
    }

    /** \brief Return the basic type of the type */
    BasicType basicType() const DUNE_DEPRECATED {
      if (isSimplex())
        return GeometryType::simplex;
      if (isCube())
        return GeometryType::cube;
      if (isPyramid())
        return GeometryType::pyramid;
      if (isPrism())
        return GeometryType::prism;
      if (isNone())
        return GeometryType::none;
      return GeometryType::extended;
    }

    /** \brief Return the topology id the type */
    unsigned int id() const {
      return topologyId_;
    }

    /*@}*/

    /** \brief Check for equality. This method knows that in dimension 0 and 1
                    all BasicTypes are equal.
     */
    bool operator==(const GeometryType& other) const {
      return ( ( none_ == other.none_ )
               && ( ( none_ == true )
                    || ( ( dim_ == other.dim_ )
                         && ( ( dim_ < 2 )
                              || ( topologyId_ == other.topologyId_ )
                              )
                         )
                    )
               );
    }

    /** \brief Check for inequality */
    bool operator!=(const GeometryType& other) const {
      return ! ((*this)==other);
    }

    /** \brief less-than operation for use with maps */
    bool operator < (const GeometryType& other) const {
      return ( ( none_ < other.none_ )
               || ( !( other.none_ < none_ )
                    && ( ( dim_ < other.dim_ )
                         || ( !( other.dim_ < dim_ )
                              && ( topologyId_ < other.topologyId_ )
                              )
                         )
                    )
               );
    }
  };

  /** \brief Prints the type to an output stream */
  inline std::ostream& operator<< (std::ostream& s, const GeometryType& a)
  {
    if (a.isSimplex())
    {
      s << "(simplex, " << a.dim() << ")";
      return s;
    }
    if (a.isCube())
    {
      s << "(cube, " << a.dim() << ")";
      return s;
    }
    if (a.isPyramid())
    {
      s << "(pyramid, 3)";
      return s;
    }
    if (a.isPrism())
    {
      s << "(prism, 3)";
      return s;
    }
    if (a.isNone())
    {
      s << "(none, " << a.dim() << ")";
      return s;
    }
    s << "(other [" << a.id() << "], " << a.dim() << ")";
    return s;
  }

  /** \brief Prints a GeometryType::BasicType to an output stream */
  inline std::ostream& operator<< (std::ostream& s, GeometryType::BasicType type)
  {
    switch (type) {
    case GeometryType::simplex :
      s << "simplex";
      break;
    case GeometryType::cube :
      s << "cube";
      break;
    case GeometryType::pyramid :
      s << "pyramid";
      break;
    case GeometryType::prism :
      s << "prism";
      break;
    case GeometryType::extended :
      s << "other";
    case GeometryType::none :
      s << "none";
      break;
    default :
      DUNE_THROW(Exception, "invalid GeometryType::BasicType");
    }
    return s;
  }
}

#endif
