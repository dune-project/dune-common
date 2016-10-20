#include <config.h>

#include <array>
#include <iostream>
#include <tuple>

#include <dune/common/classname.hh>
#include <dune/common/streamoperators.hh>
#include <dune/common/std/utility.hh>

template< class T, std::size_t... Ints >
std::tuple< typename std::tuple_element< Ints, std::array< T, sizeof...( Ints ) > >::type... >
array_to_tuple_impl ( const std::array< T, sizeof...( Ints ) > &array, Dune::Std::index_sequence< Ints... > )
{
  auto tuple = std::make_tuple( array[ Ints ]... );
  return std::move( tuple );
}

template< class T, std::size_t N >
auto array_to_tuple ( const std::array< T, N > &array )
  -> decltype( array_to_tuple_impl( array, Dune::Std::make_index_sequence< N >{} ) )
{
  return array_to_tuple_impl( array, Dune::Std::make_index_sequence< N >{} );
}

int main ( int, char** )
{
  using Dune::operator<<;
  std::array< int, 4 > array{{ 1, 2, 3, 4 }};

  auto tuple = array_to_tuple_impl( array, Dune::Std::make_index_sequence< 4 >{} );
  std::cout << Dune::className( tuple ) << std::endl;
  std::cout << tuple << std::endl;
}
