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

cmake_pop_check_state()
