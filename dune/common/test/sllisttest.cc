// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/sllist.hh>
#include <dune/common/test/iteratortest.hh>
#include <dune/common/poolallocator.hh>
#include <iostream>

class DoubleWrapper
{
public:
  DoubleWrapper(double b)
    : d(b)
  {
    std::cout<<"Constructed "<<this<<std::endl;
  }

  DoubleWrapper()
    : d()
  {
    std::cout<<"Constructed "<<this<<std::endl;
  }

  DoubleWrapper(const DoubleWrapper& other)
    : d(other.d)
  {
    std::cout<<"Copied "<<this<<" from "<<&other<<std::endl;
  }

  ~DoubleWrapper()
  {
    std::cout<<"Destructing "<<this<<std::endl;
  }

  operator double() const
  {
    return d;
  }

  bool operator==(const DoubleWrapper& other) const
  {
    return d == other.d;
  }


  bool operator!=(const DoubleWrapper& other) const
  {
    return d != other.d;
  }

private:
  double d;
};

typedef Dune::PoolAllocator<int,8*1024-16> IntAllocator;
//typedef std::allocator<int> IntAllocator;
typedef Dune::PoolAllocator<double,8*1024-16> DoubleAllocator;
//typedef std::allocator<double> DoubleAllocator;
typedef Dune::PoolAllocator<DoubleWrapper,8*1024-16> DoubleWAllocator;
//typedef std::allocator<DoubleWrapper> DoubleWAllocator;

template<typename T, typename A>
const T& tail(const Dune::SLList<T,A>& alist)
{
  typedef typename Dune::SLList<T,A>::const_iterator Iterator;
  Iterator tail=alist.begin();

  for(int i = alist.size() - 1; i > 0; --i)
    ++tail;
  return *tail;
}

template<typename T,class A>
int check(const Dune::SLList<T,A>& alist, const T* vals)
{
  typedef typename Dune::SLList<T,A>::const_iterator iterator;
  int i=0;
  for(iterator iter = alist.begin(); iter != alist.end(); ++iter, i++) {
    if( vals[i] != *iter ) {
      std::cerr<<" List mismatch! "<<__FILE__<<":"<<__LINE__<<std::endl;
      return 1;
    }
  }
  return 0;
}


template<typename T,class A>
void randomizeListBack(Dune::SLList<T,A>& alist){
  using namespace Dune;

  srand(300);

  int lowest=0, highest=1000, range=(highest-lowest)+1;

  T vals[10];

  for(int i=0; i < 10; i++) {
    T d = T(range*(rand()/(RAND_MAX+1.0)));
    alist.push_back(d);
    vals[i]=d;
  }

  check(alist, vals);
}

template<typename T,class A>
void randomizeListFront(Dune::SLList<T,A>& alist){
  using namespace Dune;

  srand(300);
  T vals[10];

  int lowest=0, highest=1000, range=(highest-lowest)+1;

  for(int i=0; i < 10; i++) {
    T d = T(range*(rand()/(RAND_MAX+1.0)));
    alist.push_front(d);
    vals[9-i]=d;
  }

  check(alist, vals);
}
int testAssign()
{
  typedef Dune::SLList<int,IntAllocator> List;
  List alist, blist;

  alist.push_back(3);
  alist.push_back(4);
  alist.push_back(5);

  blist.push_back(-1);

  blist=alist;
  List::iterator biter=blist.begin(), aiter=alist.begin();
  for(; aiter!=alist.end(); ++aiter, ++biter)
    if(*aiter!=*biter) {
      std::cerr<<"Assignment failed "<<__FILE__<<":"<<__LINE__<<std::endl;
      return 1;
    }
  return 0;
}

int testDelete()
{
  typedef Dune::SLList<int,IntAllocator> List;
  List alist;

  alist.push_back(3);
  alist.push_back(4);
  alist.push_back(5);

  List::ModifyIterator iter = alist.beginModify();
  iter.remove();
  if(*(alist.begin())!=4) {
    std::cerr<<"delete next on position before head failed! "<<__FILE__<<":"<<__LINE__<<std::endl;
    return 1;
  }
  if(*iter!=4) {
    std::cerr<<"delete next failed! "<<__FILE__<<":"<<__LINE__<<std::endl;
    return 1;
  }
  ++iter;
  iter.remove();
  if(iter!=alist.end()) {
    std::cerr<<"delete next failed! "<<__FILE__<<":"<<__LINE__<<std::endl;
    return 1;
  }
  if(tail(alist)!=4) {
    std::cerr<<"delete before tail did not change tail! "<<__FILE__<<":"<<__LINE__<<std::endl;
  }

  return 0;
}

int testEmpty()
{
  typedef Dune::SLList<int,DoubleAllocator> List;
  int ret = 0;

  List alist;
  if(!alist.empty()) {
    std::cerr<<"Newly created list not empty! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ret++;
  }

  if(0 != alist.size()) {
    std::cerr<<"Newly created list not empty! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ret++;
  }

  randomizeListBack(alist);


  if(alist.empty()) {
    std::cerr<<"Randomized list is empty! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ret++;
  }

  if(0 == alist.size()) {
    std::cerr<<"Randomized list is empty! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ret++;
  }

  for(int elements=alist.size(); elements>0; --elements)
    alist.pop_front();

  if(!alist.empty()) {
    std::cerr<<"Emptied list not empty! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ret++;
  }

  if(0 != alist.size()) {
    std::cerr<<"Emptied list not empty! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ret++;
  }


  if(ret!=0)
    // Skip next tests
    return ret;

  randomizeListFront(alist);

  if(alist.empty()) {
    std::cerr<<"Randomized list is empty! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ret++;
  }

  if(0 == alist.size()) {
    std::cerr<<"Randomized list is empty! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ret++;
  }

  alist.clear();

  if(!alist.empty()) {
    std::cerr<<"Emptied list not empty! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ret++;
  }

  if(0 != alist.size()) {
    std::cerr<<"Emptied list not empty! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ret++;
  }
  return ret;

}

int testInsert()
{
  typedef Dune::SLList<int,IntAllocator> List;
  //typedef Dune::SLList<int> List;

  List alist;

  alist.push_back(3);
  List::ModifyIterator iter=alist.beginModify();
  iter.insert(7);
  int ret=0;

  if(*iter!=3) {
    std::cerr<<"Value at current position changed due to insert! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ret++;
  }

  if(*alist.begin()!=7) {
    std::cerr<<"Insert did not change first element! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ret++;
  }

  iter=alist.beginModify();
  iter.insert(5);

  if(iter==alist.end() || *iter!=7) {
    std::cerr<<"Insertion failed.! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ++ret;
  }

  if(*(alist.begin())!=5) {
    std::cerr<<"Insert after at onebeforeBegin did not change head! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ++ret;
  }
  iter = alist.endModify();

  if(iter!=alist.end()) {
    std::cerr <<" Iterator got by endModify does not equal that got by end()! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ++ret;
  }


  iter.insert(20);

  if(iter != alist.end()) {
    std::cerr<<"Insertion changed end iterator! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ++ret;
  }

  if(tail(alist)!=20) {
    std::cerr<<"tail was not changed!! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ++ret;
  }

  alist.clear();
  iter=alist.beginModify();
  iter.insert(5);
  if(iter!=alist.end()) {
    std::cerr<<"Insertion failed! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ++ret;
  }
  return ret;
}

int testPushPop(){
  using namespace Dune;
  int ret=0;

  Dune::SLList<int,IntAllocator> alist;
  //std::cout<<"PushPop 1:"<<alist<<std::endl;

  if(alist.begin() != alist.end()) {
    ret++;
    std::cerr<<"For empty list begin and end iterator do not match! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ret++;
  }

  alist.push_back(1);

  //std::cout<<"Push back 1: "<<alist<<std::endl;

  if(*(alist.begin())!=1) {
    std::cerr<<"Entry should be 1! Push back failed! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ret++;
  }

  alist.push_back(2);

  //std::cout<<"Push back 2: "<<alist<<std::endl;

  if(*(alist.begin())!=1) {
    ret++;
    std::cerr<<"Entry should be 2! Push back failed! "<<__FILE__<<":"<<__LINE__<<std::endl;
  }

  alist.push_front(3);
  //std::cout<<"Push front 3: "<<alist<<std::endl;

  if(*(alist.begin())!=3) {
    ret++;
    std::cerr<<"Entry should be 3! Push front failed! "<<__FILE__<<":"<<__LINE__<<std::endl;
  }

  alist.pop_front();
  //std::cout<<*(alist.begin())<<" Pop front: "<<alist<<std::endl;


  if(*(alist.begin())!=1) {
    ret++;
    std::cerr<<"Entry should be 1, but is "<<*(alist.begin())<<"! Push back failed! "<<__FILE__<<":"<<__LINE__<<std::endl;
  }
  return ret;
}

int main()
{
  int ret=0;

  //Dune::SLList<double> list;
  Dune::SLList<double,DoubleAllocator> list, list1;
  Dune::SLList<DoubleWrapper, DoubleWAllocator> list2;

  randomizeListBack(list1);
  randomizeListFront(list);

  Dune::SLList<double,DoubleAllocator> copied(list);
  if(copied.size()!=list.size()) {
    std::cerr << "Size of copied list does not match!"<<std::endl;
    ++ret;
  }
  else{
    typedef Dune::SLList<double,DoubleAllocator>::const_iterator Iterator;
    Iterator iend = list.end();
    for(Iterator iter1=list.begin(), iter2=copied.begin(); iter1 != iend; ++iter1, ++iter2)
      if(*iter1!=*iter2) {
        std::cerr << "Entries of copied are not the same!"<<std::endl;
        ++ret;
      }
  }

  randomizeListFront(list2);

  Printer<std::iterator_traits<Dune::SLList<double,DoubleAllocator>::ModifyIterator>::value_type> print;

  Dune::SLList<double,DoubleAllocator>::ModifyIterator lbegin = list.beginModify(), lend = list.endModify();

  double& d = lbegin.dereference();

  d=2.0;

  double& d1 = lbegin.dereference();

  d1=3.0;

  lbegin.dereference()=5.0;

  lbegin.operator*()=5.0;

  *lbegin=5.0;

  std::cout << "Testing ConstIterator "<<std::endl;
  ret+=testConstIterator(lbegin, lend, print);
  std::cout << "Testing Iterator "<<std::endl;
  ret+=testIterator(list);
  std::cout << "Testing Iterator "<<std::endl;
  ret+=testIterator(list1);

  std::cout<< " Test PushPop "<<std::endl;
  ret+=testPushPop();
  std::cout<<" Test OneBeforeBegin"<<std::endl;

  //ret+=testOneBeforeBegin(list1);

  std::cout<< "test empty"<<std::endl;
  ret+=testEmpty();
  std::cout<< "test insert"<<std::endl;

  ret+=testInsert();
  std::cout<< "test delete"<<std::endl;
  ret+=testDelete();

  ret+=testAssign();
  list.clear();
  list1.clear();
  list2.clear();
  std::cout<<" randomize back"<<std::endl;
  randomizeListBack(list);
  std::cout<<" randomize front"<<std::endl;
  randomizeListFront(list1);
  return ret;
}
