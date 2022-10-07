// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_PYTHON_COMMON_TYPEREGISTRY_HH
#define DUNE_PYTHON_COMMON_TYPEREGISTRY_HH

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

#include <dune/common/classname.hh>
#include <dune/common/typelist.hh>
#include <dune/common/visibility.hh>

#include <dune/python/pybind11/pybind11.h>
#include <dune/python/pybind11/stl.h>

#include <dune/python/pybind11/detail/internals.h>

namespace Dune
{

  namespace Python
  {

    namespace detail
    {

      struct DUNE_PRIVATE Entry
      {
        std::string name;
        std::string pyName;
        std::vector< std::string > includes;
        pybind11::object object;
      };


      // using an unordered_map directly for the type registry leads to a compilation
      // error in the cast used in the typeRegistry function:
      //   assertion failed: Unable to cast type to reference: value is local to type caster
      struct DUNE_PRIVATE TypeRegistry : public pybind11::detail::type_map<Entry>
      {};


      inline static TypeRegistry &typeRegistry ()
      {
        // BUG: Capturing the pybind11::object in a static variable leads to a
        //      memory fault in Python 3.6 upon module unloading.
        //      As a simple fix, we reobtain the reference each time the type
        //      registry is requested.
#if 0
        static pybind11::object instance;
        if( !instance )
          instance = pybind11::module::import( "dune.typeregistry" ).attr( "typeRegistry" );
        return pybind11::cast< TypeRegistry & >( instance );
#endif
        return pybind11::cast< TypeRegistry & >( pybind11::module::import( "dune.typeregistry" ).attr( "typeRegistry" ) );
      }


      template< class T >
      inline static auto findInTypeRegistry ()
      {
        auto pos = typeRegistry().find( typeid(T) );
        return std::make_pair( pos, pos == typeRegistry().end() );
      }


      template< class T >
      inline static auto insertIntoTypeRegistry (
          const std::string &name,
          const std::string &pyName,
          std::vector< std::string > includes )
      {
        auto ret = typeRegistry().emplace( typeid(T), Entry() );
        if( ret.second )
        {
          Entry &entry = ret.first->second;
          entry.name = name;
          entry.pyName = pyName;
          entry.includes = std::move( includes );
        }
        return ret;
      }



      // TypeRegistryTag
      // ---------------

      struct TypeRegistryTag {};



      // GenerateTypeName
      // ----------------

      struct GenerateTypeName
        : public TypeRegistryTag
      {
        template <class... Templ>
        GenerateTypeName(const std::string &main, Templ... templ)
        : main_(main)
        {
          templates(templ...);
        }
        template <class T, class... options, class... Templ>
        GenerateTypeName(const std::string &outer, const std::string &main, Templ... templ)
        : main_(outer+"::"+main)
        {
          templates(templ...);
        }
        template <class... Templ>
        GenerateTypeName(pybind11::handle &outer, const std::string &main, Templ... templ)
        {
          main_ = getTypeName(outer) + "::" + main;
          includes_.push_back(getIncludes(outer));
          std::sort( includes_.begin(), includes_.end() );
          includes_.erase( std::unique( includes_.begin(), includes_.end() ), includes_.end() );
          templates(templ...);
        }
        template <class Outer, class... Templ>
        GenerateTypeName(Dune::MetaType<Outer>, const std::string &main, Templ... templ)
        {
          const auto& outerEntry = findInTypeRegistry<Outer>();
          if (outerEntry.second)
            throw std::invalid_argument( (std::string("couldn't find outer class ") +
                  className<Outer>() + " in type registry").c_str() );
          main_ = outerEntry.first->second.name + "::" + main;
          includes_.push_back(outerEntry.first->second.includes);
          std::sort( includes_.begin(), includes_.end() );
          includes_.erase( std::unique( includes_.begin(), includes_.end() ), includes_.end() );
          templates(templ...);
        }
        GenerateTypeName(const std::string &main, pybind11::args args)
        : main_(main)
        {
          const std::size_t sz = args.size();
          for( std::size_t i = 0; i < sz; ++i )
          {
            templates_.insert( templates_.end(), getTypeName( (pybind11::handle)(args[i]) ) );
            includes_.insert( includes_.end(), getIncludes( (pybind11::handle)(args[i]) ) );
            std::sort( includes_.begin(), includes_.end() );
            includes_.erase( std::unique( includes_.begin(), includes_.end() ), includes_.end() );
          }
        }

        std::string name () const
        {
          std::string fullName = main_;
          if( !templates_.empty() )
          {
            const char *delim = "< ";
            for( const auto &t : templates_ )
            {
              fullName += delim + t;
              delim = ", ";
            }
            fullName += " >";
          }
          return fullName;
        }
        std::vector<std::string> includes() const
        {
          std::vector<std::string> ret;
          for (const auto &i : includes_)
            ret.insert( ret.end(), i.begin(), i.end() );
          return ret;
        }

      private:
        template <class... Args>
        void templates(Args... args)
        {
          templates_.insert(templates_.end(), { getTypeName( std::forward< Args >( args ) )... } );
          includes_.insert( includes_.end(), { getIncludes( std::forward<Args >( args ) )... } );
          std::sort( includes_.begin(), includes_.end() );
          includes_.erase( std::unique( includes_.begin(), includes_.end() ), includes_.end() );
        }
        template <class T,class... options>
        std::string getTypeName(const pybind11::class_<T,options...> &obj)
        {
          return getTypeName(static_cast<pybind11::handle>(obj));
        }
        std::string getTypeName(const pybind11::object &obj)
        {
          return getTypeName(static_cast<pybind11::handle>(obj));
        }
        std::string getTypeName(const pybind11::handle &obj)
        {
          try
          {
            return obj.attr( "cppTypeName" ).cast<std::string>();
          }
          catch(const pybind11::error_already_set& )
          {
            return pybind11::str(obj);
          }
        }
        static std::string getTypeName ( const GenerateTypeName &name ) { return name.name(); }
        std::string getTypeName ( const std::string &name ) { return name; }
        std::string getTypeName ( const char *name ) { return name; }
        template <class T>
        std::string getTypeName ( const Dune::MetaType<T> & )
        {
          auto entry = detail::findInTypeRegistry<T>();
          if (entry.second)
            throw std::invalid_argument( (std::string("couldn't find requested type ") +
                  className<T>() + " in type registry").c_str() );
          return entry.first->second.name;
        }
        template <class T>
        std::string getTypeName ( const T& t ) { return std::to_string(t); }

        static std::vector<std::string> getIncludes(pybind11::handle obj)
        {
          try
          {
            return obj.attr( "cppIncludes" ).cast<std::vector<std::string>>();
          }
          catch(const pybind11::error_already_set& )
          {
            return {};
          }
        }
        static std::vector< std::string > getIncludes ( const GenerateTypeName &name ) { return name.includes(); }
        template <class T>
        const std::vector<std::string> &getIncludes ( const Dune::MetaType<T> & )
        {
          auto entry = detail::findInTypeRegistry<T>();
          if (entry.second)
            throw std::invalid_argument( (std::string("couldn't find requested type ") +
                className<T>() + " in type registry").c_str() );
          return entry.first->second.includes;
        }
        template <class T>
        std::vector<std::string> getIncludes ( const T& ) { return {}; }

        std::string main_;
        std::vector<std::string> templates_;
        std::vector<std::vector<std::string>> includes_;
      };



      // IncludeFiles
      // ------------

      struct IncludeFiles
        : public std::vector< std::string >,
          TypeRegistryTag
      {
        template <class... Args>
        IncludeFiles(Args... args)
        : std::vector<std::string>({args...}) {}
      };


      template< class DuneType >
      inline static auto _addToTypeRegistry ( std::string pyName, const GenerateTypeName &typeName, const std::vector< std::string > &inc = {} )
      {
        std::vector< std::string > includes = typeName.includes();
        includes.insert( includes.end(), inc.begin(), inc.end() );
        auto entry = detail::insertIntoTypeRegistry< DuneType >( typeName.name(), std::move( pyName ), includes );
        if( !entry.second )
          throw std::invalid_argument( std::string( "adding a class (" ) + className<DuneType>() + ") twice to the type registry" );
        return entry;
      }


      template <typename S, typename M, typename O = std::index_sequence<>>
      struct filter : O {};
      template <std::size_t I, std::size_t... Is,
                std::size_t... Js, std::size_t... Ks>
      struct filter<std::index_sequence<I, Is...>, std::index_sequence<0, Js...>,
             std::index_sequence<Ks...>>
          : filter<std::index_sequence<Is...>, std::index_sequence<Js...>,
                   std::index_sequence<Ks...>> {};
      template <std::size_t I, std::size_t... Is, std::size_t... Js,
                std::size_t... Ks>
      struct filter<std::index_sequence<I, Is...>, std::index_sequence<1, Js...>,
                    std::index_sequence<Ks...>>
          : filter<std::index_sequence<Is...>, std::index_sequence<Js...>,
                   std::index_sequence<Ks..., I>> {};

      template< template< class T> class F, class... Args >
      using Filter = filter< std::index_sequence_for< Args... >, std::index_sequence< F< Args >{}... > >;

      template< class DuneType >
      inline static auto
      _addToTypeRegistry( const std::string &pyName, const IncludeFiles &inc, const GenerateTypeName &typeName )
      {
        return _addToTypeRegistry<DuneType>(std::move(pyName),typeName,inc);
      }

      template< class DuneType, class... Args, std::size_t... Is >
      inline static auto
      _addToTypeRegistry_filter_impl( const std::string &pyName, std::tuple< Args... > &&tuple, std::index_sequence< Is... > )
      {
        return _addToTypeRegistry<DuneType>(std::move(pyName),std::get<Is>(std::move(tuple))...);
      }

      template< class DuneType, class... options, class... Args >
      inline static auto
      generateClass ( pybind11::handle scope, const char *name, Args... args )
      {
        return pybind11::class_<DuneType,options...>(scope,name,args...);
      }

      template< class DuneType, class... options, class... Args, std::size_t... Is >
      inline static auto
      generateClass_filter_impl ( pybind11::handle scope, const char *name, std::tuple<Args...>&& tuple, std::index_sequence< Is... > )
      {
        return generateClass<DuneType,options...>(scope,name,std::get<Is>(std::move(tuple))...);
      }

      template<class B>
      struct negation : std::integral_constant<bool,!bool(B::value)> { };
      template <class T>
      using baseTag = std::is_base_of<TypeRegistryTag,T>;
      template <class T>
      using notBaseTag = negation<baseTag<T>>;

    } // namespace detail

    /** \brief Generate class name as string given the base part plus a list of the
     * template arguments.
     *
     * This class is used to generate a string storing the full C++ class
     * name, i.e., 'Foo<A,B>' given the string 'Foo' and the two types 'A'
     * and 'B'. The template arguments can be given as
     * - string, bool, or int (anything that works with std::to_string
     *   basically, so even double...)
     * - Dune::MetaType<T>, in which case the type T must be available in
     *   the type registry
     * - pybind::object, in which case the attribute `cppTypeName` must be
     *   available
     * .
     *
     * In the last two cases the include files stored in the type registry
     * for this type or attached to the object via the `cppIncludes`
     * attribute are collected.
     *
     * The main constructor is
     *   template <class... Templ>
     *   GenerateTypeName(const std::string &main, Templ... templ)
     *
     * Further constructors are available to handle cases like
     *    Bar::Foo<A,B>
     * The outer type can again be either given as a string, a
     * Dune::MetaType, or a pybind11::object.
     *
     * At the moment constructs like Bar::Traits::Foo or Bar<A>::Foo<B> are
     * not possible except in the case where the outer type (e.g.
     * Bar::Traits) can be passed in as a string.
     *
     */
    using GenerateTypeName = detail::GenerateTypeName;

    /** \brief A class used in the free standing method insertClass to tag
     * the list of include files in the argument list
     */
    using IncludeFiles = detail::IncludeFiles;

    using detail::findInTypeRegistry;

    /** \brief add a type to the type registry without it being exported to python
     */
    template <class DuneType>
    inline static void addToTypeRegistry ( const GenerateTypeName &typeName,
                                           const std::vector< std::string > &inc = {}
                                         )
    {
      std::vector<std::string> includes = typeName.includes();
      includes.insert(includes.end(), inc.begin(), inc.end());
      detail::insertIntoTypeRegistry<DuneType>(typeName.name(),"",includes);
    }


    /** Function used to generate a new pybind11::class_ object and to add
     * an entry to the type registry.
     *
     * If 'DuneType' passed in as first argument will be exported to Python
     * by adding it to the given scope. If the type has already been insert
     * using this method the correct pybind11::class_ object is added to
     * the scope and returned.
     *
     * Usage:
     * \code
     *   auto entry = insertClass<Foo<A>,py11_toption1,py11_toption2>(scope, "Foo",
     *           py11_option1(), py11_option2(),
     *           GenerateTypeName( "Foo", Dune::MetaType<A>() ),
     *           IncludeFiles{ "file1.hh", "file2.hh" } );
     *   if (entry.second)
     *     registerFoo(entry.first);
     *   return entry.first);
     * \endcode
     *
     * \tparam Type the type of the dune class to export
     * \tparam options  variadic template arguments passed on to * pybind11::class_
     * \param scope  the scope into which to export the python class
     * \param pyName the name to use for the python export
     * \param args   variadic argument passed on to the constructor of
     *               pybind11::class_ with the exception of an argument
     *               the type 'GenerateTypeName' and 'IncludeFiles'
     * \return
     *   \code
     *   make_pair(pybind11::class_<Type,options...>("pyName",args...), isNew);
     *   \endcode
     *   The first argument is the pybind11::class_
     *   object (either newly created or extracted from the type
     *   registry) The second argument is false if the type was
     *   already registered and otherwise it is true.
     */
    template< class Type, class... options, class... Args >
    inline static std::pair< pybind11::class_< Type, options... >, bool >
    insertClass ( pybind11::handle scope, std::string pyName, Args... args )
    {
      auto entry = detail::findInTypeRegistry<Type>();
      if( !entry.second )
      {
        if( scope )
          scope.attr( pyName.c_str() ) = entry.first->second.object;
        return std::make_pair( static_cast< pybind11::class_< Type, options... > >( entry.first->second.object ), false );
      }
      else
      {
        auto entry = detail::_addToTypeRegistry_filter_impl< Type >( std::move( pyName ), std::forward_as_tuple( std::forward< Args >( args )... ), detail::Filter< detail::baseTag, std::decay_t< Args >... >{} );
        auto cls = detail::generateClass_filter_impl< Type, options...>( scope, entry.first->second.pyName.c_str(), std::forward_as_tuple( std::forward< Args >( args )... ), detail::Filter< detail::notBaseTag, std::decay_t< Args >... >{} );
        entry.first->second.object = cls;

        cls.def_property_readonly_static( "_typeName", [ entry ] ( pybind11::object ) {
#ifdef DUNE_DISABLE_PYTHON_DEPRECATION_WARNINGS
          PyErr_WarnEx(PyExc_DeprecationWarning, "attribute '_typeName' is deprecated, use 'cppTypeName' instead.", 2);
#endif
          return entry.first->second.name;
        });
        cls.def_property_readonly_static( "_includes", [ entry ] ( pybind11::object ) {
#ifdef DUNE_DISABLE_PYTHON_DEPRECATION_WARNINGS
          PyErr_WarnEx(PyExc_DeprecationWarning, "attribute '_includes' is deprecated, use 'cppIncludes' instead.", 2);
#endif
          return entry.first->second.includes;
        });
        cls.def_property_readonly_static( "cppTypeName", [ entry ] ( pybind11::object ) { return entry.first->second.name; } );
        cls.def_property_readonly_static( "cppIncludes", [ entry ] ( pybind11::object ) { return entry.first->second.includes; } );

        return std::make_pair( cls, true );
      }
    }


    // registerTypeRegistry
    // --------------------

    inline static void registerTypeRegistry ( pybind11::module scope )
    {
      using pybind11::operator""_a;

      pybind11::class_< detail::TypeRegistry > cls( scope, "TypeRegistry" );

      scope.attr( "typeRegistry" ) = pybind11::cast( std::make_unique< detail::TypeRegistry >() );

      scope.def( "generateTypeName", []( std::string className, pybind11::args targs ) {
          GenerateTypeName gtn( className, targs );
          return std::make_pair( gtn.name(), gtn.includes() );
        }, "className"_a );
    }

  } // namespace Python

} // namespace Dune

#endif // #ifndef DUNE_PYTHON_COMMON_TYPEREGISTRY_HH
