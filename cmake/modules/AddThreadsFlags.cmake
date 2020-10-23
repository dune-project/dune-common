# Always link threading libraries to targets

# set HAVE_THREADS for config.h
set(HAVE_THREADS ${Threads_FOUND})

# register the Threads imported target globally
if(Threads_FOUND)
  link_libraries(Threads::Threads)
endif()
