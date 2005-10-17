// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DGRIDDATAHANDLE_HH
#define DUNE_ALU3DGRIDDATAHANDLE_HH

#include <iostream>
#include "alu3dinclude.hh"

using std::endl;
using std::cout;
using std::flush;

namespace ALUGridSpace {

  //! the corresponding interface class is defined in bsinclude.hh
  template <class GridType, class EntityType, class DataCollectorType >
  class GatherScatterImpl : public GatherScatter
  {
    GridType & grid_;
    typedef typename GridType::template Codim<0>::Entity Entity;
    typedef typename Dune::ALU3dImplTraits<GridType::elementType>::PLLBndFaceType PLLBndFaceType;
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
    virtual void inlineData ( ObjectStreamType & str , HElementType & elem )
    {
      // set element and then start
      realEntity_.setElement(elem);
      dc_.inlineData(str,en_);
    }

    //! this method is called from the duneUnpackSelf method of the corresponding
    //! Macro element class of the BSGrid, see gitter_dune_pll*.*
    //! here the data is read from the ObjectStream
    virtual void xtractData ( ObjectStreamType & str , HElementType & elem )
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
      realEntity_.setGhost( gh );

      dc_.insertNewIndex( en_ );
      dc_.checkMemorySize();
      dc_.gather(str, en_);
    }

  };


  //! the corresponding interface class is defined in bsinclude.hh
  template <class GridType, class EntityType, class DataCollectorType >
  class GatherScatterExchange : public GatherScatter
  {
    GridType & grid_;
    typedef typename GridType::template Codim<0>::Entity Entity;
    typedef typename Dune::ALU3dImplTraits<GridType::elementType>::PLLBndFaceType PLLBndFaceType;
    Entity & en_;
    EntityType & realEntity_;

    DataCollectorType & dc_;

    typedef ObjectStream ObjectStreamType;

  public:
    //! Constructor
    GatherScatterExchange(GridType & grid, EntityType & en, DataCollectorType & dc)
      : grid_(grid), en_(en), realEntity_(en) , dc_(dc) {}

    //! this method is called from the dunePackAll method of the corresponding
    //! Macro element class of the BSGrid, see gitter_dune_pll*.*
    //! here the data is written to the ObjectStream
    virtual void inlineData ( ObjectStreamType & str , HElementType & elem )
    {
      cerr << "Wrong Operator, do not use for load balance! in: " <<__FILE__ << " line: " << __LINE__ << endl;
      abort();
    }

    //! this method is called from the duneUnpackSelf method of the corresponding
    //! Macro element class of the BSGrid, see gitter_dune_pll*.*
    //! here the data is read from the ObjectStream
    virtual void xtractData ( ObjectStreamType & str , HElementType & elem )
    {
      cerr << "Wrong Operator, do not use for load balance! in: " <<__FILE__ << " line: " << __LINE__ << endl;
      abort();
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
      realEntity_.setGhost( gh );

      // old set ghost method
      dc_.gather(str, en_);
    }

  };

  template <class GridType, class EntityType, class DofManagerType, class RestrictProlongOperatorType >
  class AdaptRestrictProlongImpl : public AdaptRestrictProlongType
  {
    GridType & grid_;
    typedef typename GridType::template Codim<0>::Entity Entity;
    typedef typename GridType::Traits::LeafIndexSet LeafIndexSetType;

    Entity & reFather_;
    Entity & reSon_;
    EntityType & realFather_;
    EntityType & realSon_;

    DofManagerType & dm_;
    RestrictProlongOperatorType & rp_;

    int maxlevel_;

    typedef typename Dune::ALU3dImplTraits<GridType::elementType>::PLLBndFaceType PLLBndFaceType;

  public:
    //! Constructor
    AdaptRestrictProlongImpl (GridType & grid, EntityType & f,
                              EntityType & s, DofManagerType & dm, RestrictProlongOperatorType & rp )
      : grid_(grid), reFather_(f), reSon_(s), realFather_(f)
        , realSon_(s) , dm_(dm) , rp_(rp) , maxlevel_(-1) {}

    virtual ~AdaptRestrictProlongImpl () {};

    //! restrict data , elem is always the father
    int preCoarsening ( HElementType & elem )
    {
      // set element and then start
      HElementType * son = elem.down();
      if(elem.level() > maxlevel_) maxlevel_ = elem.level();
      //elem.resetRefinedTag();
      assert( son );

      realSon_.setElement(*son);
      realFather_.setElement(elem);
      rp_.restrictLocal(reFather_,reSon_,true);

      son = son->next();
      while( son )
      {
        realSon_.setElement(*son);
        rp_.restrictLocal(reFather_,reSon_,false);
        son = son->next();
      }
      return 0;
    }

    //! prolong data, elem is the father
    int postRefinement ( HElementType & elem )
    {
      // set element and then start
      HElementType * son = elem.down();
      assert( son );
      //elem.resetRefinedTag();

      realFather_.setElement(elem);
      realSon_.setElement(*son);
      if(realSon_.level() > maxlevel_) maxlevel_ = realSon_.level();

      rp_.prolongLocal(reFather_,reSon_, false);

      son = son->next();
      while( son )
      {
        assert( son );

        realSon_.setElement(*son);
        rp_.prolongLocal(reFather_,reSon_, false);
        //(*son).resetRefinedTag();

        son = son->next();
      }
      return 0;
    }

    //! restrict data , elem is always the father
    //! this method is for ghost elements
    int preCoarsening ( HBndSegType & el )
    {
      // hier nur die Indices einfügen
      // da wir nur 4 ghost kenn macht das prolong keinen Sinn

      PLLBndFaceType & elem = static_cast<PLLBndFaceType &> (el);

      realFather_.setGhost(elem);
      dm_.insertNewIndex( reFather_ );

      // set element and then start
      PLLBndFaceType * son = static_cast<PLLBndFaceType *> (elem.down());
      while( son )
      {
        realSon_.setGhost(*son);
        dm_.removeOldIndex( reSon_ );
        son = static_cast<PLLBndFaceType *> (son->next());
      }
      return 0;
    }


    //! prolong data, elem is the father
    int postRefinement ( HBndSegType & el )
    {
      // for ghost only indices are inserted, because we only know 4 of 8
      // elements and therefore no prolongation can be done
      PLLBndFaceType & elem = static_cast<PLLBndFaceType &> (el);

      realFather_.setGhost(elem);
      dm_.insertNewIndex( reFather_ );

      // set element and then start
      PLLBndFaceType * son = static_cast<PLLBndFaceType *> (elem.down());
      while( son )
      {
        assert( son );
        realSon_.setGhost(*son);
        dm_.insertNewIndex( reSon_ );
        son = static_cast<PLLBndFaceType *> (son->next());
      }
      return 0;
    }

    int maxLevel () const { return maxlevel_; }
  };


  //***************************************************************
  //
  //  for load balancing
  //
  //***************************************************************

  template <class GridType, class EntityType, class DofManagerType >
  class LoadBalanceRestrictProlongImpl : public AdaptRestrictProlongType
  {
    GridType & grid_;
    typedef typename GridType::template Codim<0>::Entity Entity;

    Entity & reFather_;
    Entity & reSon_;
    EntityType & realFather_;
    EntityType & realSon_;

    DofManagerType & dm_;

    typedef typename Dune::ALU3dImplTraits<GridType::elementType>::PLLBndFaceType PLLBndFaceType;

    int newMemSize_;

  public:
    //! Constructor
    LoadBalanceRestrictProlongImpl (GridType & grid, EntityType & f,
                                    EntityType & s, DofManagerType & dm )
      : grid_(grid), reFather_(f), reSon_(s), realFather_(f)
        , realSon_(s) , dm_(dm), newMemSize_ (0) {}

    virtual ~LoadBalanceRestrictProlongImpl () {};

    //! restrict data , elem is always the father
    int postRefinement ( HElementType & elem )
    {
      //cout << "create element durin ldb , el = " << elem.getIndex() << "\n";
      realFather_.setElement(elem);
      dm_.removeOldIndex( reFather_ );

      HElementType * son = elem.down();
      while( son )
      {
        realSon_.setElement(*son);
        dm_.insertNewIndex( reSon_ );
        son = son->next();
        newMemSize_ ++ ;
      }
      return 0;
    }

    //! prolong data, elem is the father
    int preCoarsening ( HElementType & elem )
    {
      //cout << "remove element durin ldb , el = " << elem.getIndex() << "\n";
      realFather_.setElement(elem);
      dm_.insertNewIndex( reFather_ );

      HElementType * son = elem.down();
      while( son )
      {
        realSon_.setElement(*son);
        dm_.removeOldIndex( reSon_ );
        son = son->next();
      }
      return 0;
    }

    //! restrict data , elem is always the father
    //! this method is for ghost elements
    int preCoarsening ( HBndSegType & el )
    {
      //cout << "remove ghost durin ldb , el = " << el.getIndex() << "\n";
      PLLBndFaceType & elem = static_cast<PLLBndFaceType &> (el);

      realFather_.setGhost(elem);
      dm_.insertNewIndex( reFather_ );

      // set element and then start
      PLLBndFaceType * son = static_cast<PLLBndFaceType *> (elem.down());
      while( son )
      {
        realSon_.setGhost(*son);
        dm_.removeOldIndex( reSon_ );
        son = static_cast<PLLBndFaceType *> (son->next());
      }
      return 0;
    }

    //! restrict data , elem is always the father
    //! this method is for ghost elements
    int postRefinement ( HBndSegType & el )
    {
      //cout << "create ghost durin ldb , el = " << el.getIndex() << "\n";
      PLLBndFaceType & elem = static_cast<PLLBndFaceType &> (el);

      PLLBndFaceType * vati = static_cast<PLLBndFaceType *> ( elem.up());
      if( vati )
      {
        realFather_.setGhost(*vati);
        dm_.removeOldIndex( reFather_ );
      }

      realFather_.setGhost(elem);
      dm_.insertNewIndex( reFather_ );
      newMemSize_ ++;

      // set element and then start
      PLLBndFaceType * son = static_cast<PLLBndFaceType *> (elem.down());
      while( son )
      {
        realSon_.setGhost(*son);
        dm_.insertNewIndex( reSon_ );
        son = static_cast<PLLBndFaceType *> (son->next());
        newMemSize_ ++;
      }
      return 0;
    }

    int newElements () const { return newMemSize_; }
  };

} // end namespace

#endif
