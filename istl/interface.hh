// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_INTERFACE_HH
#define DUNE_INTERFACE_HH

#include "remoteindices.hh"
#include <dune/common/enumset.hh>
namespace Dune
{



  template<typename TG, typename TA>
  class InterfaceBuilder
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
     * @brief Type of the underlying remote indices class.
     */
    typedef RemoteIndices<GlobalIndexType, AttributeType> RemoteIndices;

    virtual ~InterfaceBuilder()
    {}

    /**
     * @brief Builds the interface between remote processes.
     *
     * @param remoteIndices The indices known to remote processes.
     * @param sourceFlags The set of flags marking source indices.
     * @param destFlags The setof flags markig destination indices.
     * @param functor A functor for callbacks. It should provide the
     * following methods.
     * <pre>
     * // Reserve memory for the interface to processor proc. The interface
     * // has to hold size entries
     * void reserve(int proc, int size);
     *
     * // Add an entry to the interface
     * // We will send/receive size entries at index local to process proc
     * void add(int proc, int local);
     * </pre>
     *
     * If the template parameter send is true we create interface for sending
     * in a forward communication.
     */
    template<class T1, class T2, class Op, bool send>
    void buildInterface (const RemoteIndices& remoteIndices, const T1& sourceFlags, const T2& destFlags,
                         Op& functor) const;
  };


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


  template<bool condition, typename TrueType, typename FalseType>
  struct TypeChooser
  {};

  template<typename IfType, typename ElseType>
  struct TypeChooser<true,IfType,ElseType>
  {
    typedef IfType Type;
  };

  template<typename IfType, typename ElseType>
  struct TypeChooser<false,IfType,ElseType>
  {
    typedef ElseType Type;
  };


  template<typename TG, typename TA>
  class Interface : public InterfaceBuilder<TG,TA>
  {

  public:
    typedef InterfaceInformation Information;

    typedef std::map<int,std::pair<Information,Information> > InformationMap;


    /**
     * @brief The type of the global index.
     */
    typedef TG GlobalIndexType;

    /**
     * @brief The type of the attribute.
     */
    typedef TA AttributeType;

    /**
     * @brief Type of the underlying remote indices class.
     */
    typedef RemoteIndices<GlobalIndexType, AttributeType> RemoteIndices;

    /**
     * @brief Builds the interface.
     * @param remoteIndices The indices known to remote processes.
     * @param sourceFlags The set flags marking indices we send from.
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

  template<typename TG, typename TA>
  template<class T1, class T2, class Op, bool send>
  void InterfaceBuilder<TG,TA>::buildInterface(const RemoteIndices& remoteIndices, const T1& sourceFlags, const T2& destFlags, Op& interfaceInformation) const
  {
    // Allocate the memory for the data type construction.
    typedef typename RemoteIndices::RemoteIndexMap::const_iterator const_iterator;
    typedef typename RemoteIndices::IndexSetType::const_iterator LocalIterator;

    const const_iterator end=remoteIndices.end();

    int rank;

    MPI_Comm_rank(remoteIndices.communicator(), &rank);

    // Allocate memory for the type construction.
    for(const_iterator process=remoteIndices.begin(); process != end; ++process) {
      // Messure the number of indices send to the remote process first
      int size=0;
      LocalIterator localIndex = send ? remoteIndices.source_.begin() : remoteIndices.dest_.begin();
      const LocalIterator localEnd = send ?  remoteIndices.source_.end() : remoteIndices.dest_.end();
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

    CollectiveIterator<TG,TA> remote = remoteIndices.template iterator<send>();
    LocalIterator localIndex = send ? remoteIndices.source_.begin() : remoteIndices.dest_.begin();
    const LocalIterator localEnd = send ?  remoteIndices.source_.end() : remoteIndices.dest_.end();

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
            interfaceInformation.add(validEntry.process(),localIndex->local());
          }
          ++validEntry;
        }
      }
      ++localIndex;
    }
  }

  template<typename TG, typename TA>
  inline MPI_Comm Interface<TG,TA>::communicator() const
  {
    return remoteIndices_->communicator();

  }

  template<typename TG, typename TA>
  inline const std::map<int,std::pair<InterfaceInformation,InterfaceInformation> >& Interface<TG,TA>::interfaces() const
  {
    return interfaces_;
  }

  template<typename TG, typename TA>
  void Interface<TG,TA>::print() const
  {
    typedef typename InformationMap::const_iterator const_iterator;
    const const_iterator end=interfaces_.end();
    std::cout<<this<<std::endl;

    for(const_iterator infoPair=interfaces_.begin(); infoPair!=end; ++infoPair) {
      {
        std::cout<<"send for process "<<infoPair->first<<" ";
        const InterfaceInformation& info(infoPair->second.first);
        for(size_t i=0; i < info.size(); i++)
          std::cout<<info[i]<<" ";
        std::cout<<&info<<std::endl;
      } {

        std::cout<<"receive for process "<<infoPair->first<<" ";
        const InterfaceInformation& info(infoPair->second.second);
        for(size_t i=0; i < info.size(); i++)
          std::cout<<info[i]<<" ";
        std::cout<<&info<<std::endl;
      }

    }
  }

  template<typename TG, typename TA>
  template<typename T1, typename T2>
  void Interface<TG,TA>::build(const RemoteIndices& remoteIndices, const T1& sourceFlags,
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

  template<typename TG, typename TA>
  void Interface<TG,TA>::free()
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
  template<typename TG, typename TA>
  Interface<TG,TA>::~Interface()
  {
    free();
  }
}

#endif
