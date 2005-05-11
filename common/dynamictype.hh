// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DYNAMICTYPE_HH
#define DUNE_DYNAMICTYPE_HH

#include <cassert>
//#include <iostream>

namespace Dune {

  typedef int IdentifierType;

  //! Base class used for tagging derived classes with an identifier
  class DynamicType {
  public:

    //! Definition for undefined value
    static const IdentifierType undefined = -1;

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
      //std::cout << "Id: " << identifier_ << "(" << undefined << ")" << std::endl;
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
