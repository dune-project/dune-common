// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DYNAMICTYPE_HH__
#define __DUNE_DYNAMICTYPE_HH__

namespace Dune {

  typedef int IdentifierType;

  //! ???
  class DynamicType {
  public:

    //! ???
    static const IdentifierType undefined = 0;

    //! set identifier of this class
    DynamicType (int ident) : identifier_ (ident) {};

    //! Make undefined identifier
    DynamicType () : identifier_ (undefined) {};

    //! ???
    virtual bool operator == ( const DynamicType &) {return true;};

    //! ???
    virtual bool operator != ( const DynamicType &) {return true;};

    //! ???
    virtual bool operator <= ( const DynamicType &) {return true;};

    //! ???
    virtual bool operator >= ( const DynamicType &) { return true; };

    //! return identifier
    IdentifierType getIdentifier() const
    {
      assert(identifier_ != undefined);
      return identifier_;
    }


  protected:
    //! Method to set identifier later, which we need for quadratures
    void setIdentifier (IdentifierType ident)
    {
      identifier_ = ident;
    }

    //! The identifier, up to now an unique int
    IdentifierType identifier_;
  };

}

#endif
