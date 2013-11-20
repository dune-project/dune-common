// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_PARALLEL_VARIABLESIZECOMMUNICATOR_HH // Still fits the line!
#define DUNE_COMMON_PARALLEL_VARIABLESIZECOMMUNICATOR_HH

#if HAVE_MPI
// MPI header
#include <mpi.h>
#include <vector>
#include <map>
#include "interface.hh"
#include "mpitraits.hh"

namespace Dune
{

namespace
{
/**
 * @brief A message buffer.
 * @tparam T The type of data that the buffer will hold.
 */
template<class T, class Allocator=std::allocator<T> >
class MessageBuffer
{
public:
  /**
   * @brief Constructs a message.
   * @param size The number of elements that buffer should hold,
   */
  explicit MessageBuffer(int size)
    : buffer_(new T[size]), size_(size), position_(0)
  {}
  /**
   * @brief Copy constructor.
   * @param o The instance to copy.
   */
  explicit MessageBuffer(const MessageBuffer& o)
  : buffer_(new T[o.size_]), size_(o.size_), position_(o.position_)
  {
  }
  /** @brief Destructor. */
  ~MessageBuffer()
  {
    delete[] buffer_;
  }
  /**
   * @brief Write an item to the buffer.
   * @param data The data item to write.
   */
  void write(const T& data)
  {
    buffer_[position_++]=data;
  }

  /**
   * @brief Reads a data item from the buffer
   * @param[out] data Reference to where to store the read data.
   */
  void read(T& data)
  {
    data=buffer_[position_++];
  }

  /**
   * @brief Reset the buffer.
   *
   * On return the buffer will be positioned at the start again.
   */
  void reset()
  {
    position_=0;
  }

  /**
   * @brief Test whether the whole buffer was read.
   * @return True if we read or wrot until the end of the buffer.
   */
  bool finished()
  {
    return position_==size_;
  }

  /**
   * @brief Tests whether the buffer has enough space left to read/write data.
   * @param notItems The number of items to read or write.
   * @return True if there is enough space for noItems items.
   */
  bool hasSpaceForItems(int noItems)
  {
    return position_+noItems<=size_;
  }
  /**
   * @brief Get the size of the buffer.
   * @return The number of elements the buffer can hold.
   */
  std::size_t size()
  {
    return size_;
  }
  /**
   * @brief Converts the buffer to a C array.
   * @return The underlying C array.
   */
  operator T*()
  {
    return buffer_;
  }

private:
  /**
   * @brief Pointer to the current insertion point of the buffer.
   */
  T* buffer_;
  /**
   * @brief The size of the buffer
   */
  std::size_t size_;
  /**
   * @brief The current position in the buffer.
   */
  std::size_t position_;
};

/**
 * @brief A tracker for the current position in a communication interface.
 */
class InterfaceTracker
{
public:
  /**
   * @brief Constructor.
   * @param rank The other rank that the interface communicates with.
   * @param info A list of local indices belonging to this interface.
   */
  InterfaceTracker(int rank, InterfaceInformation info)
    : fixedSize(-1), rank_(rank), index_(), interface_(info)
  {}

  /**
   * @brief Moves to the next index in the interface.
   */
  void moveToNextIndex()
  {
    index_++;
  }

  /**
   * @brief Checks whether all indices have been visited.
   * @return True if all indices have been visited.
   */
  bool finished() const
  {
    return index_==interface_.size();
  }

  /**
   * @brief Get the current local index of the interface.
   * @return The current local index of the interface.
   */
  std::size_t index() const
  {
    return interface_[index_];
  }
  /**
   * @brief Checks whether there are still indices waiting to be processed.
   * @return True if there are still indices waiting to be processed.
   */
  std::size_t indicesLeft() const
  {
    return interface_.size()-index_;
  }
  /**
   * @brief The number of data items per index if it is fixed, 0 otherwise.
   */
  std::size_t fixedSize;
  /**
   * @brief Get the process rank that this communication interface is with.
   */
  int rank() const
  {
    return rank_;
  }
private:
  /** @brief The process rank that this communication interface is with. */
  int rank_;
  /** @brief The other rank that this interface communcates with. */
  std::size_t index_;
  /** @brief The list of local indices of this interface. */
  InterfaceInformation interface_;
};


}

/**
 * @brief A buffered communicator where the amount of data sent does not have to be known a priori.
 *
 * In contrast to BufferedCommunicator the amount of data is determined by the container
 * whose entries are sent and not known at the receiving side a priori.
 */
template<class Allocator=std::allocator<std::pair<InterfaceInformation,InterfaceInformation> > >
class VariableSizeCommunicator
{
public:
  /**
     * @brief The type of the map form process number to InterfaceInformation for
     * sending and receiving to and from it.
     */
  typedef std::map<int,std::pair<InterfaceInformation,InterfaceInformation>,
                   std::less<int>,
                   typename Allocator::template rebind<std::pair<const int,std::pair<InterfaceInformation,InterfaceInformation> > >::other> InterfaceMap;

#ifndef DUNE_PARALLEL_MAX_COMMUNICATION_BUFFER_SIZE
  /**
   * @brief Creates a communicator with the default maximum buffer size.
   *
   * The default size ist either what the macro DUNE_MAX_COMMUNICATION_BUFFER_SIZE
   * is set to or 32768 if is not set.
   */
  VariableSizeCommunicator(MPI_Comm comm, const InterfaceMap& inf)
    : maxBufferSize_(32768), interface_(&inf)
  {
    MPI_Comm_dup(comm, &communicator_);
  }
  /**
   * @brief Creates a communicator with the default maximum buffer size.
   * @param interface The communication interface.
   */
  VariableSizeCommunicator(const Interface& inf)
  : maxBufferSize_(32768), interface_(&inf.interfaces())
  {
    MPI_Comm_dup(inf.communicator(), &communicator_);
  }
#else
  /**
   * @brief Creates a communicator with the default maximum buffer size.
   *
   * The default size ist either what the macro DUNE_MAX_COMMUNICATION_BUFFER_SIZE
   * is set to or 32768 if is not set.
   */
  VariableSizeCommunicator(MPI_Comm comm, InterfaceMap& inf)
    : maxBufferSize_(DUNE_PARALLEL_MAX_COMMUNICATION_BUFFER_SIZE),
      interface_(&inf)
  {
    MPI_Comm_dup(comm, &communicator_);
  }
  /**
   * @brief Creates a communicator with the default maximum buffer size.
   * @param interface The communication interface.
   */
  VariableSizeCommunicator(const Interface& inf)
  : maxBufferSize_(DUNE_PARALLEL_MAX_COMMUNICATION_BUFFER_SIZE),
    interface_(&inf.interfaces())
  {
    MPI_Comm_dup(inf.communicator(), &communicator_);
  }
#endif
  /**
  * @brief Creates a communicator with a specific maximum buffer size.
  * @param max_buffer_size The maximum buffer size allowed.
  */
  VariableSizeCommunicator(MPI_Comm comm, const InterfaceMap& inf, std::size_t max_buffer_size)
    : maxBufferSize_(max_buffer_size), interface_(&inf)
  {
    MPI_Comm_dup(comm, &communicator_);
  }

  /**
  * @brief Creates a communicator with a specific maximum buffer size.
  * @param max_buffer_size The maximum buffer size allowed.
  */
  VariableSizeCommunicator(const Interface& inf, std::size_t max_buffer_size)
    : maxBufferSize_(max_buffer_size), interface_(&inf.interfaces())
  {
    MPI_Comm_dup(inf.communicator(), &communicator_);
  }

  ~VariableSizeCommunicator()
  {
    MPI_Comm_free(&communicator_);
  }


  /**
   * @brief Communicate forward.
   *
   * @tparam DataHandle The type of the handle describing the data. This type has to adhere
   * to the following interface:
   * \code{.cpp}
   * // returns whether the number of data items per entry is fixed
   * bool fixedsize();
   * // get the number of data items for an entry with index i
   * std::size_t size(std::size_t i);
   * // gather the data at index i
   * void gather(MessageBuffer buf, std::size_t  i);
   * // scatter the n data items to index i
   * void scatter(MessageBuffer& buf, std::size_t i, std::size_t n);
   * \endcode
   * @param handle A handle responsible for describing the data, gathering, and scattering it.
   */
  template<class DataHandle>
  void forward(DataHandle& handle)
  {
    communicate<true>(handle);
  }

  /**
   * @brief Communicate backwards.
   *
   * @tparam DataHandle The type of the handle describing the data. This type has to adhere
   * to the following interface:
   * \code{.cpp}
   * // returns whether the number of data items per entry is fixed
   * bool fixedsize();
   * // get the number of data items for an entry with index i
   * std::size_t size(std::size_t i);
   * // gather the data at index i
   * void gather(MessageBuffer buf, std::size_t  i);
   * // scatter the n data items to index i
   * void scatter(MessageBuffer& buf, std::size_t i, std::size_t n);
   * \endcode
   * @param handle A handle responsible for describing the data, gathering, and scattering it.
   */
  template<class DataHandle>
  void backward(DataHandle& handle)
  {
    communicate<false>(handle);
  }

private:
  /**
   * @brief Communicates data according to the interface.
   * @tparam forward If true sends data forwards, otherwise backwards along the interface.
   * @tparame DataHandle The type of the data handle @see forward for a description of the interface.
   * @param handle The handle describing the data and responsible for gather and scatter operations.
   */
  template<bool forward,class DataHandle>
  void communicate(DataHandle& handle);
  /**
   * @brief Sets up all the send requests for the data.
   * @tparam DataHandle The type of the data handle.
   * @param handle The data handle describing the data.
   * @param trackers The trackers for the communication interfaces.
   * @param buffers The buffers for the comunication. One for each neighbour.
   * @param requests The send requests for each neigbour.
   */
  template<class DataHandle>
  void setupAllSendRequests(DataHandle& handle,
                            std::vector<InterfaceTracker>& trackers,
                            std::vector<MessageBuffer<typename DataHandle::DataType> >& buffers,
                            std::vector<MPI_Request>& requests);
  /**
   * @brief Initialize the the trackers along the interface for the communication.
   * @tparam FORWARD If true we send in the forward direction.
   * @tparam DataHandle DataHandle The type of the data handle.
   * @param handle The handle describing the data and responsible for gather
   * and scatter operations.
   * @param[out] send_trackers The trackers for the sending side.
   * @param[out] recv_trackers The trackers for the receiving side.
   */
  template<bool FORWARD, class DataHandle>
  void setupInterfaceTrackers(DataHandle& handle,
                              std::vector<InterfaceTracker>& send_trackers,
                              std::vector<InterfaceTracker>& recv_trackers);
  /**
   * @brief Communicate data with a fixed amount of data per entry.
   * @tparam FORWARD If true we send in the forward direction.
   * @tparam DataHandle DataHandle The type of the data handle.
   * @param handle The handle describing the data and responsible for gather
   * and scatter operations.
   */
  template<bool FORWARD, class DataHandle>
  void communicateFixedSize(DataHandle& handle);
  /**
   * @brief The maximum size if the buffers used for gather and scatter.
   *
   * @note If this process has n neighbours, then a maximum of 2n buffers of this size
   * is allocate. Memory needed will be n*sizeof(std::size_t)+n*sizeof(Datahandle::DataType)
   */
  std::size_t maxBufferSize_;
  /**
   * @brief description of the interface.
   *
   * This is a map of the neighboring process number to a pair of local index lists.
   * The first is a list of indices to gather data for sending from and the second is a list of
   * indices to scatter received data to during forward.
   */
  const InterfaceMap* interface_;
  /**
   * @brief The communicator.
   *
   * This is a cloned communicator to ensure there are no interferences.
   */
  MPI_Comm communicator_;
};

namespace
{
/**
 * @brief Template meta program for choosing then send or receive interface
 * information based on the direction.
 * @tparam FORWARD If true the communication happens in the forward direction.
 */
template<bool FORWARD>
struct InterfaceInformationChooser
{
  /**
   * @brief Get the interface information for the sending side.
   */
  static const InterfaceInformation&
  getSend(const std::pair<InterfaceInformation,InterfaceInformation>& info)
  {
    return info.first;
  }

  /**
   * @brief Get the interface information for the receiving side.
   */
  static const InterfaceInformation&
  getReceive(const std::pair<InterfaceInformation,InterfaceInformation>& info)
  {
    return info.second;
  }
};

template<>
struct InterfaceInformationChooser<false>
{
  static const InterfaceInformation&
  getSend(const std::pair<InterfaceInformation,InterfaceInformation>& info)
  {
    return info.second;
  }

  static const InterfaceInformation&
  getReceive(const std::pair<InterfaceInformation,InterfaceInformation>& info)
  {
    return info.first;
  }
};

/**
 * @brief A functor that packs entries into the message buffer.
 * @tparam DataHandle The type of the data handle that describes
 * the communicated data.
 */
template<class DataHandle>
struct PackEntries
{
  /**
   * @brief packs data.
   * @param handle The handle describing the data and the gather and scatter operations.
   * @param tracker The tracker of the interface to tell us where we are.
   * @param buffer The buffer to use for packing.
   * @return The number data entries that we packed.
   */
  int operator()(DataHandle& handle, InterfaceTracker& tracker,
                  MessageBuffer<typename DataHandle::DataType>& buffer)
  {
    if(tracker.fixedSize) // fixed size if variable is >0!
    {

      std::size_t noIndices=std::min(buffer.size()/tracker.fixedSize, tracker.indicesLeft());
      for(std::size_t i=0; i< noIndices; ++i)
      {
        handle.gather(buffer, tracker.index());
        tracker.moveToNextIndex();
      }
      return noIndices*tracker.fixedSize;
    }
    else
    {
      DUNE_THROW(NotImplemented, "Currently only fixed size communication is supported.");
      /*
        if(tracker.finished())
        return true;
        while(true)
        {
        if(buffer.hasSpaceForItems(handle.size(tracker.index())))
        {
        handle.gather(buffer, tracker.index());
        tracker.moveToNextIndex();
        if(tracker.finished())
        return true;
        }
        else
        return tracker.finished();
        }
      */
    }
  }
};

/**
 * @brief A functor that unpacks entries from the message buffer.
 * @tparam DataHandle The type of the data handle that describes
 * the communicated data.
 */
template<class DataHandle>
struct UnpackEntries{

  /**
   * @brief packs data.
   * @param handle The handle describing the data and the gather and scatter operations.
   * @param tracker The tracker of the interface to tell us where we are.
   * @param buffer The buffer to use for packing.
   * @return The number data entries that we packed.
   */
  bool operator()(DataHandle& handle, InterfaceTracker& tracker,
                  MessageBuffer<typename DataHandle::DataType>& buffer)
  {
    if(tracker.fixedSize) // fixed size if variable is >0!
    {
      std::size_t noIndices=std::min(buffer.size()/tracker.fixedSize, tracker.indicesLeft());

      for(std::size_t i=0; i< noIndices; ++i)
      {
        handle.scatter(buffer, tracker.index(), tracker.fixedSize);
        tracker.moveToNextIndex();
      }
      return tracker.finished();
    }
    else
    {
      DUNE_THROW(NotImplemented, "Currently only fixed size communication is supported.");
      /*    if(tracker.finished())
            return true;
            while(true)
            {
            if(buffer.hasSpaceForItems(tracker.size())))
            {
            handle.scatter(buffer, tracker.index(), tracker.size());
            tracker.moveToNextIndex();
            if(tracker.finished())
            return true;
            }
            else
            return tracker.finished();
            }
      */
    }
  }
};

/**
 * @brief Sends the size in case of communicating a fixed amount of data per entry.
 * @param[in] send_trackers The trackers for the sending side.
 * @param[out] send_requests The request for the asynchronous send operations.
 * @param[in] recv_trackers The trackers for the receiving side.
 * @param[out] recv_requests The request for the asynchronous receive operations.
 */
void sendFixedSize(std::vector<InterfaceTracker>& send_trackers,
                   std::vector<MPI_Request>& send_requests,
                   std::vector<InterfaceTracker>& recv_trackers,
                   std::vector<MPI_Request>& recv_requests,
                   MPI_Comm communicator)
{
  typedef std::vector<InterfaceTracker>::iterator TIter;
  std::vector<MPI_Request>::iterator mIter=recv_requests.begin();

  for(TIter iter=recv_trackers.begin(), end=recv_trackers.end(); iter!=end;
      ++iter, ++mIter)
  {
    MPI_Irecv(&(iter->fixedSize), 1, MPITraits<std::size_t>::getType(),
              iter->rank(), 933881, communicator, &(*mIter));
  }

  // Send our size to all neighbours using non-blocking synchronous communication.
  std::vector<MPI_Request>::iterator mIter1=send_requests.begin();
  for(TIter iter=send_trackers.begin(), end=send_trackers.end();
      iter!=end;
      ++iter, ++mIter1)
  {
    MPI_Issend(&(iter->fixedSize), 1, MPITraits<std::size_t>::getType(),
               iter->rank(), 933881, communicator, &(*mIter1));
  }
}


/**
 * @brief Functor for setting up send requests.
 * @tparam DataHandle The type of the data handle for describing the data.
 */
template<class DataHandle>
struct SetupSendRequest{
  void operator()(DataHandle& handle,
                  InterfaceTracker& tracker,
                  MessageBuffer<typename DataHandle::DataType>& buffer,
                  MPI_Request& request,
                  MPI_Comm comm)
  {
    buffer.reset();
    int size=PackEntries<DataHandle>()(handle, tracker, buffer);
    MPI_Issend(buffer, size, MPITraits<typename DataHandle::DataType>::getType(),
               tracker.rank(), 933399, comm, &request);
  }
};

/**
 * @brief Functor for setting up receive requests.
 * @tparam DataHandle The type of the data handle for describing the data.
 */
template<class DataHandle>
struct SetupRecvRequest{
  void operator()(DataHandle& /*handle*/,
                  InterfaceTracker& tracker,
                  MessageBuffer<typename DataHandle::DataType>& buffer,
                  MPI_Request& request,
                  MPI_Comm comm)
  {
    int size=std::min(buffer.size(), tracker.indicesLeft()*tracker.fixedSize);
    buffer.reset();
    MPI_Irecv(buffer, size, MPITraits<typename DataHandle::DataType>::getType(),
              tracker.rank(), 933399, comm, &request);
  }
};

/**
 * @brief A functor that does nothing.
 */
template<class DataHandle>
struct NullPackUnpackFunctor
{
  int operator()(DataHandle&, InterfaceTracker&,
                  MessageBuffer<typename DataHandle::DataType>&)
  {
    return 0;
  }
};

/**
 * @brief Check whether some of the requests finished and continue send/receive operation.
 * @tparam DataHandle The type of the data handle describing the data.
 * @tparam BufferFunctor A functor that packs or unpacks data from the buffer.
 * E.g. NullPackUnpackFunctor.
 * @tparam CommunicationFuntor A functor responsible for continuing the communication.
 * @param handle The data handle describing the data.
 * @param trackers The trackers indicating the current position in the communication.
 * @param requests The requests to test whether they finished.
 * @param requests2 The requests to use for setting up the continuing communication. Might
 * be the same as requests.
 * @param comm The MPI communicator to use.
 * @param buffer_func The functor that does the packing or unpacking of the data.
 */
template<class DataHandle, class BufferFunctor, class CommunicationFunctor>
std::size_t checkAndContinue(DataHandle& handle,
                             std::vector<InterfaceTracker>& trackers,
                             std::vector<MPI_Request>& requests,
                             std::vector<MPI_Request>& requests2,
                             std::vector<MessageBuffer<typename DataHandle::DataType> >& buffers,
                             MPI_Comm comm,
                             BufferFunctor buffer_func,
                             CommunicationFunctor comm_func,
                             bool valid=true)
{
  std::size_t size=requests.size();
  //std::vector<MPI_Status> statuses(size, MPI_STATUS_IGNORE);
  int no_completed;
  std::vector<int> indices(size, -1);

  MPI_Testsome(size, &(requests[0]),
               &no_completed, &(indices[0]), MPI_STATUSES_IGNORE);
  indices.resize(no_completed);
  for(std::vector<int>::iterator index=indices.begin(), end=indices.end();
      index!=end; ++index)
  {
    InterfaceTracker& tracker=trackers[*index];
    // Communication completed, we can reuse the buffers
    buffer_func(handle, tracker, buffers[*index]);
    if(!trackers[*index].finished()){
      comm_func(handle, tracker, buffers[*index], requests2[*index], comm);
      if(valid)
      --size; // communication not finished, decrement counter for finished ones.
    }
  }
  return no_completed;

}

/**
 * @brief Receive the size per data entry and set up requests for receiving the data.
 * @tparam DataHandle The type of the data handle.
 * @param trackers The trackers indicating the indices where we send and from which rank.
 * @param size_requests The requests for receiving the size.
 * @param data_requests The requests for sending the data.
 * @param buffers The buffers to use for sending.
 * @param comm The mpi communicator to use.
 */
template<class DataHandle>
std::size_t receiveSizeAndSetupReceive(DataHandle& handle,
                                       std::vector<InterfaceTracker>& trackers,
                                       std::vector<MPI_Request>& size_requests,
                                       std::vector<MPI_Request>& data_requests,
                                       std::vector<MessageBuffer<typename DataHandle::DataType> >& buffers,
                                       MPI_Comm comm)
{
  return checkAndContinue(handle, trackers, size_requests, data_requests, buffers, comm,
                   NullPackUnpackFunctor<DataHandle>(), SetupRecvRequest<DataHandle>(), false);
}

/**
 * @brief Check whether send request completed and continue sending if necessary.
 * @tparam DataHandle The type of the data handle.
 * @param trackers The trackers indicating the indices where we send and from which rank.
 * @param requests The requests for the asynchronous communication.
 * @param buffers The buffers to use for sending.
 * @param comm The mpi communicator to use.
 */
template<class DataHandle>
std::size_t checkSendAndContinueSending(DataHandle& handle,
                                        std::vector<InterfaceTracker>& trackers,
                                        std::vector<MPI_Request>& requests,
                                        std::vector<MessageBuffer<typename DataHandle::DataType> >& buffers,
                                        MPI_Comm comm)
{
  return checkAndContinue(handle, trackers, requests, requests, buffers, comm,
                          NullPackUnpackFunctor<DataHandle>(), SetupSendRequest<DataHandle>());
}

/**
 * @brief Check whether receive request completed and continue receiving if necessary.
 * @tparam DataHandle The type of the data handle.
 * @param trackers The trackers indicating the indices where we receive and from which rank.
 * @param requests The requests for the asynchronous communication.
 * @param buffers The buffers to use for receiving.
 * @param comm The mpi communicator to use.
 */
template<class DataHandle>
std::size_t checkReceiveAndContinueReceiving(DataHandle& handle,
                                             std::vector<InterfaceTracker>& trackers,
                                             std::vector<MPI_Request>& requests,
                                             std::vector<MessageBuffer<typename DataHandle::DataType> >& buffers,
                                             MPI_Comm comm)
{
  return checkAndContinue(handle, trackers, requests, requests, buffers, comm,
                          UnpackEntries<DataHandle>(), SetupRecvRequest<DataHandle>());
}


bool validRecvRequests(const std::vector<MPI_Request> reqs)
{
  for(std::vector<MPI_Request>::const_iterator i=reqs.begin(), end=reqs.end();
      i!=end; ++i)
    if(*i!=MPI_REQUEST_NULL)
      return true;
  return false;
}


} // end unnamed namespace

template<class Allocator>
template<class DataHandle>
void VariableSizeCommunicator<Allocator>::setupAllSendRequests(DataHandle& handle,
                                                  std::vector<InterfaceTracker>& trackers,
                                                  std::vector<MessageBuffer<typename DataHandle::DataType> >& buffers,
                                                  std::vector<MPI_Request>& requests)
{
  typedef typename std::vector<InterfaceTracker>::iterator TIter;
  typename std::vector<MessageBuffer<typename DataHandle::DataType> >::iterator
    biter=buffers.begin();
  typename std::vector<MPI_Request>::iterator riter=requests.begin();
  for(TIter titer=trackers.begin(), end=trackers.end(); titer!=end; ++titer, ++biter, ++riter)
    SetupSendRequest<DataHandle>()(handle, *titer, *biter, *riter, communicator_);
}
/*
template<bool FORWARD, class DataHandle>
void VariableSizeCommunicator::setupSendRequests(DataHandle& handle,
                                                 std::vector<std::vector<char> >& buffers,
                                                 std::vector<InterfaceTracker>& trackers,
                                                 std::vector<MPI_Request>& requests)
{
  typedef typename InterfaceInformation::const_iterator Iter;
  InterfaceInformationChooser<FORWARD> chooser;
  int i=0;
  for(Iter iter=interface_->begin(), end=interface_->end(); i!=end; ++i, ++iter)
  {
    trackers[i]=InterfaceTracker(iter->first,chooser.getSend(iter->second));
    MPIPackMessageBuffer buffer=MessageBuffer(&(*buffers[i].begin()), buffers[i].size());

    // First item send is the fixed number of items per index or zero if it varies.
    int indicesPerBuffer=0;
    if(handle.fixedsize())
    {
      int size=0, tsize;
      MPI_Pack_size(1, MPITraits<std::size_t>::datatype, communicator_, &tsize);
      size+=tsize;
      while(true)
      {
        MPI_Pack_size(1, MPITraits<typename DataHandle::DataType>::datatype, communicator_,
                      &tsize);
        size+=tsize;
        if(size<=maxBufferSize)
          ++indicesPerBuffer;
        else
          break;
      }
      buffer.write(data.size(trackers[i].index()));
    }
    else
      buffer.write((std::size_t)0);
    packEntries(handle, trackers[i], buffer, indicesPerBuffer);
  }
}
*/

template<class Allocator>
template<bool FORWARD,class DataHandle>
void VariableSizeCommunicator<Allocator>::setupInterfaceTrackers(DataHandle& handle,
                                                                   std::vector<InterfaceTracker>& send_trackers,
                                                                 std::vector<InterfaceTracker>& recv_trackers)
{
  send_trackers.reserve(interface_->size());
  recv_trackers.reserve(interface_->size());

  typedef typename InterfaceMap::const_iterator IIter;
  for(IIter inf=interface_->begin(), end=interface_->end(); inf!=end; ++inf)
  {
    send_trackers.push_back(InterfaceTracker(inf->first,
                                             InterfaceInformationChooser<FORWARD>::getSend(inf->second)));
    if(handle.fixedsize())
      send_trackers.back().fixedSize=handle.size(send_trackers.back().index());
    else
      send_trackers.back().fixedSize=0;
    recv_trackers.push_back(InterfaceTracker(inf->first,
                                             InterfaceInformationChooser<FORWARD>::getReceive(inf->second)));
  }
}

template<class Allocator>
template<bool FORWARD, class DataHandle>
void VariableSizeCommunicator<Allocator>::communicateFixedSize(DataHandle& handle)
{
  std::vector<MPI_Request> size_send_req(interface_->size());
  std::vector<MPI_Request> size_recv_req(interface_->size());

  std::vector<InterfaceTracker> send_trackers;
  std::vector<InterfaceTracker> recv_trackers;
  setupInterfaceTrackers<FORWARD>(handle,send_trackers, recv_trackers);
  sendFixedSize(send_trackers,  size_send_req, recv_trackers, size_recv_req, communicator_);

  std::vector<MPI_Request> data_send_req(interface_->size(), MPI_REQUEST_NULL);
  std::vector<MPI_Request> data_recv_req(interface_->size(), MPI_REQUEST_NULL);
  typedef typename DataHandle::DataType DataType;
  std::vector<MessageBuffer<DataType> > send_buffers(interface_->size(), MessageBuffer<DataType>(maxBufferSize_)),
    recv_buffers(interface_->size(), MessageBuffer<DataType>(maxBufferSize_));


  setupAllSendRequests(handle, send_trackers, send_buffers, data_send_req);

  std::size_t no_size_to_recv,  no_to_send, no_to_recv, old_size;
  no_size_to_recv = no_to_send = no_to_recv = old_size = interface_->size();

  while(no_size_to_recv+no_to_send+no_to_recv)
  {
    // Receive the fixedsize and setup receives accordingly
    if(no_size_to_recv)
      no_size_to_recv -= receiveSizeAndSetupReceive(handle,recv_trackers, size_recv_req,
                                                  data_recv_req, recv_buffers,
                                                  communicator_);

    // Check send completion and initiate other necessary sends
    if(no_to_send)
      no_to_send -= checkSendAndContinueSending(handle, send_trackers, data_send_req,
                                              send_buffers, communicator_);
    if(validRecvRequests(data_recv_req))
      // Receive data and setup new unblocking receives if necessary
      no_to_recv -= checkReceiveAndContinueReceiving(handle, recv_trackers, data_recv_req,
                                                     recv_buffers, communicator_);
  }

  // Wait for completion of sending the size.
  //std::vector<MPI_Status> statuses(interface_->size(), MPI_STATUSES_IGNORE);
  MPI_Waitall(size_send_req.size(), &(size_send_req[0]), MPI_STATUSES_IGNORE);

}

template<class Allocator>
template<bool FORWARD, class DataHandle>
void VariableSizeCommunicator<Allocator>::communicate(DataHandle& handle)
{
  if(handle.fixedsize())
    communicateFixedSize<FORWARD>(handle);
  else
    DUNE_THROW(NotImplemented, "Currently only communication of a fixed amount of data per"
               <<" index is supported!");

}
} // end namespace Dune
#endif
#endif
