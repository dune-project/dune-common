// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <dune/common/sllist.hh>
#include <dune/common/test/iteratortest.hh>
#include <dune/common/poolallocator.hh>
#include <iostream>

class DoubleWrapper
{
public:
  DoubleWrapper(double b)
    : d(b)
  {}

  operator double()
  {
    return d;
  }

private:
  double d;
};


template<typename T,class A>
void randomizeListBack(Dune::SLList<T,A>& alist){
  using namespace Dune;

  srand((unsigned)time(0));

  int lowest=0, highest=1000, range=(highest-lowest)+1;

  for(int i=0; i < 10; i++)
    alist.push_back(T(range*(rand()/(RAND_MAX+1.0))));
}

template<typename T,class A>
void randomizeListFront(Dune::SLList<T,A>& alist){
  using namespace Dune;

  srand((unsigned)time(0));

  int lowest=0, highest=1000, range=(highest-lowest)+1;

  for(int i=0; i < 10; i++)
    alist.push_back((range*(rand()/(RAND_MAX+1.0))));
}

int testPushPop(){
  using namespace Dune;
  int ret=0;

  Dune::SLList<int,PoolAllocator<int,8*1024-16> > alist;

  if(alist.begin() != alist.end()) {
    ret++;
    std::cout<<"For empty list begin and end iterator do not match! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ret++;
  }

  alist.push_back(1);
  if(*(alist.begin())!=1) {
    std::cout<<"Entry should be 1! Push back failed! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ret++;
  }

  alist.push_back(2);
  if(*(alist.begin())!=1) {
    ret++;
    std::cout<<"Entry should be 2! Push back failed! "<<__FILE__<<":"<<__LINE__<<std::endl;
  }

  alist.push_front(3);
  if(*(alist.begin())!=3) {
    ret++;
    std::cout<<"Entry should be 3! Push front failed! "<<__FILE__<<":"<<__LINE__<<std::endl;
  }

  alist.pop_front();
  if(*(alist.begin())!=1) {
    ret++;
    std::cout<<"Entry should be 1! Push back failed! "<<__FILE__<<":"<<__LINE__<<std::endl;
  }
  return ret;
}

int main()
{
  int ret=0;

  Dune::SLList<double> list;
  Dune::SLList<double,Dune::PoolAllocator<double, 8*1024-20> > list1;
  Dune::SLList<DoubleWrapper, Dune::PoolAllocator<DoubleWrapper, 8*1024-20> > list2;
  randomizeListBack(list1);
  randomizeListFront(list);
  randomizeListFront(list2);


  ret+=testIterator(list);
  ret+=testIterator(list1);
  ret+=testPushPop();

  list.clear();
  list1.clear();
  list2.clear();
  randomizeListBack(list);
  randomizeListFront(list1);
  exit(ret);

}
