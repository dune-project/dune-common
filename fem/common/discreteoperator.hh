// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DISCRETEOPERATOR_INTERFACE_HH__
#define __DUNE_DISCRETEOPERATOR_INTERFACE_HH__

#include <dune/common/operator.hh>
// object pointer management for combined discrete operators
#include "objpointer.hh"
#include "localoperator.hh"

namespace Dune {


  /** @defgroup DiscreteOperator DiscreteOperator
      @ingroup OperatorCommon

     @{
   */

  /*! Default Implementation class for DiscreteOperators
     Here the ObjPointer is stored which is used during
     the generation of new objects with the operator + method.
     There is corresponding interface class is Operator.
     NOTE: if operator + and operator * are used the inherited class
     DiscreteOperatorImp has to implement a Constructor which gets a copy and
     the new LocalOperatorType which is created by the operator + ...
   */
  template <class LocalOperatorImp, class DFDomainType, class DFRangeType ,
      template <class,class,class> class DiscreteOperatorImp >
  class DiscreteOperatorDefault
    : public Operator <typename DFDomainType::RangeFieldType,
          typename DFRangeType::RangeFieldType,
          DFDomainType , DFRangeType>
      , public ObjPointerStorage
  {

  protected:
    // define my type for inherited classes
    typedef DiscreteOperatorDefault < LocalOperatorImp , DFDomainType ,
        DFRangeType , DiscreteOperatorImp > DiscreteOperatorDefaultType;

    typedef typename DFRangeType::RangeFieldType RangeFieldType;

  public:
    //! make new operator with item points to null
    DiscreteOperatorDefault () : level_ (0) {}

    /*!
       no public method, but has to be public, because all discrete operator
       must be able to call this method an the template parameters are
       allways diffrent.
       NOTE: this method has to be overloaded by the DiscreteOperatorImp class
       ans must return the reference to local operator  */
    const LocalOperatorImp & getLocalOp () const
    {
      return asImp().getLocalOp();
    }

    /*! add another discrete operator by combining the two local operators
       and creating a new discrete operator whereby a pointer to the new
       object is kept in the object where the operator was called.
       If this object is deleted then all objects created during operator +
       called are deleted too.
     */
    template <class LocalOperatorType>
    DiscreteOperatorImp<  CombinedLocalOperator < LocalOperatorImp , LocalOperatorType> ,
        DFDomainType, DFRangeType >  &
    operator + (DiscreteOperatorImp < LocalOperatorType, DFDomainType , DFRangeType > &op )
    {
      if(asImp().printInfo())
        std::cout << "DiscreteOperatorDefault::operator + called! \n";

      typedef DiscreteOperatorImp < LocalOperatorType , DFDomainType, DFRangeType> CopyType;
      typedef CombinedLocalOperator < LocalOperatorImp , LocalOperatorType > COType;

      COType *locOp =
        new COType ( asImp().getLocalOp() , op.getLocalOp () , asImp().printInfo() );

      typedef DiscreteOperatorImp <COType, DFDomainType, DFRangeType > OPType;

      OPType *discrOp = new OPType (op, *locOp );

      // memorize this new generated object because is represents this
      // operator and is deleted if this operator is deleted
      // this is managed by the ObjPointerStorage class
      this->saveObjPointer( discrOp , locOp );

      return (*discrOp);
    }

    //! This method work just the same way like the operator +
    //! but this time for mutiplication with scalar
    DiscreteOperatorImp< ScaledLocalOperator < LocalOperatorImp ,
            typename  DFDomainType::RangeFieldType > ,  DFDomainType, DFRangeType > &
    operator * (const RangeFieldType& scalar)
    {
      if(asImp().printInfo())
        std::cout << "DiscreteOperatorDefault::operator * called! \n";

      typedef ScaledLocalOperator < LocalOperatorImp,
          typename  DFDomainType::RangeFieldType > ScalOperatorType;

      typedef DiscreteOperatorImp < ScalOperatorType , DFDomainType,DFRangeType > ScalDiscrType;

      ScalOperatorType * sop = new ScalOperatorType ( asImp().getLocalOp() , scalar, asImp().printInfo() );
      ScalDiscrType *discrOp = new ScalDiscrType    ( asImp() , *sop );

      // memorize this new generated object because is represents this
      // operator and is deleted if this operator is deleted
      this->saveObjPointer ( discrOp, sop );

      return (*discrOp);
    }

    // set level
    void initLevel (int level) const
    {
      level_ = level;
    }

    //! call initLevel
    void initialize (int level) const
    {
      initLevel ( level );
    }

    //! apply operator, only used by mapping
    void apply ( const DFDomainType & arg, DFRangeType & dest) const
    {
      std::cerr << "ERROR: DiscreteOperatorDefault::apply called! \n";
    }

    //! apply operator
    void operator () ( const DFDomainType & arg, DFRangeType & dest ) const
    {
      std::cerr << "ERROR: DiscreteOperatorDefault::operator () called! \n";
    }

    //! default implemenation of printInfo just returns false, no info is
    //! printed, if true every constructor of CombinedLocalOperator prints
    //! messages
    bool printInfo () { return false; }

  protected:
    //! current level of operator
    mutable int level_;

  private:
    //! Barton-Nackman
    DiscreteOperatorImp < LocalOperatorImp , DFDomainType , DFRangeType > &
    asImp() { return static_cast <DiscreteOperatorImp < LocalOperatorImp , DFDomainType , DFRangeType > &> (*this); }

    const DiscreteOperatorImp < LocalOperatorImp , DFDomainType , DFRangeType > &
    asImp() const { return static_cast <const DiscreteOperatorImp < LocalOperatorImp , DFDomainType , DFRangeType > &> (*this); }

  }; // end class DiscreteOperatorDefault

  /** @} end documentation group */

} // end namespace Dune

#endif
