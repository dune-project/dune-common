// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_COMMUNICATOR
#define DUNE_COMMUNICATOR

#include "remoteindices.hh"
#include "interface.hh"
#include <dune/common/exceptions.hh>
#include <dune/common/typetraits.hh>
namespace Dune
{
  /** @defgroup ISTL_Comm ISTL Communication
   * @ingroup ISTL
   * @brief Provides classes for syncing distributed indexed
   * data structures.
   */
  /** @addtogroup ISTL_Comm
   *
   * @{
   */
  /**
   * @file
   * @brief Provides utility classes for syncing distributed data via
   * MPI communication.
   * @author Markus Blatt
   */

  /**
   * @brief Flag for marking indexed data structures where data at
   * each index is of the same size.
   */
  struct SizeOne
  {};

  /**
   * @brief Flag for marking indexed data structures where the data at each index may
   * be a variable multiple of another type.
   */
  struct VariableSize
  {};


  /**
   * @brief Default policy used for communicating an indexed type.
   *
   * This
   */
  template<class V>
  struct CommPolicy
  {
    /**
     * @brief The type the policy is for.
     *
     * It has to provide the mode
     * <pre> Type::IndexedType operator[](int i);</pre>
     * for
     * the access of the value at index i and a typedef IndexedType.
     * It is assumed
     * that only one entry is at each index (as in scalar
     * vector.
     */
    typedef V Type;

    /**
     * @brief The type we get at each index with operator[].
     */
    typedef typename V::IndexedType IndexedType;

    /**
     * @brief Whether the indexed type has variable size or there
     * is always one value at each index.
     */
    typedef SizeOne IndexedTypeFlag;

    /**
     * @brief Get the address of entry at an index.
     *
     * The default implementation uses operator[] to
     * get the address.
     * @param v An existing representation of the type that has more elements than index.
     * @param index The index of the entry.
     */
    static const void* getAddress(const V& v, int index);

    /**
     * @brief Get the number of primitve elements at that index.
     *
     * The default always returns 1.
     */
    static int getSize(const V&, int index);
  };


  /**
   * @brief Error thrown if there was a problem with the communication.
   */
  class CommunicationError : public IOError
  {};

  /**
   * @brief An utility class for communicating distributed data structures.
   */
  template<typename TG, typename TA, int N=100>
  class DatatypeCommunicator : public InterfaceBuilder<TG,TA,N>
  {
  public:
    /**
     * @brief The type of the global index.
     */
    typedef TG GlobalIndexType;

    /**
     * @brief The type of the attribute.
     */
    typedef TA AttributeType;

    /**
     * @brief The type of the local index.
     */
    typedef ParallelLocalIndex<AttributeType> LocalIndexType;

    /**
     * @brief Type of the index set we use.
     */
    typedef IndexSet<GlobalIndexType,LocalIndexType,N> IndexSetType;

    /**
     * @brief Type of the underlying remote indices class.
     */
    typedef RemoteIndices<GlobalIndexType, AttributeType,N> RemoteIndices;


    /**
     * @brief The type of the global index.
     */
    typedef TG GlobalIndex;

    /**
     * @brief The type of the attribute of the corresponding index set.
     */
    typedef TA Attribute;

    /**
     * @brief Creates a new DatatypeCommunicator.
     * @param remote The remote index set to use.
     */
    DatatypeCommunicator();

    /**
     * @brief Destructor.
     */
    ~DatatypeCommunicator();

    /**
     * @brief Builds the interface between the index sets.
     *
     * Has to be called before the actual communication by forward or backward
     * can be called. Nonpublic indices will be ignored!
     *
     *
     * The types T1 and T2 are classes representing a set of
     * enumeration values of type DatatypeCommunicator::AttributeType.
     * They have to provide
     * a (static) method
     * <pre>
     * bool contains(AttributeType flag) const;
     * </pre>
     * for checking whether the set contains a specfic flag.
     * This functionality is for example provided the classes
     * EnumItem, EnumRange and Combine.
     *
     * @param remoteIndices The indices present on remote processes.
     * @param sourceFlags The set of attributes which mark indices we send to other
     *               processes.
     * @param sendData The indexed data structure whose data will be send.
     * @param destFlags  The set of attributes which mark the indices we might
     *               receive values from.
     * @param receiveData The indexed data structure for which we receive data.
     */
    template<class T1, class T2, class V>
    void build(const RemoteIndices& remoteIndices, const T1& sourceFlags, V& sendData, const T2& destFlags, V& receiveData);

    /**
     * @brief Sends the primitive values from the source to the destination.
     */
    void forward();

    /**
     * @brief Sends the primitive values from the destination to the source.
     */
    void backward();

    /**
     * @brief Deallocates the MPI requests and data types.
     */
    void free();
  private:
    enum {
      /**
       * @brief Tag for the MPI communication.
       */
      commTag_ = 234
    };

    /**
     * @brief The indices also known at other processes.
     */
    const RemoteIndices* remoteIndices_;

    typedef std::map<int,std::pair<MPI_Datatype,MPI_Datatype> >
    MessageTypeMap;

    /**
     * @brief The datatypes built according to the communication interface.
     */
    MessageTypeMap messageTypes;

    /**
     * @brief The pointer to the data whose entries we communicate.
     */
    void* data_;

    MPI_Request* requests_[2];

    /**
     * @brief True if the request and data types were created.
     */
    bool created_;

    /**
     * @brief Creates the MPI_Requests for the forward communication.
     */
    template<class V, bool FORWARD>
    void createRequests(V& sendData, V& receiveData);

    /**
     * @brief Creates the data types needed for the unbuffered receive.
     */
    template<class T1, class T2, class V, bool send>
    void createDataTypes(const T1& source, const T2& destination, V& data);

    /**
     * @brief initiates the sending and receive.
     */
    void sendRecv(MPI_Request* req);

    /**
     * @brief Information used for setting up the MPI Datatypes.
     */
    struct IndexedTypeInformation
    {
      void build(int i)
      {
        length = new int[i];
        displ  = new MPI_Aint[i];
        size = i;
      }

      void free()
      {
        delete[] length;
        delete[] displ;
      }
      int* length;
      MPI_Aint* displ;
      int elements;
      int size;
    };

    /**
     * @brief Functor for the InterfaceBuilder.
     *
     * It will record the information needed to build the MPI_Datatypes.
     */
    template<class V>
    struct MPIDatatypeInformation
    {
      MPIDatatypeInformation(const V& data) : data_(data)
      {}

      void reserve(int proc, int size)
      {
        information_[proc].build(size);
      }
      void add(int proc, int local)
      {
        IndexedTypeInformation& info=information_[proc];
        assert(info.elements<info.size);
        MPI_Address( const_cast<void*>(CommPolicy<V>::getAddress(data_, local)),
                     info.displ+info.elements);
        info.length[info.elements]=CommPolicy<V>::getSize(data_, local);
        info.elements++;
      }

      std::map<int,IndexedTypeInformation> information_;
      const V& data_;

    };

  };


  template<typename TG, typename TA, int N=100>
  class BufferedCommunicator
  {
  public:
    BufferedCommunicator();

    template<class Data>
    EnableIf<SameType<SizeOne,typename CommPolicy<Data>::IndexedTypeFlag>::value, void>
    build(const Interface<TG,TA,N>& interface);

    template<class Data>
    void build(const Data& source, const Data& dest, const Interface<TG,TA,N>& interface);

    template<class GatherScatter, class Data>
    void forward(Data& source, Data& dest);

    template<class GatherScatter, class Data>
    void backward(Data& source, Data& dest);

    template<class GatherScatter, class Data>
    void forward(Data& data);

    template<class GatherScatter, class Data>
    void backward(Data& data);

    void free();

    ~BufferedCommunicator();

  private:

    /**
     * @brief Functors for message size caculation
     */
    template<class Data, typename IndexedTypeFlag>
    struct MessageSizeCalculator
    {};

    template<class Data>
    struct MessageSizeCalculator<Data,SizeOne>
    {
      inline int operator()(const InterfaceInformation& info) const;
      inline int operator()(const Data& data, const InterfaceInformation& info) const;
    };

    template<class Data>
    struct MessageSizeCalculator<Data,VariableSize>
    {
      inline int operator()(const Data& data, const InterfaceInformation& info) const;
    };

    /**
     * @brief Functors for message data gathering.
     */
    template<class Data, class GatherScatter, bool send, typename IndexedTypeFlag>
    struct MessageGatherer
    {};

    template<class Data, class GatherScatter, bool send>
    struct MessageGatherer<Data,GatherScatter,send,SizeOne>
    {
      typedef typename CommPolicy<Data>::IndexedType Type;

      inline void operator()(const Interface<TG,TA,N>& interface, const Data& data, Type* buffer) const;
    };

    template<class Data, class GatherScatter, bool send>
    struct MessageGatherer<Data,GatherScatter,send,VariableSize>
    {
      typedef typename CommPolicy<Data>::IndexedType Type;

      inline void operator()(const Interface<TG,TA,N>& interface, const Data& data, Type* buffer) const;
    };

    /**
     * @brief Functors for message data scattering.
     */
    template<class Data, class GatherScatter, bool send, typename IndexedTypeFlag>
    struct MessageScatterer
    {};

    template<class Data, class GatherScatter, bool send>
    struct MessageScatterer<Data,GatherScatter,send,SizeOne>
    {
      typedef typename CommPolicy<Data>::IndexedType Type;

      inline void operator()(const Interface<TG,TA,N>& interface, Data& data, Type* buffer, const int& proc) const;
    };

    template<class Data, class GatherScatter, bool send>
    struct MessageScatterer<Data,GatherScatter,send,VariableSize>
    {
      typedef typename CommPolicy<Data>::IndexedType Type;

      inline void operator()(const Interface<TG,TA,N>& interface, Data& data, Type* buffer, const int& proc) const;
    };
    struct MessageInformation
    {
      MessageInformation()
        : start_(0), size_(0)
      {}

      MessageInformation(size_t start, size_t size)
        : start_(start), size_(size)
      {}
      /**
       * @brief Start of the message in the buffer counted in bytes.
       */
      size_t start_;
      /**
       * @brief Number of entries in message (not in bytes!).
       */
      size_t size_;
    };

    /**
     * @brief Type of the map of information about the messages to send.
     *
     * The key is the process number to communicate with and the key is
     * the pair of information about sending and receiving messages.
     */
    typedef std::map<int,std::pair<MessageInformation,MessageInformation> >
    InformationMap;
    /**
     * @brief Gathered information about the messages to send.
     */
    InformationMap messageInformation_;
    /**
     * @brief Communication buffers.
     */
    char* buffers_[2];

    enum {
      /**
       * @brief The tag we use for communication.
       */
      commTag_
    };

    /**
     * @brief The interface we currently work with.
     */
    const Interface<TG,TA,N>* interface_;

    /**
     * @brief Send and receive Data.
     */
    template<class GatherScatter, bool FORWARD, class Data>
    void sendRecv(Data& source, Data& d);

  };

  template<class V>
  inline const void* CommPolicy<V>::getAddress(const V& v, int index)
  {
    return &(v[index]);
  }

  template<class V>
  inline int CommPolicy<V>::getSize(const V& v, int index)
  {
    return 1;
  }


  template<typename TG, typename TA, int N>
  DatatypeCommunicator<TG,TA,N>::DatatypeCommunicator()
    : remoteIndices_(0), created_(false)
  {
    requests_[0]=0;
    requests_[1]=0;
  }



  template<typename TG, typename TA, int N>
  DatatypeCommunicator<TG,TA,N>::~DatatypeCommunicator()
  {
    free();
  }

  template<typename TG, typename TA, int N>
  template<class T1, class T2, class V>
  inline void DatatypeCommunicator<TG,TA,N>::build(const RemoteIndices& remoteIndices,
                                                   const T1& source, V& sendData,
                                                   const T2& destination, V& receiveData)
  {
    remoteIndices_ = &remoteIndices;
    free();
    assert(remoteIndices.isBuilt());
    createDataTypes<T1,T2,V,false>(source,destination, receiveData);
    createDataTypes<T1,T2,V,true>(source,destination, sendData);
    createRequests<V,true>(sendData, receiveData);
    createRequests<V,false>(receiveData, sendData);
    created_=true;
  }

  template<typename TG, typename TA, int N>
  void DatatypeCommunicator<TG,TA,N>::free()
  {
    if(created_) {
      delete[] requests_[0];
      delete[] requests_[1];
      typedef MessageTypeMap::iterator iterator;
      typedef MessageTypeMap::const_iterator const_iterator;

      const const_iterator end=messageTypes.end();

      for(iterator process = messageTypes.begin(); process != end; ++process) {
        MPI_Datatype *type = &(process->second.first);
        MPI_Type_free(type);
        type = &(process->second.second);
        MPI_Type_free(type);
      }
      messageTypes.clear();

    }

  }

  template<typename TG, typename TA, int N>
  template<class T1, class T2, class V, bool send>
  void DatatypeCommunicator<TG,TA,N>::createDataTypes(const T1& sourceFlags, const T2& destFlags, V& data)
  {

    MPIDatatypeInformation<V>  dataInfo(data);
    this->template buildInterface<T1,T2,MPIDatatypeInformation<V>,send>(*remoteIndices_,sourceFlags, destFlags, dataInfo);

    typedef typename RemoteIndices::RemoteIndexMap::const_iterator const_iterator;
    const const_iterator end=this->remoteIndices_->end();

    // Allocate MPI_Datatypes and deallocate memory for the type construction.
    for(const_iterator process=this->remoteIndices_->begin(); process != end; ++process) {
      IndexedTypeInformation& info=dataInfo.information_[process->first];
      // Shift the displacement
      MPI_Aint base;
      MPI_Address(const_cast<void *>(CommPolicy<V>::getAddress(data, 0)), &base);

      for(int i=0; i< info.elements; i++) {
        info.displ[i]-=base;
      }

      // Create data type
      MPI_Datatype* type = &( send ? messageTypes[process->first].first : messageTypes[process->first].second);
      MPI_Type_hindexed(info.elements, info.length, info.displ,
                        MPITraits<typename CommPolicy<V>::IndexedType>::getType(),
                        type);
      MPI_Type_commit(type);
      // Deallocate memory
      info.free();
    }
  }

  template<typename TG, typename TA, int N>
  template<class V, bool createForward>
  void DatatypeCommunicator<TG,TA,N>::createRequests(V& sendData, V& receiveData)
  {
    typedef std::map<int,std::pair<MPI_Datatype,MPI_Datatype> >::const_iterator MapIterator;
    int rank;
    static int index = createForward ? 1 : 0;
    int noMessages = messageTypes.size();
    // allocate request handles
    requests_[index] = new MPI_Request[2*noMessages];
    const MapIterator end = messageTypes.end();
    int request=0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Set up the requests for receiving first
    for(MapIterator process = messageTypes.begin(); process != end;
        ++process, ++request) {
      MPI_Datatype type = createForward ? process->second.second : process->second.first;
      void* address = const_cast<void*>(CommPolicy<V>::getAddress(receiveData,0));
      MPI_Recv_init(address, 1, type, process->first, commTag_, this->remoteIndices_->communicator(), requests_[index]+request);
    }

    // And now the send requests

    for(MapIterator process = messageTypes.begin(); process != end;
        ++process, ++request) {
      MPI_Datatype type = createForward ? process->second.first : process->second.second;
      void* address =  const_cast<void*>(CommPolicy<V>::getAddress(sendData, 0));
      MPI_Ssend_init(address, 1, type, process->first, commTag_, this->remoteIndices_->communicator(), requests_[index]+request);
    }
  }

  template<typename TG, typename TA, int N>
  void DatatypeCommunicator<TG,TA,N>::forward()
  {
    sendRecv(requests_[1]);
  }

  template<typename TG, typename TA, int N>
  void DatatypeCommunicator<TG,TA,N>::backward()
  {
    sendRecv(requests_[0]);
  }

  template<typename TG, typename TA, int N>
  void DatatypeCommunicator<TG,TA,N>::sendRecv(MPI_Request* requests)
  {
    int noMessages = messageTypes.size();
    // Start the receive calls first
    MPI_Startall(noMessages, requests);
    // Now the send calls
    MPI_Startall(noMessages, requests+noMessages);

    // Wait for completion of the communication send first then receive
    MPI_Status* status=new MPI_Status[2*noMessages];
    for(int i=0; i<2*noMessages; i++)
      status[i].MPI_ERROR=MPI_SUCCESS;

    int send = MPI_Waitall(noMessages, requests+noMessages, status+noMessages);
    int receive = MPI_Waitall(noMessages, requests, status);

    // Error checks
    int success=1, globalSuccess=0;
    if(send==MPI_ERR_IN_STATUS) {
      int rank;
      MPI_Comm_rank(this->remoteIndices_->communicator(), &rank);
      std::cerr<<rank<<": Error in sending :"<<std::endl;
      // Search for the error
      for(int i=noMessages; i< 2*noMessages; i++)
        if(status[i].MPI_ERROR!=MPI_SUCCESS) {
          char message[300];
          int messageLength;
          MPI_Error_string(status[i].MPI_ERROR, message, &messageLength);
          std::cerr<<" source="<<status[i].MPI_SOURCE<<" message: ";
          for(int i=0; i< messageLength; i++)
            std::cout<<message[i];
        }
      std::cerr<<std::endl;
      success=0;
    }

    if(receive==MPI_ERR_IN_STATUS) {
      int rank;
      MPI_Comm_rank(this->remoteIndices_->communicator(), &rank);
      std::cerr<<rank<<": Error in receiving!"<<std::endl;
      // Search for the error
      for(int i=0; i< noMessages; i++)
        if(status[i].MPI_ERROR!=MPI_SUCCESS) {
          char message[300];
          int messageLength;
          MPI_Error_string(status[i].MPI_ERROR, message, &messageLength);
          std::cerr<<" source="<<status[i].MPI_SOURCE<<" message: ";
          for(int i=0; i< messageLength; i++)
            std::cerr<<message[i];
        }
      std::cerr<<std::endl;
      success=0;
    }

    MPI_Allreduce(&success, &globalSuccess, 1, MPI_INT, MPI_MIN, this->remoteIndices_->communicator());

    if(!globalSuccess)
      DUNE_THROW(CommunicationError, "A communication error occurred!");


  }



  template<typename TG, typename TA, int N>
  BufferedCommunicator<TG,TA,N>::BufferedCommunicator()
    : interface_(0)
  {
    buffers_[0]=0;
    buffers_[1]=0;
  }


  template<typename TG, typename TA, int N>
  template<class Data>
  EnableIf<SameType<SizeOne, typename CommPolicy<Data>::IndexedTypeFlag>::value, void>
  BufferedCommunicator<TG,TA,N>::build(const Interface<TG,TA,N>& interface)
  {
    typedef typename Interface<TG,TA,N>::InformationMap::const_iterator const_iterator;
    typedef typename CommPolicy<Data>::IndexedTypeFlag Flag;
    const const_iterator end = interface.interfaces().end();
    int sendStart=0, recvStart=0;

    for(const_iterator interfacePair = interface.interfaces().begin();
        interfacePair != end; ++interfacePair) {
      int noSend = MessageSizeCalculator<Data,Flag>() (interfacePair->second.first);
      int noRecv = MessageSizeCalculator<Data,Flag>() (interfacePair->second.second);
      messageInformation_[interfacePair->first]=
        std::make_pair(MessageInformation(sendStart,
                                          noSend),
                       MessageInformation(recvStart,
                                          noRecv));
      sendStart += noSend;
      recvStart += noRecv;
    }

    // allocate the buffers
    buffers_[0] = new char[sendStart];
    buffers_[1] = new char[recvStart];
    interface_ = &interface;
    return EnableIf<SameType<SizeOne, typename CommPolicy<Data>::IndexedTypeFlag>::value, void>();

  }

  template<typename TG, typename TA, int N>
  template<class Data>
  void BufferedCommunicator<TG,TA,N>::build(const Data& source, const Data& dest, const Interface<TG,TA,N>& interface)
  {
    typedef typename Interface<TG,TA,N>::InformationMap::const_iterator const_iterator;
    typedef typename CommPolicy<Data>::IndexedTypeFlag Flag;
    const const_iterator end = interface.interfaces().end();
    int sendStart=0, recvStart=0;

    for(const_iterator interfacePair = interface.interfaces().begin();
        interfacePair != end; ++interfacePair) {
      int noSend = MessageSizeCalculator<Data,Flag>() (source, interfacePair->second.first);
      int noRecv = MessageSizeCalculator<Data,Flag>() (dest, interfacePair->second.second);

      messageInformation_[interfacePair->first]=(std::make_pair(MessageInformation(sendStart,
                                                                                   noSend),
                                                                MessageInformation(recvStart,
                                                                                   noRecv)));
      sendStart += noSend;
      recvStart  += noRecv;
    }

    // allocate the buffers
    buffers_[0] = new char[sendStart];
    buffers_[1] = new char[recvStart];
    interface_ = &interface;
  }

  template<typename TG, typename TA, int N>
  void BufferedCommunicator<TG,TA,N>::free()
  {
    if(interface_!=0) {
      messageInformation_.clear();
      delete[] buffers_[0];
      delete[] buffers_[1];
      interface_=0;
    }
  }

  template<typename TG, typename TA, int N>
  BufferedCommunicator<TG,TA,N>::~BufferedCommunicator()
  {
    free();
  }

  template<typename TG, typename TA, int N>
  template<class Data>
  inline int BufferedCommunicator<TG,TA,N>::MessageSizeCalculator<Data,SizeOne>::operator()
    (const InterfaceInformation& info) const
  {
    return info.size()*sizeof(typename CommPolicy<Data>::IndexedType);
  }

  template<typename TG, typename TA, int N>
  template<class Data>
  inline int BufferedCommunicator<TG,TA,N>::MessageSizeCalculator<Data,SizeOne>::operator()
    (const Data& data, const InterfaceInformation& info) const
  {
    return operator()(info);
  }

  template<typename TG, typename TA, int N>
  template<class Data>
  inline int BufferedCommunicator<TG,TA,N>::MessageSizeCalculator<Data, VariableSize>::operator()
    (const Data& data, const InterfaceInformation& info) const
  {
    int entries=0;

    for(int i=0; i <  info.size(); i++)
      entries += CommPolicy<Data>::getSize(data,info[i]);

    return entries * sizeof(typename CommPolicy<Data>::IndexedType);
  }

  template<typename TG, typename TA, int N>
  template<class Data, class GatherScatter, bool FORWARD>
  inline void BufferedCommunicator<TG,TA,N>::MessageGatherer<Data,GatherScatter,FORWARD,VariableSize>::operator()(const Interface<TG,TA,N>& interface,const Data& data, Type* buffer) const
  {
    typedef typename Interface<TG,TA,N>::InformationMap::const_iterator
    const_iterator;
    const const_iterator end = interface.interfaces().end();
    int offset=0;

    for(const_iterator interfacePair = interface.interfaces().begin();
        interfacePair != end; ++interfacePair) {
      int size = forward ? interfacePair->second.first.size() :
                 interfacePair->second.second.size();

      for(int i=0, index=0; i < size; i++) {
        int local = forward ? interfacePair->second->first[i] :
                    interfacePair->second->second[i];
        for(int j=0; j < CommPolicy<Data>::getSize(data, local); j++, index++)
          buffer[index]=GatherScatter::gather(data, local, j);
      }
    }

  }

  template<typename TG, typename TA, int N>
  template<class Data, class GatherScatter, bool FORWARD>
  inline void BufferedCommunicator<TG,TA,N>::MessageGatherer<Data,GatherScatter,FORWARD,SizeOne>::operator()(const Interface<TG,TA,N>& interface, const Data& data, Type* buffer) const
  {
    typedef typename Interface<TG,TA,N>::InformationMap::const_iterator
    const_iterator;
    const const_iterator end = interface.interfaces().end();
    size_t index = 0;

    for(const_iterator interfacePair = interface.interfaces().begin();
        interfacePair != end; ++interfacePair) {
      size_t size = FORWARD ? interfacePair->second.first.size() :
                    interfacePair->second.second.size();

      for(size_t i=0; i < size; i++) {
        buffer[index++] = GatherScatter::gather(data, FORWARD ? interfacePair->second.first[i] :
                                                interfacePair->second.second[i]);
      }
    }

  }

  template<typename TG, typename TA, int N>
  template<class Data, class GatherScatter, bool FORWARD>
  inline void BufferedCommunicator<TG,TA,N>::MessageScatterer<Data,GatherScatter,FORWARD,VariableSize>::operator()(const Interface<TG,TA,N>& interface, Data& data, Type* buffer, const int& proc) const
  {
    typedef typename Interface<TG,TA,N>::Information Information;
    const typename Interface<TG,TA,N>::InformationMap::const_iterator infoPair = interface.interfaces().find(proc);

    assert(infoPair!=interface.interfaces().end());

    const Information& info = FORWARD ? infoPair->second.second :
                              infoPair->second.first;

    for(int i=0, index=0; i < info.size(); i++) {
      for(int j=0; j < CommPolicy<Data>::getSize(data, info[i]); j++)
        GatherScatter::scatter(data, buffer[index++], info[i], j);
    }
  }

  template<typename TG, typename TA, int N>
  template<class Data, class GatherScatter, bool FORWARD>
  inline void BufferedCommunicator<TG,TA,N>::MessageScatterer<Data,GatherScatter,FORWARD,SizeOne>::operator()(const Interface<TG,TA,N>& interface, Data& data, Type* buffer, const int& proc) const
  {
    typedef typename Interface<TG,TA,N>::Information Information;
    const typename Interface<TG,TA,N>::InformationMap::const_iterator infoPair = interface.interfaces().find(proc);

    assert(infoPair!=interface.interfaces().end());

    const Information& info = FORWARD ? infoPair->second.second :
                              infoPair->second.first;

    for(size_t i=0; i < info.size(); i++) {
      GatherScatter::scatter(data, buffer[i], info[i]);
    }
  }

  template<typename TG, typename TA, int N>
  template<class GatherScatter,class Data>
  void BufferedCommunicator<TG,TA,N>::forward(Data& data)
  {
    this->template sendRecv<GatherScatter,true>(data, data);
  }

  template<typename TG, typename TA, int N>
  template<class GatherScatter, class Data>
  void BufferedCommunicator<TG,TA,N>::backward(Data& data)
  {
    this->template sendRecv<GatherScatter,false>(data, data);
  }

  template<typename TG, typename TA, int N>
  template<class GatherScatter, class Data>
  void BufferedCommunicator<TG,TA,N>::forward(Data& source, Data& dest)
  {
    this->template sendRecv<GatherScatter,true>(source, dest);
  }

  template<typename TG, typename TA, int N>
  template<class GatherScatter, class Data>
  void BufferedCommunicator<TG,TA,N>::backward(Data& source, Data& dest)
  {
    this->template sendRecv<GatherScatter,false>(dest, source);
  }

  template<typename TG, typename TA, int N>
  template<class GatherScatter, bool FORWARD, class Data>
  void BufferedCommunicator<TG,TA,N>::sendRecv(Data& source, Data& dest)
  {
    typedef typename CommPolicy<Data>::IndexedType Type;

    Type *sendBuffer, *recvBuffer;

    if(FORWARD) {
      sendBuffer = reinterpret_cast<Type*>(buffers_[0]);
      recvBuffer = reinterpret_cast<Type*>(buffers_[1]);
    }else{
      sendBuffer = reinterpret_cast<Type*>(buffers_[1]);
      recvBuffer = reinterpret_cast<Type*>(buffers_[0]);
    }
    typedef typename CommPolicy<Data>::IndexedTypeFlag Flag;

    MessageGatherer<Data,GatherScatter,FORWARD,Flag>() (*interface_, source, sendBuffer);

    MPI_Request* sendRequests = new MPI_Request[messageInformation_.size()];
    MPI_Request* recvRequests = new MPI_Request[messageInformation_.size()];

    // Setup receive first
    typedef typename InformationMap::const_iterator const_iterator;

    const const_iterator end = messageInformation_.end();
    size_t i=0;
    int* processMap = new int[messageInformation_.size()];

    for(const_iterator info = messageInformation_.begin(); info != end; ++info, ++i) {
      processMap[i]=info->first;
      if(FORWARD)
        MPI_Irecv(recvBuffer+info->second.second.start_, info->second.second.size_,
                  MPI_BYTE, info->first, commTag_, interface_->communicator(),
                  recvRequests+i);
      else
        MPI_Irecv(recvBuffer+info->second.first.start_, info->second.first.size_,
                  MPI_BYTE, info->first, commTag_, interface_->communicator(),
                  recvRequests+i);
    }

    // now the send requests
    i=0;
    for(const_iterator info = messageInformation_.begin(); info != end; ++info, ++i)
      if(FORWARD)
        MPI_Issend(sendBuffer+info->second.first.start_, info->second.first.size_,
                   MPI_BYTE, info->first, commTag_, interface_->communicator(),
                   sendRequests+i);
      else
        MPI_Issend(sendBuffer+info->second.second.start_, info->second.second.size_,
                   MPI_BYTE, info->first, commTag_, interface_->communicator(),
                   sendRequests+i);

    // Wait for completion of receive and immediately start scatter
    i=0;
    int success=1;
    int finished=-1;
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Status status[messageInformation_.size()];
    MPI_Waitall(messageInformation_.size(), recvRequests, status);

    for(i=0; i< messageInformation_.size(); i++) {
      //      int ret=MPI_Waitany(messageInformation_.size(), recvRequests, &finished, &status);
      if(status[i].MPI_ERROR==MPI_SUCCESS) {
        MessageScatterer<Data,GatherScatter,FORWARD,Flag>() (*interface_, dest, recvBuffer, processMap[i]);
        recvRequests[i]=MPI_REQUEST_NULL;
      }else{
        std::cerr<<rank<<": MPI_Error occurred while receiving message from "<<processMap[finished]<<std::endl;
        success=0;
      }
    }

    // Wait for completion of sends
    for(i=0; i< messageInformation_.size(); i++)
      if(MPI_SUCCESS!=MPI_Wait(sendRequests+i, status+i)) {
        std::cerr<<rank<<": MPI_Error occurred while sending message to "<<processMap[finished]<<std::endl;
        success=0;
      }
    int globalSuccess;
    MPI_Allreduce(&success, &globalSuccess, 1, MPI_INT, MPI_MIN, interface_->communicator());

    if(!globalSuccess)
      DUNE_THROW(CommunicationError, "A communication error occurred!");

    delete[] processMap;
    delete[] sendRequests;
    delete[] recvRequests;

  }

  /** @} */
}

#endif
