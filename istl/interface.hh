// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_INTERFACE_HH
#define DUNE_INTERFACE_HH

#include "remoteindices.hh"
#include <dune/common/enumset.hh>
namespace Dune
{
  /** @addtogroup ISTL_Comm
   *
   * @{
   */
  /**
   * @file
   * @brief Provides classes for building the communication
   * interface between remote inidices.
   * @author Markus Blatt
   */

  /**
   * @brief Base class of all classes representing a communication
   * interface.
   *
   * It provides an generic utility method for building the interface
   * for a set of remote indices.
   */
  template<typename T>
  class InterfaceBuilder
  {
  public:

    /**
     * @brief Type of the index set.
     */
    typedef T ParallelIndexSet;

    /**
     * @brief Type of the underlying remote indices class.
     */
    typedef RemoteIndices<ParallelIndexSet> RemoteIndices;

    /**
     * @brief The type of the global index.
     */
    typedef typename RemoteIndices::GlobalIndex GlobalIndex;

    /**
     * @brief The type of the attribute.
     */
    typedef typename RemoteIndices::Attribute Attribute;

    virtual ~InterfaceBuilder()
    {}

  protected:
    /**
     * @brief Not for public use.
     */
    InterfaceBuilder()
    {}

    /**
     * @brief Builds the interface between remote processes.
     *
     *
     * The types T1 and T2 are classes representing a set of
     * enumeration values of type InterfaceBuilder::Attribute. They have to provide
     * a (static) method
     * <pre>
     * bool contains(Attribute flag) const;
     * </pre>
     * for checking whether the set contains a specfic flag.
     * This functionality is for example provided the classes
     * EnumItem, EnumRange and Combine.
     *
     * If the template parameter send is true the sending side of
     * the interface will be built, otherwise the information for
     * receiving will be built.
     *
     *
     * If the template parameter send is true we create interface for sending
     * in a forward communication.
     *
     * @param remoteIndices The indices known to remote processes.
     * @param sourceFlags The set of flags marking source indices.
     * @param destFlags The setof flags markig destination indices.
     * @param functor A functor for callbacks. It should provide the
     * following methods:
     * <pre>
     * // Reserve memory for the interface to processor proc. The interface
     * // has to hold size entries
     * void reserve(int proc, int size);
     * </pre><pre>
     * // Add an entry to the interface
     * // We will send/receive size entries at index local to process proc
     * void add(int proc, int local);
     * </pre>
     */
    template<class T1, class T2, class Op, bool send>
    void buildInterface (const RemoteIndices& remoteIndices,
                         const T1& sourceFlags, const T2& destFlags,
                         Op& functor) const;
  };

  /**
   * @brief Information describing an interface.
   *
   * This class is used for temporary gathering information
   * about the interface needed for actually building it. It
   * is used be class Interface as functor for InterfaceBuilder::build.
   */
  class InterfaceInformation
  {

  public:

    /**
     * @brief Get the number of entries in the interface.
     */
    size_t size() const
    {
      return size_;
    }
    /**
     * @brief Get the local index for an entry.
     * @param i The  index of the entry.
     */
    uint32_t& operator[](size_t i)
    {
      assert(i<size_);
      return indices_[i];
    }
    /**
     * @brief Get the local index for an entry.
     * @param i The  index of the entry.
     */
    uint32_t operator[](size_t i) const
    {
      assert(i<size_);
      return indices_[i];
    }
    /**
     * @brief Reserve space for a number of entries.
     * @param size The maximum number of entries to hold.
     */
    void reserve(size_t size)
    {
      indices_ = new uint32_t[size];
      maxSize_ = size;

    }
    /**
     * brief Frees allocated memory.
     */
    void free()
    {
      delete[] indices_;
      maxSize_ = 0;
      size_=0;
      indices_=0;
    }
    /**
     * @brief Add a new index to the interface.
     */
    void add(uint32_t index)
    {
      assert(size_<maxSize_);
      indices_[size_++]=index;
    }

    InterfaceInformation()
      : size_(0), maxSize_(0), indices_(0)
    {}

    virtual ~InterfaceInformation()
    {
      if(indices_!=0) {
        std::cerr<<"InterfaceInformation::free() should be called before "<<
        "destructor!"<<std::endl;
      }

    }

  private:
    /**
     * @brief The number of entries in the interface.
     */
    size_t size_;
    /**
     * @brief The maximum number of indices we can hold.
     */
    size_t maxSize_;
    /**
     * @brief The local indices of the interface.
     */
    uint32_t* indices_;
  };

  /**
   * @brief Communication interface between remote and local indices.
   *
   * Describes the communication interface between
   * indices on the local process and those on remote processes.
   */
  template<typename T>
  class Interface : public InterfaceBuilder<T>
  {

  public:
    typedef InterfaceInformation Information;

    typedef std::map<int,std::pair<Information,Information> > InformationMap;

    /**
     * @brief Type of the index set.
     */
    typedef T ParallelIndexSet;

    /**
     * @brief Type of the underlying remote indices class.
     */
    typedef RemoteIndices<ParallelIndexSet> RemoteIndices;


    /**
     * @brief The type of the global index.
     */
    typedef typename RemoteIndices::GlobalIndex GlobalIndex;

    /**
     * @brief The type of the attribute.
     */
    typedef typename RemoteIndices::Attribute Attribute;

    /**
     * @brief Builds the interface.
     *
     * The types T1 and T2 are classes representing a set of
     * enumeration values of type Interface::Attribute. They have to provide
     * a (static) method
     * <pre>
     * bool contains(Attribute flag) const;
     * </pre>
     * for checking whether the set contains a specfic flag.
     * This functionality is for example provided the classes
     * EnumItem, EnumRange and Combine.
     * @param remoteIndices The indices known to remote processes.
     * @param sourceFlags The set of flags marking indices we send from.
     * @param destFlags The set of flags marking indices we receive for.
     */
    template<typename T1, typename T2>
    void build(const RemoteIndices& remoteIndices, const T1& sourceFlags,
               const T2& destFlags);

    /**
     * @brief Frees memory allocated during the build.
     */
    void free();

    /**
     * @brief Get the MPI Communicator.
     */
    MPI_Comm communicator() const;

    /**
     * @brief Get information about the interfaces.
     *
     * @return Map of the interfaces.
     * The key of the map is the process number and the value
     * is the information pair (first the send and then the receive
     * information).
     */
    const InformationMap& interfaces() const;

    Interface()
      : interfaces_()
    {}

    /**
     * @brief Print the interface to std::out for debugging.
     */
    void print() const;

    /**
     * @brief Destructor.
     */
    virtual ~Interface();

  private:
    /**
     * @brief The indices present on remote processes.
     */
    const RemoteIndices* remoteIndices_;

    /**
     * @brief Information about the interfaces.
     *
     * The key of the map is the process number and the value
     * is the information pair (first the send and then the receive
     * information).
     */
    InformationMap interfaces_;

    template<bool send>
    class InformationBuilder
    {
    public:
      InformationBuilder(InformationMap& interfaces)
        : interfaces_(interfaces)
      {}

      void reserve(int proc, int size)
      {
        if(send)
          interfaces_[proc].first.reserve(size);
        else
          interfaces_[proc].second.reserve(size);
      }
      void add(int proc, int local)
      {
        if(send) {
          interfaces_[proc].first.add(local);
        }else{
          interfaces_[proc].second.add(local);
        }
      }

    private:
      InformationMap& interfaces_;
    };
  };

  template<typename T>
  template<class T1, class T2, class Op, bool send>
  void InterfaceBuilder<T>::buildInterface(const RemoteIndices& remoteIndices, const T1& sourceFlags, const T2& destFlags, Op& interfaceInformation) const
  {
    // Allocate the memory for the data type construction.
    typedef typename RemoteIndices::RemoteIndexMap::const_iterator const_iterator;
    typedef typename RemoteIndices::ParallelIndexSet::const_iterator LocalIterator;

    const const_iterator end=remoteIndices.end();

    int rank;

    MPI_Comm_rank(remoteIndices.communicator(), &rank);

    // Allocate memory for the type construction.
    for(const_iterator process=remoteIndices.begin(); process != end; ++process) {
      // Messure the number of indices send to the remote process first
      int size=0;
      LocalIterator localIndex = send ? remoteIndices.source_->begin() : remoteIndices.target_->begin();
      const LocalIterator localEnd = send ?  remoteIndices.source_->end() : remoteIndices.target_->end();
      typedef typename RemoteIndices::RemoteIndexList::const_iterator RemoteIterator;
      const RemoteIterator remoteEnd = send ? process->second.first->end() :
                                       process->second.second->end();
      RemoteIterator remote = send ? process->second.first->begin() : process->second.second->begin();

      while(localIndex!=localEnd && remote!=remoteEnd) {
        if( send ?  destFlags.contains(remote->attribute()) :
            sourceFlags.contains(remote->attribute())) {
          // search for the matching local index
          while(localIndex->global()<remote->localIndexPair().global()) {
            localIndex++;
            assert(localIndex != localEnd);   // Should never happen
          }
          assert(localIndex->global()==remote->localIndexPair().global());

          // do we send the index?
          if( send ? sourceFlags.contains(localIndex->local().attribute()) :
              destFlags.contains(localIndex->local().attribute()))
            ++size;
        }
        ++remote;
      }
      interfaceInformation.reserve(process->first, size);
    }

    // compare the local and remote indices and set up the types

    CollectiveIterator<T> remote = remoteIndices.template iterator<send>();
    LocalIterator localIndex = send ? remoteIndices.source_->begin() : remoteIndices.target_->begin();
    const LocalIterator localEnd = send ?  remoteIndices.source_->end() : remoteIndices.target_->end();

    while(localIndex!=localEnd && !remote.empty()) {
      if( send ? sourceFlags.contains(localIndex->local().attribute()) :
          destFlags.contains(localIndex->local().attribute()))
      {
        // search for matching remote indices
        remote.advance(localIndex->global());
        // Iterate over the list that are positioned at global
        typedef typename CollectiveIterator<T>::iterator ValidIterator;
        const ValidIterator end = remote.end();
        ValidIterator validEntry = remote.begin();

        for(int i=0; validEntry != end; ++i) {
          if( send ?  destFlags.contains(validEntry->attribute()) :
              sourceFlags.contains(validEntry->attribute())) {
            // We will receive data for this index
            interfaceInformation.add(validEntry.process(),localIndex->local());
          }
          ++validEntry;
        }
      }
      ++localIndex;
    }
  }

  template<typename T>
  inline MPI_Comm Interface<T>::communicator() const
  {
    return remoteIndices_->communicator();

  }

  template<typename T>
  inline const std::map<int,std::pair<InterfaceInformation,InterfaceInformation> >& Interface<T>::interfaces() const
  {
    return interfaces_;
  }

  template<typename T>
  void Interface<T>::print() const
  {
    typedef typename InformationMap::const_iterator const_iterator;
    const const_iterator end=interfaces_.end();
    int rank;
    MPI_Comm_rank(communicator(), &rank);

    for(const_iterator infoPair=interfaces_.begin(); infoPair!=end; ++infoPair) {
      {
        std::cout<<rank<<": send for process "<<infoPair->first<<": ";
        const InterfaceInformation& info(infoPair->second.first);
        for(size_t i=0; i < info.size(); i++)
          std::cout<<info[i]<<" ";
        std::cout<<std::endl;
      } {

        std::cout<<rank<<": receive for process "<<infoPair->first<<": ";
        const InterfaceInformation& info(infoPair->second.second);
        for(size_t i=0; i < info.size(); i++)
          std::cout<<info[i]<<" ";
        std::cout<<std::endl;
      }

    }
  }

  template<typename T>
  template<typename T1, typename T2>
  void Interface<T>::build(const RemoteIndices& remoteIndices, const T1& sourceFlags,
                           const T2& destFlags)
  {
    remoteIndices_=&remoteIndices;

    assert(interfaces_.empty());

    // Build the send interface
    InformationBuilder<true> sendInformation(interfaces_);
    this->template buildInterface<T1,T2,InformationBuilder<true>,true>(remoteIndices, sourceFlags,
                                                                       destFlags, sendInformation);

    // Build the receive interface
    InformationBuilder<false> recvInformation(interfaces_);
    this->template buildInterface<T1,T2,InformationBuilder<false>,false>(remoteIndices,sourceFlags,
                                                                         destFlags, recvInformation);

  }

  template<typename T>
  void Interface<T>::free()
  {
    typedef InformationMap::iterator iterator;
    typedef InformationMap::const_iterator const_iterator;
    const const_iterator end = interfaces_.end();
    for(iterator interfacePair = interfaces_.begin(); interfacePair != end; ++interfacePair) {
      interfacePair->second.first.free();
      interfacePair->second.second.free();
    }
    interfaces_.clear();
  }
  template<typename T>
  Interface<T>::~Interface()
  {
    free();
  }
  /** @} */
}

#endif
