// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_SINGLETON_HH
#define DUNE_SINGLETON_HH
/**
 * @file
 * @brief Usefull wrapper for creating singletons.
 *
 * Inspirated by the article
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
   * Class T can be publically
   * derived from Singleton<T>:
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
  public:
    /**
     * @brief A simple smart pointer responsible for creation
     * and deletion of the instance.
     */
    class InstancePointer
    {
    public:
      /** @brief Construct a null pointer. */
      InstancePointer() : pointer_(0)
      {}
      /** @brief Delete the instance we point to. */
      ~InstancePointer()
      {
        if(pointer_ != 0)
          delete pointer_;
      }
      /**
       * @brief Get a pointer to the instance.
       * @return The instance we store.
       */
      T* get()
      {
        return pointer_;
      }
      /**
       * @brief Set the pointer.
       * @param pointer A pointer to the instance.
       */
      void set(T* pointer)
      {
        if(pointer != 0) {
          delete pointer_;
          pointer_ = pointer;
        }
      }
    private:
      T* pointer_;
    };
  private:
    /** @brief Smartpointer to the instance. */
    static InstancePointer instance_;
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
    static T& instance()
    {
      if(instance_.get() == 0)
        instance_.set(new T());
      return *instance_.get();
    }
  };

  template<class T>
  typename Singleton<T>::InstancePointer Singleton<T>::instance_;

} // namespace Dune

#endif
