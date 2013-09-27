#
# Module that detects mproctect support
#
# Sets the following variables
# HAVE_SYS_MMAN_H
# HAVE_MPROTECT
check_include_file("sys/mman.h" HAVE_SYS_MMAN_H)
include(CheckCSourceCompiles)
check_c_source_compiles("
#include <sys/mman.h>
int main(void){
  mprotect(0,0,PROT_NONE);
}" HAVE_MPROTECT)
