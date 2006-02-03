// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_FACECENTERPOINTS_HH
#define DUNE_FACECENTERPOINTS_HH

namespace Dune {

  //******************************************************************
  //
  //!  Memorization of the number of quadrature points
  //
  //******************************************************************
  template <class Domain, class RangeField, int codim >
  struct BaryCenterPoints;

  template <class Domain, class RangeField>
  struct BaryCenterPoints<Domain,RangeField,0>
  {
    enum { identifier = 0 };
    static int numberOfQuadPoints (GeometryType type) { return 1; }
    static int polynomOrder (GeometryType type) { return 1; }
    static RangeField getWeight (GeometryType type, int i)
    {
      assert(i == 0 );
      if(type.isCube()) return 1.0;
      if(type.isTriangle()) return 0.5;
      if(type.isTetrahedron()) return (1.0/6.0);
      return -1.0;
    }
    static Domain getPoint (GeometryType type, int i)
    {
      assert( i == 0 );
      if(type.isCube()) return Domain(0.5);
      if(type.isTriangle()) return Domain(1.0/3.0);
      if(type.isTetrahedron()) return Domain(0.25);
      return Domain(-1.0);
    }
  };

  template <class Domain, class RangeField>
  struct BaryCenterPoints<Domain,RangeField,1>
  {
    enum { identifier = 1 };
    static int numberOfQuadPoints (GeometryType type)
    {
      if(type.isTriangle()) return 3;
      if(type.isTetrahedron()) return 4;
      if(type.isHexahedron()) return 6;
      if(type.isQuadrilateral()) return 4;
      return -1;
    }
    static int polynomOrder (GeometryType type)
    {
      return 2;
    }
    static RangeField getWeight (GeometryType type, int i)
    {
      if(type.isQuadrilateral()) return 0.25;
      if(type.isHexahedron()) return (1.0/6.0);
      if(type.isTriangle()) return (1.0/6.0);
      if(type.isTetrahedron()) return (1.0/24.0);
      return -1.0;
    }
    static Domain getPoint (GeometryType type, int i)
    {
      // quads
      if(type.isQuadrilateral())
      {
        assert( (i>=0) && (i<4) );
        Domain tmp;

        if(i < 2 )
        {
          tmp[1] = 0.5;
          tmp[0] = static_cast<RangeField> (i);
        }
        else
        {
          tmp[0] = 0.5;
          tmp[1] = static_cast<RangeField> (i-2);
        }
        return tmp;
      }

      // triangles
      if(type.isTriangle())
      {
        Domain tmp (0.5);
        switch (i)
        {
        case 0 : {
          return tmp;
        }
        case 1 : {
          tmp[0] = 0.0;
          return tmp;
        }
        case 2 : {
          tmp[1] = 0.0;
          return tmp;
        }
        default :
        {
          std::cerr << "BaryCenterPoints<triangle>::getPoint: i out of range! \n";
          abort();
        }
        }
        return tmp;
      }

      // tetrahedron
      if(type.isTetrahedron())
      {
        Domain tmp (1.0/3.0);
        assert( (i>=0) && (i<4) );

        if(i==0) return tmp;

        tmp[i-1] = 0.0;
        return tmp;
      }

      // hexas
      if(type.isHexahedron())
      {
        assert( (i>=0) && (i<6) );
        Domain tmp;

        tmp[2] = 0.5;
        if(i < 2 )
        {
          tmp[1] = 0.5;
          tmp[0] = static_cast<RangeField> (i);
          return tmp;
        }
        if( (i >= 2) && (i<4) )
        {
          tmp[0] = 0.5;
          tmp[1] = static_cast<RangeField> (i-2);
          return tmp;
        }

        tmp = 0.5;
        tmp[2] = static_cast<RangeField> (i-4);
        return tmp;
      }
      return Domain(-1.0);
    }
  };

} // end namespace Dune
#endif
