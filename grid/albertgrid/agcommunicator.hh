// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef _DUNE_ALBERTGRID_COMMUNICATOR_HH_
#define _DUNE_ALBERTGRID_COMMUNICATOR_HH_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef MPI_ACTIVE
#undef HAVE_MPI
#endif

#ifdef HAVE_MPI
#include <mpi.h>
#endif

#include <dune/common/dlist.hh>

namespace Dune {

#define COMMUNICATOR_COMM_TAG 457

  /*!
     ProcListElement describes the link between two processors.
     It contains all needed information for cummunication between
     these two procs.
   */

#ifdef HAVE_MPI
  template <class BufferType>
  class ProcListElement
  {
  public:
    //! Constructor
    ProcListElement(const int owner, const int numFuncs )
      : owner_ (owner) , numFuncs_ (numFuncs)
        , recvCount(0), sendCount(0)
        , sendBuffer_ (0) , recvBuffer_ (0)
        , recive_(100) , send_(100)
    { }

    //! Destructor
    ~ProcListElement()
    {
      if(sendBuffer_) std::free(sendBuffer_);sendBuffer_ = 0;
      if(recvBuffer_) std::free(recvBuffer_);recvBuffer_ = 0;
    }

    //! return my processor number
    int processor () const { return owner_; };

    //! set
    void setSize( int recvSize , int sendSize )
    {
      recvCount = recvSize;
      sendCount = sendSize;
    }

    //! insert new recive element
    void insertRecive( int num )
    {
      if(recvCount >= (int) recive_.size())
      {
        recive_.resize( 2*recvCount );
      }
      recive_[recvCount] = num;
      recvCount++;
    }

    //! insert new send element
    void insertSend( int num )
    {
      if(sendCount >= (int)send_.size())
      {
        send_.resize( 2*sendCount );
      }
      send_[sendCount] = num;
      sendCount++;
    }

    //! write elements to buffer
    template <class DofArrayType>
    void loadSendBuffer (DofArrayType & vec, int pos=0)
    {
      int beg = pos * sendSize();
      for(int i=beg; i<beg+sendSize(); i++)
      {
        sendBuffer_[i] = vec[send_[i-beg]];
      }
    }

    //! read elements from buffer
    template <class DofArrayType>
    void unloadRecvBuffer (DofArrayType & vec, int pos=0)
    {
      int beg = pos * recvSize();
      for(int i=beg; i<beg+recvSize(); i++)
      {
        vec[recive_[i-beg]] = recvBuffer_[i];
      }
    }

    //! allocate memory for buffers
    void makeBuffer()
    {
      recvBuffer_ = (BufferType *) std::malloc( realRecvSize()*sizeof(BufferType) );
      for(int i=0 ; i<realRecvSize(); i++)
      {
        recvBuffer_[i] = -1.0;
      }

      sendBuffer_ = (BufferType *) std::malloc( realSendSize()*sizeof(BufferType) );
    }

    //! size of list of send elements
    int sendSize () const { return sendCount; }

    //! size of send buffer
    int realSendSize () const { return numFuncs_ * sendSize(); }

    //! pointer to send buffer
    BufferType * getSendBuffer() const { return sendBuffer_; }

    //! size of list of recive elements
    int recvSize () const { return recvCount; }

    //! size of recive buffer
    int realRecvSize () const { return numFuncs_ * recvSize(); }

    //! pointer to recive buffer
    BufferType * getRecvBuffer() const { return recvBuffer_; }

    //! print link list
    void print(std::ostream &s) const
    {
      s << "ProcList for Processor " << owner_ << "\n";
      s << "SendList " << sendSize() << "\n";
      for(int i=0; i<sendSize(); i++)
      {
        s << send_[i] << " ";
      }
      s << "\n";
      s << "RecvList " << recvSize() << "\n";
      for(int i=0; i<recvSize(); i++)
      {
        s << recive_[i] << " ";
      }
      s << "\n";
    };

    //! the send and recive MPI_Request
    MPI_Request * sendreq() { return &sendreq_; }
    MPI_Request * recvreq() { return &recvreq_; }

  private:
    const int owner_;
    const int numFuncs_;

    int recvCount;
    int sendCount;

    BufferType * sendBuffer_;
    BufferType * recvBuffer_;

    MPI_Request sendreq_;
    MPI_Request recvreq_;

    // buffer size of processor
    std::vector<int> recive_;
    std::vector<int> send_;
  };

#endif

  /*!
     AlbertGridCommunicator organizes the communication of AlbertGrid on
     diffrent processors.
   */

#ifdef HAVE_MPI
  template <class GridType, class IndexSetType>
  class AlbertGridCommunicator
  {
    typedef DoubleLinkedList< ProcListElement<double> > ProcessorListType;
  public:

    //! Constructor
    AlbertGridCommunicator(GridType &grid, IndexSetType &indexSet, int numFuncs)
      : grid_ (grid) , indexSet_(indexSet), numFuncs_ (numFuncs)
        , myProc_ (grid.myProcessor() )
    {
      initialize();
    }

    //! send and recive DiscreteFunction
    template <class DiscFuncType>
    void sendRecive( DiscFuncType & vec)
    {
      loadSendBuffer(vec);
      communicate();
      unloadRecvBuffer(vec);
    }

    //! send and recive DiscreteFunction
    template <class DiscFuncType>
    void loadSendBuffer( DiscFuncType & vec, int pos=0)
    {
      {
        typedef typename DiscFuncType::DofIteratorType DofIteratorType;
        typedef typename ProcessorListType::Iterator ListIterator;

        ListIterator procit   = otherProcs_.begin();
        ListIterator procend = otherProcs_.end();

        DofIteratorType it = vec.dbegin(-1);

        for( ; procit != procend; ++procit)
        {
          (*procit).loadSendBuffer(it,pos);
        }
      }
    }

    //! send and recive DiscreteFunction
    template <class DiscFuncType>
    void unloadRecvBuffer( DiscFuncType & vec, int pos=0)
    {
      {
        typedef typename DiscFuncType::DofIteratorType DofIteratorType;
        typedef typename ProcessorListType::Iterator ListIterator;
        ListIterator procit   = otherProcs_.begin();
        ListIterator procend = otherProcs_.end();

        DofIteratorType it = vec.dbegin(-1);

        for( ; procit != procend; ++procit)
        {
          (*procit).unloadRecvBuffer(it);
        }
      }
    }

    //! send and recive DiscreteFunction
    template <class DiscFuncList>
    void sendReciveList( DiscFuncList & list)
    {
      assert(numFuncs_ == list.size());
      {
        typedef typename DiscFuncList::Iterator DLIterator;

        DLIterator dl = list.begin();
        DLIterator de = list.end();

        int count = 0;

        for(; dl != de ; ++dl )
        {
          loadSendBuffer(* (*dl) ,count );
          count++;
        }
      }

      communicate();

      {
        typedef typename DiscFuncList::Iterator DLIterator;

        DLIterator dl = list.begin();
        DLIterator de = list.end();

        int count = 0;

        for(; dl != de ; ++dl )
        {
          unloadRecvBuffer(* (*dl) , count);
          count++;
        }
      }
    }

    // minimize timestepsize over all processors
    template <typename T>
    T timeStepSize(T timestep)
    {
      T ret=-1.0;
      MPI_Allreduce(&timestep, &ret, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
      return(ret);
    }

    // minimize timestepsize over all processors
    void waitForAll()
    {
      double timestep = 1.0;
      double ret=-1.0;
      MPI_Allreduce(&timestep, &ret, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
    }

    // send and recive a vector
    template <class DofArrayType>
    void sendReciveVec( DofArrayType & vec)
    {
      {
        typedef typename ProcessorListType::Iterator ListIterator;

        ListIterator procit   = otherProcs_.begin();
        ListIterator procend = otherProcs_.end();

        for( ; procit != procend; ++procit)
        {
          (*procit).loadSendBuffer(vec);
        }
      }

      communicate();

      {
        typedef typename ProcessorListType::Iterator ListIterator;
        ListIterator procit   = otherProcs_.begin();
        ListIterator procend = otherProcs_.end();

        for( ; procit != procend; ++procit)
        {
          (*procit).unloadRecvBuffer(vec);
        }
      }

    }

  private:

    // send and recive data to all neighbouring processors
    void communicate()
    {
      // send data
      {
        typedef typename ProcessorListType::Iterator ListIterator;
        ListIterator procit   = otherProcs_.begin();
        ListIterator procend = otherProcs_.end();

        //int tag = myProc_;
        int no=0;

        for( ; procit != procend; ++procit)
        {
          //std::cout << " Sende Proc " << (*procit).processor() << "\n";
          MPI_Issend( (void *)(*procit).getSendBuffer(),(*procit).sendSize(),
                      MPI_DOUBLE, (*procit).processor(),
                      //tag , AlbertGridComm,
                      COMMUNICATOR_COMM_TAG, MPI_COMM_WORLD ,
                      (*procit).sendreq());
          no++;
        }
      }

      MPI_Status status;
      // recive date
      {
        //int tag = myProc_;
        typedef typename ProcessorListType::Iterator ListIterator;
        ListIterator procit   = otherProcs_.begin();
        ListIterator procend = otherProcs_.end();


        int no=0;
        for( ; procit != procend; ++procit)
        {
          MPI_Irecv( (void *)(*procit).getRecvBuffer(),(*procit).recvSize(),
                     MPI_DOUBLE, (*procit).processor(),
                     COMMUNICATOR_COMM_TAG, MPI_COMM_WORLD ,
                     (*procit).recvreq());
          no++;
        }
      }

      {
        {
          typedef typename ProcessorListType::Iterator ListIterator;
          ListIterator procit  = otherProcs_.begin();
          ListIterator procend = otherProcs_.end();
          int no=0;
          for( ; procit != procend; ++procit)
          {
            MPI_Wait((*procit).sendreq(),&status);
            no++;
          }
        }
      }

      {
        {
          typedef typename ProcessorListType::Iterator ListIterator;
          ListIterator procit  = otherProcs_.begin();
          ListIterator procend = otherProcs_.end();
          int no=0;
          for( ; procit != procend; ++procit)
          {
            MPI_Wait((*procit).recvreq(),&status);
            no++;
          }
        }
      }
    }

    // build address book
    void initialize ()
    {
      typedef typename GridType::template Traits<0>::LevelIterator LevelIteratorType;

      LevelIteratorType it    = grid_.template lbegin<0> (0,Ghosts);
      LevelIteratorType endit = grid_.template lend  <0> (0,Ghosts);

      for( ; it != endit; ++it )
      {
        int owner = it->owner();
        if( owner != myProc_ )
        {
          bool wehaveit = false;
          typedef typename ProcessorListType::Iterator ListIterator;
          ListIterator procit  = otherProcs_.begin();
          ListIterator procend = otherProcs_.end();

          for( ; procit != procend; ++procit)
          {
            if( (*procit).processor() == owner) wehaveit = true;
          }

          if(!wehaveit)
          {
            ProcListElement<double> tmp (owner, numFuncs_ );
            otherProcs_.insert_before(otherProcs_.begin(), tmp);
          }
        }
      }
      // now we have the link list for the processors

      typedef typename GridType::LeafIterator LeafIteratorType;
      {
        typedef typename ProcessorListType::Iterator ListIterator;
        ListIterator procit  = otherProcs_.begin();
        ListIterator procend = otherProcs_.end();

        for( ; procit != procend; ++procit)
        {
          {
            LeafIteratorType it    = grid_.leafbegin ( grid_.maxlevel(), Border, (*procit).processor() );
            LeafIteratorType endit = grid_.leafend   ( grid_.maxlevel(), Border, (*procit).processor() );

            for(; it != endit; ++it)
            {
              (*procit).insertSend( indexSet_.template index<0> (*it, 0 ) );
            }
          }
          {
            LeafIteratorType it    = grid_.leafbegin ( grid_.maxlevel(), Ghosts, (*procit).processor() );
            LeafIteratorType endit = grid_.leafend   ( grid_.maxlevel(), Ghosts, (*procit).processor() );

            for(; it != endit; ++it)
            {
              (*procit).insertRecive( indexSet_.template index<0>(*it, 0 ) );
            }
          }

          // malloc buffer
          (*procit).makeBuffer();
          //(*procit).print(std::cout);
        }
      }
    }

    // reference to corresponding grid
    GridType & grid_;

    // the index set
    IndexSetType & indexSet_;

    int numFuncs_;

    // rank of my thread
    const int myProc_;

    // links to other processors
    ProcessorListType otherProcs_;
  };
#else
  template <class GridType, class IndexSetType>
  class AlbertGridCommunicator
  {
  public:
    AlbertGridCommunicator(GridType &grid, IndexSetType &indexSet, int numFuncs) {}

    template <class DiscFuncType>
    void sendRecive( DiscFuncType & vec)  {}

    //! send and recive DiscreteFunction
    template <class DiscFuncList>
    void sendReciveList( DiscFuncList & list) {}

    template <typename T>
    T timeStepSize(T timestep)
    {
      return timestep;
    }

  };
#endif

  template <class GridType, class CritType>
  void makeParallelGrid (GridType &grid, CritType &crit)
  {
    for(int l=0; l <= grid.maxlevel(); l++)
    {
      typedef typename GridType::template Traits<0>::LevelIterator LevelIteratorType;

      LevelIteratorType it    = grid.template lbegin<0> (l);
      LevelIteratorType endit = grid.template lend  <0> (l);

      for( ; it != endit; ++it )
      {
        crit.classify( *it );
      }
    }
  }


} // end namespace Dune


#endif
