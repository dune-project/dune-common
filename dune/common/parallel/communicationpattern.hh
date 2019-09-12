// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_PARALLEL_COMMUNICATIONPATTERN_HH
#define DUNE_COMMON_PARALLEL_COMMUNICATIONPATTERN_HH

#include <ostream>
#include <vector>
#include <map>
#include <set>

namespace Dune {

  enum class CommunicationAttributes : char
    {
     owner=1,
     overlap=2,
     copy=4
    };

  std::ostream& operator<<(std::ostream& os, const CommunicationAttributes& a){
    switch(a){
    case(CommunicationAttributes::owner):
      os << "owner";
      break;
    case(CommunicationAttributes::overlap):
      os << "overlap";
      break;
    case(CommunicationAttributes::copy):
      os << "copy";
      break;
    }
    return os;
  }

  template<class IndexType = std::size_t, class Attribute = CommunicationAttributes>
  class CommunicationIndex {
    IndexType idx_;
    Attribute localAttribute_;
    Attribute remoteAttribute_;

  public:
    CommunicationIndex(const IndexType& idx,
                       const Attribute& localAttribute,
                       const Attribute& remoteAttribute)
      : idx_(idx)
      , localAttribute_(localAttribute)
      , remoteAttribute_(remoteAttribute)
    {}

    const IndexType& index() const {
      return idx_;
    }

    IndexType& index() {
      return idx_;
    }

    operator const IndexType& () const{
      return idx_;
    }

    const Attribute& localAttribute() const {
      return localAttribute_;
    }

    Attribute& localAttribute() {
      return localAttribute_;
    }

    const Attribute remoteAttribute() const {
      return remoteAttribute_;
    }

    Attribute remoteAttribute() {
      return remoteAttribute_;
    }

    friend std::ostream& operator <<(std::ostream& os,
                            const CommunicationIndex& i){
      os << "( " << i.index()
         << ", local: " << i.localAttribute()
         << ", remote: " << i.remoteAttribute()
         << ")";
      return os;
    }
  };

  template<class Attribute = CommunicationAttributes,
           class RemoteType = int,
           class IndexContainer = std::vector<CommunicationIndex<size_t, Attribute>>>
  class CommunicationPattern {
  public:
    typedef Attribute attribute_type;
    typedef RemoteType remote_type;
    typedef IndexContainer index_container;
    typedef typename IndexContainer::value_type index_type;
    typedef std::map<RemoteType, IndexContainer> map_remote_to_indices;

    CommunicationPattern(const remote_type& me)
      : me_(me)
    {}

    CommunicationPattern(const remote_type& me,
                         std::initializer_list<typename map_remote_to_indices::value_type> pattern)
      : me_(me)
      , pattern_(pattern)
    {}

    index_container& operator[](const remote_type& r){
      return pattern_[r];
    }

    const index_container& operator[](const remote_type& r) const{
      return pattern_[r];
    }

    auto begin(){
      return pattern_.begin();
    }

    auto begin() const{
      return pattern_.begin();
    }

    auto end(){
      return pattern_.end();
    }

    auto end() const {
      return pattern_.end();
    }

    const remote_type& me() const {
      return me_;
    }

    void strip(){
      stripPattern(pattern_);
    }

    // extracts a subset of indices with given local attribute
    template<class LocalAttributeSet>
    std::set<size_t> getSubSet(LocalAttributeSet localAttributeSet){
      std::set<size_t> set;
      for(const auto& pair : pattern_){
        for(const auto& index : pair.second){
          if(localAttributeSet.contains(index.localAttribute()) &&
             remoteAttributeSet.contains(index.remoteAttribute()))
            set.insert(index);
        }
      }
      return set;
    }

  protected:
    remote_type me_;
    map_remote_to_indices pattern_;

    static void stripPattern(map_remote_to_indices& pattern){
      for(auto it = pattern.begin(); it != pattern.end();){
        if(it->second.size() == 0)
          it = pattern.erase(it);
        else
          ++it;
      }
    }
  };

  template<class RemoteType, class IndexType>
  inline std::ostream& operator<<(std::ostream& os, const CommunicationPattern<RemoteType, IndexType>& pattern)
  {
    for(const auto& pair : pattern){
      os << pair.first << ": [";
      for(const auto& idx : pair.second){
        os << idx << " ";
      }
      os << "]" << std::endl;
    }
    return os;
  }

#if HAVE_MPI
  template<class RI>
  CommunicationPattern<typename RI::Attribute>
  convertRemoteIndicesToCommunicationPattern(const RI& remoteIndices){
    // create a Communication, needed to determine own process number
    Communication<std::decay_t<decltype(remoteIndices.communicator())>>
      communication(remoteIndices.communicator());
    // create new CommunicationPattern
    CommunicationPattern<typename RI::Attribute>
      commPattern(communication.rank());

    // fill the patterns
    for(const auto& process : remoteIndices){
      auto remote = process.first;
      auto& indices = commPattern[remote];
      if(remote < communication.rank()){ // the ordering of the indices that the smaller process
                       // sends to the larger on is the order we obtain here
        for(const auto& indexPair : *process.second.first){
          indices.push_back({indexPair.localIndexPair().local().local(),indexPair.localIndexPair().local().attribute(), indexPair.attribute()});
        }
      }else{
        for(const auto& indexPair : *process.second.second){
          indices.push_back({indexPair.localIndexPair().local().local(),indexPair.localIndexPair().local().attribute(), indexPair.attribute()});
        }
      }
    }
    commPattern.strip();
    return commPattern;
  }
#endif
}

#endif
