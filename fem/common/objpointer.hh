// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_OBJPOINTER_HH__
#define __DUNE_OBJPOINTER_HH__

namespace Dune {

  //! storage class for newly generated operators during operator + on
  //! DiscreteOperator
  template <class ObjType>
  struct ObjPointer
  {
    typedef ObjPointer<ObjType> ObjPointerType;
    //! remember object item and next pointer
    ObjType * item;
    ObjPointerType *next;

    //! new ObjPointer is only created with pointer for item
    ObjPointer () : item (0) , next (0) {}

    //! new ObjPointer is only created with pointer for item
    ObjPointer (ObjType  *t) : item (t) , next (0) {}

    //! delete the next ObjPointer and the item
    virtual ~ObjPointer ()
    {
      if(next) delete next;next = 0;
      if(item) delete item;item = 0;
    }
  };

  //! ???
  class ObjPointerStorage
  {
    typedef ObjPointerStorage MyType;
  public:
    //! make new operator with item points to null
    ObjPointerStorage () : item_ (0) {}

    virtual ~ObjPointerStorage ()
    {
      if(item_) delete item_;item_ = 0;
    }

    //! Store new generated DiscreteOperator Pointer
    template <class DiscrOpType>
    void saveObjPointer ( DiscrOpType * discrOp )
    {
      ObjPointerType *next = new ObjPointerType ( discrOp );
      next->next = item_;
      item_ = next;
    }

  private:
    // store the objects created by operator + in here
    typedef ObjPointer<MyType> ObjPointerType;
    ObjPointerType * item_;

  };

} // end namespace Dune

#endif
