// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef _DUNE_ALBERTGRID_COMMUNICATOR_HH_
#define _DUNE_ALBERTGRID_COMMUNICATOR_HH_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#undef HAVE_MPI

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
    ProcListElement(int owner) : owner_ (owner) ,
                                 recvCount(0), sendCount(0), recive_(100) , send_(100) ,
                                 sendBuffer_ (NULL) , recvBuffer_ (NULL)
    { }

    //! Destructor
    ~ProcListElement()
    {
      if(sendBuffer_) std::free(sendBuffer_);sendBuffer_ = NULL;
      if(recvBuffer_) std::free(recvBuffer_);recvBuffer_ = NULL;
    }

    //! return my processor number
    int processor () const { return owner_; };

    //! set
    void setSize( int recvSize , int sendSize )
    {
      recvSize_ = recvSize;
      sendSize_ = sendSize;
    }

    //! insert new recive element
    void insertRecive( int num )
    {
      if(recvCount >= recive_.size())
      {
        recive_.resize( 2*recvCount );
      }
      recive_[recvCount] = num;
      recvCount++;
    }

    //! insert new send element
    void insertSend( int num )
    {
      if(sendCount >= send_.size())
      {
        send_.resize( 2*sendCount );
      }
      send_[sendCount] = num;
      sendCount++;
    }

    //! write elements to buffer
    template <class DofArrayType>
    void loadSendBuffer (DofArrayType & vec)
    {
      for(int i=0 ; i<sendSize(); i++)
      {
        sendBuffer_[i] = vec[send_[i]];
      }
    }

    //! read elements from buffer
    template <class DofArrayType>
    void unloadRecvBuffer (DofArrayType & vec)
    {
      for(int i=0 ; i<recvSize(); i++)
      {
        vec[recive_[i]] = recvBuffer_[i];
      }
    }

    //! allocate memory for buffers
    void makeBuffer()
    {
      recvBuffer_ = (BufferType *) std::malloc( recvSize()*sizeof(BufferType) );
      for(int i=0 ; i<recvSize(); i++)
      {
        recvBuffer_[i] = -1.0;
      }

      sendBuffer_ = (BufferType *) std::malloc( sendSize()*sizeof(BufferType) );
    }

    //! size of send buffer
    int sendSize () const { return sendCount; }

    //! pointer to send buffer
    BufferType * getSendBuffer() const { return sendBuffer_; }

    //! size of recive buffer
    int recvSize () const { return recvCount; }

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
    MPI_Request * sendreq() const { return &sendreq_; }
    MPI_Request * recvreq() const { return &recvreq_; }

  private:
    const int owner_;

    int recvCount;
    int sendCount;

    BufferType * recvBuffer_;
    BufferType * sendBuffer_;

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
  template <class GridType>
  class AlbertGridCommunicator
  {
    typedef typename DoubleLinkedList<ProcListElement<double> > ProcessorListType;
  public:

    //! Constructor
    AlbertGridCommunicator(GridType &grid)
      : grid_ (grid) , myProc_ (grid.myProcessor() )
    {
      initialize();
    }

    //! send and recive DiscreteFunction
    template <class DiscFuncType>
    void sendRecive( DiscFuncType & vec)
    {
      {
        typedef typename DiscFuncType::DofIteratorType DofIteratorType;
        typedef typename ProcessorListType::Iterator ListIterator;

        ListIterator procit   = otherProcs_.begin();
        ListIterator procend = otherProcs_.end();

        DofIteratorType it = vec.dbegin(-1);

        for( ; procit != procend; ++procit)
        {
          (*procit).loadSendBuffer(it);
        }
      }

      communicate();

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

    // minimize timestepsize over all processors
    template <typename T>
    T timeStepSize(T timestep)
    {
      T ret=-1.0;
      MPI_Allreduce(&timestep, &ret, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
      return(ret);
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

        int tag = myProc_;
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
        int tag = myProc_;
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
      typedef typename GridType::Traits<0>::LevelIterator LevelIteratorType;

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
            otherProcs_.insert_before(
              otherProcs_.begin(),ProcListElement<double>(owner));
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
              (*procit).insertSend( it->global_index() );
            }
          }
          {
            LeafIteratorType it    = grid_.leafbegin ( grid_.maxlevel(), Ghosts, (*procit).processor() );
            LeafIteratorType endit = grid_.leafend   ( grid_.maxlevel(), Ghosts, (*procit).processor() );

            for(; it != endit; ++it)
            {
              (*procit).insertRecive( it->global_index() );
            }
          }

          // malloc buffer
          (*procit).makeBuffer();
          //(*procit).print(std::cout);
        }
      }
    }

    // rank of my thread
    const int myProc_;

    // links to other processors
    ProcessorListType otherProcs_;

    // reference to corresponding grid
    GridType & grid_;
  };
#else
  template <class GridType>
  class AlbertGridCommunicator
  {
  public:
    template <class DiscFuncType>
    void sendRecive( DiscFuncType & vec)  {}

    template <typename T>
    T timeStepSize(T timestep)
    {
      return timestep;
    }

  };
#endif

} // end namespace Dune


#endif
