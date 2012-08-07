// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // #ifdef HAVE_CONFIG_H

#include <iostream>

#include <dune/common/container/staticarray.hh>

int main ( int argc, char **argv )
{
  std::cout << "sizeof( Dune::StaticArray< int > ) = " << sizeof( Dune::StaticArray< int > ) << std::endl;

  Dune::StaticArray< int > a( 4 );
  for( int i = 0; i < 4; ++i )
    a[ i ] = i+20;

  return 0;
}
