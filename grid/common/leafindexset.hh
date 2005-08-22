// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_LEAFINDEXSET_HH__
#define __DUNE_LEAFINDEXSET_HH__

#include <dune/common/array.hh>
#include <dune/grid/common/defaultindexsets.hh>

namespace Dune {

  /*!
     This class provides an Array which as an aditional resize and copy the old
     values functionality.
   */
  template <class T>
  class IndexArray : public Array<T>
  {
  public:
    IndexArray() : Array<T> ()
    {
      this->n = 0; this->p = 0;
    }

    //! reallocate array with size m
    void realloc (int m, int factor)
    {
      if(m <= this->n) return;
      int newSize = m*factor;
      T * newMem = 0;

      try
      {
        newMem = new T[newSize];
      }
      catch (std::bad_alloc)
      {
        std::cerr << "Not enough memory!" << std::endl;
        throw;
      }

      if(this->n > 0)
        std::memcpy(newMem , this->p , this->n*sizeof(T));

      this->n = newSize;
      if(this->p) delete [] this->p;
      this->p = newMem;
    }

    //! write Array to xdr stream
    bool processXdr(XDR *xdrs)
    {
      if(xdrs != 0)
      {
        int len = this->n;
        xdr_int( xdrs, &len );
        if(len != this->n) this->resize(len);
        xdr_vector(xdrs,(char *) this->p,this->n,sizeof(T),(xdrproc_t)xdr_double);
        return true;
      }
      else
        return false;
    }

  };

  //******************************************************************
  //
  // Indexset that provides consecutive indicies for the leaf level
  // this index set uses the grid hierarchical index
  //
  //******************************************************************
  /*!
     This index set generates and consecutive leaf index out of the unique
     global index of each entity. This index set can be used instead of the
     default grid index sets an can be generated for each grid implementation.

     Note that only codim = 0 is working at the moment.

     Future work. Index set for each codim.
   */
  template <class GridType>
  class AdaptiveLeafIndexSet : public DefaultGridIndexSetBase <GridType>
  {
    // busines as usual

    template <class EntityType,class IndexSetType, int enCodim, int codim>
    struct IndexWrapper
    {
      static inline int index (EntityType & en , const IndexSetType & set ,
                               const IndexArray<int> & leafIndex, int num )
      {
        // this index set works only for codim = 0 at the moment
        assert(codim == 0);

        //assert( en.isLeaf () );

        // check if we have index for given entity
        //assert(leafIndex[ set.index(en) ] >= 0);
        //assert( (leafIndex[ set.index(en) ] < 0) ? (std::cout << set.index(en) << " idx, part = " << en.partitionType() << " on p = " << __MyRank__ << "\n",0 ): 1);
        assert( leafIndex[ set.index(en) ] >= 0 );
        //if(leafIndex[ set.index(en) ] < 0) return 0;
        return leafIndex[ set.index(en) ];
      }
    };

    //! if codim > codim of entity use subIndex
    template <class EntityType,class IndexSetType>
    struct IndexWrapper<EntityType,IndexSetType,0,2>
    {
      static inline int index (EntityType & en , const IndexSetType & set ,
                               const IndexArray<int> & leafIndex, int num )
      {
        return en.template subIndex<2> (num);
      }
    };

    template <class EntityType,class IndexSetType>
    struct IndexWrapper<EntityType,IndexSetType,0,3>
    {
      static inline int index (EntityType & en , const IndexSetType & set ,
                               const IndexArray<int> & leafIndex, int num )
      {
        return en.template subIndex<3> (num);
      }
    };
    //******************************************************************

    // my type, to be revised
    enum { myType = 2 };

    enum INDEXSTATE { NEW, USED, UNUSED };

    enum { dim = GridType :: dimension };

    // the mapping of the global to leaf index
    IndexArray<int> leafIndex_;

    // old indices
    IndexArray<int> oldLeafIndex_;

    // stack for holes
    IndexArray<int> holes_;

    // the state of each index
    IndexArray<INDEXSTATE> state_;

    // next index to give away
    int nextFreeIndex_;

    // actual size of index set
    int actSize_;

    // true if all entities that we use are marked as USED
    bool marked_;

    // true if the used entities were marked by grid walkthrough
    bool markAllU_;

    const int factor_;

    typedef typename GridType :: HierarchicIndexSetType HIndexSetType;
    const HIndexSetType & hIndexSet_;

  public:
    enum { ncodim = GridType::dimension + 1 };
    AdaptiveLeafIndexSet ( GridType & grid ) : DefaultGridIndexSetBase <GridType> (grid) ,
                                               nextFreeIndex_ (0), actSize_(0), marked_(false), markAllU_ (false)
                                               , factor_(2) , hIndexSet_( grid.hierarchicIndexSet() )
    {
      resizeVectors();

      // give all entities that lie below the old entities new numbers
      markAllUsed ();
      //resize();
      markAllU_ = false;
    }

    int type () const { return myType; }

    virtual ~AdaptiveLeafIndexSet () {};

    template <class EntityType>
    void restrictLocal ( EntityType &father, EntityType &son, bool initialize )
    {
      removeOldIndex( son );
      insertNewIndex( father );
    }

    template <class EntityType>
    void prolongLocal ( EntityType &father, EntityType &son, bool initialize )
    {
      insertNewIndex( son );
      removeOldIndex( father );
    }

    void insertNewIndex (const typename GridType::template Codim<0>::Entity & en )
    {
      // here we have to add the support of higher codims
      resizeVectors();

      //std::cout << "Insert new Entity = " << en.globalIndex() << "\n";
      //std::cout << leafIndex_.size() << " l|g " << this->grid_.global_size(0) << "\n";

      this->insert( en );
      marked_ = false;
    }

    void removeOldIndex (const typename GridType::template Codim<0>::Entity & en )
    {
      //std::cout << "remove old Entity = " << en.globalIndex() << "\n";
      // here we have to add the support of higher codims
      state_[ hIndexSet_.index(en) ] = UNUSED;
    }

    // reallocate the vector for new size
    void resizeVectors()
    {
      if(leafIndex_.size() < this->grid_.global_size(0))
      {
        //std::cout << "Resize with max = " << this->grid_.global_size(0) << "\n";
        int oldSize = leafIndex_.size();

        leafIndex_.realloc(this->grid_.global_size(0), factor_ );
        state_.realloc(this->grid_.global_size(0), factor_ );

        // here we dont need to copy
        oldLeafIndex_.realloc(this->grid_.global_size(0), factor_ );

        for(int i=oldSize; i<leafIndex_.size(); i++)
        {
          leafIndex_[i] = -1;
          state_[i] = UNUSED;
        }
      }
    }

    //! if grid has changed, resize index vectors, and create
    //! indices for new entities, new entities are entities that
    //! lie below the old entities
    void resize ()
    {
      resizeVectors();

      // give all entities that lie below the old entities new numbers
      markAllBelowOld ();
    }

    //! for dof manager, to check whether it has to copy dof or not
    bool indexNew (int num, int codim) const
    {
      if(codim > 0) return false;
      assert((num >= 0) && (num < state_.size()));
      return state_[num] == NEW;
    }

    //! make to index numbers consecutive
    //! return true, if at least one hole was closed
    bool compress ()
    {
      // true if a least one dof must be copied
      bool haveToCopy = false;

      const int sizeOfVecs = state_.size();
      if( holes_.size() < sizeOfVecs )
        holes_.resize( state_.size() );

      // if not marked, mark which indices are still used
      if( (!marked_) && markAllU_ ) actSize_ = markAllUsed(); // returns actual size of leaf level

      // mark holes
      int actHole = 0;
      int newActSize = 0;
      for(int i=0; i<sizeOfVecs; i++)
      {
        if(leafIndex_[i] >= 0)
        {
          // create vector with all holes
          if((state_[i] == UNUSED) && (leafIndex_[i] >= 0))
          {
            holes_[actHole] = leafIndex_[i];
            actHole++;
          }
          // count the size of the leaf indices
          newActSize++;
        }
      }

      assert( newActSize >= actHole );
      // the new size is the actual size minus the holes
      actSize_ = newActSize - actHole;

      // copy index, for copying in dof manager
      oldLeafIndex_ = leafIndex_;

      // close holes
      //
      // NOTE: here the holes closing should be done in
      // the opposite way. future work.
      for(int i=0; i<leafIndex_.size(); i++)
      {
        // a index that is used but larger then actual size
        // has to move to a hole
        if(state_[i] != UNUSED)
        {
          // if used index lies behind size, then index has to move
          // to one of the holes
          if(leafIndex_[i] >= actSize_)
          {
            // serach next hole that is smaler than actual size
            actHole--;
            // if actHole < 0 then error, because we have index larger then
            // actual size
            assert(actHole >= 0);
            while ( holes_[actHole] >= actSize_ )
            {
              actHole--;
              if(actHole < 0) break;
            }

            assert(actHole >= 0);
            leafIndex_[i] = holes_[actHole];

            // means that dof manager has to copy the mem
            state_[i] = NEW;
            haveToCopy = true;
          }
        }
        else
        {
          // all unsed indices are reset to -1
          leafIndex_[i] = -1;
        }
      }

      // the next index that can be given away is equal to size
      nextFreeIndex_ = actSize_;

      // next turn mark again
      marked_ = false;
      markAllU_ = false;

      return haveToCopy;
    }

    //! return subIndex of given entity
    template <int cd, class EntityType>
    int subIndex (const EntityType & en, int i) const
    {
      assert(cd == dim);
      return en.template subIndex<cd>(i);
    }

    //! returns vector with geometry tpyes this index set has indices for
    const std::vector <GeometryType > & geomTypes () const
    {
      return hIndexSet_.geomTypes();
    }


    // memorise index
    template <class EntityType>
    void insert (EntityType & en)
    {
      this->insert ( hIndexSet_.index(en) );
    }

    //! return how much extra memory is needed for restriction
    int additionalSizeEstimate () const { return nextFreeIndex_; }

    //! return size of grid entities per level and codim
    int size ( int codim ) const
    {
      // this index set works only for codim = 0 at the moment
      if(codim == 0)
      {
        return nextFreeIndex_;
      }

      if(codim == dim)
        return this->grid_.global_size(codim);

      assert(false);
      return 0;
    }

    //! return global index
    //! for dof mapper
    template <int codim, class EntityType>
    int index (EntityType & en, int num) const
    {
      return IndexWrapper<EntityType,HIndexSetType,EntityType::codimension,codim>::
             index(en,hIndexSet_,leafIndex_,num);
      //std::cout << "return index \n";
      //assert( codim == 0 );
      //return leafIndex_[ hIndexSet_.index(en) ];
    }

    //! return global index
    //! for dof mapper
    template <class EntityType>
    int index (EntityType & en) const
    {
      assert( EntityType::codimension == 0 );
      assert( en.isLeaf() );
      assert( leafIndex_[ hIndexSet_.index(en) ] >= 0 );
      return leafIndex_[ hIndexSet_.index(en) ];
    }

    //! return size of grid entities per level and codim
    //! for dof mapper
    int oldSize ( int codim ) const
    {
      if(codim > 0) return this->grid_.global_size(codim);
      // this index set works only for codim = 0 at the moment
      return state_.size();
    }

    //! return old index, for dof manager only
    int oldIndex (int elNum, int codim ) const
    {
      if(codim>0) return elNum;
      return oldLeafIndex_[elNum];
    }

    //! return new index, for dof manager only returns index
    int newIndex (int elNum , int codim ) const
    {
      if(codim>0) return elNum;
      return leafIndex_[elNum];
    }

  private:
    // insert index if entities lies below used entity, return
    // false if not , otherwise return true
    template <class EntityType>
    bool insertNewIndex (const EntityType & en, bool isLeaf , bool canInsert )
    {
      // if entity isLeaf then we insert index
      if(isLeaf)
      {
        // count leaf entities
        actSize_++;
        this->insert ( en );
        return true;
      }

      // which is the case if we havent reached a entity which has
      // already a number
      if(!canInsert)
      {
        // from now on, indices can be inserted
        if(leafIndex_[ hIndexSet_.index(en) ] >= 0)
        {
          return true;
        }

        // we have to go deeper
        return false;
      }
      else
      {
        this->insert ( en );
        // set unused here, because index is only needed for prolongation
        state_[ hIndexSet_.index(en) ] = UNUSED;
      }

      return true;
    }

    // insert element and create index for element number
    void insert (int num )
    {
      assert(num < leafIndex_.size() );
      if(leafIndex_[num] < 0)
      {
        leafIndex_[num] = nextFreeIndex_;
        nextFreeIndex_++;
      }
      state_[num] = USED;
    }

    //! mark indices that are still used and give new indices to
    //! elements that need one
    int markAllUsed ()
    {
      for(int i=0; i<state_.size(); i++) state_[i] = UNUSED;

      int nSize = 0;

      typedef typename GridType::LeafIterator LeafIteratorType;
      // walk over leaf level on locate all needed entities
      {
        LeafIteratorType endit  = this->grid_.template leafend<0>   ( this->grid_.maxlevel());
        for(LeafIteratorType it = this->grid_.template leafbegin<0> ( this->grid_.maxlevel()); it != endit ; ++it )
        {
          this->insert( *it );
          nSize++;
        }
      }

      marked_ = true;
      return nSize;
    }

    //! give all entities that lie below the old entities new numbers
    void markAllBelowOld ()
    {
      typedef typename GridType::template Codim<0>::LevelIterator LevelIteratorType;

      int maxlevel = this->grid_.maxlevel();

      for(int i=0; i<state_.size(); i++) state_[i] = UNUSED;

      // actSize is increased be insertNewIndex
      actSize_ = 0;

      for(int level = 0; level<=maxlevel; level++)
      {
        LevelIteratorType levelend    = this->grid_.template lend  <0> (level);
        for(LevelIteratorType levelit = this->grid_.template lbegin<0> (level);
            levelit != levelend; ++levelit )
        {
          typedef typename GridType::template Codim<0>::
          Entity::HierarchicIterator HierarchicIteratorType;

          // if we have index all entities below need new numbers
          bool areNew = false;

          // check whether we can insert or not
          areNew = insertNewIndex ( *levelit , levelit->isLeaf() , areNew );

          HierarchicIteratorType endit  = levelit->hend   ( level + 1 );
          for(HierarchicIteratorType it = levelit->hbegin ( level + 1 ); it != endit ; ++it )
          {
            // areNew == true, then index is inserted
            areNew = insertNewIndex  ( *it , it->isLeaf() , areNew );
          }

        } // end grid walk trough
      } // end for all levels

      // means on compress we have to mark the leaf level
      marked_ = false;
      markAllU_ = true;
    }

    // print interal data, for debugging only
    // print if only done, if DEBUG_LEAFINDEXSET is defined
    void print (const char * msg, bool oldtoo = false ) const;

  public:

    // write indexset to xdr file
    bool write_xdr(const std::basic_string<char> filename, int timestep)
    {
      FILE  *file;
      XDR xdrs;
      const char *path = "";

      std::basic_string<char> fnstr = genFilename(path,filename, timestep);
      const char * fn = fnstr.c_str();
      file = fopen(fn, "wb");
      if (!file)
      {
        std::cerr << "\aERROR in AdaptiveLeafIndexSet::write_xdr(..): couldnot open " << filename << std::endl;
        std::cerr.flush();
        return false;
      }

      xdrstdio_create(&xdrs, file, XDR_ENCODE);
      this->processXdr(&xdrs);

      xdr_destroy(&xdrs);
      fclose(file);
      return true;
    }

    //! read index set from given xdr file
    bool read_xdr(const std::basic_string<char> filename , int timestep)
    {
      FILE   *file;
      XDR xdrs;
      const char *path = "";

      std::basic_string<char> fnstr = genFilename(path,filename, timestep);
      const char * fn = fnstr.c_str();
      std::cout << "Reading <" << fn << "> \n";
      file = fopen(fn, "rb");
      if(!file)
      {
        std::cerr <<"\aERROR in AdaptiveLeafIndexSet::read_xdr(..): couldnot open <%s>!\n" << filename << std::endl;
        std::cerr.flush();
        return(false);
      }

      // read xdr
      xdrstdio_create(&xdrs, file, XDR_DECODE);
      this->processXdr(&xdrs);

      xdr_destroy(&xdrs);
      fclose(file);
      return true;
    }

  private:
    // read/write from/to xdr stream
    bool processXdr(XDR *xdrs)
    {
      int type = myType;
      xdr_int ( xdrs, &type );
      if(type != myType)
      {
        std::cerr << "\nERROR: AdaptiveLeafIndexSet: wrong type choosen! \n\n";
        assert(type == myType);
      }

      xdr_int ( xdrs, &nextFreeIndex_ );
      xdr_int ( xdrs, &actSize_ );
      leafIndex_.processXdr(xdrs);
      state_.processXdr(xdrs);
      return true;
    }
  }; // end of class AdaptiveLeafIndexSet


  template <class GridType>
  class DefaultLeafIndexSet : public DefaultGridIndexSetBase <GridType>
  {
    typedef typename GridType :: LeafIndexSetType LeafIndexSetType;
    LeafIndexSetType & leafIndexSet_;

  public:
    enum { ncodim = GridType::dimension + 1 };
    DefaultLeafIndexSet ( GridType & grid ) :
      DefaultGridIndexSetBase <GridType> (grid) ,
      leafIndexSet_ ( grid.leafIndexSet() )
    {}

    virtual ~DefaultLeafIndexSet () {};

    void insertNewIndex (const typename GridType::template codim<0>::Entity & en )
    {
      leafIndexSet_.insertNewIndex(en);
    }

    void removeOldIndex (const typename GridType::template codim<0>::Entity & en )
    {
      leafIndexSet_.removeOldIndex(en);
    }

    //! if grid has changed, resize index vectors, and create
    //! indices for new entities, new entities are entities that
    //! lie below the old entities
    void resize ()
    {
      leafIndexSet_.resize();
    }

    //! for dof manager, to check whether it has to copy dof or not
    bool indexNew (int num, int codim)
    {
      return leafIndexSet_.indexNew(num,codim);
    }

    //! make to index numbers consecutive
    //! return true, if at least one hole was closed
    bool compress ()
    {
      return leafIndexSet_.compress();
    }

    //! return how much extra memory is needed for restriction
    int additionalSizeEstimate () const { return leafIndexSet_.additionalSizeEstimate (); }

    //! return size of grid entities per level and codim
    int size ( int level , int codim ) const
    {
      return leafIndexSet_.size(level,codim);
    }

    //! return global index
    //! for dof mapper
    template <int codim, class EntityType>
    int index (EntityType & en, int num) const
    {
      return leafIndexSet_.template index<codim>(en,num);
    }

    //! return global index
    //! for dof mapper
    template <class EntityType>
    int index (EntityType & en) const
    {
      return leafIndexSet_.index(en);
    }

    //! return size of grid entities per level and codim
    //! for dof mapper
    int oldSize ( int level , int codim ) const
    {
      return leafIndexSet_.oldSize(level,codim);
    }

    //! return old index, for dof manager only
    int oldIndex (int elNum, int codim ) const
    {
      return leafIndexSet_.oldIndex(elNum,codim);
    }

    //! return new index, for dof manager only returns index
    int newIndex (int elNum , int codim ) const
    {
      return leafIndexSet_.newIndex(elNum,codim);
    }

  public:

    // write indexset to xdr file
    bool write_xdr(const std::basic_string<char> filename, int timestep)
    {
      return leafIndexSet_.write_xdr( filename, timestep );
    }

    //! read index set from given xdr file
    bool read_xdr(const std::basic_string<char> filename , int timestep)
    {
      return leafIndexSet_.read_xdr( filename, timestep );
    }

  }; // end of class DefaultLeafIndexSet


  template <class GridType>
  inline void AdaptiveLeafIndexSet<GridType>::
  print (const char * msg, bool oldtoo ) const
  {
#ifdef DEBUG_LEAFINDEXSET
    std::cout << "Size " << leafIndex_.size() << "\n";
    std::cout << "i    |   val    | state  \n";
    int actSize =0;
    for(int i=0; i<leafIndex_.size(); i++)
    {
      if(state_[i] != UNUSED) actSize++;
      std::cout << i << " | " << leafIndex_[i] << " | " << state_[i];
      std::cout << "\n";
    }

    std::cout << "Real Size " << nextFreeIndex_ << "\n";
    std::cout << "ActSize   " << actSize << "\n";
    std::cout << "Grid global Size " << this->grid_.global_size(0) << "\n";

    std::cout << msg;
    return ;
#endif
  }

  //! class for combining 2 index sets together for adaptation process
  template <class A, class B >
  class CombinedAdaptProlongRestrict
  {
    //! space A and B
    const A & _a;
    const B & _b;
  public:
    CombinedAdaptProlongRestrict ( const A & a, const B & b ) : _a ( a ) , _b ( b )
    {}

    template <class EntityType>
    void restrictLocal ( EntityType &father, EntityType &son, bool initialize ) const
    {
      _a.restrictLocal(father,son,initialize);
      _b.restrictLocal(father,son,initialize);
    }

    //! prolong data to children
    template <class EntityType>
    void prolongLocal ( EntityType &father, EntityType &son, bool initialize ) const
    {
      _a.prolongLocal(father,son,initialize);
      _b.prolongLocal(father,son,initialize);
    }
  };

} // end namespace Dune

#endif
