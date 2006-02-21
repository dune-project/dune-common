// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DGRIDINDEXSETS_HH
#define DUNE_ALU3DGRIDINDEXSETS_HH

//- System includes
#include <vector>

//- Dune includes
#include <dune/common/stdstreams.hh>
#include <dune/common/bigunsignedint.hh>

#include <dune/common/array.hh>
#include <dune/grid/common/grid.hh>
#include <dune/grid/common/indexidset.hh>


//- Local includes
#include "alu3dinclude.hh"

namespace Dune {

  //! HierarchicIndexSet uses LeafIterator tpyes for all codims and partition types
  template <class GridImp>
  struct ALU3dGridHierarchicIteratorTypes
  {
    //! The types of the iterator
    template<int cd>
    struct Codim
    {
      template<PartitionIteratorType pitype>
      struct Partition
      {
        typedef typename GridImp::Traits::template Codim<cd>::template Partition<pitype>::LeafIterator Iterator;
      };
    };
  };

  // Forward declarations
  template <int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGrid;

  template<int cd, int dim, class GridImp>
  class ALU3dGridEntity;

  //! hierarchic index set of ALU3dGrid
  template <int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGridHierarchicIndexSet :
    public IndexSetDefaultImplementation <ALU3dGrid<dim,dimworld,elType>,
        ALU3dGridHierarchicIndexSet<dim,dimworld,elType>,
        ALU3dGridHierarchicIteratorTypes<ALU3dGrid<dim,dimworld,elType> > >
  {
    typedef ALU3dGrid<dim,dimworld,elType> GridType;
    enum { numCodim = dim+1 }; // i.e. 4

    ALU3dGridHierarchicIndexSet(const GridType & grid) : grid_(grid)
    {}
    friend class ALU3dGrid<dim,dimworld,elType>;

  public:
    typedef typename GridType::Traits::template Codim<0>::Entity EntityCodim0Type;

    //! return hierarchic index of given entity
    template <class EntityType>
    int index (const EntityType & ep) const
    {
      enum { cd = EntityType :: codimension };
      return (grid_.getRealImplementation(ep)).getIndex();
    }

    //! return subIndex of given entity
    template <int cd>
    int subIndex (const EntityCodim0Type & ep, int i) const
    {
      const ALU3dGridEntity<0,dim,const GridType> & en = (grid_.getRealImplementation(ep));
      return en.template getSubIndex<cd>(i);
    }

    //! return size of indexset, i.e. maxindex+1
    //! for given type, if type is not exisiting within grid 0 is returned
    int size ( int codim , GeometryType type ) const
    {
      assert( grid_.geomTypes(codim).size() == 1 );
      if( type != grid_.geomTypes(codim)[0] ) return 0;
      // return size of hierarchic index set
      return grid_.hierSetSize(codim);
    }

    //! return size of indexset, i.e. maxindex+1
    int size ( int codim ) const
    {
      // return size of hierarchic index set
      return grid_.hierSetSize(codim);
    }

    //! deliver all geometry types used in this grid
    const std::vector<GeometryType>& geomTypes (int codim) const
    {
      return grid_.geomTypes(codim);
    }

    /** @brief Iterator to one past the last entity of given codim for partition type
     */
    template<int cd, PartitionIteratorType pitype>
    typename ALU3dGridHierarchicIteratorTypes<GridType>::template Codim<cd>::
    template Partition<pitype>::Iterator end () const
    {
      return grid_.template leafend<cd,pitype> ();
    }

    /** @brief Iterator to first entity of given codimension and partition type.
     */
    template<int cd, PartitionIteratorType pitype>
    typename ALU3dGridHierarchicIteratorTypes<GridType>::template Codim<cd>::
    template Partition<pitype>::Iterator begin () const
    {
      return grid_.template leafbegin<cd,pitype> ();
    }

  private:
    // our Grid
    const GridType & grid_;
  };

  //*****************************************************************
  //
  //  --GlobalIdSet
  //
  //*****************************************************************
  //! global id set for ALU3dGrid
  template <int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGridGlobalIdSet :
    public IdSetDefaultImplementation  < ALU3dGrid<dim,dimworld,elType> ,
        ALU3dGridGlobalIdSet<dim,dimworld,elType> ,
        typename
        ALU3dGrid<dim,dimworld,elType>::Traits::GlobalIdType >
  {
    typedef ALU3dGrid<dim,dimworld,elType> GridType;
    typedef typename GridType :: HierarchicIndexSet HierarchicIndexSetType;

    typedef ALU3dImplTraits<elType> ImplTraitsType;
    typedef typename ImplTraitsType::IMPLElementType IMPLElementType;
    typedef typename ImplTraitsType::GEOFaceType GEOFaceType;
    typedef typename ImplTraitsType::GEOEdgeType GEOEdgeType;

    typedef ALU3DSPACE HElementType HElementType;
    typedef ALU3DSPACE HFaceType HFaceType;
    typedef ALU3DSPACE HEdgeType HEdgeType;
    typedef ALU3DSPACE VertexType VertexType;

    enum { vertexShiftBits = 32 };
    enum { codimShiftBits  = 2  };
    enum { levelShiftBits  = 6  };
    enum { nChildShiftBits = 4  };

  public:
    //! export type of id
    typedef typename ALU3dGrid<dim,dimworld,elType>::Traits::GlobalIdType IdType;

  private:
    enum { numCodim = dim+1 };

    // this means that only up to 300000000 entities are allowed
    typedef typename GridType::Traits::template Codim<0>::Entity EntityCodim0Type;

    //! create id set, only allowed for ALU3dGrid
    ALU3dGridGlobalIdSet(const GridType & grid) : grid_(grid), hset_(grid.hierarchicIndexSet())
    {
      if(elType == hexa)
      {
        // see ALUGrid/src/serial/gitter_mgb.cc
        // InsertUniqueHexa
        const int vxKey[4] = {0,1,3,4};
        for(int i=0; i<4; i++) vertexKey_[i] = vxKey[i];
      }
      else
      {
        assert( elType == tetra );
        // see ALUGrid/src/serial/gitter_mgb.cc
        // InsertUniqueTetra
        const int vxKey[4] = {0,1,2,3};
        for(int i=0; i<4; i++) vertexKey_[i] = vxKey[i];
      }

      // setup the id set
      buildIdSet();
    }

    // update id set after adaptation
    void updateIdSet()
    {
      // to be revised
      buildIdSet();
    }

    // print all ids
    void print () const
    {
      for(int i=0 ; i<numCodim; ++i)
      {
        std::cout << "*****************************************************\n";
        std::cout << "Ids for codim " << i << "\n";
        std::cout << "*****************************************************\n";
        for(unsigned int k=0; k<ids_[i].size(); ++k)
        {
          std::cout << "Item[" << i << "," << k <<"] has id " << ids_[i][k] << "\n";
        }
        std::cout << "\n\n\n";
      }
    }

    void checkId(const IdType & id , int codim , unsigned int num ) const
    {
      for(int i=0 ; i<numCodim; ++i)
      {
        for(unsigned int k=0; k<ids_[i].size(); ++k)
        {
          if((i == codim) && (k == num)) continue;
          assert(!(id == ids_[i][k]));
        }
      }
    }

    // check id set for uniqueness
    void uniquenessCheck() const
    {
      for(int i=0 ; i<numCodim; i++)
      {
        for(unsigned int k=0; k<ids_[i].size(); ++k)
        {
          checkId(ids_[i][k],i,k);
        }
      }
    }

    // creates the id set
    void buildIdSet ()
    {
      for(int i=0; i<numCodim; ++i)
      {
        ids_[i].resize( hset_.size(i) );
        for(unsigned int k=0; k<ids_[i].size(); ++k)
        {
          ids_[i][k] = -1;
        }
      }

      ALU3DSPACE GitterImplType & gitter = const_cast<ALU3DSPACE
                                                      GitterImplType &> (grid_.myGrid());

      {
        ALU3DSPACE AccessIterator <VertexType>::Handle fw (gitter.container ()) ;
        for (fw.first () ; ! fw.done () ; fw.next ())
        {
          int idx = fw.item().getIndex();
          ids_[3][idx] = buildVertexId( fw.item() );
        }
      }

      // create ids for all macro edges
      {
        ALU3DSPACE AccessIterator <HEdgeType> :: Handle w (gitter.container ()) ;
        for (w.first(); !w.done(); w.next())
        {
          int idx = w.item().getIndex();
          ids_[2][idx] = buildEdgeId( w.item() );
          buildEdgeIds( w.item() , ids_[2][idx] , 0 );
        }
      }

      // for all macro faces and all children
      {
        ALU3DSPACE AccessIterator <HFaceType>::Handle w (gitter.container ()) ;
        for (w.first () ; ! w.done () ; w.next ())
        {
          int idx = w.item().getIndex();
          ids_[1][idx] = buildFaceId( w.item() );
          buildFaceIds( w.item() , ids_[1][idx] , 0 );
        }
      }

      // for all macro elements and all internal entities
      {
        ALU3DSPACE AccessIterator <HElementType> :: Handle w (gitter.container ()) ;
        for (w.first () ; ! w.done () ; w.next ())
        {
          int idx = w.item().getIndex();
          ids_[0][idx] = buildMacroId( w.item() );
          buildElementIds( w.item() , ids_[0][idx] , 0);
        }
      }

      uniquenessCheck();
      //print();
    }

    IdType buildVertexId(const VertexType & item )
    {
      // first the codim
      IdType id(3);

      // then the four identifying vertex indices
      int idx = item.ident();
      id = id << vertexShiftBits;
      id = id+ IdType(idx);

      //std::cout << "Element[" << elem.getIndex() << "] has id = " <<id<< " key\n";
      // create offset
      id = id << 3*vertexShiftBits;
      return id;
    }

    IdType buildEdgeId(const HEdgeType & item )
    {
      const GEOEdgeType & elem = static_cast<const GEOEdgeType &> (item);
      assert( elem.level () == 0);
      //assert( elem.nChild() == 0);

      // first the codim
      IdType id(2);

      // then the four identifying vertex indices
      for(int i=0; i<2; i++)
      {
        int idx = elem.myvertex(i)->ident();
        id = id << vertexShiftBits;
        id = id+ IdType(idx);
      }

      // create offset
      id = id << 2*vertexShiftBits;

      //std::cout << "Element[" << elem.getIndex() << "] has id = " <<id<< " key\n";
      return id;
    }

    IdType buildFaceId(const HFaceType & item )
    {
      const GEOFaceType & elem = static_cast<const GEOFaceType &> (item);
      assert( elem.level () == 0);
      //assert( elem.nChild() == 0);

      // first the codim
      IdType id(1);

      // then the four identifying vertex indices
      for(int i=0; i<3; i++)
      {
        int idx = elem.myvertex(i)->ident();
        id = id << vertexShiftBits;
        id = id+ IdType(idx);
      }

      // create offset
      id = id << vertexShiftBits;
      //std::cout << "Element[" << elem.getIndex() << "] has id = " <<id<< " key\n";
      return id;
    }

    IdType buildMacroId(const ALU3DSPACE HElementType & item )
    {
      const IMPLElementType & elem = static_cast<const IMPLElementType &> (item);
      assert( elem.level () == 0);
      //assert( elem.nChild() == 0);


      // first the codim
      IdType id(0);

      // then the four identifying vertex indices
      for(int i=0; i<4; i++)
      {
        int idx = elem.myvertex(vertexKey_[i])->ident();
        id = id << vertexShiftBits;
        id = id+ IdType(idx);
      }

      //std::cout << "Element[" << elem.getIndex() << "] has id = " <<id<< " key\n";
      return id;
    }

    template <int cd>
    IdType createId(const typename ImplTraitsType::
                    template Codim<cd>::InterfaceType & item , const IdType & fatherId , int nChild )
    {
      // id is fathers id + number of child
      IdType id(fatherId);

      id = id << codimShiftBits;
      id = id + IdType(cd);

      id = id << nChildShiftBits;
      id = id + IdType(nChild);

      //std::cout << "Item<" << cd << ">[" << elem.getIndex() << "] has id = " <<id<< " key\n";
      return id;
    }

    void buildElementIds(const HElementType & item , const IdType & macroId , int nChild )
    {
      enum { codim = 0 };
      ids_[codim][item.getIndex()] = createId<codim>(item,macroId,nChild);
      const IdType & fatherId = ids_[codim][item.getIndex()];

      // build id for inner vertex
      {
        const VertexType * v = item.innerVertex() ;
        if(v) buildVertexIds(*v,fatherId );
      }

      // build edge ids for all inner edges
      {
        int inneredge = 0;
        for(const HEdgeType * e = item.innerHedge () ; e ; e = e->next ())
        {
          buildEdgeIds(*e,fatherId,inneredge);
          ++inneredge;
        }
      }

      // build face ids for all inner faces
      {
        int innerface = 0;
        for(const HFaceType * f = item.innerHface () ; f ; f = f->next ())
        {
          buildFaceIds(*f,fatherId,innerface);
          ++innerface;
        }
      }

      // build ids of all children
      {
        int numChild = 0;
        for(const HElementType * child = item.down(); child; child =child->next() )
        {
          buildElementIds(*child, fatherId, numChild);
          ++numChild;
        }
      }
    }

    void buildFaceIds(const HFaceType & face, const IdType & fatherId , int
                      innerFace )
    {
      enum { codim = 1 };
      ids_[codim][face.getIndex()] = createId<codim>(face,fatherId,innerFace);
      const IdType & faceId = ids_[codim][face.getIndex()];
      // build id for inner vertex
      {
        const VertexType * v = face.innerVertex() ;
        if(v) buildVertexIds(*v,faceId );
      }

      // build ids for all inner edges
      {
        int inneredge = 0;
        for (const HEdgeType * e = face.innerHedge () ; e ; e = e->next ())
        {
          buildEdgeIds(*e,faceId ,inneredge );
          ++inneredge;
        }
      }

      // build ids for all child faces
      {
        int child = 0;
        for(const HFaceType * f = face.down () ; f ; f = f->next ())
        {
          buildFaceIds(*f,faceId,child);
          ++child;
        }
      }
    }

    void buildEdgeIds(const HEdgeType & edge, const IdType & fatherId , int inneredge )
    {
      enum { codim = 2 };
      ids_[codim][edge.getIndex()] = createId<codim>(edge,fatherId,inneredge);
      const IdType & edgeId = ids_[codim][edge.getIndex()];

      // build id for inner vertex
      {
        const VertexType * v = edge.innerVertex() ;
        if(v) buildVertexIds(*v,edgeId );
      }

      // build ids for all inner edges
      {
        int child = 0;
        for (const HEdgeType * e = edge.down () ; e ; e = e->next ())
        {
          buildEdgeIds(*e,edgeId , child );
          ++child;
        }
      }
    }

    void buildVertexIds(const VertexType & vertex, const IdType & fatherId )
    {
      enum { codim = 3 };
      ids_[codim][vertex.getIndex()] = createId<codim>(vertex,fatherId,0);
    }

    friend class ALU3dGrid<dim,dimworld,elType>;
  public:
    //! return global id of given entity
    template <class EntityType>
    IdType id (const EntityType & ep) const
    {
      enum { cd = EntityType :: codimension };
      return ids_[cd][hset_.index(ep)];
    }

    //! return global id of given entity
    template <int codim>
    IdType id (const typename GridType:: template Codim<codim> :: Entity & ep) const
    {
      return ids_[codim][hset_.index(ep)];
    }

    //! return subId of given entity
    template <int cd>
    IdType subId (const EntityCodim0Type & ep, int i) const
    {
      return ids_[cd][hset_.template subIndex<cd>(ep,i)];
    }

  private:
    mutable std::vector< IdType > ids_[numCodim];
    // our Grid
    const GridType & grid_;

    // the hierarchicIndexSet
    const HierarchicIndexSetType & hset_;

    int vertexKey_[4];
  };

  //***********************************************************
  //
  //  --LocalIdSet
  //
  //***********************************************************

  //! hierarchic index set of ALU3dGrid
  template <int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGridLocalIdSet :
    public IdSetDefaultImplementation < ALU3dGrid<dim,dimworld,elType> ,
        ALU3dGridLocalIdSet<dim,dimworld,elType> ,
        int >
  {
    typedef ALU3dGrid<dim,dimworld,elType> GridType;
    typedef typename GridType :: HierarchicIndexSet HierarchicIndexSetType;

    // this means that only up to 300000000 entities are allowed
    enum { codimMultiplier = 300000000 };
    typedef typename GridType::Traits::template Codim<0>::Entity EntityCodim0Type;

    // create local id set , only for the grid allowed
    ALU3dGridLocalIdSet(const GridType & grid) : hset_(grid.hierarchicIndexSet())
    {
      for(int i=0; i<dim+1; i++)
        codimStart_[i] = i*codimMultiplier;
    }
    friend class ALU3dGrid<dim,dimworld,elType>;

    // fake method to have the same method like GlobalIdSet
    void updateIdSet() {}

  public:
    //! export type of id
    typedef int IdType;

    //! return global id of given entity
    template <class EntityType>
    int id (const EntityType & ep) const
    {
      enum { cd = EntityType :: codimension };
      assert( hset_.size(cd) < codimMultiplier );
      return codimStart_[cd] + hset_.index(ep);
    }

    //! return global id of given entity
    template <int codim>
    int id (const typename GridType:: template Codim<codim> :: Entity & ep) const
    {
      //enum { cd = EntityType :: codimension };
      assert( hset_.size(codim) < codimMultiplier );
      return codimStart_[codim] + hset_.index(ep);
    }

    //! return subId of given entity
    template <int cd>
    int subId (const EntityCodim0Type & ep, int i) const
    {
      assert( hset_.size(cd) < codimMultiplier );
      return codimStart_[cd] + hset_.template subIndex<cd>(ep,i);
    }

  private:
    // our HierarchicIndexSet
    const HierarchicIndexSetType & hset_;

    // store start of each codim numbers
    int codimStart_[dim+1];
  };

} // end namespace Dune

#include "indexsets_imp.cc"

#endif
