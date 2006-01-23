// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_P2_LAGRANGE_BASE_FUNCTIONS_HH
#define DUNE_P2_LAGRANGE_BASE_FUNCTIONS_HH

namespace Dune {

  //*****************************************************************
  //
  //! Lagrange base for lines and polynom order = 2
  //! (0) 0-----1 (1)
  //
  //*****************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction < FunctionSpaceType , GeometryIdentifier::Line , 2 >
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    RangeFieldType factor[3];

  public:
    LagrangeBaseFunction ( FunctionSpaceType & f , int baseNum  )
      : BaseFunctionInterface<FunctionSpaceType> (f)
    {
      switch (baseNum) {
      case 0 :
        // 2x^2 - 3x +1
        factor[0] =  2;
        factor[1] = -3;
        factor[2] =  1;
        break;
      case 1 :
        // -4x^2 + 4x
        factor[0] = -4;
        factor[1] =  4;
        factor[2] =  0;
        break;
      case 2 :
        // 2x^2 - x
        factor[0] =  2;
        factor[1] = -1;
        factor[2] =  0;
        break;
      }
    }

    //! evaluate the function
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      phi  = factor[0] * x[0] * x[0];
      phi += factor[1] * x[0];
      phi += factor[2];
    }

    //! evaluate first derivative
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      phi = 2*factor[0] * x[0] + factor[1];
    }

    //! evaluate second derivative
    virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      phi = 2*factor[0];
    }

  };


  //*****************************************************************
  //
  //! Lagrange base for triangles and polynom order = 2
  //
  //*****************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction < FunctionSpaceType , GeometryIdentifier::Triangle, 2 >
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    RangeFieldType factor[3];

  public:
    LagrangeBaseFunction ( FunctionSpaceType & f , int baseNum  )
      : BaseFunctionInterface<FunctionSpaceType> (f)
    {
      DUNE_THROW(NotImplemented, "Second order Lagrange elements for triangles are not implemented yet!");
    }

    //! evaluate the function
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {}

    //! evaluate first derivative
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {}

    //! evaluate second derivative
    virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {}

  };

  //*****************************************************************
  //
  //! Lagrange base for quadrilaterals and polynom order = 2
  //
  //*****************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction < FunctionSpaceType , GeometryIdentifier::Quadrilateral, 2 >
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    RangeFieldType factor[3];

  public:
    LagrangeBaseFunction ( FunctionSpaceType & f , int baseNum  )
      : BaseFunctionInterface<FunctionSpaceType> (f)
    {
      DUNE_THROW(NotImplemented, "Second order Lagrange elements for quadrilaterals are not implemented yet!");
    }

    //! evaluate the function
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {}

    //! evaluate first derivative
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {}

    //! evaluate second derivative
    virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {}

  };

  //*****************************************************************
  //
  //! Lagrange base for tetrahedra and polynom order = 2
  //
  //*****************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction < FunctionSpaceType , GeometryIdentifier::Tetrahedron, 2 >
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    RangeFieldType factor[3];

  public:
    LagrangeBaseFunction ( FunctionSpaceType & f , int baseNum  )
      : BaseFunctionInterface<FunctionSpaceType> (f)
    {
      DUNE_THROW(NotImplemented, "Second order Lagrange elements for tetrahedra are not implemented yet!");
    }

    //! evaluate the function
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {}

    //! evaluate first derivative
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {}

    //! evaluate second derivative
    virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {}

  };



  //*****************************************************************
  //
  //! Lagrange base for tetrahedra and polynom order = 2
  //
  //*****************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction < FunctionSpaceType , GeometryIdentifier::Pyramid, 2 >
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    RangeFieldType factor[3];

  public:
    LagrangeBaseFunction ( FunctionSpaceType & f , int baseNum  )
      : BaseFunctionInterface<FunctionSpaceType> (f)
    {
      DUNE_THROW(NotImplemented, "Second order Lagrange elements for pyramids are not implemented yet!");
    }

    //! evaluate the function
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {}

    //! evaluate first derivative
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {}

    //! evaluate second derivative
    virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {}

  };



  //*****************************************************************
  //
  //! Lagrange base for prisms and polynom order = 2
  //
  //*****************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction < FunctionSpaceType , GeometryIdentifier::Prism, 2 >
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    RangeFieldType factor[3];

  public:
    LagrangeBaseFunction ( FunctionSpaceType & f , int baseNum  )
      : BaseFunctionInterface<FunctionSpaceType> (f)
    {
      DUNE_THROW(NotImplemented, "Second order Lagrange elements for tetrahedra are not implemented yet!");
    }

    //! evaluate the function
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {}

    //! evaluate first derivative
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {}

    //! evaluate second derivative
    virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {}

  };

  //*****************************************************************
  //
  //! Lagrange base for hexahedra and polynom order = 2
  //
  //*****************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction < FunctionSpaceType , GeometryIdentifier::Hexahedron, 2 >
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    RangeFieldType factor[3];

  public:
    LagrangeBaseFunction ( FunctionSpaceType & f , int baseNum  )
      : BaseFunctionInterface<FunctionSpaceType> (f)
    {}

    //! evaluate the function
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      DUNE_THROW(NotImplemented, "Second order Lagrange elements for hexahedra are not implemented yet!");
    }

    //! evaluate first derivative
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      DUNE_THROW(NotImplemented, "Second order Lagrange elements for hexahedra are not implemented yet!");
    }

    //! evaluate second derivative
    virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      DUNE_THROW(NotImplemented, "Second order Lagrange elements for hexahedra are not implemented yet!");
    }

  };




}  // end namespace dune

#endif
