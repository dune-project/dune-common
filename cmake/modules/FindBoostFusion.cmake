#
# Module that checks whether boost::fusion is available and usable.
#
# Sets the follwing variable:
#
# HAVE_BOOST_FUSION True if boost::fusion is available.
#
include(DuneBoost)

if(HAVE_DUNE_BOOST)
  message(STATUS "Checking whether the Boost::FUSION library is available.")
  check_cxx_source_compiles("
\#include <boost/fusion/container.hpp>
int main(){
  boost::fusion::vector<int,char,double> v;
  return 0;
}" HAVE_BOOST_FUSION )
  if(HAVE_BOOST_FUSION)
    message(STATUS "Boost::FUSION is available")
  endif(HAVE_BOOST_FUSION)
else()
  message(STATUS "Skipping check for Boost::FUSION as Boost is not available.")
endif()
