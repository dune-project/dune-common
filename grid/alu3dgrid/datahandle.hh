// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_ALU3DGRID_DATAHANDLE_HH__
#define __DUNE_ALU3DGRID_DATAHANDLE_HH__

#include <iostream>
#include "../alu3dgrid.hh"

using std::endl;
using std::cout;
using std::flush;

namespace ALU3dGridSpace {

  //! the corresponding interface class is defined in bsinclude.hh
  template <class GridType, class EntityType, class DataCollectorType >
  class GatherScatterImpl : public GatherScatter
  {
    GridType & grid_;
    typedef typename GridType::template codim<0>::Entity Entity;
    typedef Dune::ALU3dImplTraits<GridType::elementType>::PLLBndFaceType PLLBndFaceType;
    Entity & en_;
    EntityType & realEntity_;


    DataCollectorType & dc_;

    typedef ObjectStream ObjectStreamType;

  public:
    //! Constructor
    GatherScatterImpl(GridType & grid, EntityType & en, DataCollectorType & dc)
      : grid_(grid), en_(en), realEntity_(en) , dc_(dc) {}

    //! this method is called from the dunePackAll method of the corresponding
    //! Macro element class of the BSGrid, see gitter_dune_pll*.*
    //! here the data is written to the ObjectStream
    virtual void inlineData ( ObjectStreamType & str , HElemType & elem )
    {
      // set element and then start
      realEntity_.setElement(elem);
      dc_.inlineData(str,en_);
    }

    //! this method is called from the duneUnpackSelf method of the corresponding
    //! Macro element class of the BSGrid, see gitter_dune_pll*.*
    //! here the data is read from the ObjectStream
    virtual void xtractData ( ObjectStreamType & str , HElemType & elem )
    {
      // set element and then start
      grid_.updateStatus();
      realEntity_.setElement(elem);
      dc_.xtractData(str,en_);
    }

    //! write Data of one lement to stream
    virtual void sendData ( ObjectStreamType & str , const HElementType & elem )
    {
      realEntity_.setElement( const_cast<HElementType &> (elem) );
      dc_.scatter(str, en_);
    }

    //! read Data of one element from stream
    virtual void recvData ( ObjectStreamType & str , HGhostType & ghost )
    {
      // set ghost as entity
      PLLBndFaceType & gh = static_cast <PLLBndFaceType &> (ghost);
      /*
         if(gh.ghostLevel() < ghost.level())
         {
         HElementType & up = *(gh.getGhost()->up());
         realEntity_.setGhost( up );
         }
         else
         {
         realEntity_.setGhost( *(gh.getGhost()) );
         }
       */
      realEntity_.setGhost( *(gh.getGhost()) );
      dc_.gather(str, en_);
    }

  };

  template <class GridType, class EntityType,
      class DofManagerType, class RestrictProlongOperatorType >
  class AdaptRestrictProlongImpl : public AdaptRestrictProlongType
  {
    GridType & grid_;
    typedef typename GridType::template codim<0>::Entity Entity;

    Entity & reFather_;
    Entity & reSon_;
    EntityType & realFather_;
    EntityType & realSon_;

    DofManagerType & dm_;
    RestrictProlongOperatorType & rp_;

    int maxlevel_;
    int chunkSize_;

  public:
    //! Constructor
    AdaptRestrictProlongImpl (GridType & grid, EntityType & f, EntityType & s,
                              DofManagerType &dm, RestrictProlongOperatorType & rp, int chunkSize )
      : grid_(grid), reFather_(f), reSon_(s), realFather_(f)
        , realSon_(s) , dm_(dm), rp_(rp) , maxlevel_(-1) , chunkSize_(chunkSize) {}

    virtual ~AdaptRestrictProlongImpl () {};

    //! restrict data , elem is always the father
    int preCoarsening ( HElemType & elem )
    {
      // set element and then start
      HElemType * son = elem.down();
      if(elem.level() > maxlevel_) maxlevel_ = elem.level();
      //elem.resetRefinedTag();
      assert( son );

      //dm_.resizeChunk(elem.getIndex(),chunkSize_);

      realSon_.setElement(*son);
      realFather_.setElement(elem);
      rp_.restrictLocal(reFather_,reSon_, chunkSize_ ,true);

      son = son->next();
      while( son )
      {
        realSon_.setElement(*son);
        rp_.restrictLocal(reFather_,reSon_, chunkSize_,false);
        son = son->next();
      }
      return 0;
    }

    //! prolong data, elem is the father
    int postRefinement ( HElemType & elem )
    {
      // set element and then start
      HElemType * son = elem.down();
      assert( son );
      //elem.resetRefinedTag();

      realFather_.setElement(elem);
      realSon_.setElement(*son);
      if(realSon_.level() > maxlevel_) maxlevel_ = realSon_.level();

      rp_.prolongLocal(reFather_,reSon_, chunkSize_, false);

      son = son->next();
      while( son )
      {
        assert( son );

        realSon_.setElement(*son);
        rp_.prolongLocal(reFather_,reSon_, chunkSize_, false);
        //(*son).resetRefinedTag();

        son = son->next();
      }
      return 0;
    }

    int maxlevel () const { return maxlevel_; }
  };

} // end namespace

#endif
