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
    typedef std::map<RemoteType, IndexContainer> MapRemoteToPattern;

    CommunicationPattern(const remote_type& me)
      : me_(me)
    {}

    CommunicationPattern(const remote_type& me,
                         std::initializer_list<typename MapRemoteToPattern::value_type> sendInterface,
                         std::initializer_list<typename MapRemoteToPattern::value_type> recvInterface)
      : me_(me)
      , send_pattern_(sendInterface)
      , recv_pattern_(recvInterface)
    {}

    MapRemoteToPattern& send_pattern(){
      return send_pattern_;
    }

    MapRemoteToPattern& recv_pattern(){
      return recv_pattern_;
    }

    const MapRemoteToPattern& send_pattern() const {
      return send_pattern_;
    }

    const MapRemoteToPattern& recv_pattern() const{
      return recv_pattern_;
    }

    const remote_type& me() const {
      return me_;
    }

    void strip(){
      strip_pattern(send_pattern_);
      strip_pattern(recv_pattern_);
    }

  protected:
    remote_type me_;
    MapRemoteToPattern send_pattern_;
    MapRemoteToPattern recv_pattern_;

    static void strip_pattern(MapRemoteToPattern& pattern){
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
    for(const auto& pair : pattern.send_pattern()){
      os << pair.first << ": [";
      for(const auto& idx : pair.second){
        os << idx << " ";
      }
      os << "]" << std::endl;
    }

    os << "recv pattern:" << std::endl;
    for(const auto& pair : pattern.recv_pattern()){
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
  CommunicationPattern<> convertRemoteIndicesToCommunicationPattern(const RI& remote_indices,
                                                                    const SourceFlags& source_flags,
                                                                    const DestFlags& dest_flags){
    int me = 0;
    MPI_Comm_rank(remote_indices.communicator(), &me);
    CommunicationPattern<> comm_pattern(me);
    auto& send_patterns = comm_pattern.send_pattern();
    auto& recv_patterns = comm_pattern.recv_pattern();
    for(const auto& process : remote_indices){
      auto remote = process.first;
      auto& spattern = send_patterns[remote];
      auto& rpattern = recv_patterns[remote];
      for(const auto& indexPair : *process.second.first){
        if(dest_flags.contains(indexPair.attribute()) &&
           source_flags.contains(indexPair.localIndexPair().local().attribute()))
          spattern.push_back(indexPair.localIndexPair().local().local());
      }
      for(const auto& indexPair : *process.second.second){
        if(source_flags.contains(indexPair.attribute()) &&
           dest_flags.contains(indexPair.localIndexPair().local().attribute()))
          rpattern.push_back(indexPair.localIndexPair().local().local());
      }
    }
    comm_pattern.strip();
    return comm_pattern;
  }
#endif
}

#endif
