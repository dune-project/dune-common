// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DEFAULTINDEXSETS_HH__
#define __DUNE_DEFAULTINDEXSETS_HH__


namespace Dune {

  /*!
     The DefaultGridIndexSet is a wrapper for the grid index which can be
     index of entity or global_index. The DofMapper uses an IndexSet for
     mapping the dofs, so we can hide the real grid index behijnd the index
     set. Furthermore if an grid doesn't provide the consecutive index set
     then this can be calculated in the IndexSet. These two following index
     sets are just the identiy to the grid indices.

     The DefaultGridIndexSetBase defines some methods that are needed for
     index sets that cope adaptation, but arent needed for the following
     index set, so most of this methods do notin'.
   */
  template <class GridType>
  class DefaultGridIndexSetBase
  {
    // dummy value
    enum { myType = -1 };
  public:
    enum { ncodim = GridType::dimension + 1 };

    DefaultGridIndexSetBase ( GridType & grid ) : grid_ (grid) {}

    //! return false mean the no memory has to be allocated
    bool compress () { return false; }

    //! father index should already exist
    template <class EntityType>
    void createFatherIndex (EntityType &en)  {}

    //! nothing to do here
    void resize () {}

    //! no extra memory for restriction is needed
    int tmpSize () const { return 0; }

    //! all indices are old
    bool indexNew(int num) const { return false; }

    //! we have no old size
    int oldSize ( int level , int codim ) const
    {
      assert(false);
      return 0;
    }

    //! return old index, for dof manager only
    int oldIndex (int elNum) const
    {
      assert(false);
      return 0;
    }

    //! return new index, for dof manager only
    int newIndex (int elNum) const
    {
      assert(false);
      return 0;
    }

    //! write index set to xdr file
    bool write_xdr(const char * filename , int timestep)
    {
      FILE  *file;
      XDR xdrs;
      const char *path = NULL;

      const char * fn  = genFilename(path,filename, timestep);
      file = fopen(fn, "wb");
      if (!file)
      {
        fprintf(stderr,"\aERROR in DefaultGridIndexSet::write_xdr(..): couldnot open <%s>!\n", filename);
        fflush(stderr);
        return false;
      }

      xdrstdio_create(&xdrs, file, XDR_ENCODE);
      this->processXdr(&xdrs);

      xdr_destroy(&xdrs);
      fclose(file);

      return true;
    }

    //! read index set to xdr file
    bool read_xdr(const char * filename , int timestep)
    {
      FILE   *file;
      XDR xdrs;
      const char *path = NULL;

      const char * fn  = genFilename(path,filename, timestep);
      std::cout << "Reading <" << fn << "> \n";
      file = fopen(fn, "rb");
      if(!file)
      {
        fprintf(stderr,"\aERROR in DefaultGridIndexSet::read_xdr(..): couldnot open <%s>!\n", filename);
        fflush(stderr);
        return(false);
      }

      // read xdr
      xdrstdio_create(&xdrs, file, XDR_DECODE);
      this->processXdr(&xdrs);

      xdr_destroy(&xdrs);
      fclose(file);
      return true;
    }

  protected:
    // read/write from/to xdr stream
    bool processXdr(XDR *xdrs)
    {
      int type = myType;
      xdr_int ( xdrs, &type);
      if(type != myType)
      {
        std::cerr << "\nERROR: DefaultGridIndex: wrong type choosen! \n\n";
        assert(type == myType);
      }
      return true;
    }

    // the corresponding grid
    GridType & grid_;
  };

  //! Default is the Identity
  template <class GridType, GridIndexType GridIndex = GlobalIndex>
  class DefaultGridIndexSet : public DefaultGridIndexSetBase <GridType>
  {
    // my type, to be revised
    enum { myType = 0 };

    template <class EntityType,int enCodim, int codim>
    struct IndexWrapper
    {
      static inline int index (EntityType & en , int num )
      {
        return en.global_index();
      }
    };

    template <class EntityType, int codim>
    struct IndexWrapper<EntityType,codim,codim>
    {
      static inline int index (EntityType & en , int num )
      {
        return en.global_index();
      }
    };

    //! if codim > codim of entity use subIndex
    template <class EntityType>
    struct IndexWrapper<EntityType,0,2>
    {
      static inline int index (EntityType & en , int num )
      {
        return en.template subIndex<2> (num);
      }
    };

    template <class EntityType>
    struct IndexWrapper<EntityType,0,3>
    {
      static inline int index (EntityType & en , int num )
      {
        return en.template subIndex<3> (num);
      }
    };

  public:
    enum { ncodim = GridType::dimension + 1 };
    DefaultGridIndexSet ( GridType & grid ) : DefaultGridIndexSetBase <GridType> (grid) {}

    int size ( int level , int codim ) const
    {
      return this->grid_.global_size(codim);
    }

    template <int codim, class EntityType>
    int index (EntityType & en, int num) const
    {
      return IndexWrapper<EntityType,EntityType::codimension,codim>::index(en,num);
    }
  };

  template <class GridType>
  class DefaultGridIndexSet<GridType,LevelIndex>
    : public DefaultGridIndexSetBase <GridType>
  {
    // my type, to be revised
    enum { myType = 1 };

    //! return global number of sub entity with local number 'num'
    //! and codim of the sub entity = codim
    //! the enCodim is needed because the method index and subIndex have
    //! different names, ;)
    //! the wrapper classes allow partial specialisation,
    //! which is not allowed, but needed badly, :) the bob-trick
    //
    //! default implementation returns index of given entity
    template <class EntityType,int enCodim, int codim>
    struct IndexWrapper
    {
      static inline int index (EntityType & en , int num )
      {
        return en.index();
      }
    };

    // if codim and enCodim are equal, then return index
    template <class EntityType, int codim>
    struct IndexWrapper<EntityType,codim,codim>
    {
      static inline int index (EntityType & en , int num )
      {
        return en.index();
      }
    };

    // return number of vertex num
    template <class EntityType>
    struct IndexWrapper<EntityType,0,3>
    {
      static inline int index (EntityType & en , int num )
      {
        return en.template subIndex<3> (num);
      }
    };

    // return number of vertex num for dim == 2
    // return number of edge num for dim == 3
    template <class EntityType>
    struct IndexWrapper<EntityType,0,2>
    {
      static inline int index (EntityType & en , int num )
      {
        return en.template subIndex<2> (num);
      }
    };

    // return number of vertex for dim == 1
    // return number of edge num for dim == 2
    // return number of face num for dim == 3
    template <class EntityType>
    struct IndexWrapper<EntityType,0,1>
    {
      static inline int index (EntityType & en , int num )
      {
        return en.template subIndex<1> (num);
      }
    };

  public:
    enum { ncodim = GridType::dimension + 1 };
    DefaultGridIndexSet ( GridType & grid ) : DefaultGridIndexSetBase <GridType> (grid) {}

    //! return size of grid entities per level and codim
    int size ( int level , int codim ) const
    {
      return this->grid_.size(level,codim);
    }

    //! return index of entity with codim codim belonging to entity en which
    //! could have a bigger codim (for example return num of vertex num of an
    //! element en
    template <int codim, class EntityType>
    int index (EntityType & en, int num) const
    {
      return IndexWrapper<EntityType,EntityType::codimension,codim>::index(en,num);
    }
  };

} // end namespace Dune

#endif
