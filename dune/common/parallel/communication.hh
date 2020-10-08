// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_PARALLEL_COMMUNICATION_HH
#define DUNE_COMMON_PARALLEL_COMMUNICATION_HH
/*!
   \file
   \brief Implements an utility class that provides
   collective communication methods for sequential programs.

   \ingroup ParallelCommunication
 */
#include <iostream>
#include <complex>
#include <algorithm>
#include <vector>

#include <dune/common/binaryfunctions.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/unused.hh>
#include <dune/common/parallel/future.hh>

/*! \defgroup ParallelCommunication Parallel Communication
   \ingroup Common

   \brief Abstractions for parallel computing

   Dune offers an abstraction to the basic methods of parallel
   communication. It allows one to switch parallel features on and off,
   without changing the code. This is done using either Communication
   or MPICommunication.

 */

/*!
   \file
   \brief An abstraction to the basic methods of parallel communication,
     following the message-passing paradigm.
   \ingroup ParallelCommunication
 */

namespace Dune
{

  /* define some type that definitely differs from MPI_Comm */
  struct No_Comm {};

  /*! @brief Comparison operator for MPI compatibility

    Always returns true.
  */
  inline bool operator==(const No_Comm&, const No_Comm&)
  {
    return true;
  }

  /*! @brief Comparison operator for MPI compatibility

    Always returns false.
  */
  inline bool operator!=(const No_Comm&, const No_Comm&)
  {
    return false;
  }

  /*! @brief Collective communication interface and sequential default implementation

     Communication offers an abstraction to the basic methods
     of parallel communication, following the message-passing
     paradigm. It allows one to switch parallel features on and off, without
     changing the code. Currently only MPI and sequential code are
     supported.

     A Communication object is returned by all grids (also
     the sequential ones) in order to allow code to be written in
     a transparent way for sequential and parallel grids.

     This class provides a default implementation for sequential grids.
     The number of processes involved is 1, any sum, maximum, etc. returns
     just its input argument and so on.

     In specializations one can implement the real thing using appropriate
     communication functions, e.g. there exists an implementation using
     the Message Passing %Interface (MPI), see Dune::Communication<MPI_Comm>.

     Moreover, the communication subsystem used by an implementation
     is not visible in the interface, i.e. Dune grid implementations
     are not restricted to MPI.

     \tparam Communicator The communicator type used by your message-passing implementation.
       For MPI this will be MPI_Comm.  For sequential codes there is the dummy communicator No_Comm.
       It is assumed that if you want to specialize the Communication class for a
       message-passing system other than MPI, that message-passing system will have something
       equivalent to MPI communicators.

     \ingroup ParallelCommunication
   */
  template<typename Communicator>
  class Communication
  {
  public:
    //! Construct default object
    Communication()
    {}

    /** \brief Constructor with a given communicator
     *
     * As this is implementation for the sequential setting, the communicator is a dummy and simply discarded.
     */
    Communication (const Communicator&)
    {}

    //! Return rank, is between 0 and size()-1
    int rank () const
    {
      return 0;
    }

    //! cast to the underlying Fake MPI communicator
    operator No_Comm() const
    {
      return {};
    }

    //! Number of processes in set, is greater than 0
    int size () const
    {
      return 1;
    }

    /** @brief Sends the data to the dest_rank
        @returns MPI_SUCCESS (==0) if successful, an MPI error code otherwise
     */
    template<class T>
    int send(const T& data, int dest_rank, int tag){
      DUNE_UNUSED_PARAMETER(data);
      DUNE_UNUSED_PARAMETER(dest_rank);
      DUNE_UNUSED_PARAMETER(tag);
      DUNE_THROW(ParallelError, "This method is not supported in sequential programs");
    }

    /** @brief Sends the data to the dest_rank nonblocking
        @returns Future<T> containing the send buffer, completes when data is send
     */
    template<class T>
    PseudoFuture<T> isend(const T&& data, int dest_rank, int tag){
      DUNE_UNUSED_PARAMETER(data);
      DUNE_UNUSED_PARAMETER(dest_rank);
      DUNE_UNUSED_PARAMETER(tag);
      DUNE_THROW(ParallelError, "This method is not supported in sequential programs");
    }

    /** @brief Receives the data from the source_rank
        @returns MPI_SUCCESS (==0) if successful, an MPI error code otherwise
     */
    template<class T>
    T recv(T&& data, int source_rank, int tag, void* status = 0){
      DUNE_UNUSED_PARAMETER(data);
      DUNE_UNUSED_PARAMETER(source_rank);
      DUNE_UNUSED_PARAMETER(tag);
      DUNE_UNUSED_PARAMETER(status);
      DUNE_THROW(ParallelError, "This method is not supported in sequential programs");
    }

    /** @brief Receives the data from the source_rank nonblocking
        @returns Future<T> containing the received data when complete
     */
    template<class T>
    PseudoFuture<T> irecv(T&& data, int source_rank, int tag){
      DUNE_UNUSED_PARAMETER(data);
      DUNE_UNUSED_PARAMETER(source_rank);
      DUNE_UNUSED_PARAMETER(tag);
      DUNE_THROW(ParallelError, "This method is not supported in sequential programs");
    }

    template<class T>
    T rrecv(T&& data, int source_rank, int tag, void* status = 0) const
    {
      DUNE_UNUSED_PARAMETER(data);
      DUNE_UNUSED_PARAMETER(source_rank);
      DUNE_UNUSED_PARAMETER(tag);
      DUNE_UNUSED_PARAMETER(status);
      DUNE_THROW(ParallelError, "This method is not supported in sequential programs");
    }
    /** @brief  Compute the sum of the argument over all processes and
            return the result in every process. Assumes that T has an operator+
     */
    template<typename T>
    T sum (const T& in) const
    {
      return in;
    }

    /** @brief Compute the sum over all processes for each component of an array and return the result
            in every process. Assumes that T has an operator+

        @returns MPI_SUCCESS (==0) if successful, an MPI error code otherwise
     */
    template<typename T>
    int sum (T* inout, int len) const
    {
      DUNE_UNUSED_PARAMETER(inout);
      DUNE_UNUSED_PARAMETER(len);
      return 0;
    }

    /** @brief  Compute the product of the argument over all processes and
            return the result in every process. Assumes that T has an operator*
     */
    template<typename T>
    T prod (const T& in) const
    {
      return in;
    }

    /** @brief Compute the product over all processes
            for each component of an array and return the result
            in every process. Assumes that T has an operator*
        @returns MPI_SUCCESS (==0) if successful, an MPI error code otherwise
     */
    template<typename T>
    int prod (T* inout, int len) const
    {
      DUNE_UNUSED_PARAMETER(inout);
      DUNE_UNUSED_PARAMETER(len);
      return 0;
    }

    /** @brief  Compute the minimum of the argument over all processes and
            return the result in every process. Assumes that T has an operator<
     */
    template<typename T>
    T min (const T& in) const
    {
      return in;
    }

    /** @brief Compute the minimum over all processes
            for each component of an array and return the result
            in every process. Assumes that T has an operator<
        @returns MPI_SUCCESS (==0) if successful, an MPI error code otherwise
     */
    template<typename T>
    int min (T* inout, int len) const
    {
      DUNE_UNUSED_PARAMETER(inout);
      DUNE_UNUSED_PARAMETER(len);
      return 0;
    }

    /** @brief  Compute the maximum of the argument over all processes and
            return the result in every process. Assumes that T has an operator<
     */
    template<typename T>
    T max (const T& in) const
    {
      return in;
    }

    /** @brief Compute the maximum over all processes
            for each component of an array and return the result
            in every process. Assumes that T has an operator<
        @returns MPI_SUCCESS (==0) if successful, an MPI error code otherwise
     */
    template<typename T>
    int max (T* inout, int len) const
    {
      DUNE_UNUSED_PARAMETER(inout);
      DUNE_UNUSED_PARAMETER(len);
      return 0;
    }

    /** @brief Wait until all processes have arrived at this point in the program.
        @returns MPI_SUCCESS (==0) if successful, an MPI error code otherwise
     */
    int barrier () const
    {
      return 0;
    }

    /** @brief Nonblocking barrier
        @returns Future<void> which is complete when all processes have reached the barrier
     */
    PseudoFuture<void> ibarrier () const
    {
      return {true}; // return a valid future
    }

    /** @brief Distribute an array from the process with rank root to all other processes
        @returns MPI_SUCCESS (==0) if successful, an MPI error code otherwise
     */
    template<typename T>
    int broadcast (T* inout, int len, int root) const
    {
      DUNE_UNUSED_PARAMETER(inout);
      DUNE_UNUSED_PARAMETER(len);
      DUNE_UNUSED_PARAMETER(root);
      return 0;
    }

    /** @brief Distribute an array from the process with rank root to all other processes nonblocking
        @returns Future<T> containing the distributed data
     */
    template<class T>
    PseudoFuture<T> ibroadcast(T&& data, int root) const{
      return {std::forward<T>(data)};
    }


    /** @brief  Gather arrays on root task.
     *
     * Each process sends its in array of length len to the root process
     * (including the root itself). In the root process these arrays are stored in rank
     * order in the out array which must have size len * number of processes.
     * @param[in] in The send buffer with the data to send.
     * @param[out] out The buffer to store the received data in. Might have length zero on non-root
     *                  tasks.
     * @param[in] len The number of elements to send on each task.
     * @param[in] root The root task that gathers the data.
     * @returns MPI_SUCCESS (==0) if successful, an MPI error code otherwise
     */
    template<typename T>
    int gather (const T* in, T* out, int len, int root) const     // note out must have same size as in
    {
      DUNE_UNUSED_PARAMETER(root);
      for (int i=0; i<len; i++)
        out[i] = in[i];
      return 0;
    }

    /** @brief  Gather arrays on root task nonblocking
        @returns Future<TOUT, TIN> containing the gathered data
     */
    template<class TIN, class TOUT = std::vector<TIN>>
    PseudoFuture<TOUT> igather(TIN&& data_in, TOUT&& data_out, int root){
      *(data_out.begin()) = std::forward<TIN>(data_in);
      return {std::forward<TOUT>(data_out)};
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
     * @returns MPI_SUCCESS (==0) if successful, an MPI error code otherwise
     */
    template<typename T>
    int gatherv (const T* in, int sendlen, T* out, int* recvlen, int* displ, int root) const
    {
      DUNE_UNUSED_PARAMETER(recvlen);
      DUNE_UNUSED_PARAMETER(root);
      for (int i=*displ; i<sendlen; i++)
        out[i] = in[i];
      return 0;
    }

    /** @brief Scatter array from a root to all other task.
     *
     * The root process sends the elements with index from k*len to (k+1)*len-1 in its array to
     * task k, which stores it at index 0 to len-1.
     * @param[in] send The array to scatter. Might have length zero on non-root
     *                  tasks.
     * @param[out] recv The buffer to store the received data in. Upon completion of the
     *                 method each task will have same data stored there as the one in
     *                 send buffer of the root task before.
     * @param[in] len The number of elements in the recv buffer.
     * @param[in] root The root task that gathers the data.
     * @returns MPI_SUCCESS (==0) if successful, an MPI error code otherwise
     */
    template<typename T>
    int scatter (const T* send, T* recv, int len, int root) const // note out must have same size as in
    {
      DUNE_UNUSED_PARAMETER(root);
      for (int i=0; i<len; i++)
        recv[i] = send[i];
      return 0;
    }

    /** @brief Scatter array from a root to all other task nonblocking.
     * @returns Future<TOUT, TIN> containing scattered data;
     */
    template<class TIN, class TOUT = TIN>
    PseudoFuture<TOUT> iscatter(TIN&& data_in, TOUT&& data_out, int root){
      data_out = *(std::forward<TIN>(data_in).begin());
      return {std::forward<TOUT>(data_out)};
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
     * @returns MPI_SUCCESS (==0) if successful, an MPI error code otherwise
     */
    template<typename T>
    int scatterv (const T* send, int* sendlen, int* displ, T* recv, int recvlen, int root) const
    {
      DUNE_UNUSED_PARAMETER(recvlen);
      DUNE_UNUSED_PARAMETER(root);
      for (int i=*displ; i<*sendlen; i++)
        recv[i] = send[i];
      return 0;
    }

    /**
     * @brief Gathers data from all tasks and distribute it to all.
     *
     * The block of data sent from the  jth  process  is  received  by  every
     *  process and placed in the jth block of the buffer recvbuf.
     *
     * @param[in] sbuf The buffer with the data to send. Has to be the same for
     *                 each task.
     * @param[in] count The number of elements to send by any process.
     * @param[out] rbuf The receive buffer for the data. Has to be of size
     *  notasks*count, with notasks being the number of tasks in the communicator.
     * @returns MPI_SUCCESS (==0) if successful, an MPI error code otherwise
     */
    template<typename T>
    int allgather(const T* sbuf, int count, T* rbuf) const
    {
      for(const T* end=sbuf+count; sbuf < end; ++sbuf, ++rbuf)
        *rbuf=*sbuf;
      return 0;
    }

    /**
     * @brief Gathers data from all tasks and distribute it to all nonblocking.
     @returns Future<TOUT, TIN> containing the distributed data
     */
    template<class TIN, class TOUT = TIN>
    PseudoFuture<TOUT> iallgather(TIN&& data_in, TOUT&& data_out){
      return {std::forward<TOUT>(data_out)};
    }

    /**
     * @brief Gathers data of variable length from all tasks and distribute it to all.
     *
     * The block of data sent from the jth process is received by every
     *  process and placed in the jth block of the buffer out.
     *
     * @param[in] in The send buffer with the data to send.
     * @param[in] sendlen The number of elements to send on each task.
     * @param[out] out The buffer to store the received data in.
     * @param[in] recvlen An array with size equal to the number of processes containing the number
     *                    of elements to receive from process i at position i, i.e. the number that
     *                    is passed as sendlen argument to this function in process i.
     * @param[in] displ An array with size equal to the number of processes. Data received from
     *                  process i will be written starting at out+displ[i].
     * @returns MPI_SUCCESS (==0) if successful, an MPI error code otherwise
     */
    template<typename T>
    int allgatherv (const T* in, int sendlen, T* out, int* recvlen, int* displ) const
    {
      DUNE_UNUSED_PARAMETER(recvlen);
      for (int i=*displ; i<sendlen; i++)
        out[i] = in[i];
      return 0;
    }

    /**
     * @brief Compute something over all processes
     * for each component of an array and return the result
     * in every process.
     *
     * The template parameter BinaryFunction is the type of
     * the binary function to use for the computation
     *
     * @param inout The array to compute on.
     * @param len The number of components in the array
     * @returns MPI_SUCCESS (==0) if successful, an MPI error code otherwise
     */
    template<typename BinaryFunction, typename Type>
    int allreduce(Type* inout, int len) const
    {
      DUNE_UNUSED_PARAMETER(inout);
      DUNE_UNUSED_PARAMETER(len);
      return 0;
    }

    /**
     * @brief Compute something over all processes nonblocking
     @return Future<TOUT, TIN> containing the computed something
     */
    template<class BinaryFunction, class TIN, class TOUT = TIN>
    PseudoFuture<TOUT> iallreduce(TIN&& data_in, TOUT&& data_out){
      data_out = std::forward<TIN>(data_in);
      return {std::forward<TOUT>(data_out)};
    }

    /**
     * @brief Compute something over all processes nonblocking and in-place
     @return Future<T> containing the computed something
     */
    template<class BinaryFunction, class T>
    PseudoFuture<T> iallreduce(T&& data){
      return {std::forward<T>(data)};
    }


    /**
     * @brief Compute something over all processes
     * for each component of an array and return the result
     * in every process.
     *
     * The template parameter BinaryFunction is the type of
     * the binary function to use for the computation
     *
     * @param in The array to compute on.
     * @param out The array to store the results in.
     * @param len The number of components in the array
     * @returns MPI_SUCCESS (==0) if successful, an MPI error code otherwise
     */
    template<typename BinaryFunction, typename Type>
    int allreduce(const Type* in, Type* out, int len) const
    {
      std::copy(in, in+len, out);
      return 0;
    }

  };

  template<class T>
  using CollectiveCommunication
  // Will be deprecated after the 2.7 release
  //[[deprecated("CollectiveCommunication is deprecated. Use Communication instead.")]]
  = Communication<T>;
}

#endif
