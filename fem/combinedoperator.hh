// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_COMBINED_OPERATOR_HH__
#define __DUNE_COMBINED_OPERATOR_HH__


namespace Dune
{

  //! All types of conjunctions allowed with the local operators
  enum  ConjunctionType { ADD , ADD_SCALED ,
                          SUBSTRACT , MULTIPLY_SCALED ,MULTIPLY , DIVIDE , NESTED_SCALED , NESTED };

  //! Interface of a combined operator.
  template <class A , class B , class CombinedOperatorImp>
  class CombinedOperatorInterface
  {
  public:
    typedef typename A::Domain Domain;
    typedef typename A::Range Range;

    //! call applyLocal of CombinedOperatorImp
    template <class EntityType>
    void applyLocal ( EntityType &en, const Domain & arg , Range & dest )
    {
      asImp().applyLocal(en,arg,dest);
    }

    //! prepare apply and get temporary variable
    void prepare ( Range * t)
    {
      asImp().prepare(t);
    }

    //! finalize the operator
    void finalize ()
    {
      asImp().finalize();
    }

  private:
    //! Barton Nackman
    CombinedOperatorImp & asImp()
    { return static_cast<CombinedOperatorImp &> (*this); };
  };


  //*******************************************************************
  //
  // --CombinedOperatorDefault
  //
  //! Default implementation of a combined operator.
  //
  //*******************************************************************
  template <class A , class B , class CombinedOperatorImp>
  class CombinedOperatorDefault
    : public CombinedOperatorInterface <A,B,CombinedOperatorImp>
  {
  public:
    //! Constructor for combinations with factors
    CombinedOperatorDefault
      ( double f1, A & a, double f2, B & b ) :
      _a ( a ) , _b ( b ) , _fA ( f1 ), _fB (f2) , _tmp ( NULL ) { };

    //! Constructor for combinations with factors
    CombinedOperatorDefault
      ( A & a, B & b ) :
      _a ( a ) , _b ( b ) , _fA ( 1.0 ) , _fB (1.0) , _tmp ( NULL ) { };

    //! Constructor for combinations without factors
    CombinedOperatorDefault ( double f1 , A & a, B & b ) :
      _a ( a ) , _b ( b ) , _fA ( f1 ), _fB ( 1.0 ) , _tmp ( NULL ){ };

  protected:
    //! operator A and B
    A & _a;
    B & _b;

    //! scaling factors
    double _fA;
    double _fB;

    //! for temporary use
    Range *_tmp;

  private:
    //! Barton Nackman
    CombinedOperatorImp & asImp()
    { return static_cast<CombinedOperatorImp &> (*this); };
  };


  //*******************************************************************
  //
  //
  //! Combine Operator A and Operator B with the defined conjunction.
  //! There are diffrent types of conjunctions. All different types
  //! are specialized in a class.
  //
  //*******************************************************************
  template <ConjunctionType conjunction, class A, class B >
  class CombinedOperator :
    public CombinedOperatorDefault <A,B,
        CombinedOperator <conjunction,A,B> >
  {
    typedef CombinedOperatorDefault <A ,B,
        CombinedOperator <conjunction,A,B> > InterfaceType;
  public:
    typedef typename A::Domain Domain;
    typedef typename A::Range Range;
    typedef CombinedOperator <conjunction,A,B> MyType;

    CombinedOperator ( A & a,  B & b ) :
      InterfaceType (a,b) {};

    CombinedOperator ( double f1 , A & a, double f2 ,  B & b) :
      InterfaceType (f1,a,f2,b) {};

    //! prepare apply and get temporary variable
    void prepare ( Range * t)
    {
      _tmp = t;
    }

    //! finalize the operator
    void finalize ()
    {
      _tmp = NULL;
    }

    template <class EntityType>
    void applyLocal ( EntityType &en , const Domain & arg , Range & dest ) //const
    {
      //B::Range tmp;

      //std::cout << "Op::applyLocal b \n";
      _b.applyLocal ( en , arg , dest );
      //    dest *= _fA;

      //std::cout << "Op::applyLocal a \n";
      //_a.applyLocal ( en , arg , (*_tmp) );
      _a.applyLocal ( en , arg , dest );
      //    dest += (_fB * (*_tmp));
    }



  };

  //! add OP = a*A + b*B
  template <class A, class B >
  class CombinedOperator <ADD_SCALED, A,B>:
    public CombinedOperatorDefault <A,B,
        CombinedOperator < ADD_SCALED ,A,B> >
  {
    typedef CombinedOperatorDefault <A ,B,
        CombinedOperator <ADD_SCALED ,A,B> > InterfaceType;
  public:
    typedef typename A::Domain Domain;
    typedef typename B::Range Range;
    typedef CombinedOperator <ADD_SCALED ,A,B > MyType;

    CombinedOperator ( double f1 , A & a, double f2, B & b) :
      InterfaceType (f1,a,f2,b) {};

    CombinedOperator ( A & a, double f2, B & b) :
      InterfaceType (a,f2,b) {};

    CombinedOperator ( double f1 , A & a, B & b) :
      InterfaceType (f1,a,b) {};

    //! prepare apply and get temporary variable
    void prepare ( Range * t)
    {
      _tmp = t;
    }

    //! finalize the operator
    void finalize ()
    {
      _tmp = NULL;
    }


    //! apply  dest = (_fA*_a + _fB*_b)(arg)
    template <class EntityType>
    void applyLocal ( EntityType &en, const Domain & arg , Range & dest ) //const
    {
      // first apply _b
      _b.applyLocal     ( en, arg , dest );
      _tmp->assignLocal (en, dest, _fA);

      // then apply _a
      _a.applyLocal (en , arg , dest );
      dest.addLocal (en , (*_tmp) , _fB);
    }

  };
#if 0
  //! add OP = A + B
  template <class A, class B >
  class CombinedOperator <ADD, A,B>:
    public CombinedOperatorInterface <A,B,
        CombinedOperator < ADD ,A,B> >
  {
    typedef CombinedOperatorInterface <A ,B,
        CombinedOperator <ADD ,A,B> > InterfaceType;
  public:
    typedef typename A::Domain Domain;
    typedef typename A::Range Range;
    typedef CombinedOperator <ADD,A,B > MyType;

    CombinedOperator ( const A & a, const B & b ) :
      InterfaceType (a,b) {};

    //! apply  x + y
    void applyLocal ( const Domain & arg , Range & dest ) const
    {
      B::Range tmp;
      _b.applyLocal ( arg , tmp );

      _a.applyLocal (arg , dest );
      dest += tmp;
    }

  };

  template <class A, class B >
  class CombinedOperator < NESTED, A,B>:
    public CombinedOperatorInterface <A,B,
        CombinedOperator < NESTED ,A,B> >
  {
    typedef CombinedOperatorInterface <A ,B,
        CombinedOperator < NESTED ,A,B> > InterfaceType;
  public:
    typedef typename B::Domain Domain;
    typedef typename A::Range Range;
    typedef CombinedOperator < NESTED ,A,B > MyType;

    CombinedOperator ( const A & a, const B & b ) :
      InterfaceType (a,b) {};

    CombinedOperator ( double f1 , const A & a, double f2, const B & b) :
      InterfaceType (f1,a,f2,b) {};

    CombinedOperator ( const A & a, double f2, const B & b) :
      InterfaceType (a,f2,b) {};

    CombinedOperator ( double f1 , const A & a, const B & b) :
      InterfaceType (f1,a,b) {};

    void applyLocal ( const Domain & arg , Range & dest ) const
    {
      B::Range tmp;
      _b.applyLocal ( arg , tmp );
      _a.applyLocal (tmp , dest );
    }

  };

#endif

  //! return OP = A + B
  template <class A , class B >
  CombinedOperator < ADD, A,B> add ( A & a, B & b )
  {
    CombinedOperator <ADD, A,B> tmp (a,b);
    return tmp;
  }

  //! return OP = A - B
  template <class A , class B >
  CombinedOperator < SUBSTRACT, A,B> substract (double aF , A & a, double bF , B & b )
  {
    CombinedOperator <SUBSTRACT, A,B> tmp (a,b);
    return tmp;
  }

  //! return OP = a * A + b * B
  template <class A , class B >
  CombinedOperator < ADD_SCALED, A,B> addScaled (double aF , A & a, double bF , B & b )
  {
    CombinedOperator <ADD_SCALED, A,B> tmp (aF,a,bF,b);
    return tmp;
  }

  //! return OP = a * A * B
  template <class A , class B >
  CombinedOperator < MULTIPLY , A,B> multiply (double aF , A & a, B & b )
  {
    if((aF == 1.) )
    {
      CombinedOperator <MULTIPLY, A,B> tmp (a,b);
      return tmp;
    }
    else
    {
      CombinedOperator <MULTIPLY, A,B> tmp (aF,a,b);
      return tmp;
    }
  }

  //! return OP ( u ) = A ( B ( u ) )
  template <class A , class B >
  CombinedOperator < NESTED , A,B> nested (double aF , A & a, B & b )
  {
    if((aF == 1.) )
    {
      CombinedOperator < NESTED , A,B> tmp (a,b);
      return tmp;
    }
    else
    {
      CombinedOperator < NESTED, A,B> tmp (aF,a,b);
      return tmp;
    }
  }

} // end namespace Dune

#endif
