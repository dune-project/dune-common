// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_AGINDEX_HH__
#define __DUNE_AGINDEX_HH__

#include "../../common/array.hh"

namespace Dune {

  enum INDEXSTATE { NEW, OLD , USED, UNUSED };

  //template <class T>
  class SerialIndexSet
  {
    typedef int T;

    Array<int>    globalIndex_;
    Array<int>    oldGlobalIndex_;
    Array<INDEXSTATE> state_;

    int maxIndex_;
    int nextFreeIndex_;
    int nextIndex_;
  public:
    SerialIndexSet ()
    {
      maxIndex_ = 0;
    };

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
