// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_PARALLEL_COLLECTIVECOMMUNICATION_HH
#define DUNE_COMMON_PARALLEL_COLLECTIVECOMMUNICATION_HH
/*!
   \file
   \brief Implements an utility class that provides
   collective communication methods for sequential programs.

   \ingroup ParallelCommunication
 */

#include <algorithm>
#include <memory>

#include <dune/common/binaryfunctions.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/unused.hh>
#include <dune/common/deprecated.hh>

#include "managedmpicomm.hh"
#include "span.hh"
#include "pseudofuture.hh"
/*! \defgroup ParallelCommunication Parallel Communication
   \ingroup Common

   \brief Abstractions for parallel computing

   Dune offers an abstraction to the basic methods of parallel
   communication. It allows one to switch parallel features on and off,
   without changing the code. This is done using either CollectiveCommunication
   or MPICollectiveCommunication.

 */

namespace Dune
{

  /*! @brief Collective communication interface and sequential default implementation

     CollectiveCommunication offers an abstraction to the basic methods
     of parallel communication, following the message-passing
     paradigm. It allows one to switch parallel features on and off, without
     changing the code. Currently only MPI and sequential code are
     supported.

     A CollectiveCommunication object is returned by all grids (also
     the sequential ones) in order to allow code to be written in
     a transparent way for sequential and parallel grids.

     This class provides a default implementation for sequential grids.
     The number of processes involved is 1, any sum, maximum, etc. returns
     just its input argument and so on.

     In specializations one can implement the real thing using appropriate
     communication functions, e.g. there exists an implementation using
     the Message Passing %Interface (MPI),
     see Dune::CollectiveCommunication<ManagedMPIComm>.

     Moreover, the communication subsystem used by an implementation
     is not visible in the interface, i.e. Dune grid implementations
     are not restricted to MPI.

     \tparam Communicator The communicator type used by your
       message-passing implementation.  For MPI this will be
       Dune::ManagedMPIComm.  For sequential codes there is the dummy
       communicator NoComm.  It is assumed that if you want to
       specialize the CollectiveCommunication class for a
       message-passing system other than MPI, that message-passing
       system will have something equivalent to MPI communicators.

     \ingroup ParallelCommunication
   */
  template<typename Communicator>
  class CollectiveCommunication;

  template<>
  class CollectiveCommunication<NoComm>
  {
  public:
    // Export Future type
    template<class T>
    using FutureType = PseudoFuture<T>;
    //! Construct default object
    CollectiveCommunication()
    {}

    /** \brief Constructor with a given communicator
     *
     * As this is implementation for the sequential setting, the communicator is a dummy and simply discarded.
     */
    CollectiveCommunication (const NoComm&)
    {}

    //! Return rank, is between 0 and size()-1
    int rank () const
    {
      return 0;
    }

    //! Number of processes in set, is greater than 0
    int size () const
    {
      return 1;
    }

    /**
     * @brief Compute the sum of the argument over all processes and
     * return the result in every process. Assumes that T has an operator+
     *
     * @param in
     *
     * @return sum of \p in over all processes
     */
    template<typename T>
    T sum (const T& in) const
    {
      return in;
    }

    /** \copybrief sum(const T&)const
     *
     * @param inout pointer to data
     * @param len number of elements
     *
     * @return error code
     *
     * \deprecated Use sum(const T&) const instead
     */
    template<typename T>
    DUNE_DEPRECATED
    int sum (T* inout, int len) const
    {
      DUNE_UNUSED_PARAMETER(inout);
      DUNE_UNUSED_PARAMETER(len);
      return 0;
    }

    /**
     * @brief  Compute the product of the argument over all processes and
     * return the result in every process. Assumes that T has an operator*
     *
     * @param in
     *
     * @return Product of \p in over all processes
     */
    template<typename T>
    T prod (const T& in) const
    {
      return in;
    }

    /** \copybrief prod(const T&)const
     *
     *
     * @param inout
     * @param len
     *
     * @throw MPIError
     *
     * \deprecated Use prod(const T&) const instead
     */
    template<typename T>
    DUNE_DEPRECATED
    int prod (T* inout, int len) const
    {
      DUNE_UNUSED_PARAMETER(inout);
      DUNE_UNUSED_PARAMETER(len);
      return 0;
    }

    /** @brief Compute the minimum of the argument over all processes
     * and return the result in every process. Assumes that T has an
     * operator<
     *
     * @param in
     *
     * @return Minimum of \p in over all processes
     */
    template<typename T>
    T min (const T& in) const
    {
      return in;
    }

    /** \copybrief min(const T&)const
     *
     * @param inout
     * @param len
     *
     * @return error code
     * @throw MPIError
     *
     * \deprecated Use min(const T&) const instead
     */
    template<typename T>
    DUNE_DEPRECATED
    int min (T* inout, int len) const
    {
      DUNE_UNUSED_PARAMETER(inout);
      DUNE_UNUSED_PARAMETER(len);
      return 0;
    }

    /** @brief Compute the maximum of the argument over all processes
     *  and return the result in every process. Assumes that T has an
     *  operator<
     *
     * @param in
     *
     * @return
     */
    template<typename T>
    T max (const T& in) const
    {
      return in;
    }

    /** \copybrief max(const T&)const
     *
     * @param inout
     * @param len
     *
     * @return error code
     * @throw MPIError
     *
     * \deprecated Use max(const T&) const instead
     */
    template<typename T>
    DUNE_DEPRECATED
    int max (T* inout, int len) const
    {
      DUNE_UNUSED_PARAMETER(inout);
      DUNE_UNUSED_PARAMETER(len);
      return 0;
    }

    /** @brief Wait until all processes have arrived at this point in
     *  the program. (See MPI_Barrier)
     *
     *  @throw MPIError
     */
    void barrier () const
    {
      return;
    }

    /** @brief Non-Blocking Barrier. (See MPI_Ibarrier)
     *
     * @return Futures which is ready when all process reached the
     * barrier.
     * @throw MPIError
     */
    FutureType<void> ibarrier () const
    {
      return {};
    }

    /** @brief Distribute an array from the process with rank root to
     * all other processes. (See MPI_Broadcast)
     *
     * @param inout
     * @param root
     *
     * @throw MPIError
     */
    template<typename T>
    void broadcast (T& inout, int root) const
    {
      DUNE_UNUSED_PARAMETER(inout);
      DUNE_UNUSED_PARAMETER(root);
    }

    /** \copybrief broadcast(T&,int)const
     *
     * @param inout
     * @param len
     * @param root
     *
     * @return error code
     * @throw MPIError
     *
     * \deprecated Use broadcast(T&, int) const instead
     */
    template<typename T>
    DUNE_DEPRECATED
    int broadcast (T* inout, int len, int root) const
    {
      DUNE_UNUSED_PARAMETER(inout);
      DUNE_UNUSED_PARAMETER(len);
      DUNE_UNUSED_PARAMETER(root);
      return 0;
    }

    /** @brief Non-blocking broadcast
     *  @throw MPIError
     *
     *
     * @param[in] data
     * @param[in] root
     *
     * @return Future containing the data
     */
    template<typename T>
    FutureType<std::decay_t<T>> ibroadcast (T&& data, int root) const
    {
      DUNE_UNUSED_PARAMETER(root);
      return {true, std::forward<T>(data)};
    }

    /** @brief  Gather data on root task.
     *
     * Each process sends its in array of length len to the root process
     * (including the root itself). In the root process these arrays are stored in rank
     * order in the out array which must have size len * number of processes.
     * @param[in] in The send buffer with the data to send.
     * @param[out] out The buffer to store the received data in. Might have length zero on non-root
     *                  tasks.
     * @param[in] root The root task that gathers the data.
     * @throw MPIError
     */
    template<typename T, typename S>
    void gather (const T& in, S& out, int root) const     // note out must have same size as in
    {
      DUNE_UNUSED_PARAMETER(root);
      out = in;
      return;
    }

    /** \copybrief gather(const T&,S&,int)const
     * Each process sends its in array of length len to the root process
     * (including the root itself). In the root process these arrays are stored in rank
     * order in the out array which must have size len * number of processes.
     *
     * @param[in] in The send buffer with the data to send.
     * @param[out] out The buffer to store the received data in. Might have length zero on non-root
     *                  tasks.
     * @param[in] len The number of elements to send on each task.
     * @param[in] root The root task that gathers the data.
     * @throw MPIError
     *
     * \deprecated Use gather(const T&, S&, int) const instead
     */
    template<typename T, typename S>
    DUNE_DEPRECATED
    int gather (T* in, S* out, int len, int root) const     // note out must have same size as in
    {
      DUNE_UNUSED_PARAMETER(root);
      for(int i = 0; i<len; i++)
        out[i] = in[i];
      return 0;
    }

    /** @brief Non-blocking. Gather arrays on root task.
     *
     * Each process sends its in array of length len to the root process
     * (including the root itself). In the root process these arrays are stored in rank
     * order in the out array which must have size len * number of processes.
     * @param[in] in The send buffer with the data to send.
     * @param[in] out The buffer to store the received data in. Might have length zero on non-root
     *                  tasks.
     * @param[in] root The root task that gathers the data.
     * @return Future containing the gathered data when ready
     * @warning \p in should not be modified until the returned
     * future is ready.
     * @throw MPIError
     */
    template<typename T, typename S>
    FutureType<std::decay<S>> igather (const T& in, S&& out, int root) const
    {
      DUNE_UNUSED_PARAMETER(root);
      FutureType<std::decay_t<S>> f(false, std::forward<S>(out));
      f.buffer() = in;
      return f;
    }

    /** @brief  Gather arrays of variable size on root task.
     *
     * Each process sends its in array of length sendlen to the root process
     * (including the root itself). In the root process these arrays are stored in rank
     * order in the out array.
     * @param[in] in The send buffer with the data to be sent
     * @param[in] sendlen The number of elements to send on each task
     * @param[out] out The buffer to store the received data in. May have length zero on non-root
     *                 tasks.
     * @param[in] recvlen An array with size equal to the number of processes containing the number
     *                    of elements to receive from process i at position i, i.e. the number that
     *                    is passed as sendlen argument to this function in process i.
     *                    May have length zero on non-root tasks.
     * @param[out] displ An array with size equal to the number of processes. Data received from
     *                  process i will be written starting at out+displ[i] on the root process.
     *                  May have length zero on non-root tasks.
     * @param[in] root The root task that gathers the data.
     * @throw MPIError
     *
     * \deprecated Use gatherv(const T&, S&, Span<int*>, Span<int*>, int) const instead
     */
    template<typename T, typename S>
    DUNE_DEPRECATED
    int gatherv (T* in, int sendlen, S* out, int* recvlen, int* displ, int root) const
    {
      DUNE_UNUSED_PARAMETER(recvlen);
      DUNE_UNUSED_PARAMETER(displ);
      DUNE_UNUSED_PARAMETER(root);
      for(int i = 0; i<sendlen; i++)
        out[i] = in[i];
      return 0;
    }

    /** \copybrief gatherv(T*,int,S*,int*,int*,int)const
     * Each process sends its in array of length sendlen to the root process
     * (including the root itself). In the root process these arrays are stored in rank
     * order in the out array.
     * @param[in] in The send buffer with the data to be sent
     * @param[out] out The buffer to store the received data in. May have length zero on non-root
     *                 tasks.
     * @param[in] recvlen An array with size equal to the number of processes containing the number
     *                    of elements to receive from process i at position i, i.e. the number that
     *                    is passed as sendlen argument to this function in process i.
     *                    May have length zero on non-root tasks.
     * @param[out] displ An array with size equal to the number of processes. Data received from
     *                  process i will be written starting at out+displ[i] on the root process.
     *                  May have length zero on non-root tasks.
     * @param[in] root The root task that gathers the data.
     * @warning \p in should not be modified until the returned
     * future is ready.
     * @throw MPIError
     */
    template<typename T, typename S>
    void gatherv (const T& in, S& out, Span<int*> recvlen, Span<int*> displ, int root) const
    {
      DUNE_UNUSED_PARAMETER(recvlen);
      DUNE_UNUSED_PARAMETER(displ);
      DUNE_UNUSED_PARAMETER(root);
      out = in;
      return;
    }

    /** @brief Scatter array from a root to all other task.
     *
     * The root process sends the elements with index from k*len to (k+1)*len-1 in its array to
     * task k, which stores it at index 0 to len-1.
     *
     * @param[in] send The array to scatter. Might have length zero on non-root
     *                  tasks.
     * @param[out] recv The buffer to store the received data in. Upon completion of the
     *                 method each task will have same data stored there as the one in
     *                 send buffer of the root task before.
     * @param[in] len The number of elements in the recv buffer.
     * @param[in] root The root task that gathers the data.
     * @throw MPIError
     *
     * \deprecated Use scatter(const T&, S&, int) const instead
     */
    template<typename T, typename S>
    DUNE_DEPRECATED
    int scatter (T* send, S* recv, int len, int root) const // note out must have same size as in
    {
      DUNE_UNUSED_PARAMETER(root);
      for(int i = 0; i<len; i++)
        recv[i] = send[i];
      return 0;
    }

    /** \copybrief scatter(T*,S*,int,int)const
     *
     * The root process sends the elements with index from k*len to (k+1)*len-1 in its array to
     * task k, which stores it at index 0 to len-1.
     *
     * @param[in] send The array to scatter. Might have length zero on non-root
     *                  tasks.
     * @param[out] recv The buffer to store the received data in. Upon completion of the
     *                 method each task will have same data stored there as the one in
     *                 send buffer of the root task before.
     * @param[in] root The root task that gathers the data.
     * @throw MPIError
     */
    template<typename T, typename S>
    void scatter (const T& send, S& recv, int root) const // note out must have same size as in
    {
      DUNE_UNUSED_PARAMETER(root);
      recv = send;
      return;
    }

    /** @brief Non-Blocking. Scatter array from a root to all other task.
     *
     * The root process sends the elements with index from k*len to (k+1)*len-1 in its array to
     * task k, which stores it at index 0 to len-1.
     * @param[in] send The array to scatter. Might have length zero on non-root
     *                  tasks.
     * @param[out] recv The buffer to store the received data in. Upon completion of the
     *                 method each task will have same data stored there as the one in
     *                 send buffer of the root task before.
     * @param[in] root The root task that gathers the data.
     * @warning \p send should not be modified until the returned
     * future is ready.
     * @throw MPIError
     */
    template<typename T, typename S>
    FutureType<std::decay_t<S>> iscatter (const T& send, S&& recv, int root) const // note: out must have same size as in
    {
      DUNE_UNUSED_PARAMETER(root);
      FutureType<std::decay_t<S>> f(false, std::forward<S>(recv));
      f.buffer() = send;
      return f;
    }

    /** @brief Scatter arrays of variable length from a root to all other tasks.
     *
     * The root process sends the elements with index from send+displ[k] to send+displ[k]-1 in
     * its array to task k, which stores it at index 0 to recvlen-1.
     * @param[in] send The array to scatter. May have length zero on non-root
     *                  tasks.
     * @param[in] sendlen An array with size equal to the number of processes containing the number
     *                    of elements to scatter to process i at position i, i.e. the number that
     *                    is passed as recvlen argument to this function in process i.
     * @param[in] displ An array with size equal to the number of processes. Data scattered to
     *                  process i will be read starting at send+displ[i] on root the process.
     * @param[out] recv The buffer to store the received data in. Upon completion of the
     *                  method each task will have the same data stored there as the one in
     *                  send buffer of the root task before.
     * @param[in] recvlen The number of elements in the recv buffer.
     * @param[in] root The root task that gathers the data.
     * @throw MPIError
     *
     * \deprecated Use scatterv(const T&, S&, Span<int*>, Span<int*>, int) const instead
     */
    template<typename T, typename S>
    DUNE_DEPRECATED
    int scatterv (T* send, int* sendlen, int* displ, S* recv, int recvlen, int root) const
    {
      DUNE_UNUSED_PARAMETER(sendlen);
      DUNE_UNUSED_PARAMETER(displ);
      DUNE_UNUSED_PARAMETER(recvlen);
      DUNE_UNUSED_PARAMETER(root);
      for(int i = 0; i<recvlen; i++)
        recv[i] = send[i];
      return 0;
    }

    /** \copybrief scatterv(T*,int*,int*,S*,int,int)const
     *
     * The root process sends the elements with index from send+displ[k] to send+displ[k]-1 in
     * its array to task k, which stores it at index 0 to recvlen-1.
     *
     * @param[in] send The array to scatter. May have length zero on non-root
     *                  tasks.
     * @param[in] sendlen An array with size equal to the number of processes containing the number
     *                    of elements to scatter to process i at position i, i.e. the number that
     *                    is passed as recvlen argument to this function in process i.
     * @param[in] displ An array with size equal to the number of processes. Data scattered to
     *                  process i will be read starting at send+displ[i] on root the process.
     * @param[out] recv The buffer to store the received data in. Upon completion of the
     *                  method each task will have the same data stored there as the one in
     *                  send buffer of the root task before.
     * @param[in] root The root task that gathers the data.
     * @throw MPIError
     */
    template<typename T, typename S>
    void scatterv (const T& send, S& recv, Span<int*> sendlen, Span<int*> displ, int root) const
    {
      DUNE_UNUSED_PARAMETER(sendlen);
      DUNE_UNUSED_PARAMETER(displ);
      DUNE_UNUSED_PARAMETER(root);
      recv = send;
      return;
    }

    /**
     * @brief Gathers data from all tasks and distribute it to all.
     *
     * The block of data sent from the  jth  process  is  received  by  every
     *  process and placed in the jth block of the buffer recvbuf.
     *
     * @param[in] send The buffer with the data to send. Has to be the same for
     *                 each task.
     * @param[in] count The number of elements to send by any process.
     * @param[out] recv The receive buffer for the data. Has to be of size
     *  notasks*count, with notasks being the number of tasks in the communicator.
     * @throw MPIError
     *
     * \deprecated Use allgather(const T&, S&) const instead
     */
    template<typename T, typename S>
    DUNE_DEPRECATED
    int allgather(T* send, int count, S* recv) const
    {
      for(int i = 0; i < count; i++)
        recv[i] = send[i];
      return 0;
    }

    /** \copybrief allgather(T*,int,S*)
     *
     * The block of data sent from the  jth  process  is  received  by  every
     *  process and placed in the jth block of the buffer recvbuf.
     *
     * @param[in] send The buffer with the data to send. Has to be the same for
     *                 each task.
     * @param[out] recv The receive buffer for the data. Has to be of size
     *  notasks*count, with notasks being the number of tasks in the communicator.
     * @throw MPIError
     */
    template<typename T, typename S>
    void allgather(const T& send, S& recv) const
    {
      recv = send;
      return;
    }

    /**
     * @brief Non-blocking. Gathers data from all tasks and distribute it to all.
     *
     * The block of data sent from the  jth  process  is  received  by  every
     *  process and placed in the jth block of the buffer recvbuf.
     *
     * @param[in] send The buffer with the data to send. Has to be the same for
     *                 each task.
     * @param[in] recv The receive buffer for the data. Has to be of size
     *  notasks*count, with notasks being the number of tasks in the communicator.
     * @warning \p send should not be modified until the returned
     * future is ready.
     * @throw MPIError
     */
    template<typename T, typename S>
    FutureType<std::decay_t<S>> iallgather(const T& send, S&& recv) const
    {
      return igather(send, std::forward<S>(recv));
    }

    /**
     * @brief Gathers data of variable length from all tasks and distribute it to all.
     *
     * The block of data sent from the jth process is received by every
     * process and placed in the jth block of the buffer out.
     *
     * @param[in] in The send buffer with the data to send.
     * @param[in] sendlen The number of elements to send on each task.
     * @param[out] out The buffer to store the received data in.
     * @param[in] recvlen An array with size equal to the number of processes containing the number
     *                    of elements to recieve from process i at position i, i.e. the number that
     *                    is passed as sendlen argument to this function in process i.
     * @param[in] displ An array with size equal to the number of processes. Data recieved from
     *                  process i will be written starting at out+displ[i].
     * @throw MPIError
     *
     * \deprecated Use allgatherv(const T&, S&, Span<int*>, Span<int*>) const instead
     */
    template<typename T, typename S>
    DUNE_DEPRECATED
    int allgatherv (T* in, int sendlen, S* out, int* recvlen, int* displ) const
    {
      DUNE_UNUSED_PARAMETER(recvlen);
      DUNE_UNUSED_PARAMETER(displ);
      for(int i = 0; i<sendlen; i++)
        out[i] = in[i];
      return 0;
    }

    /** \copybrief allgatherv(T*,int,S*,int*,int*)const
      * The block of data sent from the jth process is received by every
     * process and placed in the jth block of the buffer out.
     *
     * @param[in] in The send buffer with the data to send.
     * @param[out] out The buffer to store the received data in.
     * @param[in] recvlen An array with size equal to the number of processes containing the number
     *                    of elements to recieve from process i at position i, i.e. the number that
     *                    is passed as sendlen argument to this function in process i.
     * @param[in] displ An array with size equal to the number of processes. Data recieved from
     *                  process i will be written starting at out+displ[i].
     * @throw MPIError
     */
    template<typename T, typename S>
    void allgatherv (const T& in, S& out, Span<int*> recvlen, Span<int*> displ) const
    {
      DUNE_UNUSED_PARAMETER(recvlen);
      DUNE_UNUSED_PARAMETER(displ);
      out = in;
      return;
    }

    /**
     * @brief Compute something over all processes
     * for each component of an array and return the result
     * in every process.
     *
     * @tparam BinaryFunction Function class which implements operator()
     *
     * @param inout The array to compute on.
     * @param len The number of components in the array
     * @throw MPIError
     *
     * \deprecated Use allreduce(T&) const instead
     */
    template<typename BinaryFunction, typename T>
    DUNE_DEPRECATED
    int allreduce(T* inout, int len) const
    {
      DUNE_UNUSED_PARAMETER(inout);
      DUNE_UNUSED_PARAMETER(len);
      return 0;
    }

    /** \copybrief allreduce(T*,int)const
     *
     * @tparam BinaryFunction Function class which implements operator()
     *
     * @param[in,out] inout The array to compute on.
     * @throw MPIError
     */
    template<typename BinaryFunction, typename T>
    void allreduce(T& inout) const
    {
      DUNE_UNUSED_PARAMETER(inout);
      return;
    }

    /**
     * @brief Non-blocking. Compute something over all processes
     * for each component of an array and return the result
     * in every process.
     *
     * @tparam BinaryFunction Function class which implements operator()
     *
     * @param[in] data The array to compute on.
     * @throw MPIError
     */
    template<typename BinaryFunction, typename T>
    FutureType<std::decay_t<T>> iallreduce(T&& data) const
    {
      return ireduce(std::forward<T>(data), 0);
    }

    /** \copybrief allreduce(T*,int)const
     *
     * @tparam BinaryFunction Function class which implements operator()
     *
     * @param in The array to compute on
     * @param out The array to store the results in
     * @param len The number of components in the array
     * @throw MPIError
     *
     * \deprecated Use allreduce(const T&, T&) const instead
     */
    template<typename BinaryFunction, typename T>
    DUNE_DEPRECATED
    int allreduce(T* in, T* out, int len) const
    {
      for(int i = 0; i< len; i++)
        out[i] = in[i];
      return 0;
    }

    /**
     * @brief Compute something over all processes
     * for each component of an array and return the result
     * in every process.
     *
     * @tparam BinaryFunction Function class which implements operator()
     *
     * @param in The array to compute on.
     * @param out The array to store the results in.
     * @throw MPIError
     */
    template<typename BinaryFunction, typename T>
    void allreduce(const T& in, T& out) const
    {
      out = in;
      return;
    }

    /**
     * @brief Non-blocking. Compute something over all processes
     * for each component of an array and return the result
     * in every process.
     *
     * @tparam BinaryFunction Function class which implements operator()
     *
     * @param in The array to compute on.
     * @param out The array to store the results in.
     * @warning \p in should not be modified until the returned
     * future is ready.
     * @throw MPIError
     */
    template<typename BinaryFunction, typename T>
    FutureType<std::decay_t<T>> iallreduce(const T& in, T&& out) const
    {
      FutureType<std::decay_t<T>> f(false, std::forward<T>(out));
      f.buffer() = in;
      return f;
    }

    /**
     * @brief Reduces the input data of the processes 0,...,i on
     * process i.
     *
     * @param in
     * @param out
     *
     * @throw MPIError
     */
    template<typename BinaryFunction, typename T>
    void scan (const T& in, T& out) const
    {
      out = in;
    }

    /**
     * @brief Non-blocking. Reduces the input data of the processes
     * 0,...,i on process i.
     *
     * @param in
     * @param out
     *
     * @return Future object containing the result when it is ready
     * @warning \p in should not be modified until the returned
     * future is ready.
     * @throw MPIError
     */
    template<typename BinaryFunction, typename T>
    FutureType<std::decay_t<T>> iscan (const T& in, T&& out) const
    {
      FutureType<std::decay_t<T>> f(false, std::forward<T>(out));
      f.buffer() = in;
      return f;
    }

    /**
     * @brief Reduces the input data of the processes 0,...,i-1 on
     * process i.
     *
     * @param in
     * @param out
     *
     * @throw MPIError
     */
    template<typename BinaryFunction, typename T>
    void exscan (const T& in, T& out) const
    {
      DUNE_UNUSED_PARAMETER(out);
      DUNE_UNUSED_PARAMETER(in);
    }

    /**
     * @brief Non-blocking. Reduces the input data of the processes
     * 0,...,i-1 on process i.
     *

     * @param in
     * @param out
     *
     * @return Function object containing the result when it is ready
     * @warning \p in should not be modified until the returned
     * future is ready.
     * @throw MPIError
     */
    template<typename BinaryFunction, typename T>
    FutureType<std::decay_t<T>> iexscan (const T& in, T&& out) const
    {
      DUNE_UNUSED_PARAMETER(in);
      return {false, std::forward<T>(out)};
    }
  };
}

#endif
