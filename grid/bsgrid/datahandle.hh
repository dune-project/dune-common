// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_BSGRID_DATAHANDLE_HH__
#define __DUNE_BSGRID_DATAHANDLE_HH__

namespace BernhardSchuppGrid {


  //! the corresponding interface class is defined in bsinclude.hh
  template <class EntityType, class DataCollectorType >
  class GatherScatterImpl : public GatherScatter
  {
    DataCollectorType & dc_;
    EntityType & en_;

    typedef ObjectStream ObjectStreamType;

  public:
    GatherScatterImpl(EntityType & en, DataCollectorType & dc) : en_(en), dc_(dc) {}

    //! this method is called from the dunePackAll method of the corresponding
    //! Macro element class of the BSGrid, see gitter_dune_pll*.*
    //! here the data is written to the ObjectStream
    virtual void inlineData ( ObjectStreamType & str , HElemType & elem )
    {
      // set element and then start
      en_.setelement(elem,-1);
      dc_.inlineData(str,en_);
    }

    //! this method is called from the duneUnpackSelf method of the corresponding
    //! Macro element class of the BSGrid, see gitter_dune_pll*.*
    //! here the data is read from the ObjectStream
    virtual void xtractData ( ObjectStreamType & str , HElemType & elem )
    {
      // set element and then start
      en_.setelement(elem,-1);
      dc_.xtractData(str,en_);
    }

  };

}

#endif
