// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DGRIDCOMMUNICATOR_HH
#define DUNE_ALU3DGRIDCOMMUNICATOR_HH

// use this define to control if Albert should use the found MPI

#include <dune/grid/common/grid.hh>

#ifdef _BSGRID_PARALLEL_
#if HAVE_MPI
#include <mpi.h>
#endif
#endif

namespace Dune {

  /* Not used here
     template <class DofManagerType>
     class CommunicatorInterface
     {
     public:
     virtual bool firstMark() {return false;}
     virtual bool secondMark(){return false;};
     virtual bool thirdMark() {return false;};

     virtual bool markFirstLevel() {return false;};
     virtual bool markNextLevel () {return false;};

     virtual bool xtractData (DofManagerType & dm) = 0;

     virtual bool repartition (DofManagerType & dm) = 0;
     virtual bool communicate (DofManagerType & dm) = 0;
     virtual bool consistencyGhosts () = 0;
     };
   */

  /*!
     ALU3dGridCommunicator organizes the communication of ALU3dGrid on
     diffrent processors.
   */

  template <class GridType>
  class ALU3dGridCommunicator
  {
  public:
    //! Constructor
    ALU3dGridCommunicator(GridType &grid) : grid_(grid) {}

    template <class DataType>
    bool communicate(DataType & data) const
    {
      grid_.communicate(data,All_All_Interface,ForwardCommunication);
      return false;
    }

    template <class DataType>
    bool loadBalance(DataType & data) const
    {
      return grid_.loadBalance(data);
    }

    bool loadBalance() const
    {
      return grid_.loadBalance();
    }

    //! minimize val over all processors
    template <class T>
    T globalMin (T val) const
    {
      T ret = grid_.globalMin(val);
      return ret;
    }

    //! maximize val over all processors
    template <class T>
    T globalMax (T val) const
    {
      T ret = grid_.globalMax(val);
      return ret;
    }

    //! maximize val over all processors
    template <class T>
    T globalSum (T val) const
    {
      return grid_.globalSum(val);
    }

    template <class T>
    void globalSum (T *send, int s , T *recv) const
    {
      grid_.globalSum(send,s,recv);
      return ;
    }

  private:
    // reference to corresponding grid
    GridType & grid_;
  };

} // end namespace Dune


#endif
