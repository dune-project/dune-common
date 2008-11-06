// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#ifdef __GNUC__
#include <ext/malloc_allocator.h>
#endif

#include <dune/common/blockbitfield.hh>

template<int block_size, class Alloc>
void doTest() {
  typedef Dune::BlockBitField<block_size, Alloc> BBF;
  typedef typename BBF::value_type bitset;
  typedef typename BBF::reference reference;

  BBF bbf(10,true);

  bitset x = bbf[3];
  reference y = bbf[4];
  y[2] = true;
  y = x;
  x = y;

  const BBF & cbbf = bbf;
  y = cbbf[1];
  x = cbbf[1];
}

int main()
{
  doTest<4, std::allocator<bool> >();
#ifdef __GNUC__
  doTest<4, __gnu_cxx::malloc_allocator<bool> >();
#endif

  return 0;
}
