// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_MAPPING_HH__
#define __DUNE_MAPPING_HH__

#include "vectorspace.hh"

#include <vector>

namespace Dune {


  // Note: Range has to have Vector structure as well.
  template<typename Field, class Domain, class Range>
  class Mapping : public Vector < Field > {

  public:
    Mapping( ) {
      lincomb_.push_back( term( *this, 1.0 ) );
    }

    typedef Mapping<Field,Domain,Range> MappingType;

    virtual MappingType operator + (const Vector<Field> &) const ;
    virtual MappingType operator - (const Vector<Field> &) const ;
    virtual MappingType operator * (const Field &) const  ;
    virtual MappingType operator / (const Field &) const  ;
    virtual Vector<Field>& operator  = (const Vector<Field> &) ;
    virtual Vector<Field>& operator += (const Vector<Field> &) ;
    virtual Vector<Field>& operator -= (const Vector<Field> &) ;
    virtual Vector<Field>& operator *= (const Field &)  ;
    virtual Vector<Field>& operator /= (const Field &)  ;

    virtual void apply( const Domain &Arg, Range &Dest ) const {
      std::cerr << "ERROR: Mapping::apply called. \n";
    }

    void operator()( const Domain &Arg, Range &Dest ) const {
      Dest.clear();
      Range tmp( Dest );

      for ( typename std::vector<term>::const_iterator it = lincomb_.begin(); it != lincomb_.end(); it++ ) {
        fprintf( stderr, " %+f*", it->scalar_ );
        it->v_->apply( Arg, tmp );
        if ( it->scalar_ == 1. ) {
          Dest += tmp;
        } else if ( it->scalar_ == -1. ) {
          Dest -= tmp;
        } else {
          tmp *= it->scalar_;
          Dest += tmp;
        }
      }
      std::cerr << std::endl;
    }

    virtual void applyAdd( const Domain &Arg, Range &Dest ) const {}

  private:

    struct term {
      term() : v_(NULL), scalar_(1.0), scaleIt_(false) { }

      term(const MappingType &mapping, Field scalar ) : v_(&mapping), scalar_(scalar), scaleIt_( true ) {
        if ( scalar_ == 1. ) {
          scaleIt_ = false;
        }
      }

      const MappingType *v_;
      Field scalar_;
      bool scaleIt_;
    };

    std::vector<term> lincomb_;

  };

#include "mapping.cc"

}

#endif
