// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_ALU3DGRID_DATAHANDLE_HH__
#define __DUNE_ALU3DGRID_DATAHANDLE_HH__

namespace ALU3dGridSpace {

  //! the corresponding interface class is defined in bsinclude.hh
  template <class GridType, class EntityType, class DataCollectorType >
  class GatherScatterImpl : public GatherScatter
  {
    GridType & grid_;
    typedef typename GridType::template codim<0>::Entity Entity;
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
      //en_.setGhost( static_cast <PLLBndFaceType &> (ghost) );

      PLLBndFaceType & gh = static_cast <PLLBndFaceType &> (ghost);
      realEntity_.setGhost( *(gh.getGhost()) );

      dc_.gather(str, en_);
    }

  };

}

#endif
