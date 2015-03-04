// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_SINGLETON_HH
#define DUNE_SINGLETON_HH

#include <memory>

#include <dune/common/visibility.hh>

/**
 * @file
 * @brief Useful wrapper for creating singletons.
 *
 * Inspired by the article
 * <a href="http://www.codeguru.com/cpp/cpp/cpp_mfc/singletons/article.php/c755/">CodeGuru: A Leak-Free Singleton class</a>
 */
namespace Dune
{
  /**
   * @brief An adapter to turn a class into a singleton.
   *
   * The class represented by the template parameter T must
   * have a parameterless constructor.
   *
   * Class T can be publicly derived from Singleton<T>:
   *
   * \code
   * #include<dune/common/singleton.hh>
   * class Foo : public Dune::Singleton<Foo>
   * {
   * public:
   *   Foo()
   *   {
   *     bytes = new char[1000];
   *   }
   *
   *   ~Foo()
   *   {
   *     delete[] bytes;
   *   }
   * private:
   *   char* bytes;
   * };
   * \endcode
   *
   * Or one can construct a Singleton of an existing class. Say Foo1 is a class
   * with parameterless constructor then
   * \code
   * typedef Dune::Singleton<Foo1> FooSingleton;
   * Foo1 instance& = FooSingleton::instance();
   * \endcode
   * Creates a singleton of that class and accesses its instance.
   */
  template<class T>
  class Singleton
  {
  protected:
    /* @brief Private constructor. */
    Singleton(){}
    /** @brief Private copy constructor. */
    Singleton(const Singleton&){}
    /** @brief Private assignment operator. */
    Singleton& operator=(const Singleton&){}

  public:
    /**
     * @brief Get the instance of the singleton.
     * @return The instance of the singleton.
     */
    DUNE_EXPORT static T& instance()
    {
      /* Smartpointer to the instance. */
      static std::unique_ptr<T> instance_;
      if(instance_.get() == 0)
        instance_ = std::unique_ptr<T>(new T());
      return *instance_;
    }
  };

} // namespace Dune

#endif
