// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_MAPPING_HH__
#define __DUNE_MAPPING_HH__

#include "vectorspace.hh"

#include <vector>

namespace Dune {

  // Note: Range has to have Vector structure as well.
  template<typename DFieldType,typename RFieldType, class DType, class RType>
  class Mapping : public Vector < RFieldType >
  {
  public:
    //! remember template parameters for derived classes
    typedef DType DomainType;
    typedef RType RangeType;
    typedef DFieldType DomainFieldType;
    typedef RFieldType RangeFieldType;
    typedef RangeFieldType Field;

    //! remember what type this class has
    typedef Mapping<DFieldType,RFieldType,DType,RType> MappingType;

    //! create Mappiung with empty linear combination
    Mapping( )
    {
      lincomb_.push_back( term( *this, 1.0 ) );
    }

    //! delete linear combination if necessary
    virtual ~Mapping( ) {}

    //! this method has to be overloaded by derived classes
    virtual void initLevel ( int level ) const
    {
      std::cerr << "ERROR: Mapping::initLevel called. \n";
    }

    //! initialize all operators with this level
    virtual void initialize ( int level ) const
    {
      int count = 0;
      for ( typename std::vector<term>::const_iterator it = lincomb_.begin(); it != lincomb_.end(); it++ )
        it->v_->initLevel( level );
    }

    //! operators for linear combinations
    virtual MappingType operator + (const Vector<Field> &) const ;
    virtual MappingType operator - (const Vector<Field> &) const ;
    virtual MappingType operator * (const Field &) const  ;
    virtual MappingType operator / (const Field &) const  ;
    virtual Vector<Field>& operator  = (const Vector<Field> &) ;
    virtual Vector<Field>& operator += (const Vector<Field> &) ;
    virtual Vector<Field>& operator -= (const Vector<Field> &) ;
    virtual Vector<Field>& operator *= (const Field &)  ;
    virtual Vector<Field>& operator /= (const Field &)  ;

    //! apply must not called here, because the apply method has
    //! to be overloaded by the implementation class
    virtual void apply( const DomainType &Arg, RangeType &Dest ) const {
      std::cerr << "ERROR: Mapping::apply called. \n";
    }

    //! apply the hole linear combination which was created with the
    //! operators above, using the apply method of the combined operators
    void operator()( const DomainType &Arg, RangeType &Dest ) const
    {
      //Dest.clear();

      int count = 0;
      for ( typename std::vector<term>::const_iterator it = lincomb_.begin(); it != lincomb_.end(); it++ )
      {
        if ( count == 0 ) {
          it->v_->apply( Arg, Dest );
          if ( it->scalar_ != 1. ) {
            Dest *= it->scalar_;
          }
        } else {
          RangeType tmp( Dest );
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
        count++;
      }
    }
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
