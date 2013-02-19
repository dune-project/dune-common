include(DuneBoost)

message(STATUS "Checking whether the Boost::FUSION library is available")
check_cxx_source_compiles("
\#include <boost/fusion/container.hpp>
int main(){
  boost::fusion::vector<int,char,double> v;
  return 0;
}" HAVE_BOOST_FUSION )
if(HAVE_BOOST_FUSION)
message(STATUS "Boost::FUSION is available")
endif(HAVE_BOOST_FUSION)
