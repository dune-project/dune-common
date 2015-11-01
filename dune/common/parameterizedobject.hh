// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
#ifndef PARAMETERIZED_OBJECT_HH
#define PARAMETERIZED_OBJECT_HH

#include <dune/common/shared_ptr.hh>
#include <dune/common/parametertree.hh>

namespace Dune {

/** default tag parameter for ParameterizedObjectFactory */
struct ParameterizedObjectDefaultTag {};

    namespace Impl {

        template<typename Interface,
                 typename Tag,
                 typename KeyT,
                 typename... Args
                 >
        class ParameterizedObjectFactoryBase {
        public:
            /** @brief The typ of the keys. */
            typedef KeyT Key;
            /** @brief The type of the shared pointer to the interface. */
            typedef Dune::shared_ptr<Interface> Type;
            /** @brief The type of the function that creates the object. */
            typedef Type (*Creator)(const Args& ... );

            /**
             * @brief Creates an object identified by a key from a parameter object.
             * @param key The key the object is registered with @see define.
             * @param d The parameter object used for the construction.
             * @return a shared_pointer to created object.
             */
            static
            Type create(Key const& key, const Args & ... args) {
                typename Registry::const_iterator i = registry().find(key);
                if (i == registry().end()) {
                    DUNE_THROW(Dune::InvalidStateException,
                        "ParametrizedObjectFactory: key ``" <<
                        key << "'' not registered");
                }
                else return i->second(args...);
            }

            /**
             * @brief Registers a new type with a key.
             *
             * After registration objects of this type can be cnstructed with the
             * specified key using the creat function.
             * @tparam Impl The type. It must implement and subclass Interface.
             */
            template<class Impl>
            static void define (Key const& key)
            {
                registry()[key] =
                    ParameterizedObjectFactoryBase::create_func<Impl>;
            }

        private:

            template<class Impl>
            static
            Type create_func(const Args & ... args) {
                return Dune::make_shared<Impl>(args...);
            }

            typedef std::map<Key, Creator> Registry;

            // singleton registry
            static Registry & registry()
            {
                static Registry _registry;
                return _registry;
            }
        };

    } // end namespace Impl

/**
 * @brief A factory class for parameterized objects.
 *
 * It allows the construction of objects adhering to a certain interface that
 * might be constructed quite differently for one another.
 *
 * Each type constructed by this factory is identified by a different key. This class
 * allows for easy registration of type with new keys.
 *
 * @tparam Interface The type that describes the interface of the objects.
 * @tparam Param The type of the parameter class used for constructing the objects [DEFAULT: Dune::ParameterTree].
 * @tparam Tag A class tag which allows to have different factories for the same interface [DEFAULT: ParameterizedObjectDefaultTag].
 * @tparam KeyT The type of the objects that are used as keys in the lookup [DEFAULT: std::string].
 */
template<typename Signature,
         typename Tag=ParameterizedObjectDefaultTag,
         typename KeyT=std::string>
class ParameterizedObjectFactory :
        public Impl::ParameterizedObjectFactoryBase<Signature,Tag,KeyT,ParameterTree> {};

template<typename Interface,
         typename Tag,
         typename KeyT,
         typename... Args>
class ParameterizedObjectFactory<Interface(Args...), Tag, KeyT> :
        public Impl::ParameterizedObjectFactoryBase<Interface,Tag,KeyT,Args...> {};


} // end namespace Dune

#endif // PARAMETRIZED_OBJECT_HH
