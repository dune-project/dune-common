// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DISCRETEOPERATORIMP_HH
#define DUNE_DISCRETEOPERATORIMP_HH

#include <dune/fem/common/discreteoperator.hh>
#include <dune/fem/common/localoperator.hh>

namespace Dune {

  /** @defgroup DiscreteOperatorImp DiscreteOperatorImp
      @ingroup DiscreteOperator

      The DiscreteFunction is resposible for the dof storage. This can be
      done in various ways an is left to the user. The user has to derive his
      own implemenation from the DiscreteFunctionDefault class. If some of
      the implementations in the default class are for his dof storage
      unefficient, then one can overload this functions.

     A discrete operator of this type consists of a local operator and a grid
     walkthrough. On each Entity then the local operator is called.
     Before starting the walk through the prepareGlobal method of the local
     operator is called once. Then during the walkthrough on each entity the
     methods prepareLocal, applyLocal, and finalizeLocal are called respectively.
     After the walkthorugh the method finalizeGlobal of the local operator is
     called once.

     There are two ways to change a discrete operator, namely by adding
     another discrete operator or by multiplying with a scalar.
     The result of a sum has to be a mapping ( :-( ).
     DiscreteOperators which differ only by there LocalOperatorImp
     can be combinded via operator + and sclaed
     via operator *. This means you can combine your schemes all working on the
     same types of DiscreteFunctions but all doing different things on one
     entity. An example how to use this operator class can be found in
     duneapps/tester/operator.
     @{
   */

  /*!
     DiscreteOperator manages the grid walk through of and numerical scheme.
     Here the scheme is defined by the LocalOperatorImp which defines the
     action of the algorithm one on grid entity.
   */
  template <class LocalOperatorImp, class DFDomainType, class DFRangeType = DFDomainType >
  class DiscreteOperator
    : public DiscreteOperatorDefault < LocalOperatorImp , DFDomainType, DFRangeType , DiscreteOperator >
  {
    typedef typename DFDomainType::FunctionSpaceType::RangeFieldType RangeFieldType;

    typedef typename DFDomainType::DomainType DomainType;
    typedef typename DFDomainType::RangeType RangeType;
    typedef typename DFDomainType::DomainFieldType DFieldType;
    typedef typename DFDomainType::RangeFieldType RFieldType;

    //! remember what type this class has
    typedef Mapping<DFieldType,RFieldType,DomainType,RangeType> MappingType;

  public:
    //! create DiscreteOperator with a LocalOperator
    DiscreteOperator (LocalOperatorImp &op, bool leaf=false , bool printMsg = false )
      : localOp_ (op)
        , leaf_ (leaf), prepared_ (false) , printMsg_ (printMsg)
    {
      if(printMsg_)
        std::cout << "Make new Operator " << this << "\n";
    }

    //! this Constructor is called from the operator + method of DiscreteOperatorDefault
    template <class LocalOperatorType>
    DiscreteOperator (const DiscreteOperator<LocalOperatorType,DFDomainType,DFRangeType> &copy,
                      LocalOperatorImp &op)
      : localOp_ (op)
        , leaf_ (copy.leaf_), prepared_ (copy.prepared_) , printMsg_ (copy.printMsg_)
    {
      if(printMsg_)
        std::cout << "Make new Operator " << this << "\n";
    }

    virtual ~DiscreteOperator ()
    {
      if(printMsg_)
        std::cout << "Delete operator " << this << "\n";
    }

    //********************************************************************
    // Interface methods
    //********************************************************************
    //! apply operator which means make a grid walktrough on spezified level
    //! and call the local operator on each entity
    void apply ( const DFDomainType &Arg, DFRangeType &Dest ) const
    {
      if(printMsg_)
        std::cout << "DiscrOP::apply \n";
      applyNow(Arg,Dest);
    }

    //! \todo Please doc me!
    template <class ArgParamType , class DestParamType>
    void applyNow ( const ArgParamType &arg, DestParamType &dest ) const
    {
      if(!prepared_)
      {
        prepare( arg, dest);
      }

      // useful typedefs
      typedef typename DFDomainType::DiscreteFunctionSpaceType FunctionSpaceType;
      typedef typename FunctionSpaceType::GridType GridType;
      typedef typename FunctionSpaceType::IteratorType IteratorType;
      // the corresponding grid
      const FunctionSpaceType & functionSpace_= dest.getFunctionSpace();

      // get iterator from space
      IteratorType it = functionSpace_.begin();
      IteratorType endit = functionSpace_.end();
      applyOnGrid(it, endit, arg, dest);

      finalize( arg, dest );
    }


    //! apply the operator, see apply
    template <class ArgParamType , class DestParamType>
    void operator () ( const ArgParamType &arg, DestParamType &dest ) const
    {
      apply(arg,dest);
    }

    //********************************************************************
    //  end of interface methods
    //********************************************************************

  private:
    //! remember time step size
    template <class ArgParamType , class DestParamType>
    void prepare ( const ArgParamType &arg, DestParamType &dest ) const
    {
      localOp_.prepareGlobal(arg,dest);
      prepared_ = true;
    }

    //! remember time step size
    void prepare ( const DFDomainType &Arg, DFRangeType &Dest ) const
    {
      //localOp_.setArguments(Arg,Dest);
      localOp_.prepareGlobal(Arg,Dest);
      prepared_ = true;
    }

    //! finalize the operation
    void finalize ( const DFDomainType &Arg, DFRangeType &Dest ) const
    {
      prepared_ = false;
      localOp_.finalizeGlobal();
    }

    //! \todo Please doc me!
    template <class GridIteratorType>
    void applyOnGrid ( GridIteratorType &it, GridIteratorType &endit,
                       const DFDomainType &Arg, DFRangeType &Dest ) const
    {
      // erase destination function
      Dest.clear ();

      // run through grid and apply the local operator
      for( ; it != endit; ++it )
      {
        localOp_.prepareLocal (*it);
        localOp_.applyLocal   (*it);
        localOp_.finalizeLocal(*it);
      }
    }

    //! \todo Please doc me!
    template <class GridIteratorType>
    void applyOnGrid ( GridIteratorType &it, GridIteratorType &endit ) const
    {
      // run through grid and apply the local operator
      for( ; it != endit; ++it )
      {
        localOp_.prepareLocal (*it);
        localOp_.applyLocal   (*it);
        localOp_.finalizeLocal(*it);
      }
    }
  public:
    //! return reference to local operator, is called from operator + and
    //! operator *
    LocalOperatorImp & getLocalOp () { return localOp_; }

    // return printMsg variable for DiscreteOperatorDefault
    bool printInfo () const { return printMsg_; }

    //! Local operator which represents the numerical scheme
    mutable LocalOperatorImp & localOp_;

    //! if true use LeafIterator else LevelIterator
    mutable bool leaf_;

    //! true if operator was prepared for apply
    mutable bool prepared_;

    bool printMsg_;
  };

  /** @} end documentation group */

}

#endif
