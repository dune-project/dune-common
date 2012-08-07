// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // #ifdef HAVE_CONFIG_H

#include <iostream>

#include <dune/common/container/dynamicarray.hh>

int main ( int argc, char **argv )
{
  std::cout << "sizeof( Dune::DynamicArray< int > ) = " << sizeof( Dune::DynamicArray< int > ) << std::endl;

  std::cout << "sizeof( Dune::DynamicArray< int, Dune::StaticCapacityManager<> > ) = "
            << sizeof( Dune::DynamicArray< int, Dune::StaticCapacityManager<> > ) << std::endl;

  std::cout << "sizeof( Dune::DynamicArray< int, Dune::AdaptiveCapacityManager<> > ) = "
            << sizeof( Dune::DynamicArray< int, Dune::AdaptiveCapacityManager<> > ) << std::endl;

  Dune::DynamicArray< int > a( 4 );
  for( int i = 0; i < 4; ++i )
    a[ i ] = i+20;

  Dune::DynamicArray< int, Dune::StaticCapacityManager<> > b( 4 );
  for( int i = 0; i < 4; ++i )
    b[ i ] = a[ i ] + 20;

  Dune::DynamicArray< int, Dune::AdaptiveCapacityManager<> > c( 4 );
  for( int i = 0; i < 4; ++i )
    c[ i ] = b[ i ] + 20;

  return 0;
}
