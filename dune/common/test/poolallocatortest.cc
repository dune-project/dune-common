// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/poolallocator.hh>
#include <dune/common/fmatrix.hh>

using namespace Dune;

struct UnAligned
{
  char t;
  char s;
  char k;
};



template<std::size_t size, typename T>
struct testPoolMain
{
  static int test()
  {
    int ret=0;

    Pool<T,size> pool;

    int elements = Pool<T,size>::elements;
    //int poolSize        = Pool<T,size>::size;
    //int chunkSize   = Pool<T,size>::chunkSize;
    //int alignedSize = Pool<T,size>::alignedSize;

    unsigned long* oelements = new unsigned long[10*elements];

    typedef typename Pool<T,size>::Chunk Chunk;

    //Fill 10 chunks
    for(int chunk=0; chunk < 10; ++chunk) {
      //std::cout<< std::endl<<"Chunk "<<chunk<<" ";
      unsigned long element = reinterpret_cast<unsigned long>(pool.allocate());
      //void* celement = reinterpret_cast<void*>(element);
      //std::cout << element<<" "<< celement<<",  "<<std::endl;

      Chunk* currentChunk = pool.chunks_;

      assert(element==reinterpret_cast<unsigned long>(currentChunk->memory_));
      unsigned long end = reinterpret_cast<unsigned long>(currentChunk->chunk_)+Pool<T,size>::chunkSize;

      if(element< reinterpret_cast<unsigned long>(currentChunk->chunk_))
      {
        std::cerr <<" buffer overflow during first alloc: "<<reinterpret_cast<unsigned long>(currentChunk->chunk_)
                  <<">"<<element<<"+"<<sizeof(T)<<std::endl;
        return ++ret;
      }

      if(end < element + sizeof(T)) {
        std::cerr <<" buffer overflow during first alloc: "<<end<<"<"<<element<<"+"<<sizeof(T)<<std::endl;
        return ++ret;
      }

      oelements[chunk*elements]=element;

      for(int i=1; i < elements; i++)
      {
        element = reinterpret_cast<unsigned long>(pool.allocate());
        //celement = reinterpret_cast<void*>(element);
        //std::cout << element<<" "<<celement<<",  "<<std::endl;

        if(element< reinterpret_cast<unsigned long>(currentChunk->chunk_)) {
          std::cerr <<" buffer underflow during first alloc: "<<reinterpret_cast<unsigned long>(currentChunk->chunk_)
                    <<">"<<element<<"+"<<sizeof(T)<<std::endl;
          return ++ret;
        }

        if(end < element + sizeof(T)) {
          std::cerr <<" buffer overflow during "<<i<<" alloc: "<<end<<"<"<<element+sizeof(T)<<std::endl;
          return ++ret;

        }

        if(oelements[chunk*elements+i-1]+sizeof(T)>element) {
          std::cerr<<"allocated elements overlap!"<<std::endl;
          return ++ret;
        }

        oelements[chunk*elements+i]=element;
      }
    }



    for(int i=0; i < elements*10; ++i)
      pool.free(reinterpret_cast<void*>(oelements[i]));
    delete[] oelements;

    return ret;
  }
};

template<typename T>
int testPool()
{
  const std::size_t size = sizeof(T)>=2 ? sizeof(T)-2 : 0;

  int ret=0;

  std::cout<<"Checking "<<typeid(T).name()<<" sizeof="<<sizeof(T)<<" with size "<< size<<
  " alignment="<< alignof(T) <<std::endl;

  ret += testPoolMain<0,T>::test();
  ret += testPoolMain<size,T>::test();
  ret += testPoolMain<5*size,T>::test();
  ret += testPoolMain<11*size,T>::test();
  ret += testPoolMain<33*size,T>::test();

  return ret;
}

int testPoolAllocator()
{
  int ret=0;
  PoolAllocator<double,10> pool;
  double *d=pool.allocate(1);
  PoolAllocator<float,5> pool1=pool;
  PoolAllocator<double,10> pool2=pool;
  try
  {
    pool2.deallocate(d,1);
#ifndef NDEBUG
    ++ret;
    std::cerr<<"ERROR: deallocation should not work with copied allocators."<<std::endl;
#endif
  }
  catch(std::bad_alloc)
  {}
  pool1.allocate(1);
  double *d1=pool2.allocate(1);
  pool.deallocate(d,1);
  pool2.deallocate(d1,1);
  pool2.allocate(1);
  return ret;
}
int main(int, char **)
{
  int ret=0;

  ret += testPool<int>();

  ret+= testPool<double>();

  ret+= testPool<char>();

  ret += testPool<Dune::FieldMatrix<double,10,10> >();

  ret+=testPoolAllocator();

  std::cout<< alignof(UnAligned) <<" "<<sizeof(UnAligned)<<std::endl;

  ret += testPool<UnAligned>();

  return ret;
}
