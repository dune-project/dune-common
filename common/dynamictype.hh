// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DYNAMICTYPE_HH__
#define __DUNE_DYNAMICTYPE_HH__

namespace Dune {

  typedef int IdentifierType;

  class DynamicType {
  public:

    DynamicType (int ident) : identifier_ (ident) {};

    virtual bool operator == ( const DynamicType &) {return true;};
    virtual bool operator != ( const DynamicType &) {return true;};
    virtual bool operator <= ( const DynamicType &) {return true;};
    virtual bool operator >= ( const DynamicType &) { return true; };

    IdentifierType getIdentifier() const { return identifier_; }

    static const IdentifierType undefined = 0;

  protected:

    IdentifierType identifier_;
  };

}

#endif
