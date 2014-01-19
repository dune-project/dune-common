# Module checks for shared_ptr support.
#
# Sets the following variables:
#
# HAVE_BOOST_MAKE_SHARED_HPP: True if boost/make_shared.hpp is found
# SHARED_PTR_NAMESPACE: Namespace of shared_ptr (e.g. std)
# SHARED_PTR_HEADER: The name of header file supplying shared_ptr
#
# check if make_shared works
macro(check_make_shared)
  include(CheckIncludeFileCXX)

  if(SHARED_PTR_NAMESPACE EQUAL "boost")
    check_include_file_cxx("boost/make_shared.hpp" HAVE_BOOST_MAKE_SHARED_HPP)
  endif(SHARED_PTR_NAMESPACE EQUAL "boost")

  check_include_file_cxx("boost/shared_ptr.hpp" HAVE_BOOST_SHARED_PTR_HPP)

  check_cxx_source_compiles("
    #if defined(HAVE_MEMORY)
    # include <memory>
    #endif
    #if defined(HAVE_TR1_MEMORY)
    # include <tr1/memory>
    #endif
    #if defined(HAVE_BOOST_SHARED_PTR_HPP) && defined(HAVE_BOOST_MAKE_SHARED_HPP)
    # include <boost/shared_ptr.hpp>
    # include <boost/make_shared.hpp>
    #endif

    int main(void)
    {
       ${SHARED_PTR_NAMESPACE}::make_shared<int>(3);
       return 0;
    }
    " HAVE_MAKE_SHARED)
endmacro(check_make_shared)

# check location of shared_ptr header file and the necessary namespace
include(CheckCXXSourceCompiles)

# search namespace
foreach(SHARED_PTR_NAMESPACE_ "std" "tr1" "std::tr1" "boost")
  check_cxx_source_compiles("
    #include <memory>
    #include <string>

    using ${SHARED_PTR_NAMESPACE_}::shared_ptr;
    using namespace std;

    int main(void)
    {
      shared_ptr<string> test_ptr(new string(\"test string\"));
      return 0;
    }"
    SHARED_PTR_NAMESPACE_FOUND)

  if(SHARED_PTR_NAMESPACE_FOUND)
    #search header name
    foreach(SHARED_PTR_HEADER_ "<memory>" "<tr1/memory>" "<boost/shared_ptr.hpp>")
      check_cxx_source_compiles("
        # include ${SHARED_PTR_HEADER_}
        #include <string>

        using ${SHARED_PTR_NAMESPACE_}::shared_ptr;
        using namespace std;

        int main(void)
        {
          shared_ptr<string> test_ptr(new string(\"test string\"));
          return 0;
        }"
        SHARED_PTR_HEADER_FOUND)

      if(SHARED_PTR_HEADER_FOUND)
        # save result
        set(SHARED_PTR_NAMESPACE ${SHARED_PTR_NAMESPACE_})
        set(SHARED_PTR_HEADER ${SHARED_PTR_HEADER_})
        check_make_shared()
        return()
      endif(SHARED_PTR_HEADER_FOUND)
    endforeach(SHARED_PTR_HEADER_)
  endif(SHARED_PTR_NAMESPACE_FOUND)
endforeach(SHARED_PTR_NAMESPACE_)
