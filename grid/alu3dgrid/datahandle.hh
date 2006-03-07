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
  template <class GridType, class DataCollectorType , int codim >
  class GatherScatterImpl : public GatherScatter
  {
    GridType & grid_;
    typedef typename GridType::template Codim<codim>::Entity EntityType;
    typedef typename EntityType :: ImplementationType RealEntityType;

    //typedef typename Dune::ALU3dImplTraits<GridType::elementType>::PLLBndFaceType PLLBndFaceType;

    typedef typename Dune::ALU3dImplTraits<GridType::elementType>::
    template Codim<codim>::ImplementationType ImplElementType;
    typedef typename Dune::ALU3dImplTraits<GridType::elementType>::
    template Codim<codim>::InterfaceType HElementType;

    EntityType     & entity_;
    RealEntityType & realEntity_;

    DataCollectorType & dc_;

    typedef ObjectStream ObjectStreamType;

  public:
    //! Constructor
    GatherScatterImpl(GridType & grid, EntityType & en, RealEntityType & realEntity , DataCollectorType & dc)
      : grid_(grid), entity_(en), realEntity_(realEntity) , dc_(dc) {}

    //! returns contains of dc_
    bool contains(int dim, int cd) const { return dc_.contains(dim,cd); }

    //! this method is called from the dunePackAll method of the corresponding
    //! Macro element class of the BSGrid, see gitter_dune_pll*.*
    //! here the data is written to the ObjectStream
    void inlineData ( ObjectStreamType & str , HElementType & elem )
    {
      // set element and then start
      realEntity_.setElement(elem);
      //dc_.inlineData(str,entity_);
      std::cout << "inline entity " <<  grid_.hierarchicIndexSet().index(entity_) << "\n";
      dc_.gather(str,entity_);
    }

    //! this method is called from the duneUnpackSelf method of the corresponding
    //! Macro element class of the BSGrid, see gitter_dune_pll*.*
    //! here the data is read from the ObjectStream
    void xtractData ( ObjectStreamType & str , HElementType & elem )
    {
      // set element and then start
      grid_.updateStatus();
      realEntity_.setElement(elem);
      /*
         dc_.xtractData(str,entity_);
       */
      std::cout << "xtract entity " <<  grid_.hierarchicIndexSet().index(entity_) << "\n";
      dc_.scatter(str,entity_,dc_.size(entity_));
    }

    void setData ( ObjectStreamType & str , HElementType & elem )
    {
      // set element and then start
      realEntity_.setElement(elem);
      /*
         dc_.xtractData(str,entity_);
       */
      std::cout << "set entity " <<  grid_.hierarchicIndexSet().index(entity_) << "\n";
      dc_.set(str,entity_);
    }

    //! write Data of one lement to stream
    void sendData ( ObjectStreamType & str , const HElementType & elem )
    {
      realEntity_.setElement( const_cast<HElementType &> (elem));
      dc_.scatter(str, entity_);
    }

    //! read Data of one element from stream
    void recvData ( ObjectStreamType & str , HElementType & elem )
    {
      // set ghost as entity
      realEntity_.setElement( elem );
      dc_.insertNewIndex( entity_ );
      dc_.checkMemorySize();
      dc_.gather(str, entity_ );
    }
  };

  //***********************************************************
  //
  //  --specialisation for codim 0
  //
  //***********************************************************

  //! the corresponding interface class is defined in bsinclude.hh
  template <class GridType, class DataCollectorType >
  class GatherScatterImpl<GridType,DataCollectorType,0> : public GatherScatter
  {
    enum { codim = 0 };
    GridType & grid_;
    typedef typename GridType::template Codim<codim>::Entity EntityType;
    typedef typename EntityType :: ImplementationType RealEntityType;

    typedef typename Dune::ALU3dImplTraits<GridType::elementType>::
    template Codim<codim>::ImplementationType IMPLElementType;
    typedef typename Dune::ALU3dImplTraits<GridType::elementType>::
    template Codim<codim>::InterfaceType HElementType;

    typedef typename Dune::ALU3dImplTraits<GridType::elementType>::
    template Codim<codim>::GhostInterfaceType HGhostType;
    typedef typename Dune::ALU3dImplTraits<GridType::elementType>::
    template Codim<codim>::GhostImplementationType ImplGhostType;



    EntityType     & entity_;
    RealEntityType & realEntity_;

    DataCollectorType & dc_;

    typedef ObjectStream ObjectStreamType;

  public:
    //! Constructor
    GatherScatterImpl(GridType & grid, EntityType & en, RealEntityType & realEntity , DataCollectorType & dc)
      : grid_(grid), entity_(en), realEntity_(realEntity) , dc_(dc) {}

    bool contains(int dim, int codim) const
    {
      return dc_.contains(dim,codim);
    }

    //! this method is called from the dunePackAll method of the corresponding
    //! Macro element class of the BSGrid, see gitter_dune_pll*.*
    //! here the data is written to the ObjectStream
    void inlineData ( ObjectStreamType & str , HElementType & elem )
    {
      // set element and then start
      realEntity_.setElement(elem,0,0);
      dc_.inlineData(str,entity_);
    }

    //! this method is called from the duneUnpackSelf method of the corresponding
    //! Macro element class of the BSGrid, see gitter_dune_pll*.*
    //! here the data is read from the ObjectStream
    void xtractData ( ObjectStreamType & str , HElementType & elem )
    {
      // set element and then start
      grid_.updateStatus();
      realEntity_.setElement(elem,0,0);
      dc_.xtractData(str,entity_);
    }

    //! write Data of one lement to stream
    void sendData ( ObjectStreamType & str , const HElementType & elem )
    {
      realEntity_.setElement( const_cast<HElementType &> (elem),0,0 );
      dc_.scatter(str, entity_);
    }

    //! read Data of one element from stream
    void recvData ( ObjectStreamType & str , HGhostType & ghost )
    {
      // set ghost as entity
      ImplGhostType & gh = static_cast <ImplGhostType &> (ghost);
      realEntity_.setGhost( gh );

      dc_.insertNewIndex( entity_ );
      dc_.checkMemorySize();
      dc_.gather(str, entity_);
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
        , realSon_(s) , dm_(dm) , rp_(rp) , maxlevel_(-1)
    {
      //#ifndef NDEBUG
      //    global_Geometry_lock = true;
      //#endif
    }

    virtual ~AdaptRestrictProlongImpl ()
    {
      //#ifndef NDEBUG
      //    global_Geometry_lock = false;
      //#endif
    }

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
      int level = realSon_.level();
      if(level > maxlevel_) maxlevel_ = level;

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
