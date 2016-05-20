// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COLLECTIVECOMMUNICATION_HH
#define DUNE_COLLECTIVECOMMUNICATION_HH
/*!
   \file
   \brief Implements an utility class that provides
   collective communication methods for sequential programs.

   \ingroup ParallelCommunication
 */
#include <iostream>
#include <complex>
#include <algorithm>

#include <dune/common/binaryfunctions.hh>
#include <dune/common/exceptions.hh>

/*! \defgroup ParallelCommunication Parallel Communication
   \ingroup Common

   \brief Abstractions for paralle computing

   Dune offers an abstraction to the basic methods of parallel
   communication. It allows one to switch parallel features on and off,
   without changing the code. This is done using either CollectiveCommunication
   or MPICollectiveCommunication.

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
     the Message Passing %Interface (MPI), see Dune::CollectiveCommunication<MPI_Comm>.

     Moreover, the communication subsystem used by an implementation
     is not visible in the interface, i.e. Dune grid implementations
     are not restricted to MPI.

     \tparam Communicator The communicator type used by your message-passing implementation.
       For MPI this will be MPI_Comm.  For sequential codes there is the dummy communicator No_Comm.
       It is assumed that if you want to specialize the CollectiveCommunication class for a
       message-passing system other than MPI, that message-passing system will have something
       equivalent to MPI communicators.

     \ingroup ParallelCommunication
   */
  template<typename Communicator>
  class CollectiveCommunication
  {
  public:
    //! Construct default object
    CollectiveCommunication()
    {}

    /** \brief Constructor with a given communicator
     *
     * As this is implementation for the sequential setting, the communicator is a dummy and simply discarded.
     */
    CollectiveCommunication (const Communicator&)
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

    /** @brief  Compute the sum of the argument over all processes and
            return the result in every process. Assumes that T has an operator+
     */
    template<typename T>
    T sum (T& in) const     // MPI does not know about const :-(
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
      return 0;
    }

    /** @brief  Compute the product of the argument over all processes and
            return the result in every process. Assumes that T has an operator*
     */
    template<typename T>
    T prod (T& in) const     // MPI does not know about const :-(
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
      return 0;
    }

    /** @brief  Compute the minimum of the argument over all processes and
            return the result in every process. Assumes that T has an operator<
     */
    template<typename T>
    T min (T& in) const     // MPI does not know about const :-(
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
      return 0;
    }

    /** @brief  Compute the maximum of the argument over all processes and
            return the result in every process. Assumes that T has an operator<
     */
    template<typename T>
    T max (T& in) const     // MPI does not know about const :-(
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
      return 0;
    }

    /** @brief Wait until all processes have arrived at this point in the program.
        @returns MPI_SUCCESS (==0) if successful, an MPI error code otherwise
     */
    int barrier () const
    {
      return 0;
    }

    /** @brief Distribute an array from the process with rank root to all other processes
        @returns MPI_SUCCESS (==0) if successful, an MPI error code otherwise
     */
    template<typename T>
    int broadcast (T* inout, int len, int root) const
    {
      return 0;
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
    int gather (T* in, T* out, int len, int root) const     // note out must have same size as in
    {
      for (int i=0; i<len; i++)
        out[i] = in[i];
      return 0;
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
    int gatherv (T* in, int sendlen, T* out, int* recvlen, int* displ, int root) const
    {
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
    int scatter (T* send, T* recv, int len, int root) const // note out must have same size as in
    {
      for (int i=0; i<len; i++)
        recv[i] = send[i];
      return 0;
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
    int scatterv (T* send, int* sendlen, int* displ, T* recv, int recvlen, int root) const
    {
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
    int allgather(T* sbuf, int count, T* rbuf) const
    {
      for(T* end=sbuf+count; sbuf < end; ++sbuf, ++rbuf)
        *rbuf=*sbuf;
      return 0;
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
     *                    of elements to recieve from process i at position i, i.e. the number that
     *                    is passed as sendlen argument to this function in process i.
     * @param[in] displ An array with size equal to the number of processes. Data recieved from
     *                  process i will be written starting at out+displ[i].
     * @returns MPI_SUCCESS (==0) if successful, an MPI error code otherwise
     */
    template<typename T>
    int allgatherv (T* in, int sendlen, T* out, int* recvlen, int* displ) const
    {
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
     * @param in The array to compute on.
     * @param out The array to store the results in.
     * @param len The number of components in the array
     * @returns MPI_SUCCESS (==0) if successful, an MPI error code otherwise
     */
    template<typename BinaryFunction, typename Type>
    void allreduce(Type* in, Type* out, int len) const
    {
      std::copy(in, in+len, out);
      return;
    }

  };
}

#endif
