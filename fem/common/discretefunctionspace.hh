// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DISCRETEFUNCTIONSPACE_HH
#define DUNE_DISCRETEFUNCTIONSPACE_HH

#include <dune/common/functionspace.hh>
#include "basefunctions.hh"

namespace Dune {

  /** @defgroup DiscreteFunctionSpace DiscreteFunctionSpace
      @ingroup DiscreteFunction
      This provides the interfaces for discrete function spaces.

      @{
   */

  class GeometryIdentifier {
  public:
    enum { numTypes = 9 };
    enum IdentifierType { Vertex = 0, Line, Triangle, Quadrilateral,
                          Tetrahedron, Hexahedron, Pyramid, Prism, Unknown };
  public:
    GeometryIdentifier(IdentifierType idType) :
      identifier_(idType) {}

    GeometryIdentifier(int dimension, NewGeometryType geoType) :
      identifier_(GeometryIdentifier::fromGeo(dimension, geoType)) {}

    operator NewGeometryType() const {
      return GeometryIdentifier::toGeo(identifier_);
    }
    operator IdentifierType() const { return identifier_; }

    static NewGeometryType toGeo(IdentifierType id) {
      switch(id) {
      case Vertex :
        return NewGeometryType(NewGeometryType::simplex,0);
      case Line :
      {
        //lines are simplices, more than cubes ;)
        return NewGeometryType(NewGeometryType::simplex,1);
      }
      case Triangle :
        return NewGeometryType(NewGeometryType::simplex,2);
      case Tetrahedron :
        return NewGeometryType(NewGeometryType::simplex,3);
      case Quadrilateral :
        return NewGeometryType(NewGeometryType::cube,2);
      case Hexahedron :
        return NewGeometryType(NewGeometryType::cube,3);
      case Pyramid :
        return NewGeometryType(NewGeometryType::pyramid,3);
      case Prism :
        return NewGeometryType(NewGeometryType::prism,3);
      default :
      {
        assert(false);
        abort();
        // vertex is the new unknown ;)
        return NewGeometryType(NewGeometryType::simplex,0);
      }
      }
    }

    //! return conversion from geometry to identifier
    static IdentifierType fromGeo(NewGeometryType geo) {
      return fromGeo(geo.dim(),geo);
    }

    //! return conversion from geometry to identifier
    static IdentifierType fromGeo(int dimension, NewGeometryType geo) {

      switch(dim) {
      case 0 : return Vertex;
      case 1 : return Line;
      case 2 :
      {
        if( geo.isSimplex() ) return Triangle;
        if( geo.isCube() ) return Quadrilateral;

        std::cerr<<"Wrong GeometryType in fremGrom \n";
        DUNE_THROW(NotImplemented,"GeometryType not implemented");
        abort();
        return Unknown;
      }
      case 3 :
      {
        if(geo.isSimplex()) return Tetrahedron;
        if(geo.isCube()   ) return Hexahedron;
        if(geo.isPyramid()) return Pyramid;
        if(geo.isPrism()  ) return Prism;

        std::cerr<<"Wrong GeometryType in fremGrom \n";
        DUNE_THROW(NotImplemented,"GeometryType not implemented");
        abort();
        return Unknown;
      }
      default :
        std::cerr<<"Wrong GeometryType in fremGrom \n";
        DUNE_THROW(NotImplemented,"GeometryType not implemented");
        abort();
        return Unknown;
      }
    }

  private:
    IdentifierType identifier_;
  };

  enum DFSpaceIdentifier {  LagrangeSpace_id , DGSpace_id , RaviartThomasSpace_id, PerLagrangeSpace_id };


  //**************************************************************************
  //
  //  --DiscreteFunctionSpaceInterface
  //
  /*! This is the interface for discrete function spaces. All methods
     declared here have to be implemented by the implementation class.
     The discrete function space always depends on a given grid.
     For all diffrent element types of the grid the function space provides
     a set of base functions for the different elements.
     Because of the knowledge of on the one hand the grid an on the other
     hand the base functions sets, the discrete function space provides the size
     of the function space and a mapping from entity and local dof number
     to global dof number of the level of the entity.
     NOTE: A FunctionSpace is defined on a certain level of the grid and can
     also be defined for a certain leaf level, which means on each grid
     level we can have different function spaces each with a different
     number of unknowns.
   */
  template<class FunctionSpaceTraits>
  class DiscreteFunctionSpaceInterface :
    public FunctionSpaceTraits::FunctionSpaceType
  {
  public:
    //- Typedefs and enums
    typedef typename FunctionSpaceTraits::FunctionSpaceType FunctionSpaceType;
    typedef typename FunctionSpaceTraits::DiscreteFunctionSpaceType DiscreteFunctionSpaceType;
    typedef typename FunctionSpaceTraits::BaseFunctionSetType BaseFunctionSetType;
    typedef typename FunctionSpaceTraits::GridType GridType;
    typedef typename FunctionSpaceTraits::IndexSetType IndexSetType;
    typedef typename FunctionSpaceTraits::IteratorType IteratorType;

  public:
    //- Public methods
    //! Constructor
    DiscreteFunctionSpaceInterface(int ident) :
      FunctionSpaceType(ident) {};

    //! Get base function set for given entity.
    //! For a type of element the base function set is unique.
    template <class EntityType>
    const BaseFunctionSetType&  getBaseFunctionSet ( EntityType &en ) const
    {
      return asImp().getBaseFunctionSet( en );
    }

    //! Is space continuous?
    bool continuous() const { return asImp().continuous(); }

    //! Return the corresponding Grid (const version)
    const GridType& grid() const { return asImp().grid(); }

    //! Return the corresponding Grid
    GridType& grid() { return asImp().grid(); }

    //! Return the index set corresponding to the iterator
    const IndexSetType& indexSet() const { return asImp().indexSet(); }

    //! Return number of degrees of freedom for specified grid
    //! which can also be the leaflevel and furthermore
    //! this depends also on the base function set
    int size () const { return asImp().size(); }

    //! For given entity map local dof number to global dof number
    //! at the level of the given entity.
    template <class EntityType>
    int mapToGlobal ( EntityType &en, int localNum ) const
    {
      return asImp().mapToGlobal ( en , localNum );
    }

    //! Iterator over the entities of a space
    //! The index set specifies the subset of grid entities of all available
    //! codimensions; usually, the elements of the space normally belong only
    //! to one codimension, which is selected by the space
    IteratorType begin() const {
      return asImp().begin();
    }

    //! End iterator
    IteratorType end() const {
      return asImp().end();
    }

  protected:
    //! Barton-Nackman trick
    DiscreteFunctionSpaceType& asImp()
    {
      return static_cast<DiscreteFunctionSpaceType&>(*this);
    }

    const DiscreteFunctionSpaceType& asImp() const
    {
      return static_cast<const DiscreteFunctionSpaceType&>(*this);
    }

  }; // end class DiscreteFunctionSpaceInterface

  //**************************************************************************
  //
  // --DiscreteFunctionSpaceDefault
  //
  //! This is the class with default implementations for discrete function
  //! space.
  //!
  //**************************************************************************
  template <class FunctionSpaceTraits>
  class DiscreteFunctionSpaceDefault :
    public DiscreteFunctionSpaceInterface<FunctionSpaceTraits>
  {
  public:
    //! The implementation type
    typedef typename FunctionSpaceTraits::DiscreteFunctionSpaceType DiscreteFunctionSpaceType;

  public:
    //! Constructor
    DiscreteFunctionSpaceDefault(int id) :
      DiscreteFunctionSpaceInterface<FunctionSpaceTraits>(id)
    {}

  private:
    //! Barton-Nackman trick
    DiscreteFunctionSpaceType& asImp()
    {
      return static_cast<DiscreteFunctionSpaceType&>(*this);
    }

    const DiscreteFunctionSpaceType &asImp() const
    {
      return static_cast<const DiscreteFunctionSpaceType&>(*this);
    }
  };

  /** @} end documentation group */

} // end namespace Dune

#endif
