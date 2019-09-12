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
    typedef typename IndexContainer::value_type index_type;
    typedef std::map<RemoteType, IndexContainer> map_remote_to_pattern;

    CommunicationPattern(const remote_type& me)
      : me_(me)
    {}

    CommunicationPattern(const remote_type& me,
                         std::initializer_list<typename map_remote_to_pattern::value_type> sendInterface,
                         std::initializer_list<typename map_remote_to_pattern::value_type> recvInterface)
      : me_(me)
      , sendPattern_(sendInterface)
      , recvPattern_(recvInterface)
    {}

    map_remote_to_pattern& sendPattern(){
      return sendPattern_;
    }

    map_remote_to_pattern& recvPattern(){
      return recvPattern_;
    }

    const map_remote_to_pattern& sendPattern() const {
      return sendPattern_;
    }

    const map_remote_to_pattern& recvPattern() const{
      return recvPattern_;
    }

    const remote_type& me() const {
      return me_;
    }

    void strip(){
      stripPattern(sendPattern_);
      stripPattern(recvPattern_);
    }

  protected:
    remote_type me_;
    map_remote_to_pattern sendPattern_;
    map_remote_to_pattern recvPattern_;

    static void stripPattern(map_remote_to_pattern& pattern){
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
    os << "send pattern:" << std::endl;
    for(const auto& pair : pattern.sendPattern()){
      os << pair.first << ": [";
      for(const auto& idx : pair.second){
        os << idx << " ";
      }
      os << "]" << std::endl;
    }

    os << "recv pattern:" << std::endl;
    for(const auto& pair : pattern.recvPattern()){
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
    // create nur CommunicationPattern
    CommunicationPattern<typename RI::Attribute>
      commPattern(communication.rank());

    // fill the patterns
    auto& sendPatterns = commPattern.sendPattern();
    auto& recvPatterns = commPattern.recvPattern();
    for(const auto& process : remoteIndices){
      auto remote = process.first;
      auto& spattern = sendPatterns[remote];
      auto& rpattern = recvPatterns[remote];
      for(const auto& indexPair : *process.second.first){
        spattern.push_back({indexPair.localIndexPair().local().local(),indexPair.localIndexPair().local().attribute(), indexPair.attribute()});
      }
      for(const auto& indexPair : *process.second.second){
        rpattern.push_back({indexPair.localIndexPair().local().local(),indexPair.localIndexPair().local().attribute(), indexPair.attribute()});
      }
    }
    commPattern.strip();
    return commPattern;
  }
#endif
}

#endif
