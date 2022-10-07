// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/arraylist.hh>
#include <dune/common/test/iteratortest.hh>
#include <iostream>
#include <cstdlib>
#include <algorithm>

class Double {
public:
  double val;
  Double() : val(0){}
  Double(double d) : val(d){}
  Double& operator=(double d){
    val=d;
    return *this;
  }
};

bool operator<(Double a, Double b){
  return a.val<b.val;
}

template<class T, int size>
void randomizeList(Dune::ArrayList<T,size>& alist){
  using namespace Dune;

  srand(300);

  int lowest=0, highest=1000, range=(highest-lowest)+1;

  for(int i=0; i < 250; i++)
    alist.push_back(T(static_cast<int>(range*(rand()/(RAND_MAX+1.0)))));
}

int testSorting(){
  using namespace Dune;
  ArrayList<double,10> alist;

  randomizeList(alist);
  std::sort(alist.begin(), alist.end());
  double last=-1;

  for(ArrayList<double,10>::iterator iter=alist.begin(), end=alist.end();
      iter != end; ++iter) {
    if((*iter)>=last) {
      last=*iter;
    }else{
      std::cerr << last<<">"<<(*iter)<<" List is not sorted! "<<__FILE__ <<":"<<__LINE__<<std::endl;
      return 1;
    }
  }

  return 0;
}

template<int size>
void initConsecutive(Dune::ArrayList<double,size>& alist){
  using namespace Dune;

  for(int i=0; i < 100; i++)
    alist.push_back(i);
}

int testIteratorRemove(){
  using namespace Dune;
  ArrayList<double,10> alist;
  initConsecutive(alist);
  ArrayList<double,10>::iterator iter=alist.begin();

  iter+=8;

  iter.eraseToHere();
  ++iter;

  if((*iter)!=10) {
    std::cerr<<"Removing from iterator failed! "<<__FILE__<<":"<<__LINE__<<std::endl;
    return 1;
  }

  iter = alist.begin();

  if((*iter)!=9) {
    std::cerr<<"Removing from iterator failed! "<<__FILE__<<":"<<__LINE__<<std::endl;
    return 1;
  }

  iter +=3;
  iter.eraseToHere();
  iter +=4;

  if((*iter)!=17) {
    std::cerr<<"Removing from iterator failed! "<<__FILE__<<":"<<__LINE__<<std::endl;
    return 1;
  }

  alist.purge();
  if(*(alist.begin())!=13) {
    std::cerr<<"Purging iterator failed! "<<__FILE__<<":"<<__LINE__<<std::endl;
    return 1;
  }
  return 0;
}
int testRandomAccess(){
  using namespace Dune;
  ArrayList<double,10> alist;
  initConsecutive(alist);

  ArrayList<double,10>::iterator iter=alist.begin();


  for(int i=0; i < 100; i++) {
    if(iter[i]!=i) {
      std::cerr << "Random Access failed: "<<iter[i]<<"!="<<i<<" "<< __FILE__ <<":"<<__LINE__<<std::endl;
      return 1;
    }

    if(*(iter+i)!=i) {
      std::cerr << "Random Access failed "<< __FILE__ <<":"<<__LINE__<<std::endl;
      return 1;
    }
  }
  return 0;
}

int testComparison(){
  using namespace Dune;
  ArrayList<double,10> alist;
  initConsecutive(alist);

  ArrayList<double,10>::iterator iter=alist.begin(), iter1=alist.begin();
  iter1=iter+5;
  iter1=iter-5;
  iter1=iter+5;


  if(!(iter<iter1)) {
    std::cerr<<*iter<<">="<<*iter1<<" Operator< seems to be wrong! "<< __FILE__ <<__LINE__<<std::endl;
    return 1;
  }

  if(!(iter1>iter)) {
    std::cerr<<"operator> seems to be wrong! "<< __FILE__ <<__LINE__<<std::endl;
    return 1;
  }

  if(!(iter<=iter1)) {
    std::cerr<<"operator<= seems to be wrong! "<< __FILE__ <<__LINE__<<std::endl;
    return 1;
  }

  if(!(iter1>=iter)) {
    std::cerr<<"operator>= seems to be wrong! "<< __FILE__ <<__LINE__<<std::endl;
    return 1;
  }

  if(!(iter1 != iter)) {
    std::cerr<<"operator!= seems to be wrong! "<< __FILE__ <<__LINE__<<std::endl;
    return 1;
  }

  if(!(iter1 == iter+5)) {
    std::cerr<<"operator== seems to be wrong! "<< __FILE__ <<__LINE__<<std::endl;
    return 1;
  }
  return 0;
}


int main(){
  using namespace Dune;
  using namespace std;
  ArrayList<double,100> alist;

  randomizeList(alist);

  int ret=testIterator(alist);

  if(0!=testComparison()) {
    ret++;
    cerr<< "Comparison failed!"<<endl;
  }

  if(0!=testRandomAccess()) {
    ret++;
    cerr<< "Random Access failed!"<<endl;
  }

  if(0!=testSorting()) {
    ret++;
    cerr<< "Sorting failed!"<<endl;
  }

  if(0!=testIteratorRemove()) {
    ret++;
    cerr<< "Erasing failed!"<<endl;
  }
  return ret;

}
