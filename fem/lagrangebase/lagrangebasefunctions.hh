// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_LAGRANGEBASEFUNCTIONS_HH
#define DUNE_LAGRANGEBASEFUNCTIONS_HH

// - System includes
#include <vector>

#include <dune/common/array.hh>
#include <dune/grid/common/grid.hh>

#include <dune/fem/common/fastbase.hh>
#include <dune/fem/common/basefunctionfactory.hh>

namespace Dune {

  //! definition of LagrangeBaseFunction, implementation via specialization
  template<class FunctionSpaceType, GeometryIdentifier::IdentifierType ElType, int polOrd>
  class LagrangeBaseFunction;

  //! Piecewise const base functions
  template<class FunctionSpaceType, GeometryIdentifier::IdentifierType ElType>
  class LagrangeBaseFunction < FunctionSpaceType , ElType , 0 >
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    enum { DimRange = FunctionSpaceType::DimRange };
    int baseNum_;

    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;

  public:
    LagrangeBaseFunction (int baseNum) :
      BaseFunctionInterface<FunctionSpaceType> (),
      baseNum_ ( baseNum )
    {
      assert((baseNum_ >= 0) || (baseNum_ < DimRange));
    }

    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      phi = 0.0;
      phi[baseNum_] = 1.0;
    }

    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      phi = 0.0;
    }

    virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      phi = 0.0 ;
    }

  };


  //*****************************************************************
  //
  //! Lagrange base for Lines and polynom order = 1
  //! (0) 0-----1 (1)
  //
  //*****************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction < FunctionSpaceType , GeometryIdentifier::Line , 1 >
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    RangeFieldType factor[2];

  public:
    LagrangeBaseFunction ( int baseNum  )
      : BaseFunctionInterface<FunctionSpaceType> ()
    {
      if(baseNum == 0)
      { // 1 - x
        factor[0] = -1.0;
        factor[1] =  1.0;
      }
      else
      { // x
        factor[0] = 1.0;
        factor[1] = 0.0;
      }
    }

    //! evaluate the function
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      phi = factor[1];
      phi += factor[0] * x[0];
    }

    //! evaluate first derivative
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      int num = diffVariable[0];
      phi = factor[num];
    }

    //! evaluate second derivative
    virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      // function is Linear, therefore
      phi = 0.0 ;
    }

  };

  //*****************************************************************
  //
  /** \brief ???

     \verbatim
     (0,1)
      2|\    coordinates and local node numbers
   | \
   |  \
   |   \
   |    \
   |     \
      0|______\1
      (0,0)    (1,0)
     \endverbatim
   */
  //
  //*****************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction < FunctionSpaceType , GeometryIdentifier::Triangle , 1 >
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    RangeFieldType factor[3];
    int baseNum_;

  public:
    //! \todo Please doc me!
    LagrangeBaseFunction (int baseNum)
      : BaseFunctionInterface<FunctionSpaceType> ()
    {
      baseNum_ = baseNum;
      if(baseNum == 0)
      { // 1 - x - y
        factor[0] =  1.0;
        factor[1] = -1.0;
        factor[2] = -1.0;
      }
      else
      {
        factor[0] = 0.0;
        for(int i=1; i<3; i++) // x , y
          if(baseNum == i)
            factor[i] = 1.0;
          else
            factor[i] = 0.0;
      }
    }
    //! evaluate the function
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      phi = factor[0];
      for(int i=1; i<3; i++)
        phi += factor[i] * x[i-1];
    }

    //! \todo Please doc me!
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      // x or y ==> 1 or 2
      int num = diffVariable[0];
      assert( (num >= 0) && ( num <= 1));
      phi = factor[num+1];
    }

    //! \todo Please doc me!
    virtual void evaluate ( const DiffVariable<2>::Type &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      // function is Linear, therefore
      phi = 0.0 ;
    }
  };

  //*****************************************************************
  //
  //! LagrangeBaseFunction for Tetrahedrons and polynom order = 1
  //!
  //!  see reference element Dune tetrahedra
  //!
  //*****************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction < FunctionSpaceType , GeometryIdentifier::Tetrahedron , 1 >
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    RangeFieldType factor[4];
  public:

    //! \todo Please doc me!
    LagrangeBaseFunction ( int baseNum  )
      : BaseFunctionInterface<FunctionSpaceType> ()
    {
      for(int i=1; i<4; i++) // x,y,z
        if(baseNum == i)
          factor[i] = 1.0;
        else
          factor[i] = 0.0;

      if(baseNum == 0) // 1 - x - y - z
      {
        for(int i=1; i<4; i++)
          factor[i] = -1.0;
        factor[0] = 1.0;
      }
      else
        factor[0] = 0.0;
    }

    //! evaluate function
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      phi = factor[0];
      for(int i=1; i<4; i++)
        phi += factor[i]*x[i-1];
    }

    //! first Derivative
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      // num = 0 ==> derivative respect to x
      int num = diffVariable[0];
      phi = factor[num+1];
    }

    //! second Derivative
    virtual void evaluate ( const DiffVariable<2>::Type &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      // function is Linear, therfore
      phi = 0.0 ;
    }
  };

  //*********************************************************************
  //
  //! BiLinear BaseFunctions for Quadrilaterals
  //! v(x,y) = (alpha + beta * x) * ( gamma + delta * y)
  //! see W. Hackbusch, page 162
  //
  //*********************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction<FunctionSpaceType,GeometryIdentifier::Quadrilateral,1>
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    enum { dim = 2 };
    //! phi(x,y) = (factor[0][0] + factor[0][1] * x) * ( factor[1][0] + factor[1][1] * y)
    RangeFieldType factor[dim][2];
  public:

    //! Constructor making base function number baseNum
    LagrangeBaseFunction ( int baseNum )
      : BaseFunctionInterface<FunctionSpaceType>()
    {
      assert((baseNum >= 0) || (baseNum < 4));
      // looks complicated but works
      int fak[dim] = {0,0};

      fak[0] = baseNum%2; // 0,2 ==> 0, 1,3 ==> 1
      fak[1] = (baseNum%4 > 1) ? 1 : 0; // 2,3,6,7 ==> 1 | 0,1,4,5 ==> 0

      // tensor product
      for(int i=0; i<dim; i++)
      {
        factor[i][0] = ( fak[i] == 0 ) ?  1.0 : 0.0 ;
        factor[i][1] = ( fak[i] == 0 ) ? -1.0 : 1.0 ;
      }
    };

    //! evaluate the basefunction on point x
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      // dim == 2, tested
      phi = 1.0;
      for(int i=0; i<dim; i++)
        phi *= (factor[i][0] + (factor[i][1] * x[i]));
    }

    //! derivative with respect to x or y
    //! diffVariable(0) == 0   ==> x
    //! diffVariable(0) == 1   ==> y
    //! diffVariable(0) == 2   ==> z,  and so on
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      int num = diffVariable[0];
      assert( (num >= 0) && ( num <= 1));
      phi = 1.0;
      for(int i=0; i<dim; i++)
      {
        if(num == i)
          phi *= factor[num][1];
        else
          phi *= (factor[i][0] + factor[i][1] * x[i]);
      }
      return;
    }

    //! evaluate second derivative
    virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      // which means derivative xx or yy
      if(diffVariable[0] == diffVariable[1])
      {
        phi = 0.0;
        return;
      }
      // which means derivative xy or yx
      else
      {
        phi = factor[0][1] * factor[1][1];
        return;
      }
      phi = 0.0;
    }
  };

  // *********************************************************************
  //
  //   First-order Lagrange shape functions for the Pyramid
  //
  // *********************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction<FunctionSpaceType,GeometryIdentifier::Pyramid,1>
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    enum { dim = 3 };
    /* phi(x,y,z) = (factor[0] + factor[1]*x)
     *            * (factor[2] + factor[3]*y)
     *            + factor[4] * z * (factor[5] + factor[6]* min(x,y))
     */
    RangeFieldType factor[7];
  public:

    //! Constructor making base function number baseNum
    LagrangeBaseFunction ( int baseNum )
      : BaseFunctionInterface<FunctionSpaceType>()
    {
      assert((baseNum >= 0) || (baseNum < 5));

      switch (baseNum) {
      case 0 :
        factor[0] = 1;
        factor[1] = -1;
        factor[2] = 1;
        factor[3] = -1;
        factor[4] = -1;
        factor[5] = 1;
        factor[6] = -1;
        break;
      case 1 :
        factor[0] = 0;
        factor[1] = 1;
        factor[2] = 1;
        factor[3] = -1;
        factor[4] = -1;
        factor[5] = 0;
        factor[6] = 1;
        break;
      case 2 :
        factor[0] = 0;
        factor[1] = 1;
        factor[2] = 0;
        factor[3] = 1;
        factor[4] = 1;
        factor[5] = 0;
        factor[6] = 1;
        break;
      case 3 :
        factor[0] = 1;
        factor[1] = -1;
        factor[2] = 0;
        factor[3] = 1;
        factor[4] = -1;
        factor[5] = 0;
        factor[6] = 1;
        break;
      case 4 :
        factor[0] = 0;
        factor[1] = 0;
        factor[2] = 0;
        factor[3] = 0;
        factor[4] = 1;
        factor[5] = 1;
        factor[6] = 0;
        break;
      }

    }


    //! evaluate the basefunction on point x
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      phi = (factor[0] + factor[1]*x[0])
            * (factor[2] + factor[3]*x[1])
            + factor[4] * x[2]
            * (factor[5] + factor[6]* std::min(x[0],x[1]));
    }

    //! derivative with respect to x or y or z
    //! diffVariable(0) == 0   ==> x
    //! diffVariable(0) == 1   ==> y
    //! diffVariable(0) == 2   ==> z,  and so on
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      switch (diffVariable[0]) {
      case 0 :
        phi =  factor[1]*(factor[2] + factor[3]*x[1]) + ((x[0] <= x[1]) ? factor[4]*factor[6]*x[2] : 0);
        return;
      case 1 :
        phi = (factor[0] + factor[1]*x[0])*factor[3] + ((x[0] <= x[1]) ? 0 : factor[4]*factor[6]*x[2]);
        return;
      case 2 :
        phi = factor[4] * (factor[5] + factor[6]*std::min(x[0],x[1]));
        return;

      }
    }

    //! evaluate second derivative
    virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      std::cout << "BaseFunction for Pyramid, evaluate 2nd derivative not implemented! \n";
      phi = 0.0;
    }

  };



  // *********************************************************************
  //
  //   First-order Lagrange shape functions for the Prism
  //
  // *********************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction<FunctionSpaceType,GeometryIdentifier::Prism,1>
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    enum { dim = 3 };
    /* phi(x,y,z) = (factor[0] + factor[1]*x + factor[2]*y)
     *            * (factor[3] + factor[4]*z)
     */
    RangeFieldType factor[5];
  public:

    //! Constructor making base function number baseNum
    LagrangeBaseFunction (  int baseNum )
      : BaseFunctionInterface<FunctionSpaceType>()
    {
      assert((baseNum >= 0) || (baseNum < 6));

      switch (baseNum) {
      case 0 :
        factor[0] = 1;
        factor[1] = -1;
        factor[2] = -1;
        factor[3] = 1;
        factor[4] = -1;
        break;
      case 1 :
        factor[0] = 0;
        factor[1] = 1;
        factor[2] = 0;
        factor[3] = 1;
        factor[4] = -1;
        break;
      case 2 :
        factor[0] = 0;
        factor[1] = 0;
        factor[2] = 1;
        factor[3] = 1;
        factor[4] = -1;
        break;
      case 3 :
        factor[0] = 1;
        factor[1] = -1;
        factor[2] = -1;
        factor[3] = 0;
        factor[4] = 1;
        break;
      case 4 :
        factor[0] = 0;
        factor[1] = 1;
        factor[2] = 0;
        factor[3] = 0;
        factor[4] = 1;
        break;
      case 5 :
        factor[0] = 0;
        factor[1] = 0;
        factor[2] = 1;
        factor[3] = 0;
        factor[4] = 1;
        break;
      }

    }


    //! evaluate the basefunction on point x
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      phi = (factor[0] + factor[1]*x[0] + factor[2]*x[1])
            * (factor[3] + factor[4]*x[2]);
    }

    //! derivative with respect to x or y or z
    //! diffVariable(0) == 0   ==> x
    //! diffVariable(0) == 1   ==> y
    //! diffVariable(0) == 2   ==> z,  and so on
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      switch (diffVariable[0]) {
      case 0 :
        phi = factor[1] * (factor[3] + factor[4]*x[2]);
        return;
      case 1 :
        phi = factor[2] * (factor[3] + factor[4]*x[2]);
        return;
      case 2 :
        phi = (factor[0] + factor[1]*x[0] + factor[2]*x[1]) * factor[4];
        return;

      }
    }

    //! evaluate second derivative
    virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      std::cout << "BaseFunction for Pyramid, evaluate 2nd derivative not implemented! \n";
      phi = 0.0;
    }

  };


  //*********************************************************************
  //
  //
  //! Trilinear BaseFunctions for Hexahedrons
  //! v(x,y,z) = (alpha + beta * x) * ( gamma + delta * y) * (omega + eps * z)
  //!
  //!
  //! local node numbers and face numbers for DUNE Hexahedrons
  //!
  //!             6---------7
  //!            /.        /|
  //!           / .  5    / |
  //!          /  .      /  |
  //!         4---------5   | <-- 3 (back side)
  //!   0 --> |   .     | 1 |
  //!         |   2.....|...3 (1,1,0)
  //!         |  .      |  /
  //!         | .   2   | / <-- 4 (front side)
  //!         |.        |/
  //!         0---------1
  //!      (0,0,0)    (1,0,0)
  //!  this is the DUNE local coordinate system for Hexahedrons
  //!
  //*********************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction<FunctionSpaceType,GeometryIdentifier::Hexahedron,1>
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    enum { dim = 3 };
    //! phi(x,y,z) = (factor[0][0] + factor[0][1] * x)
    //!            = (factor[1][0] + factor[1][1] * y)
    //!            = (factor[2][0] + factor[2][1] * z)
    RangeFieldType factor[dim][2];
  public:

    //! Constructor making base function number baseNum
    LagrangeBaseFunction ( int baseNum )
      : BaseFunctionInterface<FunctionSpaceType>()
    {
      assert((baseNum >= 0) || (baseNum < 8));
      // looks complicated but works
      int fak[dim] = {0,0,0};
      fak[0] =  baseNum%2; // 0,2 ==> 0, 1,3 ==> 1
      fak[1] = (baseNum%4 > 1) ? 1 : 0; // 2,3,6,7 ==> 1 | 0,1,4,5 ==> 0
      fak[2] = (baseNum > 3) ? 1 : 0;

      // tensor product
      for(int i=0; i<dim; i++)
      {
        factor[i][0] = ( fak[i] == 0 ) ?  1.0 : 0.0 ;
        factor[i][1] = ( fak[i] == 0 ) ? -1.0 : 1.0 ;
      }

    };

    //! evaluate the basefunction on point x
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      // dim == 3, tested
      phi = 1.0;
      for(int i=0; i<dim; i++)
        phi *= (factor[i][0] + (factor[i][1] * x[i]));
    }

    //! derivative with respect to x or y or z
    //! diffVariable(0) == 0   ==> x
    //! diffVariable(0) == 1   ==> y
    //! diffVariable(0) == 2   ==> z,  and so on
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      int num = diffVariable[0];
      phi = 1.0;
      for(int i=0; i<dim; i++)
      {
        if(num == i)
          phi *= factor[num][1];
        else
          phi *= (factor[i][0] + factor[i][1] * x[i]);
      }
    }

    //! evaluate second derivative
    virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      std::cout << "BaseFunction for Hexahedron, evaluate 2nd derivative not implemented! \n";
      phi = 0.0;
    }

  };

  //! default definition stays empty because implementation via
  //! specialization
  template <GeometryIdentifier::IdentifierType ElType, int polOrd ,int dimrange > struct LagrangeDefinition;

  //! Lagrange Definition for Lines
  template <int polOrd , int dimrange >
  struct LagrangeDefinition< GeometryIdentifier::Line , polOrd, dimrange >
  {
    enum { numOfBaseFct = (dimrange * (polOrd+1)) };
  };

  //! Lagrange Definition for GeometryIdentifier::Triangles
  template <int polOrd , int dimrange >
  struct LagrangeDefinition< GeometryIdentifier::Triangle , polOrd, dimrange >
  {
    enum { numOfBaseFct = (dimrange * (polOrd+1) * (polOrd+2) / 2) };
  };

  //! Lagrange Definition for Quadrilaterals
  template <int polOrd , int dimrange >
  struct LagrangeDefinition< GeometryIdentifier::Quadrilateral , polOrd, dimrange >
  {
    enum { numOfBaseFct = dimrange * (polOrd+1) * (polOrd+1) };
  };

  //! Lagrange Definition for Tetrahedrons
  template <int polOrd , int dimrange >
  struct LagrangeDefinition< GeometryIdentifier::Tetrahedron , polOrd, dimrange >
  {
    /** \bug This formula is wrong! */
    enum { numOfBaseFct = (polOrd == 0) ? (1*dimrange) : (dimrange * 4 * polOrd) };
  };

  //! Lagrange Definition for Pyramids
  /** \todo Generalize this to higher orders */
  template <int polOrd, int dimrange >
  struct LagrangeDefinition< GeometryIdentifier::Pyramid , polOrd, dimrange >
  {
    enum { numOfBaseFct = dimrange * 5};
  };

  //! Lagrange Definition for Prisms
  /** \todo Generalize this to higher orders */
  template <int polOrd, int dimrange >
  struct LagrangeDefinition< GeometryIdentifier::Prism , polOrd, dimrange >
  {
    enum { numOfBaseFct = dimrange * 6};
  };

  //! Lagrange Definition for Hexahedrons
  template <int polOrd , int dimrange >
  struct LagrangeDefinition< GeometryIdentifier::Hexahedron , polOrd, dimrange >
  {
    enum { numOfBaseFct = dimrange * (polOrd+1) * (polOrd+1) * (polOrd+1) };
  };

  //*********************************************************************
  //
  //  --LinFastBaseFunctionSet
  //
  /*!
     This class is in charge for setting the correct pointers for the
     FastBaseFunctionSet via the Constructor of this class
     Note that each function space hold a basefunction set of type
     FastBaseFunctionSet, because one grid can contain different element
     types for which we then need different basefunction set :(.
     But this isn't a problem, because we use caching of evaluations of
     basefunction which is always done on the reference element.
     Therefore the basefunction have virtual methods.
   */
  //
  //*********************************************************************
  template<class FunctionSpaceType, GeometryIdentifier::IdentifierType ElType, int polOrd >
  class LagrangeFastBaseFunctionSet
    : public FastBaseFunctionSet<FunctionSpaceType >
  {
    enum { dimrange = FunctionSpaceType::DimRange };
    //! know the number of base functions
    enum { numOfBaseFct = LagrangeDefinition < ElType, polOrd, dimrange >::numOfBaseFct };

    //! type of LagrangeBaseFunctions
    typedef LagrangeBaseFunction < FunctionSpaceType , ElType , polOrd >
    LagrangeBaseFunctionType;
  public:

    //! Constructor, calls Constructor of FastBaseFunctionSet, which is the
    //! InterfaceImplementation
    LagrangeFastBaseFunctionSet( FunctionSpaceType &fuSpace ) :
      FastBaseFunctionSet<FunctionSpaceType >( fuSpace, numOfBaseFct )
    {
      int numOfDifferentFuncs = (int) numOfBaseFct / dimrange;
      for(int i=0; i<numOfDifferentFuncs; i++)
      {
        for(int k=0; k<dimrange; k++)
        {
          size_t idx = i*dimrange + k;
          // for each dimrange we have a base function
          this->setBaseFunctionPointer ( idx, new LagrangeBaseFunctionType(idx) );
        }
      }
      this->setNumOfDiffFct ( numOfDifferentFuncs );
    };

    //! Destructor deleting the base functions
    ~LagrangeFastBaseFunctionSet( ) {}

    //! return number of base function for this base function set
    int getNumberOfBaseFunctions() const { return numOfBaseFct; };

    //! return number of different basefunction, i.e. we can have more than
    //! one dof a vertices for example
    int getNumberOfDifferentBaseFunctions () const
    {
      return (int) (numOfBaseFct / dimrange);
    }

  };

  //! Factory class for base functions
  template <class ScalarFunctionSpaceImp, int polOrd>
  class LagrangeBaseFunctionFactory :
    public BaseFunctionFactory<ScalarFunctionSpaceImp>
  {
  public:
    typedef ScalarFunctionSpaceImp FunctionSpaceType;
    typedef BaseFunctionInterface<FunctionSpaceType> BaseFunctionType;
  public:
    LagrangeBaseFunctionFactory(GeometryType geo) :
      BaseFunctionFactory<FunctionSpaceType>(geo)
    {}

    virtual BaseFunctionType* baseFunction(int i) const
    {
      switch (GeometryIdentifier::fromGeo(this->geometry()))
      {
      case GeometryIdentifier::Line :
        return new
               LagrangeBaseFunction<FunctionSpaceType, GeometryIdentifier::Line, polOrd>(i);
      case GeometryIdentifier::Triangle :
        return new
               LagrangeBaseFunction<FunctionSpaceType, GeometryIdentifier::Triangle, polOrd>(i);
      case GeometryIdentifier::Tetrahedron :
        return new
               LagrangeBaseFunction<FunctionSpaceType, GeometryIdentifier::Tetrahedron, polOrd>(i);
      case GeometryIdentifier::Quadrilateral :
        return new
               LagrangeBaseFunction<FunctionSpaceType, GeometryIdentifier::Quadrilateral, polOrd>(i);
      case GeometryIdentifier::Hexahedron :
        return new
               LagrangeBaseFunction<FunctionSpaceType, GeometryIdentifier::Hexahedron, polOrd>(i);
      case GeometryIdentifier::Prism :
        return new
               LagrangeBaseFunction<FunctionSpaceType, GeometryIdentifier::Prism, polOrd>(i);
      case GeometryIdentifier::Pyramid :
        return new
               LagrangeBaseFunction<FunctionSpaceType, GeometryIdentifier::Pyramid, polOrd>(i);
      default :
        DUNE_THROW(NotImplemented,
                   "The chosen geometry type is not implemented");
      }
      return 0;
    }

    virtual int numBaseFunctions() const
    {
      const int dimRange = FunctionSpaceType::DimRange;

      switch (GeometryIdentifier::fromGeo(this->geometry()))
      {
      case GeometryIdentifier::Line :
        return
          LagrangeDefinition<GeometryIdentifier::Line, polOrd, dimRange>::numOfBaseFct;
      case GeometryIdentifier::Triangle :
        return
          LagrangeDefinition<GeometryIdentifier::Triangle, polOrd, dimRange>::numOfBaseFct;
      case GeometryIdentifier::Tetrahedron :
        return
          LagrangeDefinition<GeometryIdentifier::Tetrahedron, polOrd, dimRange>::numOfBaseFct;
      case GeometryIdentifier::Quadrilateral :
        return
          LagrangeDefinition<GeometryIdentifier::Quadrilateral, polOrd, dimRange>::numOfBaseFct;
      case GeometryIdentifier::Hexahedron :
        return
          LagrangeDefinition<GeometryIdentifier::Hexahedron, polOrd, dimRange>::numOfBaseFct;
      case GeometryIdentifier::Prism :
        return
          LagrangeDefinition<GeometryIdentifier::Prism, polOrd, dimRange>::numOfBaseFct;
      case GeometryIdentifier::Pyramid :
        return
          LagrangeDefinition<GeometryIdentifier::Pyramid, polOrd, dimRange>::numOfBaseFct;
      default :
        DUNE_THROW(NotImplemented,
                   "The chosen geometry type is not implemented");
      }
      return 0;
    }
  };

} // end namespace Dune

#include "p2lagrangebasefunctions.hh"

#endif
