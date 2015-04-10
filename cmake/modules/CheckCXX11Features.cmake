#
# Module that checks for supported C++11 (former C++0x) features.
#
# Sets the follwing variables:
#
# HAVE_NULLPTR                     True if nullptr is available
# HAS_ATTRIBUTE_UNUSED             True if attribute unused is supported
# HAS_ATTRIBUTE_DEPRECATED         True if attribute deprecated is supported
# HAS_ATTRIBUTE_DEPRECATED_MSG     True if attribute deprecated("msg") is supported
# HAVE_CONSTEXPR                   True if constexpr is supported
# HAVE_KEYWORD_FINAL               True if final is supported.
# HAVE_RANGE_BASED_FOR             True if range-based for is supported and working.
# HAVE_NOEXCEPT_SPECIFIER          True if nonexcept specifier is supported.

include(CMakePushCheckState)
cmake_push_check_state()

# test for C++11 flags
include(TestCXXAcceptsFlag)

if(NOT DISABLE_CXX11CHECK)
  # try to use compiler flag -std=c++11
  check_cxx_accepts_flag("-std=c++11" CXX_FLAG_CXX11)
endif(NOT DISABLE_CXX11CHECK)

if(CXX_FLAG_CXX11)
  set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -std=c++11")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11 ")
  set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -std=c++11 ")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11 ")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -std=c++11 ")
  set(CXX_STD11_FLAGS "-std=c++11")
else()
  if(NOT DISABLE_CXX11CHECK)
    # try to use compiler flag -std=c++0x for older compilers
    check_cxx_accepts_flag("-std=c++0x" CXX_FLAG_CXX0X)
    if(NOT CXX_FLAG_CXX0X)
      MESSAGE(FATAL_ERROR "Your compiler does not seem to support C++11. If it does, please add any required flags to your CXXFLAGS and run dunecontrol with --disable-cxx11check")
    endif(NOT CXX_FLAG_CXX0X)
  endif(NOT DISABLE_CXX11CHECK)
  if(CXX_FLAG_CXX0X)
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -std=c++0x" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x ")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++0x ")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -std=c++0x ")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++0x ")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -std=c++0x ")
  set(CXX_STD11_FLAGS "-std=c++0x")
  endif(CXX_FLAG_CXX0X)
endif(CXX_FLAG_CXX11)
# perform tests
include(CheckCXXSourceCompiles)

# nullptr
check_cxx_source_compiles("
    int main(void)
    {
      char* ch = nullptr;
      return 0;
    }
"  HAVE_NULLPTR
)

# __attribute__((unused))
check_cxx_source_compiles("
   int main(void)
   {
     int __attribute__((unused)) foo;
     return 0;
   };
"  HAS_ATTRIBUTE_UNUSED
)

# __attribute__((deprecated))
check_cxx_source_compiles("
#define DEP __attribute__((deprecated))
   class bar
   {
     bar() DEP;
   };

   class peng { } DEP;

   template <class T>
   class t_bar
   {
     t_bar() DEP;
   };

   template <class T>
   class t_peng {
     t_peng() {};
   } DEP;

   void foo() DEP;

   void foo() {}

   int main(void)
   {
     return 0;
   };
"  HAS_ATTRIBUTE_DEPRECATED
)

# __attribute__((deprecated("msg")))
check_cxx_source_compiles("
#define DEP __attribute__((deprecated(\"message\")))
   class bar {
     bar() DEP;
   };

   class peng { } DEP;

   template <class T>
   class t_bar
   {
     t_bar() DEP;
   };

   template <class T>
   class t_peng
   {
     t_peng() {};
   } DEP;

   void foo() DEP;

   void foo() {}

   int main(void)
   {
     return 0;
   };
"  HAS_ATTRIBUTE_DEPRECATED_MSG
)

# constexpr
check_cxx_source_compiles("
  constexpr int foo()
  { return 0; }

  template<int v>
  struct A
  {
    static const int value = v;
  };

  int main(void)
  {
    return A<foo()>::value;
  }
" HAVE_CONSTEXPR
)

# keyword final
check_cxx_source_compiles("
  struct Foo
  {
    virtual void foo() final;
  };

  int main(void)
  {
    return 0;
  }
" HAVE_KEYWORD_FINAL
)

# range-based for
check_cxx_source_compiles("
  int main(void)
  {
    int arr[3];
    for(int &val : arr)
      val = 0;
  }
" HAVE_RANGE_BASED_FOR
)

# nonexcept specifier
check_cxx_source_compiles("
  void func1() noexcept {}

  void func2() noexcept(true) {}

  template <class T>
  void func3() noexcept(noexcept(T())) {}

  int main(void)
  {
    func1();
    func2();
    func3<int>();
  }
" HAVE_NOEXCEPT_SPECIFIER
)

# std::declval()
check_cxx_source_compiles("
  #include <utility>

  template<typename T>
  struct check;

  template<>
  struct check<int&&>
  {
    int pass() { return 0; }
  };

  int main(void)
  {
    return check<decltype(std::declval<int>())>().pass();
  }
" HAVE_STD_DECLVAL
  )

# full support for is_indexable (checking whether a type supports operator[])
check_cxx_source_compiles("
  #include <utility>
  #include <type_traits>
  #include <array>

  template <class T>
  typename std::add_rvalue_reference<T>::type declval();

  namespace detail {

    template<typename T, typename I, typename = int>
    struct _is_indexable
      : public std::false_type
    {};

    template<typename T, typename I>
    struct _is_indexable<T,I,typename std::enable_if<(sizeof(declval<T>()[declval<I>()]) > 0),int>::type>
      : public std::true_type
    {};

  }

  template<typename T, typename I = std::size_t>
  struct is_indexable
    : public detail::_is_indexable<T,I>
  {};

  struct foo_type {};

  int main()
  {
    double x;
    std::array<double,4> y;
    double z[5];
    foo_type f;

    static_assert(not is_indexable<decltype(x)>::value,\"scalar type\");
    static_assert(is_indexable<decltype(y)>::value,\"indexable class\");
    static_assert(is_indexable<decltype(z)>::value,\"array\");
    static_assert(not is_indexable<decltype(f)>::value,\"not indexable class\");
    static_assert(not is_indexable<decltype(y),foo_type>::value,\"custom index type\");

    return 0;
  }
" HAVE_IS_INDEXABLE_SUPPORT
  )


cmake_pop_check_state()

# find the threading library
# Use a copy FindThreads from CMake 3.1 due to its support of pthread
if(NOT DEFINED THREADS_PREFER_PTHREAD_FLAG)
  set(THREADS_PREFER_PTHREAD_FLAG 1)
endif()
if(${CMAKE_VERSION} VERSION_LESS "3.1")
  find_package(ThreadsCMake31)
else()
  find_package(Threads)
endif()
set(STDTHREAD_LINK_FLAGS "${CMAKE_THREAD_LIBS_INIT}"
    CACHE STRING "Linker flags needed to get working C++11 threads support")

# set linker flags
#
# in all implementations I know it is sufficient to set the linker flags when
# linking the final executable, so this should work.  In cmake, this appears
# to only work when building the project however, not for later config tests
# (contrary to CMAKE_CXX_FLAGS).  Luckily, later tests don't seem to use any
# threading...  (except for our own sanity check)
if(NOT STDTHREAD_LINK_FLAGS STREQUAL "")
  #set(vars CMAKE_EXE_LINKER_FLAGS ${CMAKE_CONFIGURATION_TYPES})
  # CMAKE_CONFIGURATION_TYPES seems to be empty.  Use the configurations from
  # adding -std=c++11 above instead.
  set(vars CMAKE_EXE_LINKER_FLAGS DEBUG MINSIZEREL RELEASE RELWITHDEBINFO)
  string(REPLACE ";" ";CMAKE_EXE_LINKER_FLAGS_" vars "${vars}")
  string(TOUPPER "${vars}" vars)
  foreach(var ${vars})
    if(NOT var STREQUAL "")
      set(${var} "${${var}} ${STDTHREAD_LINK_FLAGS}")
    endif()
  endforeach(var ${vars})
endif(NOT STDTHREAD_LINK_FLAGS STREQUAL "")

include(CheckCXXSourceRuns)
# check that the found configuration works
if(CMAKE_CROSSCOMPILING)
  message(WARNING "Crosscompiling, cannot run test program to see whether "
    "std::thread works.  Assuming that the found configuration does indeed "
    "work.")
endif(CMAKE_CROSSCOMPILING)

if(NOT DEFINED STDTHREAD_WORKS)
  if(NOT CMAKE_CROSSCOMPILING)
    # The value is not in the cache, so run check
    cmake_push_check_state()
    # tests seem to ignore CMAKE_EXE_LINKER_FLAGS
    set(CMAKE_REQUIRED_LIBRARIES "${STDTHREAD_LINK_FLAGS} ${CMAKE_REQUIRED_LIBRARIES}")
    check_cxx_source_runs("
      #include <thread>

      void dummy() {}

      int main() {
        std::thread t(dummy);
        t.join();
      }
    " STDTHREAD_WORKS)
    cmake_pop_check_state()
  endif(NOT CMAKE_CROSSCOMPILING)
  # put the found value into the cache.  Put it there even if we're
  # cross-compiling, so the user can find it.  Use FORCE:
  # check_cxx_source_runs() already puts the value in the cache but without
  # documentation; also the "if(NOT DEFINED STDTHREAD_WORKS)" will prevent us
  # from overwriting a value set by the user.
  set(STDTHREAD_WORKS "${STDTHREAD_WORKS}"
      CACHE BOOL "Whether std::thread works." FORCE)
endif(NOT DEFINED STDTHREAD_WORKS)

if(NOT STDTHREAD_WORKS)
  # Working C++11 threading support is required for dune.  In particular to
  # make things like lazyly initialized caches thread safe
  # (e.g. QuadratureRules::rule(), which needs std::call_once()).  If we don't
  # include the correct options during linking, there will be very funny
  # errors at runtime, ranging from segfaults to
  #
  #  terminate called after throwing an instance of 'std::system_error'
  #    what():  Unknown error 18446744073709551615
  message(FATAL_ERROR "Your system does not seem to have a working "
    "implementation of std::thread.  If it does, please set the linker flags "
    "required to get std::thread working in the cache variable "
    "STDTHREAD_LINK_FLAGS.  If you think this test is wrong, set the cache "
    "variable STDTHREAD_WORKS.")
endif(NOT STDTHREAD_WORKS)
