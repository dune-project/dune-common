// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMUNICATOR
#define DUNE_COMMUNICATOR

#include <dune/istl/remoteindices.hh>
#include <dune/common/exceptions.hh>
namespace Dune
{
  /** @defgroup ISTL_Comm ISTL Communication
   * @ingroup ISTL
   * @brief Provides classes for syncing distributed indexed
   * data structures.
   */
  /**
   * @file
   * @brief Provides utility classes for syncing distributed data via MPI communication.
   * @author Markus Blatt
   */
  /** @addtogroup ISTL_Comm
   *
   * @{
   */
  template<bool i>
  struct Bool2Type
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
     * @brief Get the address of entry at an index.
     *
     * The default implementation uses operator[] to
     * get the address.
     * @param v An existing representation of the type that has more elements than index.
     * @param index The index of the entry.
     */
    static void* getAddress(V& v, int index);

    /**
     * @brief Get the number of primitve elements at that index.
     *
     * The default always returns 1.
     */
    static int getSize(const V&, int index);
  };

  /**
   * @brief An utility class for communicating distributed data structures.
   */
  template<typename TG, typename TA>
  class Communicator
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
    typedef IndexSet<GlobalIndexType,LocalIndexType > IndexSetType;

    /**
     * @brief Type of the underlying remote indices class.
     */
    typedef RemoteIndices<GlobalIndexType, AttributeType> RemoteIndices;


    /**
     * @brief The type of the global index.
     */
    typedef TG GlobalIndex;

    /**
     * @brief The type of the attribute of the corresponding index set.
     */
    typedef TA Attribute;

    /**
     * @brief Error thrown if there was a problem with the communication.
     */
    class CommunicationError : public IOError
    {};

    /**
     * @brief Creates a new communicator.
     * @param source The index set to use at the source of the communication.
     * @param destination The index set to use at the destination of the communication.
     * @param comm The MPI communicator to use.
     */
    Communicator(const IndexSetType& source, const IndexSetType& destination,
                 const MPI_Comm& comm);

    /**
     * @brief Creates a new Communicator.
     * @param remote The remote index set to use.
     */
    Communicator(const RemoteIndices& remote);

    /**
     * @brief Destructor.
     */
    ~Communicator();

    /**
     * @brief Builds the interface between the index sets.
     *
     * Has to be called before the actual communication by forward or backward
     * can be called.
     *
     * If the template paramter ignorePublic is true, even values for nonpublic indices
     * might be communicated. This is for example necessary for redistributing or accumulating
     * data.
     *
     * @param sourceFlags The set of attributes which mark indices we send to other
     *               processes.
     * @param sendData The indexed data structure whose data will be send.
     * @param destFlags  The set of attributes which mark the indices we might
     *               receive values from.
     * @param receiveData The indexed data structure for which we receive data.
     * @param flag A flag to prevent users from  specifying all template paramters.
     */
    template<class T1, class T2, class V, bool ignorePublic>
    void build(const T1& sourceFlags, V& sendData, const T2& destFlags, V& receiveData,
               const Bool2Type<ignorePublic>& flag);

    /**
     * @brief Builds the interface between the index sets.
     *
     * Has to be called before the actual communication by forward or backward
     * can be called. Nonpublic indices will be ignored!
     *
     * @param sourceFlags The set of attributes which mark indices we send to other
     *               processes.
     * @param sendData The indexed data structure whose data will be send.
     * @param destFlags  The set of attributes which mark the indices we might
     *               receive values from.
     * @param receiveData The indexed data structure for which we receive data.
     */
    template<class T1, class T2, class V>
    void build(const T1& sourceFlags, V& sendData, const T2& destFlags, V& receiveData);

    /**
     * @brief Sends the primitive values from the source to the destination.
     */
    void forward();

    /**
     * @brief Sends the primitive values from the destination to the source.
     */
    void backward();

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
    RemoteIndices remoteIndices_;

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
    template<class V, bool forward>
    void createRequests(V& sendData, V& receiveData);

    /**
     * @brief Creates the data types needed for the unbuffered receive.
     */
    template<class T1, class T2, class V, bool send>
    void createDataTypes(const T1& source, const T2& destination, V& data);

    /**
     * @brief Deallocates the MPI requests and data types.
     */
    void clean();

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
      }

      void free()
      {
        delete[] length;
        delete[] displ;
      }
      int* length;
      MPI_Aint* displ;
      int elements;
    };
  };

  template<class V>
  inline void* CommPolicy<V>::getAddress(V& v, int index)
  {
    return &(v[index]);
  }

  template<class V>
  inline int CommPolicy<V>::getSize(const V& v, int index)
  {
    return 1;
  }

  template<typename TG, typename TA>
  Communicator<TG,TA>::Communicator(const IndexSetType& source,
                                    const IndexSetType& destination,
                                    const MPI_Datatype& comm)
    : remoteIndices_(source, destination, comm), created_(false)
  {
    requests_[0]=0;
    requests_[1]=0;
  }


  template<typename TG, typename TA>
  Communicator<TG,TA>::Communicator(const RemoteIndices& remote)
    : remoteIndices_(remote), created_(false)
  {
    requests_[0]=0;
    requests_[1]=0;
  }

  template<typename TG, typename TA>
  Communicator<TG,TA>::~Communicator()
  {
    clean();
  }

  template<typename TG, typename TA>
  template<class T1, class T2, class V>
  inline void Communicator<TG,TA>::build(const T1& source, V& sendData,
                                         const T2& destination, V& receiveData)
  {
    build(source, sendData, destination, receiveData, Bool2Type<false>());
  }

  template<typename TG, typename TA>
  template<class T1, class T2, class V, bool ignorePublic>
  void Communicator<TG,TA>::build(const T1& source, V& sendData,
                                  const T2& destination, V& receiveData,
                                  const Bool2Type<ignorePublic>& flag)
  {
    clean();
    remoteIndices_.template rebuild<ignorePublic>();
    std::cout<<remoteIndices_<<std::endl;
    createDataTypes<T1,T2,V,false>(source,destination, receiveData);
    createDataTypes<T1,T2,V,true>(source,destination, sendData);
    createRequests<V,true>(sendData, receiveData);
    createRequests<V,false>(receiveData, sendData);
    created_=true;
  }

  template<typename TG, typename TA>
  void Communicator<TG,TA>::clean()
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

  template<typename TG, typename TA>
  template<class T1, class T2, class V, bool send>
  void Communicator<TG,TA>::createDataTypes(const T1& sourceFlags, const T2& destFlags, V& data)
  {
    // Allocate the memory for the data type construction.
    typedef typename RemoteIndices::RemoteIndexMap::const_iterator const_iterator;

    const const_iterator end=remoteIndices_.end();

    std::map<int,IndexedTypeInformation> information;
    int rank;

    MPI_Comm_rank(remoteIndices_.communicator(), &rank);

    // Allocate memory for the type construction.
    for(const_iterator process=remoteIndices_.begin(); process != end; ++process) {
      IndexedTypeInformation& info =information[process->first];
      int _size = send ? (process->second.first->size()) : process->second.second->size();
      info.build(_size);
      info.elements=0;
    }

    // compare the local and remote indices and set up the types
    typedef typename RemoteIndices::IndexSetType::const_iterator LocalIterator;

    CollectiveIterator<TG,TA> remote = remoteIndices_.template iterator<send>();
    LocalIterator localIndex = send ? remoteIndices_.source_.begin() : remoteIndices_.dest_.begin();
    const LocalIterator localEnd = send ?  remoteIndices_.source_.end() : remoteIndices_.dest_.end();

    while(localIndex!=localEnd && !remote.empty()) {
      if( send ? sourceFlags.contains(localIndex->local().attribute()) :
          destFlags.contains(localIndex->local().attribute()))
      {
        // search for matching remote indices
        remote.advance(localIndex->global());
        // Iterate over the list that are positioned at global
        typedef typename CollectiveIterator<TG,TA>::iterator ValidIterator;
        const ValidIterator end = remote.end();
        ValidIterator validEntry = remote.begin();

        for(int i=0; validEntry != end; ++i) {
          if( send ?  destFlags.contains(validEntry->attribute()) :
              sourceFlags.contains(validEntry->attribute())) {
            // We will receive data for this index
            IndexedTypeInformation& info=information[validEntry.process()];
            MPI_Address(CommPolicy<V>::getAddress(data, localIndex->local()),
                        info.displ+info.elements);
            info.length[info.elements]=CommPolicy<V>::getSize(data, localIndex->local());
            info.elements++;
          }
          ++validEntry;
        }

      }
      ++localIndex;

    }


    // Allocate MPI_Datatypes and deallocate memory for the type construction.
    for(const_iterator process=remoteIndices_.begin(); process != end; ++process) {
      IndexedTypeInformation& info=information[process->first];
      // Shift the displacement
      MPI_Aint base;
      MPI_Address(CommPolicy<V>::getAddress(data, 0), &base);

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

    information.clear();
  }

  template<typename TG, typename TA>
  template<class V, bool createForward>
  void Communicator<TG,TA>::createRequests(V& sendData, V& receiveData)
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
      void* address = CommPolicy<V>::getAddress(receiveData,0);
      MPI_Recv_init(address, 1, type, process->first, commTag_, remoteIndices_.communicator(), requests_[index]+request);
    }

    // And now the send requests

    for(MapIterator process = messageTypes.begin(); process != end;
        ++process, ++request) {
      MPI_Datatype type = createForward ? process->second.first : process->second.second;
      void* address = CommPolicy<V>::getAddress(sendData, 0);
      MPI_Ssend_init(address, 1, type, process->first, commTag_, remoteIndices_.communicator(), requests_[index]+request);
    }
  }

  template<typename TG, typename TA>
  void Communicator<TG,TA>::forward()
  {
    sendRecv(requests_[1]);
  }

  template<typename TG, typename TA>
  void Communicator<TG,TA>::backward()
  {
    sendRecv(requests_[0]);
  }

  template<typename TG, typename TA>
  void Communicator<TG,TA>::sendRecv(MPI_Request* requests)
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
      MPI_Comm_rank(remoteIndices_.communicator(), &rank);
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
      MPI_Comm_rank(remoteIndices_.communicator(), &rank);
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

    MPI_Allreduce(&success, &globalSuccess, 1, MPI_INT, MPI_MIN, remoteIndices_.communicator());

    if(!globalSuccess)
      DUNE_THROW(CommunicationError, "A communication error occurred!");


  }
  /** @} */
}

#endif
