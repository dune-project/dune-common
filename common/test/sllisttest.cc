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

  DoubleWrapper()
    : d()
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
    alist.push_front((range*(rand()/(RAND_MAX+1.0))));
}

int testDelete()
{
  typedef Dune::SLList<int,Dune::PoolAllocator<int,8*1024-16> > List;
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
    std::cerr<<"delete next faild! "<<__FILE__<<":"<<__LINE__<<std::endl;
    return 1;
  }
  if(*(alist.tail())!=4) {
    std::cerr<<"delete before tail did not change tail! "<<__FILE__<<":"<<__LINE__<<std::endl;
  }

  return 0;
}

int testInsert()
{
  typedef Dune::SLList<int,Dune::PoolAllocator<int,8*1024-16> > List;
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

  if(*(alist.tail())!=20) {
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

template<typename T>
int testOneBeforeBegin(T& alist)
{
  typename T::iterator iterBefore = alist.oneBeforeBegin(),
  iter = alist.begin();
  typename T::const_iterator citerBefore = alist.oneBeforeBegin();

  int ret=0;
  ++iterBefore;
  ++citerBefore;

  if(iterBefore!=iter || &(*iterBefore) != &(*iter)) {
    std::cerr<<"one before iterator incremented once should point to begin()! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ret++;
  }
  if(citerBefore!=iter || &(*citerBefore) != &(*iter)) {
    std::cerr<<"one before iterator incremented once should point to begin()! "<<__FILE__<<":"<<__LINE__<<std::endl;
    ret++;
  }
  return ret;
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

  Printer<std::iterator_traits<Dune::SLList<double>::ModifyIterator>::value_type> print;

  Dune::SLList<double>::ModifyIterator lbegin = list.beginModify(), lend = list.endModify();

  double& d = lbegin.dereference();

  d= 2.0;

  double& d1 = lbegin.dereference();

  lbegin.dereference()=5.0;

  lbegin.operator*()=5.0;

  *lbegin=5.0;

  ret+=testConstIterator(lbegin, lend, print);
  ret+=testIterator(list);
  ret+=testIterator(list1);
  ret+=testPushPop();
  ret+=testOneBeforeBegin(list1);
  ret+=testInsert();
  ret+=testDelete();

  list.clear();
  list1.clear();
  list2.clear();
  randomizeListBack(list);
  randomizeListFront(list1);
  exit(ret);

}
