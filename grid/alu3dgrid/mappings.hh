// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DGRIDMAPPINGS_HH
#define DUNE_ALU3DGRIDMAPPINGS_HH

// System includes
#include <limits>

// Dune includes
#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>

// Local includes
#include "alu3dinclude.hh"

namespace Dune {

  static const alu3d_ctype ALUnumericEpsilon = 10.0 * std::numeric_limits< alu3d_ctype >::epsilon();

  //! A trilinear mapping from the Dune reference hexahedron into the physical
  //! space (same as in mapp_cube_3d.h, but for a different reference hexahedron)
  class TrilinearMapping {
    typedef FieldVector<double, 3> coord_t;
    typedef FieldMatrix<double, 3, 3> mat_t;
    static const double _epsilon ;

    // the internal mapping
    double a [8][3] ;
    mat_t Df;
    mat_t Dfi;
    double DetDf ;
    void linear (const coord_t&) ;
    void inverse (const coord_t&) ;
  public:
    TrilinearMapping (const coord_t&, const coord_t&,
                      const coord_t&, const coord_t&,
                      const coord_t&, const coord_t&,
                      const coord_t&, const coord_t&);
    TrilinearMapping (const TrilinearMapping &) ;
    ~TrilinearMapping () {}
    double det (const coord_t&) ;
    mat_t jacobianInverse(const coord_t&);
    void map2world (const coord_t&, coord_t&) const ;
    void map2world (const double , const double , const double ,
                    coord_t&) const ;
    void world2map (const coord_t&, coord_t&) ;
  };

  //! A bilinear surface mapping
  // NOTE: this class is different to the BilinearSurfaceMapping in
  // ALUGrid, for example the reference elements differ
  // here we have [0,1]^2 and in ALUGrid its [-1,1]^2
  // also the point numbering is different
  class BilinearSurfaceMapping {
    // our coordinate types
    typedef FieldVector<double, 3> coord3_t;
    typedef FieldVector<double, 2> coord2_t;

    // type of coordinate vectors from elements
    typedef double double3_t[3];

    double _b [4][3] ;
    double _n [3][3] ;
    static const double _epsilon ;

  public:
    //! Constructor creating empty mapping with double , i.e. zero
    BilinearSurfaceMapping (double);

    //! Constructor getting FieldVectors
    BilinearSurfaceMapping (const coord3_t&, const coord3_t&,
                            const coord3_t&, const coord3_t&) ;
    //! Constructor for double[3]
    BilinearSurfaceMapping (const double3_t &, const double3_t &,
                            const double3_t &, const double3_t &) ;
    BilinearSurfaceMapping (const BilinearSurfaceMapping &) ;
    ~BilinearSurfaceMapping () {}

    void map2world(const coord2_t&, coord3_t&) const ;
    void map2world(double x, double y, coord3_t&) const ;
    void world2map(const coord3_t &, coord2_t & ) const;
    void normal(const coord2_t&, coord3_t&) const ;

    // builds _b and _n, called from the constructors
    template <class vector_t>
    void buildMapping (const vector_t & , const vector_t & ,
                       const vector_t & , const vector_t & );
  } ;

  //! General form of non-conforming face mapping
  //! This class is empty and needs to be specialised
  template <ALU3dGridElementType type>
  class NonConformingFaceMapping {};

  //! Non-conforming face mappings for tetrahedra
  template <>
  class NonConformingFaceMapping<tetra> {
  private:
    //- private typedefs and enums
  public:
    //- public typedefs and enums
    typedef FieldVector<alu3d_ctype, 3> CoordinateType;
    typedef ALU3DSPACE Hface3RuleType RefinementRuleType;

  public:
    //- public interface methods
    NonConformingFaceMapping(RefinementRuleType rule,
                             int nChild);
    NonConformingFaceMapping(const NonConformingFaceMapping<tetra>& orig);
    ~NonConformingFaceMapping();

    void child2parent(const CoordinateType& childCoordinates,
                      CoordinateType& parentCoordinates) const ;

  private:
    void child2parentNosplit(const CoordinateType& childCoordinates,
                             CoordinateType& parentCoordinates) const;
    void child2parentE01(const CoordinateType& childCoordinates,
                         CoordinateType& parentCoordinates) const;
    void child2parentE12(const CoordinateType& childCoordinates,
                         CoordinateType& parentCoordinates) const;
    void child2parentE20(const CoordinateType& childCoordinates,
                         CoordinateType& parentCoordinates) const;
    void child2parentIso4(const CoordinateType& childCoordinates,
                          CoordinateType& parentCoordinates) const;
  private:
    //- private data
    RefinementRuleType rule_;
    int nChild_;
  };

  //! Non-conforming face mappings for hexahedra
  template <>
  class NonConformingFaceMapping<hexa> {
  private:
    //- private typedefs and enums
  public:
    //- public typedefs and enums
    typedef FieldVector<alu3d_ctype, 2> CoordinateType;
    typedef ALU3DSPACE Hface4RuleType RefinementRuleType;
  public:
    NonConformingFaceMapping(RefinementRuleType rule, int nChild);
    NonConformingFaceMapping(const NonConformingFaceMapping<hexa>& orig);
    ~NonConformingFaceMapping();

    void child2parent(const CoordinateType& childCoordinates,
                      CoordinateType& parentCoordinates) const;
  private:
    void child2parentNosplit(const CoordinateType& childCoordinates,
                             CoordinateType& parentCoordinates) const;
    void child2parentIso4(const CoordinateType& childCoordinates,
                          CoordinateType& parentCoordinates) const;

  private:
    RefinementRuleType rule_;
    int nChild_;
  };

} // end namespace Dune

#include "mappings_imp.cc"

#endif
