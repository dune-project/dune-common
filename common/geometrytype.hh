// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GEOMETRY_TYPE_HH
#define DUNE_GEOMETRY_TYPE_HH

/** \file
    \brief A unique label for each type of element that can occur in a grid
 */

#include <dune/common/exceptions.hh>

namespace Dune {

  /** \brief Enum that declares identifiers for different geometry types.

     This list can be extended in the future. Not all meshes need to implement
     all geometry types.

     \deprecated Only here during transition to the new ElementType class
   */

  enum GeometryType {vertex,line, triangle, quadrilateral, tetrahedron,
                     pyramid, prism, hexahedron,
                     simplex, cube};

  /** \brief Associates a string to a GeometryType
     \deprecated Only here during transition to the new ElementType class
   */
  inline std::string GeometryName(GeometryType type){
    switch(type) {
    case vertex :
      return "vertex";
    case line :
      return "line";
    case triangle :
      return "triangle";
    case quadrilateral :
      return "quadrilateral";
    case tetrahedron :
      return "tetrahedron";
    case pyramid :
      return "pyramid";
    case prism :
      return "prism";
    case hexahedron :
      return "hexahedron";
    case simplex :
      return "simplex";
    case cube :
      return "cube";

    default :
      DUNE_THROW(NotImplemented, "name of unknown geometry requested");
    }
  }

  /** \brief Stream output of GeometryType
     \deprecated Only here during transition to the new ElementType class
   */
  inline std::ostream& operator<< (std::ostream& s, Dune::GeometryType t)
  {
    s << Dune::GeometryName(t);
    return s;
  }

  /** \brief Unique label for each type of entities that can occur in DUNE grids

     This class has to be extended if a grid implementation with new entity types
     is added to DUNE.
   */
  class NewGeometryType
  {
  public:
    /** \brief Each entity can be tagged by one of these basic types
        plus its space dimension */
    enum BasicType {simplex, cube, pyramid, prism};

  private:

    /** \brief Basic type of the element */
    BasicType basicType_ : 16;

    /** \brief Dimension of the element */
    short dim_;

  public:

    /** \brief Default constructor, not initializing anything */
    NewGeometryType () {}

    /** \brief Constructor */
    NewGeometryType(BasicType basicType, unsigned int dim)
      : basicType_(basicType), dim_(dim)
    {}

    /** \brief Constructor for vertices and segments
        \todo Add check for dim={0,1} when compiled with a suitable flag
     */
    NewGeometryType(unsigned int dim)
      : basicType_(cube), dim_(dim)
    {}

    NewGeometryType(Dune::GeometryType type, unsigned int dim) DUNE_DEPRECATED
      : dim_(dim)
    {
      switch(type) {
      case Dune::vertex :
        basicType_ = Dune::NewGeometryType::cube;
        break;
      case Dune::line :
        basicType_ = Dune::NewGeometryType::cube;
        break;
      case Dune::triangle :
        basicType_ = Dune::NewGeometryType::simplex;
        break;
      case Dune::quadrilateral :
        basicType_ = Dune::NewGeometryType::cube;
        break;
      case Dune::tetrahedron :
        basicType_ = Dune::NewGeometryType::simplex;
        break;
      case Dune::pyramid :
        basicType_ = Dune::NewGeometryType::pyramid;
        break;
      case Dune::prism :
        basicType_ = Dune::NewGeometryType::prism;
        break;
      case Dune::hexahedron :
        basicType_ = Dune::NewGeometryType::cube;
        break;
      case Dune::simplex :
        basicType_ = Dune::NewGeometryType::simplex;
        break;
      case Dune::cube :
        basicType_ = Dune::NewGeometryType::cube;
        break;
      };
    }

    /** \brief Cast to old-style GeometryType
        \deprecated Only here for backward compatibility
     */
    operator GeometryType() const DUNE_DEPRECATED {
      switch (basicType_) {
      case simplex : return Dune::simplex;
      case cube :    return Dune::cube;
      case pyramid : return Dune::pyramid;
      case prism :   return Dune::prism;
      }
      DUNE_THROW(Exception, "Non-existing BasicType found!");
    }

    /** @name Setup Methods */
    /*@{*/

    /** \brief Make a vertex */
    void makeVertex() {dim_ = 0;}

    /** \brief Make a line segment */
    void makeLine() {dim_ = 1;}

    /** \brief Make a triangle */
    void makeTriangle() {basicType_ = simplex; dim_ = 2;}

    /** \brief Make a quadrilateral */
    void makeQuadrilateral() {basicType_ = cube; dim_ = 2;}

    /** \brief Make a tetrahedron */
    void makeTetrahedron() {basicType_ = simplex; dim_ = 3;}

    /** \brief Make a pyramid */
    void makePyramid() {basicType_ = pyramid;}

    /** \brief Make a prism */
    void makePrism() {basicType_ = prism;}

    /** \brief Make a hexahedron */
    void makeHexahedron() {basicType_ = cube; dim_ = 3;}

    /** \brief Make a simplex of given dimension */
    void makeSimplex(unsigned int dim) {basicType_ = simplex; dim_ = dim;}

    /** \brief Make a hypercube of given dimension */
    void makeCube(unsigned int dim) {basicType_ = cube; dim_ = dim;}

    /*@}*/


    /** @name Query Methods */
    /*@{*/
    /** \brief Return true if entity is a vertex */
    bool isVertex() const {return dim_==0;}

    /** \brief Return true if entity is a line segment */
    bool isLine() const {return dim_==1;}

    /** \brief Return true if entity is a triangle */
    bool isTriangle() const {return basicType_==simplex && dim_==2;}

    /** \brief Return true if entity is a quadrilateral */
    bool isQuadrilateral() const {return basicType_==cube && dim_==2;}

    /** \brief Return true if entity is a tetrahedron */
    bool isTetrahedron() const {return basicType_==simplex && dim_==3;}

    /** \brief Return true if entity is a pyramid */
    bool isPyramid() const {return basicType_==pyramid;}

    /** \brief Return true if entity is a prism */
    bool isPrism() const {return basicType_==prism;}

    /** \brief Return true if entity is a hexahedron */
    bool isHexahedron() const {return basicType_==cube && dim_==3;}

    /** \brief Return true if entity is a simplex of any dimension */
    bool isSimplex() const {return basicType_==simplex || dim_ < 2;}

    /** \brief Return true if entity is a cube of any dimension */
    bool isCube() const {return basicType_==cube || dim_ < 2;}

    /** \brief Return dimension of the entity */
    unsigned int dim() const {return dim_;}

    /** \brief Return the basic type of the entity */
    BasicType basicType() const {return basicType_;}

    /*@}*/

    /** \brief Check for equality */
    bool operator==(const NewGeometryType& other) const {
      return ( (dim()==0 && other.dim()==0)
               || (dim()==1 && other.dim()==1)
               || (dim()==other.dim() && basicType_==other.basicType_) );
    }

    /** \brief Check for inequality */
    bool operator!=(const NewGeometryType& other) const {
      return ! ((*this)==other);
    }

    /** \brief Prints the type to an output stream */
    friend std::ostream& operator<< (std::ostream& s, const NewGeometryType& a)
    {
      switch (a.basicType_) {
      case simplex :
        s << "(simplex, " << a.dim_ << ")";
        break;
      case cube :
        s << "(cube, " << a.dim_ << ")";
        break;
      case pyramid :
        s << "pyramid";
        break;
      case prism :
        s << "prims";
      }

      return s;
    }

  };

  /** \brief Prints a NewGeometryType::BasicType to an output stream */
  std::ostream& operator<< (std::ostream& s, NewGeometryType::BasicType type)
  {
    switch (type) {
    case NewGeometryType::simplex : s << "simplex"; break;
    case NewGeometryType::cube :    s << "cube";    break;
    case NewGeometryType::pyramid : s << "pyramid"; break;
    case NewGeometryType::prism :   s << "prims";   break;
    default : s << "[unknown NewGeometryType::BasicType: " << int(type) << "]";
    }

    return s;
  }
}

#endif
