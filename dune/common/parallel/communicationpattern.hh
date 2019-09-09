// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_PARALLEL_COMMUNICATIONPATTERN_HH
#define DUNE_COMMON_PARALLEL_COMMUNICATIONPATTERN_HH

#include <ostream>
#include <vector>
#include <map>
#include <set>

namespace Dune {

  template<class RemoteType = int, class IndexContainer = std::set<std::size_t>>
  class CommunicationPattern {
  public:
    typedef std::map<RemoteType, IndexContainer> MapRemoteToPattern;

    CommunicationPattern() = default;

    CommunicationPattern(std::initializer_list<typename MapRemoteToPattern::value_type> sendInterface,
                         std::initializer_list<typename MapRemoteToPattern::value_type> recvInterface)
      : send_pattern_(sendInterface)
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

    void strip(){
      strip_pattern(send_pattern_);
      strip_pattern(recv_pattern_);
    }

  protected:
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
}

#endif
