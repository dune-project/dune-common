// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_COMBINED_OPERATOR_HH__
#define __DUNE_COMBINED_OPERATOR_HH__


namespace Dune
{

  //! All types of conjunctions allowed with the local operators
  enum  ConjunctionType { ADD , ADD_SCALED , ADD_SCALED_B ,
                          SUBSTRACT , MULTIPLY_SCALED ,MULTIPLY , DIVIDE , NESTED_SCALED , NESTED };


  // Interface of Local Operators for DiscreteOperators.
  class LocalOperatorInterface;


  //! Interface of a combined operator.
  template <class A , class B , class CombinedOperatorImp>
  class CombinedOperatorInterface
  {
  public:
    typedef typename A::Domain Domain;
    typedef typename A::Range Range;

    typedef double RangeFieldType;

    //! call applyLocal of CombinedOperatorImp
    template <class GridIteratorType>
    void applyLocal ( GridIteratorType &it , const Domain & arg , Range & dest )
    {
      asImp().applyLocal(it,arg,dest);
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
    //! Barton Nackman Trick
    CombinedOperatorImp & asImp()
    {
      return static_cast<CombinedOperatorImp &> (*this);
    };
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
      ( RangeFieldType f1, A & a, RangeFieldType f2, B & b ) :
      _a ( a ) , _b ( b ) , _fA ( f1 ), _fB (f2) , _tmp ( NULL ) { };

    //! Constructor for combinations with factors
    CombinedOperatorDefault
      ( A & a, B & b ) :
      _a ( a ) , _b ( b ) , _fA ( 1.0 ) , _fB (1.0) , _tmp ( NULL ) { };

    //! Constructor for combinations without factors
    CombinedOperatorDefault ( RangeFieldType f1 , A & a, B & b ) :
      _a ( a ) , _b ( b ) , _fA ( f1 ), _fB ( 1.0 ) , _tmp ( NULL ){ };

    //! Constructor for combinations without factors
    CombinedOperatorDefault ( A & a, RangeFieldType f2, B & b ) :
      _a ( a ) , _b ( b ) , _fA ( 1.0 ), _fB ( f2 ) , _tmp ( NULL ){ };

    //! prepare apply and get temporary variable
    //! set fA_ = fak1 and fB_ = fak2  ( i.e. delta t)
    void prepareGlobal (int level, const Domain & arg , Range & dest,
                        Range * t, RangeFieldType & fak1, RangeFieldType & fak2 )
    {
      _b.prepareGlobal(level,arg,dest,t,fak1,fak2);
      _a.prepareGlobal(level,arg,dest,t,fak1,fak2);

      _fA = fak1;
      _fB = fak2;
      _tmp = t;
    }

    //! do a local cleanup or what
    template <class GridIteratorType>
    void prepareLocal ( GridIteratorType &it , const Domain & arg , Range & dest )
    {
      _b.prepareLocal(it,arg,dest);
      _a.prepareLocal(it,arg,dest);
    }

    //! do some local stuff, overload for functionality
    template <class GridIteratorType>
    void finalizeLocal ( GridIteratorType &it , const Domain & arg , Range & dest )
    {
      _b.finalizeLocal(it,arg,dest);
      _a.finalizeLocal(it,arg,dest);
    }

    //! finalize the operator
    void finalizeGlobal (int level, const Domain & arg , Range & dest,
                         Range * t, RangeFieldType & fak1, RangeFieldType & fak2 )
    {
      _b.finalizeGlobal(level,arg,dest,t,fak1,fak2);
      _a.finalizeGlobal(level,arg,dest,t,fak1,fak2);
      _tmp = NULL;
    }
  protected:
    //! operator A and B
    A & _a;
    B & _b;

    //! scaling factors
    RangeFieldType _fA;
    RangeFieldType _fB;

    //! for temporary use
    mutable Range *_tmp;

  private:
    //! Barton Nackman
    CombinedOperatorImp & asImp()
    { return static_cast<CombinedOperatorImp &> (*this); };
  };


  //*******************************************************************
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
      InterfaceType (a,b)
    {
      std::cerr << "Combined Operator::Constructor: No Default Implemenation is provieded!\n";
      abort();
    }

    template <class GridIteratorType>
    void applyLocal ( GridIteratorType &it , const Domain & arg , Range & dest ) const
    {
      std::cerr << "Combined Operator::applyLocal: No Default Implemenation is provieded!\n";
      abort();
    }
  };
  //*******************************************************************
  //
  //! Combine Operator A and Operator B with the defined conjunction.
  //! There are diffrent types of conjunctions. All different types
  //! are specialized in a class.
  //
  //*******************************************************************
  // ADD
  //! OP = A + B
  template <class A, class B >
  class CombinedOperator<ADD, A, B> :
    public CombinedOperatorDefault <A,B,
        CombinedOperator <ADD,A,B> >
  {
    typedef CombinedOperatorDefault <A ,B,
        CombinedOperator <ADD,A,B> > InterfaceType;
  public:
    typedef typename A::Domain Domain;
    typedef typename A::Range Range;
    typedef CombinedOperator <ADD,A,B> MyType;

    CombinedOperator ( A & a,  B & b ) :
      InterfaceType (a,b) {};

    template <class GridIteratorType>
    void applyLocal ( GridIteratorType &it , const Domain & arg , Range & dest )  const
    {
      //std::cout << "Warning: CombinedOperator<ADD>::applyLocal not correct! \n";
      _b.applyLocal ( it , arg , dest );
      _a.applyLocal ( it , arg , dest );

      //_tmp->setLocal(it,0.0);
      //_b.applyLocal ( it , arg , (*_tmp) );
      //dest.addLocal(it,(*_tmp));

      //_tmp->setLocal(it,0.0);
      //_a.applyLocal ( it , arg , (*_tmp) );
      //dest.addLocal(it,(*_tmp));
    }
  };

  //**********************************************************************
  //  SUBSTRACT
  //! OP = A - B
  template <class A, class B >
  class CombinedOperator<SUBSTRACT, A, B> :
    public CombinedOperatorDefault <A,B,
        CombinedOperator <SUBSTRACT,A,B> >
  {
    typedef CombinedOperatorDefault <A ,B,
        CombinedOperator <SUBSTRACT,A,B> > InterfaceType;
  public:
    typedef typename A::Domain Domain;
    typedef typename A::Range Range;
    typedef CombinedOperator <SUBSTRACT,A,B> MyType;

    CombinedOperator ( A & a,  B & b ) :
      InterfaceType (a,b) {};

    template <class GridIteratorType>
    void applyLocal ( GridIteratorType &it , const Domain & arg , Range & dest )
    {
      _tmp->setLocal(it,0.0);
      _b.applyLocal ( it , arg , (*_tmp) );
      dest.substractLocal(it,(*_tmp));

      _tmp->setLocal(it,0.0);
      _a.applyLocal ( it , arg , (*_tmp) );
      dest.addLocal(it,(*_tmp));
    }
  };

  //*******************************************************************
  //  ADD_SCALED_B
  //! OP = A + b*B
  template <class A, class B >
  class CombinedOperator<ADD_SCALED_B , A, B> :
    public CombinedOperatorDefault <A,B,
        CombinedOperator <ADD_SCALED_B ,A,B> >
  {
    typedef CombinedOperatorDefault <A ,B,
        CombinedOperator <ADD_SCALED_B ,A,B> > InterfaceType;
  public:
    typedef typename A::Domain Domain;
    typedef typename A::Range Range;
    typedef CombinedOperator <ADD_SCALED_B ,A,B> MyType;

    CombinedOperator ( A & a,  RangeFieldType fB, B & b ) :
      InterfaceType (a,fB,b) {};

    template <class GridIteratorType>
    void applyLocal ( GridIteratorType &it , const Domain & arg , Range & dest )
    {
      _tmp->setLocal(it,0.0);
      _b.applyLocal ( it , arg , (*_tmp) );
      dest.addScaledLocal(it,(*_tmp),_fB);

      _tmp->setLocal(it,0.0);
      _a.applyLocal ( it , arg , (*_tmp) );
      dest.addLocal(it,(*_tmp));
    }
  };


  //! ADD_SCALED
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

    CombinedOperator ( RangeFieldType f1 , A & a, RangeFieldType f2, B & b) :
      InterfaceType (f1,a,f2,b) {};

    //! apply  dest = (_fA*_a + _fB*_b)(arg)
    template <class GridIteratorType>
    void applyLocal ( GridIteratorType &it , const Domain & arg , Range & dest )
    {
      _tmp->setLocal(it,0.0);
      _b.applyLocal ( it , arg , (*_tmp) );
      dest.addScaledLocal(it,(*_tmp), _fB);

      _tmp->setLocal(it,0.0);
      _a.applyLocal ( it , arg , (*_tmp) );
      dest.addLocal(it,(*_tmp), _fA);
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

    CombinedOperator ( RangeFieldType f1 , const A & a, RangeFieldType f2, const B & b) :
      InterfaceType (f1,a,f2,b) {};

    CombinedOperator ( const A & a, RangeFieldType f2, const B & b) :
      InterfaceType (a,f2,b) {};

    CombinedOperator ( RangeFieldType f1 , const A & a, const B & b) :
      InterfaceType (f1,a,b) {};

    void applyLocal ( const Domain & arg , Range & dest ) const
    {
      typename B::Range tmp;
      _b.applyLocal ( arg , tmp );
      _a.applyLocal (tmp , dest );
    }

  };

  //! return OP = A + B
  template <class A , class B >
  CombinedOperator < ADD, A,B> add ( A & a, B & b )
  {
    CombinedOperator <ADD, A,B> tmp (a,b);
    return tmp;
  }

  //! return OP = A - B
  template <class A , class B , typename RangeFieldType>
  CombinedOperator < ADD_SCALED_B , A,B> addScaled (A & a, RangeFieldType bF , B & b )
  {
    CombinedOperator <ADD_SCALED_B , A,B> tmp (a,bF,b);
    return tmp;
  }

  //! return OP = A - B
  template <class A , class B , typename RangeFieldType>
  CombinedOperator < SUBSTRACT, A,B> substract (RangeFieldType aF , A & a, RangeFieldType bF , B & b )
  {
    CombinedOperator <SUBSTRACT, A,B> tmp (a,b);
    return tmp;
  }

  //! return OP = a * A + b * B
  template <class A , class B , typename RangeFieldType>
  CombinedOperator < ADD_SCALED, A,B> addScaled (RangeFieldType aF , A & a, RangeFieldType bF , B & b )
  {
    CombinedOperator <ADD_SCALED, A,B> tmp (aF,a,bF,b);
    return tmp;
  }

  //! return OP = a * A * B
  template <class A , class B , typename RangeFieldType>
  CombinedOperator < MULTIPLY , A,B> multiply (RangeFieldType aF , A & a, B & b )
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
  template <class A , class B , typename RangeFieldType>
  CombinedOperator < NESTED , A,B> nested (RangeFieldType aF , A & a, B & b )
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
