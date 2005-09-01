// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_MAPPING_HH__
#define __DUNE_MAPPING_HH__

#include "vectorspace.hh"

#include <vector>

namespace Dune {


  /** @defgroup Mapping Mapping
     \ingroup AlgebraCommon
     Mappings in Dune always map from one vector space into another vector space.

     @{
   */

  /** \brief A mapping from one vector space into another
      This class describes a general mapping from the domain vector space into
      the range vector space.
      It can also be used to construct linear combinations of mappings.

      This two-sided character has the following consequence: when you address
      an object of type mapping or any of its descendants through a reference
      or pointer of type Mapping, the linear combination defined for that mapping
      is evaluated. On the other hand, if you address through a reference of the
      type of any of its descendants (notably Operator and Function), you'll
      get the functionality specific for that type.
   */
  template<typename DFieldType,typename RFieldType, class DType, class RType>
  class Mapping //: public Vector < RFieldType >
  {
  public:
    //! domain vector space (for usage in derived classes)
    typedef DType DomainType;
    //! range vector space
    typedef RType RangeType;
    //! integral type used in the construction of the domain vector space
    typedef DFieldType DomainFieldType;
    //! integral type used in the construction of the range vector space
    typedef RFieldType RangeFieldType;
    //! \todo why that one?
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

    //! operators for linear combinations
    virtual MappingType operator + (const MappingType &) const ;
    virtual MappingType operator - (const MappingType &) const ;
    virtual MappingType operator * (const Field &) const  ;
    virtual MappingType operator / (const Field &) const  ;
    virtual MappingType& operator  = (const MappingType &) ;
    virtual MappingType& operator += (const MappingType &) ;
    virtual MappingType& operator -= (const MappingType &) ;
    virtual MappingType& operator *= (const Field &)  ;
    virtual MappingType& operator /= (const Field &)  ;

    //! apply the whole linear combination which was created with the
    //! operators above, using the apply method of the combined operators
    void operator() (const DomainType &Arg, RangeType &Dest ) const
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
    virtual void apply (const DomainType &Arg, RangeType &Dest) const {
      operator()(Arg, Dest);
    }

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

  /** @} end documentation group */


}

#endif
