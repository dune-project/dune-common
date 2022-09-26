// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_PARAMETERIZEDOBJECT_HH
#define DUNE_COMMON_PARAMETERIZEDOBJECT_HH

#include <functional>
#include <map>
#include <memory>

#include <dune/common/exceptions.hh>
#include <dune/common/typeutilities.hh>

namespace Dune {

/**
 * @brief A factory class for parameterized objects.
 *
 * It allows the construction of objects adhering to a certain interface that
 * might be constructed quite differently for one another.
 *
 * The Signature parameter defined the "virtual" constructor signature
 * in the form of Interface(Args...), where Interface is the type of
 * the (abstract) interface class and Args... is the set of
 * constructor parameters.
 *
 * Each type constructed by this factory is identified by a different key. This class
 * allows for easy registration of type with new keys.
 *
 * @tparam Signature Signature of the "virtual" constructor call in the form for Interface(Args...). For default constructors one can omit the ()-brackets.
 * @tparam KeyT The type of the objects that are used as keys in the lookup [DEFAULT: std::string].
 */
template<typename Signature,
         typename KeyT = std::string>
class ParameterizedObjectFactory;

template<typename TypeT,
         typename KeyT,
         typename... Args>
class ParameterizedObjectFactory<TypeT(Args...), KeyT>
{
    public:

        /** @brief The typ of the keys. */
        typedef KeyT Key;

        /** @brief The type of objects created by the factory. */
        using Type = TypeT;

    protected:

        using Creator = std::function<Type(Args...)>;

        template<class F>
        static constexpr auto has_proper_signature(Dune::PriorityTag<1>)
            -> decltype( std::declval<F>()(std::declval<Args>()...), std::true_type())
        {
            return {};
        }

        template<class F>
        static constexpr std::false_type has_proper_signature(Dune::PriorityTag<0>)
        {
            return {};
        }

    public:

        /**
         * @brief Creates an object identified by a key from given parameters
         *
         * @param key The key the object is registered with @see define.
         * @param args The parameters used for the construction.
         * @return The object wrapped as Type
         */
        Type create(Key const& key, Args ... args) const {
            typename Registry::const_iterator i = registry_.find(key);
            if (i == registry_.end()) {
                DUNE_THROW(Dune::InvalidStateException,
                    "ParametrizedObjectFactory: key ``" <<
                    key << "'' not registered");
            }
            else return i->second(args...);
        }

        /**
         * @brief Registers a new type with a key.
         *
         * After registration objects of this type can be constructed with the
         * specified key using a matching default creation function. If Type
         * is a unique_ptr or shared_ptr, the object is created via make_unique
         * or make_shared, respectively. Otherwise a constructor of Impl
         * is called.
         *
         * @tparam Impl The type of objects to create.
         *
         * @param key The key associated with this type.
         */
        template<class Impl>
        void define(Key const& key)
        {
            registry_[key] = DefaultCreator<Impl>();
        }

        /**
         * @brief Registers a new creator with a key.
         *
         * After registration objects can be constructed using
         * the given creator function.
         *
         * @tparam F Type of creator function. This must be callable with Args... .
         *
         * @param key The key associated with this type.
         * @param f Function for creation of objects of type Impl
         *
         * \todo Replace has_proper_signature by concept check
         */
        template<class F,
            typename std::enable_if<has_proper_signature<F>(PriorityTag<42>()), int>::type = 0>
        void define(Key const& key, F&& f)
        {
            registry_[key] = f;
        }

        /**
         * @brief Registers a new type with a key.
         *
         * After registration objects of this type can be created.
         * This method will store a copy of the given object and
         * create will hand out a copy to this.
         *
         * @tparam Impl The type of objects to create.
         *
         * @param key The key associated with this type.
         * @param t reference object, "create" will call the copy-constructor
         *
         * note, this does not work fundamental types
         */
        template<class Impl,
            typename std::enable_if<
                std::is_convertible<Impl, Type>::value
                and not std::is_convertible<Impl, Creator>::value,
                int>::type = 0>
        void define(Key const& key, Impl&& t)
        {
            registry_[key] = [=](Args...) { return t;};
        }

        bool contains(Key const& key) const
        {
            return registry_.count(key);
        }

    private:

        template<class T>
        struct Tag{};

        template<class Impl>
        struct DefaultCreator
        {
            template<class...  T>
            Type operator()(T&&... args) const
            {
                return DefaultCreator::create(Tag<Type>(), PriorityTag<42>(), std::forward<T>(args)...);
            }

            template<class Target, class... T>
            static Type create(Tag<Target>, PriorityTag<1>, T&& ... args) {
                return Impl(std::forward<T>(args)...);
            }

            template<class Target, class... T>
            static Type create(Tag<std::unique_ptr<Target>>, PriorityTag<2>, T&& ... args) {
                return std::make_unique<Impl>(std::forward<T>(args)...);
            }

            template<class Target, class... T>
            static Type create(Tag<std::shared_ptr<Target>>, PriorityTag<3>, T&& ... args) {
                return std::make_shared<Impl>(std::forward<T>(args)...);
            }

        };

        typedef std::map<Key, Creator> Registry;
        Registry registry_;
};



} // end namespace Dune

#endif // DUNE_COMMON_PARAMETERIZEDOBJECT_HH
