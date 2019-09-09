// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#include <iostream>

#include <dune/common/parallel/communicationpattern.hh>

int main(){

  Dune::CommunicationPattern<> pattern({// send pattern:
                                        {3, {1,4,7}},
                                        {1, {4,3,1}},
                                        {42, {}}
    },
                                        {// receive pattern:
                                         {6, {1,2,3}},
                                         {4711, {4,7,1}},
                                         {3, {}}
                                        }
    );

  // add manually
  pattern.send_pattern()[3].insert(6);
  pattern.recv_pattern()[666].insert(0);

  pattern.strip();
  std::cout << pattern << std::endl;
  return 0;
}
