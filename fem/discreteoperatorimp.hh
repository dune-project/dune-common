// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DISCRETEOPERATORIMP_HH__
#define __DUNE_DISCRETEOPERATORIMP_HH__

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
     Before strating the walkthrough the prepareGlobal method of the local
     operator is called once. Then during the walkthrough on each entity the
     methods prepareLocal, applyLocal, and finalizeLocal are called respectively.
     After the walkthorugh the method finalizeGlobal of the local operator is
     called once.

     There are two ways to change a discrete operator, namely by adding
     another discrete operator or by multiplying with a scalar.
     The result of a sum has to be a mapping ( :-( ).


     @{
   */

  template <class DiscreteFunctionType, class LocalOperatorImp >
  class DiscreteOperator
    : public DiscreteOperatorDefault <DiscreteFunctionType, DiscreteFunctionType >
  {
    typedef typename DiscreteFunctionType::FunctionSpaceType::
    RangeField RangeFieldType;
    typedef DiscreteOperator<DiscreteFunctionType,LocalOperatorImp> MyType;

    typedef ScaledLocalOperator < LocalOperatorImp,
        typename  DiscreteFunctionType::RangeFieldType > ScalOperatorType;
    typedef DiscreteOperator<DiscreteFunctionType,ScalOperatorType> ScalDiscrType;

    typedef DiscreteOperatorDefault <DiscreteFunctionType, DiscreteFunctionType >
    DiscOpDefType;

    typedef typename DiscOpDefType::DomainType DomainType;
    typedef typename DiscOpDefType::RangeType RangeType;
    typedef typename DiscOpDefType::DomainFieldType DFieldType;
    typedef typename DiscOpDefType::RangeFieldType RFieldType;

    //! remember what type this class has
    typedef Mapping<DFieldType,RFieldType,DomainType,RangeType> MappingType;

  public:
    //! create DiscreteOperator with a LocalOperator
    DiscreteOperator (LocalOperatorImp &op, bool leaf=false , bool printMsg = false )
      : localOp_ ( op ) , leaf_ (leaf), prepared_ (false) , printMsg_ (printMsg)
    {
      if(printMsg_)
        std::cout << "Make new Operator " << this << "\n";
    }

    virtual ~DiscreteOperator ()
    {
      if(printMsg_)
        std::cout << "Delete operator " << this << "\n";
    }

    /*! add another discrete operator by combining the two local operators
       and creating a new discrete operator whereby a pointer to the new
       object is kept in the object where the operator was called.
       If this object is deleted then all objects created during operator +
       called are deleted too.
     */
    template <class LocalOperatorType>
    DiscreteOperator<DiscreteFunctionType,
        CombinedLocalOperator<LocalOperatorImp,LocalOperatorType> > &
    operator + (const DiscreteOperator<DiscreteFunctionType,LocalOperatorType> &op)
    {
      typedef DiscreteOperator<DiscreteFunctionType,LocalOperatorType> CopyType;
      typedef CombinedLocalOperator <LocalOperatorImp,LocalOperatorType> COType;

      COType *locOp =
        new COType ( localOp_ , const_cast<CopyType &> (op).getLocalOp ());
      typedef DiscreteOperator < DiscreteFunctionType , COType > OPType;

      OPType *discrOp = new OPType ( *locOp, leaf_, printMsg_ );

      // memorize this new generated object because is represents this
      // operator and is deleted if this operator is deleted
      this->saveObjPointer( discrOp );

      return *discrOp;
    }

    //! This method work just the same way like the operator +
    ScalDiscrType & operator * (const RangeFieldType& scalar)
    {
      ScalOperatorType * sop = new ScalOperatorType ( localOp_ , scalar);
      ScalDiscrType *discrOp = new ScalDiscrType ( *sop, leaf_, printMsg_ );

      // memorize this new generated object because is represents this
      // operator and is deleted if this operator is deleted
      saveObjPointer ( discrOp );

      return (*discrOp);
    }

    //! no public method, but has to be public, because all discrete operator
    //! must be able to call this method an the template parameters are
    //! allways diffrent
    LocalOperatorImp & getLocalOp ()
    {
      return localOp_;
    }

    //********************************************************************
    // Interface methods
    //********************************************************************
    //! apply operator which means make a grid walktrough on spezified level
    //! and call the local operator on each entity
    void apply ( const DomainType &Arg, RangeType &Dest ) const
    {
      if(printMsg_)
        std::cout << "DiscrOP::apply \n";
      applyNow(Arg,Dest);
    }

    template <class ArgParamType , class DestParamType>
    void applyNow ( const ArgParamType &arg, DestParamType &dest ) const
    {
      if(!prepared_)
      {
        prepare( arg, dest);
      }

      // useful typedefs
      typedef typename DiscreteFunctionType::FunctionSpace FunctionSpaceType;
      typedef typename FunctionSpaceType::GridType GridType;
      // the corresponding grid
      FunctionSpaceType & functionSpace_= dest.getFunctionSpace();
      GridType &grid = functionSpace_.getGrid();

      if(leaf_)
      {
        //std::cout << "using  Leaf! \n";
        typedef typename GridType::LeafIterator LeafIterator;

        this->level_ = grid.maxlevel();

        // make run through grid
        LeafIterator it     = grid.leafbegin ( this->level_ );
        LeafIterator endit  = grid.leafend   ( this->level_ );
        applyOnGrid( it, endit , arg, dest );
      }
      else
      {
        typedef typename GridType::template Traits<0>::LevelIterator LevelIterator;


        // make run through grid
        LevelIterator it    = grid.template lbegin<0>( this->level_ );
        LevelIterator endit = grid.template lend<0>  ( this->level_ );
        applyOnGrid( it, endit , arg, dest );
      }

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
    void prepare ( const ArgParamType &arg, DestParamType &dest )
    {
      localOp_.prepareGlobal(arg,dest);
      prepared_ = true;
    }

    //! remember time step size
    void prepare ( const DomainType &Arg, RangeType &Dest ) const
    {
      //localOp_.setArguments(Arg,Dest);
      localOp_.prepareGlobal(Arg,Dest);
      prepared_ = true;
    }

    //! finalize the operation
    void finalize ( const DomainType &Arg, RangeType &Dest ) const
    {
      prepared_ = false;
      localOp_.finalizeGlobal();
    }

    template <class GridIteratorType>
    void applyOnGrid ( GridIteratorType &it, GridIteratorType &endit,
                       const DomainType &Arg, RangeType &Dest ) const
    {
      // erase destination function
      Dest.clearLevel ( this->level_ );

      // run through grid and apply the local operator
      for( ; it != endit; ++it )
      {
        localOp_.prepareLocal (*it);
        localOp_.applyLocal   (*it);
        localOp_.finalizeLocal(*it);
      }
    }

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

    //! Operator which is called on each entity
    LocalOperatorImp & localOp_;

    //! if true use LeafIterator else LevelIterator
    mutable bool leaf_;

    //! true if operator was prepared for apply
    mutable bool prepared_;

    bool printMsg_;

    //*******************************************************
    // derived from mappiung, don't need this here
    //*******************************************************
    virtual MappingType operator * (const RFieldType &) const
    {
      std::cout << "MappingType DiscreteOperator::operator * ( const Field &) const: do not use this method\n";
      abort();
      return (*this);
    }
  };

  /** @} end documentation group */

}

#endif
