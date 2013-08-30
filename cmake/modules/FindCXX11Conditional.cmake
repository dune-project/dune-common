# Module that checks whether the compiler supports
# C++11 std::conditional.
#
# Sets the following variable:
# HAVE_STD_CONDITIONAL
#
# perform tests
include(CheckCXXSourceCompiles)

check_cxx_source_compiles("

  #include <type_traits>

  int main(void){
      return std::conditional<true,std::integral_constant<int,0>,void>::type::value;
  }"
  HAVE_STD_CONDITIONAL)
