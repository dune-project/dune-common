// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_AGINDEX_HH__
#define __DUNE_AGINDEX_HH__

#include "../../common/array.hh"

namespace Dune {

  enum INDEXSTATE { NEW, OLD , USED, UNUSED };

  template <class GridType>
  class SerialIndexSet
  {
    typedef int T;

    Array<int>    globalIndex_;
    Array<int>    oldGlobalIndex_;
    Array<INDEXSTATE> state_;

    int maxIndex_;
    int nextFreeIndex_;
    int nextIndex_;

    GridType & grid_;
  public:
    SerialIndexSet (GridType & grid) : grid_ (grid)
    {
      maxIndex_ = 0;
    };

    // calculate new highest index
    //template <class GridType>
    void insertNew (GridType & grid)
    {
      this->resize ( grid );
      for(int l=0; l<=grid.maxlevel(); l++)
      {
        typedef typename GridType::template Traits<0>::LevelIterator LevelIteratorType;
        LevelIteratorType endit = grid.template lend<0>(l);
        LevelIteratorType it = grid.template lbegin<0>(l);

        for( ; it != endit ; ++it)
        {
          this->insert( *it );
        }
      }
      this->finish();
    }

    //template <class GridType>
    void resize (GridType & grid)
    {
      this->resize ( grid.global_size (0));
    }

    // calculate new highest index
    void resize (int newMaxInd )
    {
      if( globalIndex_.size() < newMaxInd )
      {
        globalIndex_.swap ( oldGlobalIndex_ );
        globalIndex_.resize ( 2*newMaxInd );
        state_.resize ( 2*newMaxInd );
        for(int i=0; i<globalIndex_.size(); i++)
          globalIndex_[i] = -1;

        maxIndex_ = 2*newMaxInd;

        nextFreeIndex_ = -1;
        for(int i=0; i<oldGlobalIndex_.size(); i++)
        {
          globalIndex_[i] = oldGlobalIndex_[i];
          if(state_[i] != UNUSED)
            state_[i] = OLD;
          if(globalIndex_[i] > nextFreeIndex_)
            nextFreeIndex_ = globalIndex_[i];
          if(globalIndex_[i] < 0)
            state_[i] = UNUSED;
        }
        for(int i=oldGlobalIndex_.size(); i<state_.size(); i++)
          state_[i] = UNUSED;

        nextFreeIndex_++;
        nextIndex_ = 0;
      }
      else
      {
        for(int i=0; i<state_.size(); i++)
        {
          if(state_[i] != UNUSED)
            state_[i] = OLD;
        }
      }
      //std::cout << nextFreeIndex_ << " freeInd \n";
    }

    void finish ()
    {
      for(int i=0; i<state_.size(); i++)
      {
        if(state_[i] == OLD)
        {
          state_[i] = UNUSED;
        }
      }
      std::cout << maxIndex() << " max Index of Set \n";
    }

    int maxIndex () const
    {
      int max = 0;
      for(int i =0; i<globalIndex_.size(); i++)
      {
        if(globalIndex_[i] > max ) max = globalIndex_[i];
      }
      return max;
    }



    int searchNext ()
    {
      if(nextIndex_ >= maxIndex_)
        return -1;

      while((state_[nextIndex_] != UNUSED) ||
            (globalIndex_[nextIndex_] < 0) )
      {
        nextIndex_++;
        if(nextIndex_ >= maxIndex_ )
          return -1;
      }
      //std::cout << state_[nextIndex_] << " " << globalIndex_[nextIndex_] << "\n";
      nextIndex_++;
      return globalIndex_[nextIndex_-1];
    }

    // memorise index
    template <class EntityType>
    void insert (EntityType & en)
    {
      this->insert ( en.global_index() );
    }

    // memorise index
    void insert (int num )
    {
      assert(num < globalIndex_.size() );
      //std::cout << " insert  num = " << num << "\n";
      if(globalIndex_[num] < 0)
      {
        int ind = searchNext ();
        if( ind >= 0 )
        {
          globalIndex_[num] = ind;
        }
        else
        {
          globalIndex_[num] = nextFreeIndex_;
          nextFreeIndex_++;
        }
        state_[num] = NEW;
        return;
      }
      state_[num] = USED;
    }

    void print ( ) const
    {
      std::cout << "Size " << globalIndex_.size() << "\n";
      std::cout << "i    |   val    | state \n";
      for(int i=0; i<globalIndex_.size(); i++)
      {
        std::cout << i << " | " << globalIndex_[i] << " | " << state_[i] << "\n";
      }
    }

    bool write_xdr(const char * filename, int timestep)
    {
      FILE  *file;
      XDR xdrs;
      const char *path = NULL;

      const char * fn  = genFilename(path,filename, timestep);
      file = fopen(fn, "wb");
      if (!file)
      {
        printf( "\aERROR in AGIndexSet::write_xdr(..): couldnot open <%s>!\n", filename);
        fflush(stderr);
        return false;
      }

      xdrstdio_create(&xdrs, file, XDR_ENCODE);
      this->processXdr(&xdrs);

      xdr_destroy(&xdrs);
      fclose(file);
    }

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
        printf( "\aERROR in AGIndexSet::read_xdr(..): couldnot open <%s>!\n", filename);
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

    bool processXdr(XDR *xdrs)
    {
      xdr_int ( xdrs, &maxIndex_ );
      xdr_int ( xdrs, &nextIndex_ );
      xdr_int ( xdrs, &nextFreeIndex_ );
      globalIndex_.processXdr(xdrs);
      state_.processXdr(xdrs);
      return true;
    }

    int size () const
    {
      return nextFreeIndex_;
      //return grid_.global_size(0);
    }

    bool isNew (int index) const
    {
      if(state_[index] == NEW)
        return true;

      return false;
    }

    int operator [] (int i) const
    {
      //printf(" gIndex_[%d] = %d \n",i,globalIndex_[i]);
      return globalIndex_[i];
    }
  };

} // end namespace

#endif
