// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMUNICATOR
#define DUNE_COMMUNICATOR

#if HAVE_MPI

#include <cassert>
#include <cstddef>
#include <iostream>
#include <map>
#include <type_traits>
#include <utility>

#include <mpi.h>

#include <dune/common/exceptions.hh>
#include <dune/common/parallel/interface.hh>
#include <dune/common/parallel/remoteindices.hh>
#include <dune/common/stdstreams.hh>

namespace Dune
{
  /** @defgroup Common_Parallel Parallel Computing based on Indexsets
   * @ingroup ParallelCommunication
   * @brief Provides classes for syncing distributed indexed
   * data structures.
   *
   * In a parallel representation a container \f$x\f$,
   * e.g. a plain C-array, cannot be stored with all entries on each process
   * because of limited memory and efficiency reasons. Therefore
   * it is represented by individual
   * pieces \f$x_p\f$, \f$p=0, \ldots, P-1\f$, where \f$x_p\f$ is the piece stored on
   * process \f$p\f$ of the \f$P\f$ processes participating in the calculation.
   * Although the global representation of the container is not
   * available on any process, a process \f$p\f$ needs to know how the entries
   * of it's local piece \f$x_p\f$ correspond to the entries of the global
   * container \f$x\f$, which would be used in a sequential program. In this
   * module we present classes describing the mapping of the local pieces
   * to the global
   * view and the communication interfaces.
   *
   * @section IndexSet Parallel Index Sets
   *
   * Form an abstract point of view a random access container \f$x: I
   * \rightarrow K\f$ provides a
   * mapping from an index set \f$I \subset N_0\f$ onto a set of objects
   * \f$K\f$. Note that we do not require \f$I\f$ to be consecutive. The piece
   * \f$x_p\f$ of the container \f$x\f$ stored on process \f$p\f$ is a mapping \f$x_p:I_p
   * \rightarrow K\f$, where \f$I_p \subset I\f$. Due to efficiency the entries
   * of \f$x_p\f$ should be stored in consecutive memory.
   *
   * This means that for the local computation the data must be addressable
   * by a consecutive index starting from \f$0\f$. When using adaptive
   * discretisation methods there might be the need to reorder the indices
   * after adding and/or deleting some of the discretisation
   * points. Therefore this index does not have to be persistent. Further
   * on we will call this index <em>local index</em>.
   *
   * For the communication phases of our algorithms these locally stored
   * entries must also be addressable by a global identifier to be able to
   * store the received values tagged with the global identifiers at the
   * correct local index in the consecutive local memory chunk. To ease the
   * addition and removal of discretisation points this global identifier has
   * to be persistent. Further on we will call this global identifier
   * <em>global index</em>.
   *
   * Classes to build the mapping are ParallelIndexSet and ParallelLocalIndex.
   * As these just provide a mapping from the global index to the local index,
   * the wrapper class GlobalLookupIndexSet facilitates the reverse lookup.
   *
   * @section remote Remote Index Information
   *
   * To setup communication between the processes every process needs to
   * know what indices are also known to other processes and what
   * attributes are attached to them on the remote side. This information is
   * calculated and encapsulated in class RemoteIndices.
   *
   * @section comm Communication
   *
   * Based on the information about the distributed index sets,  data
   * independent interfaces between different sets of the index sets
   * can be setup using the class Interface.  For the actual communication
   * data dependent communicators can be setup using BufferedCommunicator,
   * DatatypeCommunicator VariableSizeCommunicator based on the interface
   * information. In contrast to the former
   * the latter is independent of the class Interface which can work on a map
   * from process number to a pair of index lists describing which local indices
   * are sent and received from that process.
   */
  /** @addtogroup Common_Parallel
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
   * @see VariableSize
   */
  struct SizeOne
  {};

  /**
   * @brief Flag for marking indexed data structures where the data at each index may
   * be a variable multiple of another type.
   * @see SizeOne
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
     * \code Type::IndexedType operator[](int i);\endcode
     * for
     * the access of the value at index i and a typedef IndexedType.
     * It is assumed
     * that only one entry is at each index (as in scalar
     * vector.
     */
    typedef V Type;

    /**
     * @brief The type we get at each index with operator[].
     *
     * The default is the value_type typedef of the container.
     */
    typedef typename V::value_type IndexedType;

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
     * @brief Get the number of primitive elements at that index.
     *
     * The default always returns 1.
     */
    static int getSize(const V&, int index);
  };

  template<class K, int n> class FieldVector;

  template<class B, class A> class VariableBlockVector;

  template<class K, class A, int n>
  struct CommPolicy<VariableBlockVector<FieldVector<K, n>, A> >
  {
    typedef VariableBlockVector<FieldVector<K, n>, A> Type;

    typedef typename Type::B IndexedType;

    typedef VariableSize IndexedTypeFlag;

    static const void* getAddress(const Type& v, int i);

    static int getSize(const Type& v, int i);
  };

  /**
   * @brief Error thrown if there was a problem with the communication.
   */
  class CommunicationError : public IOError
  {};

  /**
   * @brief GatherScatter default implementation that just copies data.
   */
  template<class T>
  struct CopyGatherScatter
  {
    typedef typename CommPolicy<T>::IndexedType IndexedType;

    static const IndexedType& gather(const T& vec, std::size_t i);

    static void scatter(T& vec, const IndexedType& v, std::size_t i);

  };

  /**
   * @brief An utility class for communicating distributed data structures via MPI datatypes.
   *
   * This communicator creates special MPI datatypes that address the non contiguous elements
   * to be send and received. The idea was to prevent the copying to an additional buffer and
   * the mpi implementation decide whether to allocate buffers or use buffers offered by the
   * interconnection network.
   *
   * Unfortunately the implementation of MPI datatypes seems to be poor. Therefore for most MPI
   * implementations using a BufferedCommunicator will be more efficient.
   */
  template<typename T>
  class DatatypeCommunicator : public InterfaceBuilder
  {
  public:

    /**
     * @brief Type of the index set.
     */
    typedef T ParallelIndexSet;

    /**
     * @brief Type of the underlying remote indices class.
     */
    typedef Dune::RemoteIndices<ParallelIndexSet> RemoteIndices;

    /**
     * @brief The type of the global index.
     */
    typedef typename RemoteIndices::GlobalIndex GlobalIndex;

    /**
     * @brief The type of the attribute.
     */
    typedef typename RemoteIndices::Attribute Attribute;

    /**
     * @brief The type of the local index.
     */
    typedef typename RemoteIndices::LocalIndex LocalIndex;

    /**
     * @brief Creates a new DatatypeCommunicator.
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
     * enumeration values of type DatatypeCommunicator::Attribute.
     * They have to provide
     * a (static) method
     * \code
     * bool contains(Attribute flag) const;
     * \endcode
     * for checking whether the set contains a specific flag.
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
    /**
     * @brief Tag for the MPI communication.
     */
    constexpr static int commTag_ = 234;

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
     * @brief Initiates the sending and receive.
     */
    void sendRecv(MPI_Request* req);

    /**
     * @brief Information used for setting up the MPI Datatypes.
     */
    struct IndexedTypeInformation
    {
      /**
       * @brief Allocate space for setting up the MPI datatype.
       *
       * @param i The number of values the datatype will have.
       */
      void build(int i)
      {
        length = new int[i];
        displ  = new MPI_Aint[i];
        size = i;
      }

      /**
       * @brief Free the allocated space.
       */
      void free()
      {
        delete[] length;
        delete[] displ;
      }
      /**  @brief The number of values at each index. */
      int* length;
      /** @brief The displacement at each index. */
      MPI_Aint* displ;
      /**
       * @brief The number of elements we send.
       * In case of variable sizes this will differ from
       * size.
       */
      int elements;
      /**
       * @param The number of indices in the data type.
       */
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
      /**
       * @brief Constructor.
       * @param data The data we construct an MPI data type for.
       */
      MPIDatatypeInformation(const V& data) : data_(data)
      {}

      /**
       * @brief Reserver space for the information about the datatype.
       * @param proc The rank of the process this information is for.
       * @param size The number of indices the datatype will contain.
       */
      void reserve(int proc, int size)
      {
        information_[proc].build(size);
      }
      /**
       * @brief Add a new index to the datatype.
       * @param proc The rank of the process this index is send to
       * or received from.
       * @param local The index to add.
       */
      void add(int proc, int local)
      {
        IndexedTypeInformation& info=information_[proc];
        assert((info.elements)<info.size);
        MPI_Get_address( const_cast<void*>(CommPolicy<V>::getAddress(data_, local)),
                         info.displ+info.elements);
        info.length[info.elements]=CommPolicy<V>::getSize(data_, local);
        info.elements++;
      }

      /**
       * @brief The information about the datatypes to send to or
       * receive from each process.
       */
      std::map<int,IndexedTypeInformation> information_;
      /**
       * @brief A representative of the indexed data we send.
       */
      const V& data_;

    };

  };

  /**
   * @brief A communicator that uses buffers to gather and scatter
   * the data to be send or received.
   *
   * Before the data is sent it is copied to a consecutive buffer and
   * then that buffer is sent.
   * The data is received in another buffer and then copied to the actual
   * position.
   */
  class BufferedCommunicator
  {

  public:
    /**
     * @brief Constructor.
     */
    BufferedCommunicator();

    /**
     * @brief Build the buffers and information for the communication process.
     *
     *
     * @param interface The interface that defines what indices are to be communicated.
     */
    template<class Data, class Interface>
    typename std::enable_if<std::is_same<SizeOne,typename CommPolicy<Data>::IndexedTypeFlag>::value, void>::type
    build(const Interface& interface);

    /**
     * @brief Build the buffers and information for the communication process.
     *
     * @param source The source in a forward send. The values will be copied from here to the send buffers.
     * @param target The target in a forward send. The received values will be copied to here.
     * @param interface The interface that defines what indices are to be communicated.
     */
    template<class Data, class Interface>
    void build(const Data& source, const Data& target, const Interface& interface);

    /**
     * @brief Send from source to target.
     *
     * The template parameter GatherScatter (e.g. CopyGatherScatter) has to have a static method
     * \code
     * // Gather the data at index index of data
     * static const typename CommPolicy<Data>::IndexedType>& gather(Data& data, int index);
     *
     * // Scatter the value at a index of data
     * static void scatter(Data& data, typename CommPolicy<Data>::IndexedType> value,
     *                     int index);
     * \endcode
     * in the case where CommPolicy<Data>::IndexedTypeFlag is SizeOne
     * and
     *
     * \code
     * static const typename CommPolicy<Data>::IndexedType> gather(Data& data, int index, int subindex);
     *
     * static void scatter(Data& data, typename CommPolicy<Data>::IndexedType> value,
     *                     int index, int subindex);
     * \endcode
     * in the case where CommPolicy<Data>::IndexedTypeFlag is VariableSize. Here subindex is the
     * subindex of the block at index.
     * @warning The source and target data have to have the same layout as the ones given
     * to the build function in case of variable size values at the indices.
     * @param source The values will be copied from here to the send buffers.
     * @param dest The received values will be copied to here.
     */
    template<class GatherScatter, class Data>
    void forward(const Data& source, Data& dest);

    /**
     * @brief Communicate in the reverse direction, i.e. send from target to source.
     *
     * The template parameter GatherScatter (e.g. CopyGatherScatter) has to have a static method
     * \code
     * // Gather the data at index index of data
     * static const typename CommPolicy<Data>::IndexedType>& gather(Data& data, int index);
     *
     * // Scatter the value at a index of data
     * static void scatter(Data& data, typename CommPolicy<Data>::IndexedType> value,
     *                     int index);
     * \endcode
     * in the case where CommPolicy<Data>::IndexedTypeFlag is SizeOne
     * and
     *
     * \code
     * static const typename CommPolicy<Data>::IndexedType> gather(Data& data, int index, int subindex);
     *
     * static void scatter(Data& data, typename CommPolicy<Data>::IndexedType> value,
     *                     int index, int subindex);
     * \endcode
     * in the case where CommPolicy<Data>::IndexedTypeFlag is VariableSize. Here subindex is the
     * subindex of the block at index.
     * @warning The source and target data have to have the same layout as the ones given
     * to the build function in case of variable size values at the indices.
     * @param dest The values will be copied from here to the send buffers.
     * @param source The received values will be copied to here.
     */
    template<class GatherScatter, class Data>
    void backward(Data& source, const Data& dest);

    /**
     * @brief Forward send where target and source are the same.
     *
     * The template parameter GatherScatter has to have a static method
     * \code
     * // Gather the data at index index of data
     * static const typename CommPolicy<Data>::IndexedType>& gather(Data& data, int index);
     *
     * // Scatter the value at a index of data
     * static void scatter(Data& data, typename CommPolicy<Data>::IndexedType> value,
     *                     int index);
     * \endcode
     * in the case where CommPolicy<Data>::IndexedTypeFlag is SizeOne
     * and
     *
     * \code
     * static const typename CommPolicy<Data>::IndexedType> gather(Data& data, int index, int subindex);
     *
     * static void scatter(Data& data, typename CommPolicy<Data>::IndexedType> value,
     *                     int index, int subindex);
     * \endcode
     * in the case where CommPolicy<Data>::IndexedTypeFlag is VariableSize. Here subindex is the
     * subindex of the block at index.
     * @param data Source and target of the communication.
     */
    template<class GatherScatter, class Data>
    void forward(Data& data);

    /**
     * @brief Backward send where target and source are the same.
     *
     * The template parameter GatherScatter has to have a static method
     * \code
     * // Gather the data at index index of data
     * static const typename CommPolicy<Data>::IndexedType>& gather(Data& data, int index);
     *
     * // Scatter the value at a index of data
     * static void scatter(Data& data, typename CommPolicy<Data>::IndexedType> value,
     *                     int index);
     * \endcode
     * in the case where CommPolicy<Data>::IndexedTypeFlag is SizeOne
     * and
     *
     * \code
     * static const typename CommPolicy<Data>::IndexedType> gather(Data& data, int index, int subindex);
     *
     * static void scatter(Data& data, typename CommPolicy<Data>::IndexedType> value,
     *                     int index, int subindex);
     * \endcode
     * in the case where CommPolicy<Data>::IndexedTypeFlag is VariableSize. Here subindex is the
     * subindex of the block at index.
     * @param data Source and target of the communication.
     */
    template<class GatherScatter, class Data>
    void backward(Data& data);

    /**
     * @brief Free the allocated memory (i.e. buffers and message information.
     */
    void free();

    /**
     * @brief Destructor.
     */
    ~BufferedCommunicator();

  private:

    /**
     * @brief The type of the map that maps interface information to processors.
     */
    typedef std::map<int,std::pair<InterfaceInformation,InterfaceInformation> >
    InterfaceMap;


    /**
     * @brief Functors for message size calculation
     */
    template<class Data, typename IndexedTypeFlag>
    struct MessageSizeCalculator
    {};

    /**
     * @brief Functor for message size calculation for datatypes
     * where at each index is only one value.
     */
    template<class Data>
    struct MessageSizeCalculator<Data,SizeOne>
    {
      /**
       * @brief Calculate the number of values in message
       * @param info The information about the interface corresponding
       * to the message.
       * @return The number of values in th message.
       */
      inline int operator()(const InterfaceInformation& info) const;
      /**
       * @brief Calculate the number of values in message
       *
       * @param info The information about the interface corresponding
       * to the message.
       * @param data ignored.
       * @return The number of values in th message.
       */
      inline int operator()(const Data& data, const InterfaceInformation& info) const;
    };

    /**
     * @brief Functor for message size calculation for datatypes
     * where at each index can be a variable number of values.
     */
    template<class Data>
    struct MessageSizeCalculator<Data,VariableSize>
    {
      /**
       * @brief Calculate the number of values in message
       *
       * @param info The information about the interface corresponding
       * to the message.
       * @param data A representative of the data we send.
       * @return The number of values in th message.
       */
      inline int operator()(const Data& data, const InterfaceInformation& info) const;
    };

    /**
     * @brief Functors for message data gathering.
     */
    template<class Data, class GatherScatter, bool send, typename IndexedTypeFlag>
    struct MessageGatherer
    {};

    /**
     * @brief Functor for message data gathering for datatypes
     * where at each index is only one value.
     */
    template<class Data, class GatherScatter, bool send>
    struct MessageGatherer<Data,GatherScatter,send,SizeOne>
    {
      /** @brief The type of the values we send. */
      typedef typename CommPolicy<Data>::IndexedType Type;

      /**
       * @brief The type of the functor that does the actual copying
       * during the data Scattering.
       */
      typedef GatherScatter Gatherer;

      /**
       * @brief The communication mode
       *
       * True if this was a forward communication.
       */
      constexpr static bool forward = send;

      /**
       * @brief Copies the values to send into the buffer.
       * @param interface The interface used in the send.
       * @param data The data from which we copy the values.
       * @param buffer The send buffer to copy to.
       * @param bufferSize The size of the buffer in bytes. For checks.
       */
      inline void operator()(const InterfaceMap& interface, const Data& data, Type* buffer, size_t bufferSize) const;
    };

    /**
     * @brief Functor for message data scattering for datatypes
     * where at each index can be a variable size of values
     */
    template<class Data, class GatherScatter, bool send>
    struct MessageGatherer<Data,GatherScatter,send,VariableSize>
    {
      /** @brief The type of the values we send. */
      typedef typename CommPolicy<Data>::IndexedType Type;

      /**
       * @brief The type of the functor that does the actual copying
       * during the data Scattering.
       */
      typedef GatherScatter Gatherer;

      /**
       * @brief The communication mode
       *
       * True if this was a forward communication.
       */
      constexpr static bool forward = send;

      /**
       * @brief Copies the values to send into the buffer.
       * @param interface The interface used in the send.
       * @param data The data from which we copy the values.
       * @param buffer The send buffer to copy to.
       * @param bufferSize The size of the buffer in bytes. For checks.
       */
      inline void operator()(const InterfaceMap& interface, const Data& data, Type* buffer, size_t bufferSize) const;
    };

    /**
     * @brief Functors for message data scattering.
     */
    template<class Data, class GatherScatter, bool send, typename IndexedTypeFlag>
    struct MessageScatterer
    {};

    /**
     * @brief Functor for message data gathering for datatypes
     * where at each index is only one value.
     */
    template<class Data, class GatherScatter, bool send>
    struct MessageScatterer<Data,GatherScatter,send,SizeOne>
    {
      /** @brief The type of the values we send. */
      typedef typename CommPolicy<Data>::IndexedType Type;

      /**
       * @brief The type of the functor that does the actual copying
       * during the data Scattering.
       */
      typedef GatherScatter Scatterer;

      /**
       * @brief The communication mode
       *
       * True if this was a forward communication.
       */
      constexpr static bool forward = send;

      /**
       * @brief Copy the message data from the receive buffer to the data.
       * @param interface The interface used in the send.
       * @param data The data to which we copy the values.
       * @param buffer The receive buffer to copy from.
       * @param proc The rank of the process the message is from.
       */
      inline void operator()(const InterfaceMap& interface, Data& data, Type* buffer, const int& proc) const;
    };
    /**
     * @brief Functor for message data scattering for datatypes
     * where at each index can be a variable size of values
     */
    template<class Data, class GatherScatter, bool send>
    struct MessageScatterer<Data,GatherScatter,send,VariableSize>
    {
      /** @brief The type of the values we send. */
      typedef typename CommPolicy<Data>::IndexedType Type;

      /**
       * @brief The type of the functor that does the actual copying
       * during the data Scattering.
       */
      typedef GatherScatter Scatterer;

      /**
       * @brief The communication mode
       *
       * True if this was a forward communication.
       */
      constexpr static bool forward = send;

      /**
       * @brief Copy the message data from the receive buffer to the data.
       * @param interface The interface used in the send.
       * @param data The data to which we copy the values.
       * @param buffer The receive buffer to copy from.
       * @param proc The rank of the process the message is from.
       */
      inline void operator()(const InterfaceMap& interface, Data& data, Type* buffer, const int& proc) const;
    };

    /**
     * @brief Information about a message to send.
     */
    struct MessageInformation
    {
      /** @brief Constructor. */
      MessageInformation()
        : start_(0), size_(0)
      {}

      /**
       * @brief Constructor.
       * @param start The start of the message in the global buffer.
       * Not in bytes but in number of values from the beginning of
       * the buffer
       * @param size The size of the message in bytes.
       */
      MessageInformation(size_t start, size_t size)
        : start_(start), size_(size)
      {}
      /**
       * @brief Start of the message in the buffer counted in number of value.
       */
      size_t start_;
      /**
       * @brief Number of bytes in the message.
       */
      size_t size_;
    };

    /**
     * @brief Type of the map of information about the messages to send.
     *
     * The key is the process number to communicate with and the value is
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
    /**
     * @brief The size of the communication buffers
     */
    size_t bufferSize_[2];

    /**
     * @brief The tag we use for communication.
     */
    constexpr static int commTag_ = 0;

    /**
     * @brief The interface we currently work with.
     */
    std::map<int,std::pair<InterfaceInformation,InterfaceInformation> > interfaces_;

    MPI_Comm communicator_;

    /**
     * @brief Send and receive Data.
     */
    template<class GatherScatter, bool FORWARD, class Data>
    void sendRecv(const Data& source, Data& target);

  };

#ifndef DOXYGEN

  template<class V>
  inline const void* CommPolicy<V>::getAddress(const V& v, int index)
  {
    return &(v[index]);
  }

  template<class V>
  inline int CommPolicy<V>::getSize([[maybe_unused]] const V& v, [[maybe_unused]] int index)
  {
    return 1;
  }

  template<class K, class A, int n>
  inline const void* CommPolicy<VariableBlockVector<FieldVector<K, n>, A> >::getAddress(const Type& v, int index)
  {
    return &(v[index][0]);
  }

  template<class K, class A, int n>
  inline int CommPolicy<VariableBlockVector<FieldVector<K, n>, A> >::getSize(const Type& v, int index)
  {
    return v[index].getsize();
  }

  template<class T>
  inline const typename CopyGatherScatter<T>::IndexedType& CopyGatherScatter<T>::gather(const T & vec, std::size_t i)
  {
    return vec[i];
  }

  template<class T>
  inline void CopyGatherScatter<T>::scatter(T& vec, const IndexedType& v, std::size_t i)
  {
    vec[i]=v;
  }

  template<typename T>
  DatatypeCommunicator<T>::DatatypeCommunicator()
    : remoteIndices_(0), created_(false)
  {
    requests_[0]=0;
    requests_[1]=0;
  }



  template<typename T>
  DatatypeCommunicator<T>::~DatatypeCommunicator()
  {
    free();
  }

  template<typename T>
  template<class T1, class T2, class V>
  inline void DatatypeCommunicator<T>::build(const RemoteIndices& remoteIndices,
                                             const T1& source, V& sendData,
                                             const T2& destination, V& receiveData)
  {
    remoteIndices_ = &remoteIndices;
    free();
    createDataTypes<T1,T2,V,false>(source,destination, receiveData);
    createDataTypes<T1,T2,V,true>(source,destination, sendData);
    createRequests<V,true>(sendData, receiveData);
    createRequests<V,false>(receiveData, sendData);
    created_=true;
  }

  template<typename T>
  void DatatypeCommunicator<T>::free()
  {
    if(created_) {
      delete[] requests_[0];
      delete[] requests_[1];
      typedef MessageTypeMap::iterator iterator;
      typedef MessageTypeMap::const_iterator const_iterator;

      const const_iterator end=messageTypes.end();

      for(iterator process = messageTypes.begin(); process != end; ++process) {
        MPI_Datatype *type = &(process->second.first);
        int finalized=0;
        MPI_Finalized(&finalized);
        if(*type!=MPI_DATATYPE_NULL && !finalized)
          MPI_Type_free(type);
        type = &(process->second.second);
        if(*type!=MPI_DATATYPE_NULL && !finalized)
          MPI_Type_free(type);
      }
      messageTypes.clear();
      created_=false;
    }

  }

  template<typename T>
  template<class T1, class T2, class V, bool send>
  void DatatypeCommunicator<T>::createDataTypes(const T1& sourceFlags, const T2& destFlags, V& data)
  {

    MPIDatatypeInformation<V>  dataInfo(data);
    this->template buildInterface<RemoteIndices,T1,T2,MPIDatatypeInformation<V>,send>(*remoteIndices_,sourceFlags, destFlags, dataInfo);

    typedef typename RemoteIndices::RemoteIndexMap::const_iterator const_iterator;
    const const_iterator end=this->remoteIndices_->end();

    // Allocate MPI_Datatypes and deallocate memory for the type construction.
    for(const_iterator process=this->remoteIndices_->begin(); process != end; ++process) {
      IndexedTypeInformation& info=dataInfo.information_[process->first];
      // Shift the displacement
      MPI_Aint base;
      MPI_Get_address(const_cast<void *>(CommPolicy<V>::getAddress(data, 0)), &base);

      for(int i=0; i< info.elements; i++) {
        info.displ[i]-=base;
      }

      // Create data type
      MPI_Datatype* type = &( send ? messageTypes[process->first].first : messageTypes[process->first].second);
      MPI_Type_create_hindexed(info.elements, info.length, info.displ,
                               MPITraits<typename CommPolicy<V>::IndexedType>::getType(), type);
      MPI_Type_commit(type);
      // Deallocate memory
      info.free();
    }
  }

  template<typename T>
  template<class V, bool createForward>
  void DatatypeCommunicator<T>::createRequests(V& sendData, V& receiveData)
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

  template<typename T>
  void DatatypeCommunicator<T>::forward()
  {
    sendRecv(requests_[1]);
  }

  template<typename T>
  void DatatypeCommunicator<T>::backward()
  {
    sendRecv(requests_[0]);
  }

  template<typename T>
  void DatatypeCommunicator<T>::sendRecv(MPI_Request* requests)
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
          for(int j = 0; j < messageLength; j++)
            std::cout << message[j];
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
          for(int j = 0; j < messageLength; j++)
            std::cerr << message[j];
        }
      std::cerr<<std::endl;
      success=0;
    }

    MPI_Allreduce(&success, &globalSuccess, 1, MPI_INT, MPI_MIN, this->remoteIndices_->communicator());

    delete[] status;

    if(!globalSuccess)
      DUNE_THROW(CommunicationError, "A communication error occurred!");

  }

  inline BufferedCommunicator::BufferedCommunicator()
  {
    buffers_[0]=0;
    buffers_[1]=0;
    bufferSize_[0]=0;
    bufferSize_[1]=0;
  }

  template<class Data, class Interface>
  typename std::enable_if<std::is_same<SizeOne, typename CommPolicy<Data>::IndexedTypeFlag>::value, void>::type
  BufferedCommunicator::build(const Interface& interface)
  {
    interfaces_=interface.interfaces();
    communicator_=interface.communicator();
    typedef typename std::map<int,std::pair<InterfaceInformation,InterfaceInformation> >
    ::const_iterator const_iterator;
    typedef typename CommPolicy<Data>::IndexedTypeFlag Flag;
    const const_iterator end = interfaces_.end();
    int lrank;
    MPI_Comm_rank(communicator_, &lrank);

    bufferSize_[0]=0;
    bufferSize_[1]=0;

    for(const_iterator interfacePair = interfaces_.begin();
        interfacePair != end; ++interfacePair) {
      int noSend = MessageSizeCalculator<Data,Flag>() (interfacePair->second.first);
      int noRecv = MessageSizeCalculator<Data,Flag>() (interfacePair->second.second);
      if (noSend + noRecv > 0)
        messageInformation_.insert(std::make_pair(interfacePair->first,
                                                std::make_pair(MessageInformation(bufferSize_[0],
                                                                                  noSend*sizeof(typename CommPolicy<Data>::IndexedType)),
                                                               MessageInformation(bufferSize_[1],
                                                                                  noRecv*sizeof(typename CommPolicy<Data>::IndexedType)))));
      bufferSize_[0] += noSend;
      bufferSize_[1] += noRecv;
    }

    // allocate the buffers
    bufferSize_[0] *= sizeof(typename CommPolicy<Data>::IndexedType);
    bufferSize_[1] *= sizeof(typename CommPolicy<Data>::IndexedType);

    buffers_[0] = new char[bufferSize_[0]];
    buffers_[1] = new char[bufferSize_[1]];
  }

  template<class Data, class Interface>
  void BufferedCommunicator::build(const Data& source, const Data& dest, const Interface& interface)
  {

    interfaces_=interface.interfaces();
    communicator_=interface.communicator();
    typedef typename std::map<int,std::pair<InterfaceInformation,InterfaceInformation> >
    ::const_iterator const_iterator;
    typedef typename CommPolicy<Data>::IndexedTypeFlag Flag;
    const const_iterator end = interfaces_.end();

    bufferSize_[0]=0;
    bufferSize_[1]=0;

    for(const_iterator interfacePair = interfaces_.begin();
        interfacePair != end; ++interfacePair) {
      int noSend = MessageSizeCalculator<Data,Flag>() (source, interfacePair->second.first);
      int noRecv = MessageSizeCalculator<Data,Flag>() (dest, interfacePair->second.second);
      if (noSend + noRecv > 0)
        messageInformation_.insert(std::make_pair(interfacePair->first,
                                                std::make_pair(MessageInformation(bufferSize_[0],
                                                                                  noSend*sizeof(typename CommPolicy<Data>::IndexedType)),
                                                               MessageInformation(bufferSize_[1],
                                                                                  noRecv*sizeof(typename CommPolicy<Data>::IndexedType)))));
      bufferSize_[0] += noSend;
      bufferSize_[1] += noRecv;
    }

    bufferSize_[0] *= sizeof(typename CommPolicy<Data>::IndexedType);
    bufferSize_[1] *= sizeof(typename CommPolicy<Data>::IndexedType);
    // allocate the buffers
    buffers_[0] = new char[bufferSize_[0]];
    buffers_[1] = new char[bufferSize_[1]];
  }

  inline void BufferedCommunicator::free()
  {
    messageInformation_.clear();
    if(buffers_[0])
      delete[] buffers_[0];

    if(buffers_[1])
      delete[] buffers_[1];
    buffers_[0]=buffers_[1]=0;
  }

  inline BufferedCommunicator::~BufferedCommunicator()
  {
    free();
  }

  template<class Data>
  inline int BufferedCommunicator::MessageSizeCalculator<Data,SizeOne>::operator()
    (const InterfaceInformation& info) const
  {
    return info.size();
  }


  template<class Data>
  inline int BufferedCommunicator::MessageSizeCalculator<Data,SizeOne>::operator()
    (const Data&, const InterfaceInformation& info) const
  {
    return operator()(info);
  }


  template<class Data>
  inline int BufferedCommunicator::MessageSizeCalculator<Data, VariableSize>::operator()
    (const Data& data, const InterfaceInformation& info) const
  {
    int entries=0;

    for(size_t i=0; i < info.size(); i++)
      entries += CommPolicy<Data>::getSize(data,info[i]);

    return entries;
  }


  template<class Data, class GatherScatter, bool FORWARD>
  inline void BufferedCommunicator::MessageGatherer<Data,GatherScatter,FORWARD,VariableSize>::operator()(const InterfaceMap& interfaces,const Data& data, Type* buffer, [[maybe_unused]] size_t bufferSize) const
  {
    typedef typename InterfaceMap::const_iterator
    const_iterator;

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    const const_iterator end = interfaces.end();
    size_t index=0;

    for(const_iterator interfacePair = interfaces.begin();
        interfacePair != end; ++interfacePair) {
      int size = forward ? interfacePair->second.first.size() :
                 interfacePair->second.second.size();

      for(int i=0; i < size; i++) {
        int local = forward ? interfacePair->second.first[i] :
                    interfacePair->second.second[i];
        for(std::size_t j=0; j < CommPolicy<Data>::getSize(data, local); j++, index++) {

#ifdef DUNE_ISTL_WITH_CHECKING
          assert(bufferSize>=(index+1)*sizeof(typename CommPolicy<Data>::IndexedType));
#endif
          buffer[index]=GatherScatter::gather(data, local, j);
        }

      }
    }

  }


  template<class Data, class GatherScatter, bool FORWARD>
  inline void BufferedCommunicator::MessageGatherer<Data,GatherScatter,FORWARD,SizeOne>::operator()(
    const InterfaceMap& interfaces, const Data& data, Type* buffer, [[maybe_unused]] size_t bufferSize) const
  {
    typedef typename InterfaceMap::const_iterator
    const_iterator;
    const const_iterator end = interfaces.end();
    size_t index = 0;

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    for(const_iterator interfacePair = interfaces.begin();
        interfacePair != end; ++interfacePair) {
      size_t size = FORWARD ? interfacePair->second.first.size() :
                    interfacePair->second.second.size();

      for(size_t i=0; i < size; i++) {

#ifdef DUNE_ISTL_WITH_CHECKING
        assert(bufferSize>=(index+1)*sizeof(typename CommPolicy<Data>::IndexedType));
#endif

        buffer[index++] = GatherScatter::gather(data, FORWARD ? interfacePair->second.first[i] :
                                                interfacePair->second.second[i]);
      }
    }

  }


  template<class Data, class GatherScatter, bool FORWARD>
  inline void BufferedCommunicator::MessageScatterer<Data,GatherScatter,FORWARD,VariableSize>::operator()(const InterfaceMap& interfaces, Data& data, Type* buffer, const int& proc) const
  {
    typedef typename InterfaceMap::value_type::second_type::first_type Information;
    const typename InterfaceMap::const_iterator infoPair = interfaces.find(proc);

    assert(infoPair!=interfaces.end());

    const Information& info = FORWARD ? infoPair->second.second :
                              infoPair->second.first;

    for(size_t i=0, index=0; i < info.size(); i++) {
      for(size_t j=0; j < CommPolicy<Data>::getSize(data, info[i]); j++)
        GatherScatter::scatter(data, buffer[index++], info[i], j);
    }
  }


  template<class Data, class GatherScatter, bool FORWARD>
  inline void BufferedCommunicator::MessageScatterer<Data,GatherScatter,FORWARD,SizeOne>::operator()(const InterfaceMap& interfaces, Data& data, Type* buffer, const int& proc) const
  {
    typedef typename InterfaceMap::value_type::second_type::first_type Information;
    const typename InterfaceMap::const_iterator infoPair = interfaces.find(proc);

    assert(infoPair!=interfaces.end());

    const Information& info = FORWARD ? infoPair->second.second :
                              infoPair->second.first;

    for(size_t i=0; i < info.size(); i++) {
      GatherScatter::scatter(data, buffer[i], info[i]);
    }
  }


  template<class GatherScatter,class Data>
  void BufferedCommunicator::forward(Data& data)
  {
    this->template sendRecv<GatherScatter,true>(data, data);
  }


  template<class GatherScatter, class Data>
  void BufferedCommunicator::backward(Data& data)
  {
    this->template sendRecv<GatherScatter,false>(data, data);
  }


  template<class GatherScatter, class Data>
  void BufferedCommunicator::forward(const Data& source, Data& dest)
  {
    this->template sendRecv<GatherScatter,true>(source, dest);
  }


  template<class GatherScatter, class Data>
  void BufferedCommunicator::backward(Data& source, const Data& dest)
  {
    this->template sendRecv<GatherScatter,false>(dest, source);
  }


  template<class GatherScatter, bool FORWARD, class Data>
  void BufferedCommunicator::sendRecv(const Data& source, Data& dest)
  {
    int rank, lrank;

    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_rank(MPI_COMM_WORLD,&lrank);

    typedef typename CommPolicy<Data>::IndexedType Type;
    Type *sendBuffer, *recvBuffer;
    size_t sendBufferSize;
#ifndef NDEBUG
    size_t recvBufferSize;
#endif

    if(FORWARD) {
      sendBuffer = reinterpret_cast<Type*>(buffers_[0]);
      sendBufferSize = bufferSize_[0];
      recvBuffer = reinterpret_cast<Type*>(buffers_[1]);
#ifndef NDEBUG
      recvBufferSize = bufferSize_[1];
#endif
    }else{
      sendBuffer = reinterpret_cast<Type*>(buffers_[1]);
      sendBufferSize = bufferSize_[1];
      recvBuffer = reinterpret_cast<Type*>(buffers_[0]);
#ifndef NDEBUG
      recvBufferSize = bufferSize_[0];
#endif
    }
    typedef typename CommPolicy<Data>::IndexedTypeFlag Flag;

    MessageGatherer<Data,GatherScatter,FORWARD,Flag>() (interfaces_, source, sendBuffer, sendBufferSize);

    MPI_Request* sendRequests = new MPI_Request[messageInformation_.size()];
    MPI_Request* recvRequests = new MPI_Request[messageInformation_.size()];
    /* Number of recvRequests that are not MPI_REQUEST_NULL */
    size_t numberOfRealRecvRequests = 0;

    // Setup receive first
    typedef typename InformationMap::const_iterator const_iterator;

    const const_iterator end = messageInformation_.end();
    size_t i=0;
    int* processMap = new int[messageInformation_.size()];

    for(const_iterator info = messageInformation_.begin(); info != end; ++info, ++i) {
      processMap[i]=info->first;
      if(FORWARD) {
        assert(info->second.second.start_*sizeof(typename CommPolicy<Data>::IndexedType)+info->second.second.size_ <= recvBufferSize );
        Dune::dvverb<<rank<<": receiving "<<info->second.second.size_<<" from "<<info->first<<std::endl;
        if(info->second.second.size_) {
          MPI_Irecv(recvBuffer+info->second.second.start_, info->second.second.size_,
                    MPI_BYTE, info->first, commTag_, communicator_,
                    recvRequests+i);
          numberOfRealRecvRequests += 1;
        } else {
          // Nothing to receive -> set request to inactive
          recvRequests[i]=MPI_REQUEST_NULL;
        }
      }else{
        assert(info->second.first.start_*sizeof(typename CommPolicy<Data>::IndexedType)+info->second.first.size_ <= recvBufferSize );
        Dune::dvverb<<rank<<": receiving "<<info->second.first.size_<<" to "<<info->first<<std::endl;
        if(info->second.first.size_) {
          MPI_Irecv(recvBuffer+info->second.first.start_, info->second.first.size_,
                    MPI_BYTE, info->first, commTag_, communicator_,
                    recvRequests+i);
          numberOfRealRecvRequests += 1;
        } else {
          // Nothing to receive -> set request to inactive
          recvRequests[i]=MPI_REQUEST_NULL;
        }
      }
    }

    // now the send requests
    i=0;
    for(const_iterator info = messageInformation_.begin(); info != end; ++info, ++i)
      if(FORWARD) {
        assert(info->second.second.start_*sizeof(typename CommPolicy<Data>::IndexedType)+info->second.second.size_ <= recvBufferSize );
        Dune::dvverb<<rank<<": sending "<<info->second.first.size_<<" to "<<info->first<<std::endl;
        assert(info->second.first.start_*sizeof(typename CommPolicy<Data>::IndexedType)+info->second.first.size_ <= sendBufferSize );
        if(info->second.first.size_)
          MPI_Issend(sendBuffer+info->second.first.start_, info->second.first.size_,
                     MPI_BYTE, info->first, commTag_, communicator_,
                     sendRequests+i);
        else
          // Nothing to send -> set request to inactive
          sendRequests[i]=MPI_REQUEST_NULL;
      }else{
        assert(info->second.second.start_*sizeof(typename CommPolicy<Data>::IndexedType)+info->second.second.size_ <= sendBufferSize );
        Dune::dvverb<<rank<<": sending "<<info->second.second.size_<<" to "<<info->first<<std::endl;
        if(info->second.second.size_)
          MPI_Issend(sendBuffer+info->second.second.start_, info->second.second.size_,
                     MPI_BYTE, info->first, commTag_, communicator_,
                     sendRequests+i);
        else
          // Nothing to send -> set request to inactive
          sendRequests[i]=MPI_REQUEST_NULL;
      }

    // Wait for completion of receive and immediately start scatter
    i=0;
    //int success = 1;
    int finished = MPI_UNDEFINED;
    MPI_Status status; //[messageInformation_.size()];
    //MPI_Waitall(messageInformation_.size(), recvRequests, status);

    for(i=0; i< numberOfRealRecvRequests; i++) {
      status.MPI_ERROR=MPI_SUCCESS;
      MPI_Waitany(messageInformation_.size(), recvRequests, &finished, &status);
      assert(finished != MPI_UNDEFINED);

      if(status.MPI_ERROR==MPI_SUCCESS) {
        int& proc = processMap[finished];
        typename InformationMap::const_iterator infoIter = messageInformation_.find(proc);
        assert(infoIter != messageInformation_.end());

        MessageInformation info = (FORWARD) ? infoIter->second.second : infoIter->second.first;
        assert(info.start_+info.size_ <= recvBufferSize);

        MessageScatterer<Data,GatherScatter,FORWARD,Flag>() (interfaces_, dest, recvBuffer+info.start_, proc);
      }else{
        std::cerr<<rank<<": MPI_Error occurred while receiving message from "<<processMap[finished]<<std::endl;
        //success=0;
      }
    }

    MPI_Status recvStatus;

    // Wait for completion of sends
    for(i=0; i< messageInformation_.size(); i++)
      if(MPI_SUCCESS!=MPI_Wait(sendRequests+i, &recvStatus)) {
        std::cerr<<rank<<": MPI_Error occurred while sending message to "<<processMap[finished]<<std::endl;
        //success=0;
      }
    /*
       int globalSuccess;
       MPI_Allreduce(&success, &globalSuccess, 1, MPI_INT, MPI_MIN, interface_->communicator());

       if(!globalSuccess)
       DUNE_THROW(CommunicationError, "A communication error occurred!");
     */
    delete[] processMap;
    delete[] sendRequests;
    delete[] recvRequests;

  }

#endif  // DOXYGEN

  /** @} */
}

#endif // HAVE_MPI

#endif
