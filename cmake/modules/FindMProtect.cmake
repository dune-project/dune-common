# .. cmake_module::
#
#    Module that detects mprotect support
#
#    Sets the following variables:
#
#    * :code:`HAVE_SYS_MMAN_H`
#    * :code:`HAVE_MPROTECT`
#

include(CheckIncludeFile)
check_include_file("sys/mman.h" HAVE_SYS_MMAN_H)
include(CheckCSourceCompiles)
check_c_source_compiles("
#include <sys/mman.h>
int main(void){
  mprotect(0,0,PROT_NONE);
}" HAVE_MPROTECT)
