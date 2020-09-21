# .. cmake_module::
#
#    This module internally sets the following variables, which are then
#    exported into the config.h of the current dune module.
#
#    :code:`HAS_ATTRIBUTE_UNUSED`
#       True if attribute unused is supported
#
#    :code:`HAS_ATTRIBUTE_DEPRECATED`
#       True if attribute deprecated is supported
#
#    :code:`HAS_ATTRIBUTE_DEPRECATED_MSG`
#       True if attribute deprecated("msg") is supported
#
# .. cmake_variable:: DISABLE_CXX_VERSION_CHECK
#
#    You may set this variable to TRUE to disable checking for
#    std=c++11 (c++14, c++1y). For more details, check :ref:`CheckCXXFeatures`.
#


include(CMakePushCheckState)
include(CheckCXXCompilerFlag)
include(CheckIncludeFileCXX)
include(CheckCXXSourceCompiles)
include(CheckCXXSymbolExists)

function(dune_require_cxx_standard)
  message(DEPRECATION "No need to call dune_require_cxx_standard() any more.
    If you want to enforce a higher standard than set in the core modules, simply use the cmake
    command target_compile_features(<target> <PRIVATE|PUBLIC|INTERFACE> cxx_std_[17|20|...])
    to set the requirement on your target directly.")
endfunction()

# perform tests

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


# find the threading library
if(NOT DEFINED THREADS_PREFER_PTHREAD_FLAG)
  set(THREADS_PREFER_PTHREAD_FLAG 1)
endif()
find_package(Threads)
# text for feature summary
set_package_properties("Threads" PROPERTIES
  DESCRIPTION "Multi-threading library")

# see whether threading needs -no-as-needed
if(EXISTS /etc/dpkg/origins/ubuntu)
  set(NO_AS_NEEDED "-Wl,-no-as-needed ")
else(EXISTS /etc/dpkg/origins/ubuntu)
  set(NO_AS_NEEDED "")
endif(EXISTS /etc/dpkg/origins/ubuntu)

set(STDTHREAD_LINK_FLAGS "${NO_AS_NEEDED}${CMAKE_THREAD_LIBS_INIT}"
    CACHE STRING "Linker flags needed to get working C++11 threads support.  On Ubuntu it may be necessary to include -Wl,-no-as-needed (see FS#1650).")

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



# ******************************************************************************
#
# Checks for standard library features
#
# While there are __cpp_lib_* feature test macros for all of these, those are
# unfortunately unreliable, as libc++ does not have feature test macros yet.
#
# In order to keep the tests short, they use check_cxx_symbol_exists(). That
# function can only test for macros and linkable symbols, however, so we wrap
# tested types into a call to std::move(). That should be safe, as std::move()
# does not require a complete type.
#
# ******************************************************************************

# Check whether we have <experimental/type_traits> (for is_detected et. al.)
check_include_file_cxx(
  experimental/type_traits
  DUNE_HAVE_HEADER_EXPERIMENTAL_TYPE_TRAITS
  )

check_cxx_symbol_exists(
  "std::move<std::bool_constant<true>>"
  "utility;type_traits"
  DUNE_HAVE_CXX_BOOL_CONSTANT
  )

if (NOT DUNE_HAVE_CXX_BOOL_CONSTANT)
  check_cxx_symbol_exists(
    "std::move<std::experimental::bool_constant<true>>"
    "utility;experimental/type_traits"
    DUNE_HAVE_CXX_EXPERIMENTAL_BOOL_CONSTANT
    )
endif()

check_cxx_symbol_exists(
  "std::apply<std::negate<int>,std::tuple<int>>"
  "functional;tuple"
  DUNE_HAVE_CXX_APPLY
  )

if (NOT DUNE_HAVE_CXX_APPLY)
  check_cxx_symbol_exists(
    "std::experimental::apply<std::negate<int>,std::tuple<int>>"
    "functional;experimental/tuple"
    DUNE_HAVE_CXX_EXPERIMENTAL_APPLY
  )
endif()

check_cxx_symbol_exists(
  "std::experimental::make_array<int,int>"
  "experimental/array"
  DUNE_HAVE_CXX_EXPERIMENTAL_MAKE_ARRAY
  )

check_cxx_symbol_exists(
  "std::move<std::experimental::detected_t<std::decay_t,int>>"
  "utility;experimental/type_traits"
  DUNE_HAVE_CXX_EXPERIMENTAL_IS_DETECTED
  )

check_cxx_symbol_exists(
  "std::identity"
  "functional"
  DUNE_HAVE_CXX_STD_IDENTITY
  )
