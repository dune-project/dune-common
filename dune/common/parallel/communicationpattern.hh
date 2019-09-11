// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_PARALLEL_COMMUNICATIONPATTERN_HH
#define DUNE_COMMON_PARALLEL_COMMUNICATIONPATTERN_HH

#include <ostream>
#include <vector>
#include <map>
#include <set>

namespace Dune {

  template<class RemoteType = int, class IndexContainer = std::vector<std::size_t>>
  class CommunicationPattern {
  public:
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
  template<class RI, class SourceFlags, class DestFlags>
  CommunicationPattern<> convertRemoteIndicesToCommunicationPattern(const RI& remoteIndices,
                                                                    const SourceFlags& sourceFlags,
                                                                    const DestFlags& destFlags){
    int me = 0;
    MPI_Comm_rank(remoteIndices.communicator(), &me);
    CommunicationPattern<> commPattern(me);
    auto& sendPatterns = commPattern.sendPattern();
    auto& recvPatterns = commPattern.recvPattern();
    for(const auto& process : remoteIndices){
      auto remote = process.first;
      auto& spattern = sendPatterns[remote];
      auto& rpattern = recvPatterns[remote];
      for(const auto& indexPair : *process.second.first){
        if(destFlags.contains(indexPair.attribute()) &&
           sourceFlags.contains(indexPair.localIndexPair().local().attribute()))
          spattern.push_back(indexPair.localIndexPair().local().local());
      }
      for(const auto& indexPair : *process.second.second){
        if(sourceFlags.contains(indexPair.attribute()) &&
           destFlags.contains(indexPair.localIndexPair().local().attribute()))
          rpattern.push_back(indexPair.localIndexPair().local().local());
      }
    }
    commPattern.strip();
    return commPattern;
  }
#endif
}

#endif
