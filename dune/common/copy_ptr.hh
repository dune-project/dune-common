#include <memory>
#include <functional>

/**
 * @file
 * @author Christian Engwer
 */

namespace Dune {

    /**
        \brief a smart pointer that prohibits copy operations, but support an explit deep copy (clone)

        A nocopy_ptr can
        * not be copies
        * be moved
        * be cloned/deep-copied

        The object is disposed using the associated deleter when either of the following happens:
        * the managing unique_ptr object is destroyed
        * the managing unique_ptr object is assigned another pointer via operator= or reset().

        The object is disposed using a potentially user-supplied deleter by calling get_deleter()(ptr).

        The nocopy_ptr and the internal data is cloned using a potentially user-supplied creator by calling get_creator()(ptr).

        The default deleter uses the delete operator, which destroys the object and deallocates the memory.

        A nocopy_ptr may alternatively own no object, in which case it is called empty.
     */
    template <typename T>
    class nocopy_ptr {
    public:
        using Type = T;
        using Creator = std::function<nocopy_ptr(Type*)>;
        using Deleter = std::function<void(Type*)>;

        /** @{ */

        /** Default contructor */
        nocopy_ptr() : _ptr(nullptr) {}

        /** \brief Create a nocopy_ptr from a raw pointer

            \note the default deleter is used, which calls `delete ptr`
            \note the default creater is used, which uses the copy constructor `new Imp(*ptr)`
        */
        template <typename Imp>
        nocopy_ptr(Imp * imp) :
            _ptr(imp)
        {
            _creator = [](Type* t) {
                return nocopy_ptr(new Imp(static_cast<Imp&>(*t)));
            };
            _deleter = [](Type* t) {
                delete t;
            };
        }

        /** \brief Create a nocopy_ptr from a raw pointer and assign a user-supplied deleter

            \note the deleter has to match to way the user created imp outside
            \note the default creater is used, which uses the copy constructor `new Imp(*ptr)`
        */
        template <typename Imp, typename Del>
        nocopy_ptr(Imp * imp, Del && my_deleter) :
            _ptr(imp),
            _deleter(std::forward<Del>(my_deleter))
        {
            _creator = [](Type* t) {
                return nocopy_ptr(new Imp(static_cast<Imp&>(*t)));
            };
        }

        /** \brief Create a nocopy_ptr from a raw pointer and assign a user-supplied creator and deleter

            \note the deleter has to match to way the user created imp outside
            \note the creater has to make sure the the newly created nocopy_ptr gets a matching deleter
        */
        template <typename Imp, typename Crea, typename Del>
        nocopy_ptr(Imp * imp, Crea && create, Del && my_deleter) :
            _ptr(imp),
            _creator(std::forward<Crea>(create)),
            _deleter(std::forward<Del>(my_deleter))
        {}

        /** we prohibit copy construction */
        nocopy_ptr(const nocopy_ptr &) = delete;

        /** \brief move constructor */
        nocopy_ptr(nocopy_ptr && other) :
            _ptr(nullptr)
        {
            swap(*this,other);
        }

        /** @} */

        /** \brief destructor

            if the pointer is valid, the object will be cleaned up using the deleter
         */
        ~nocopy_ptr() {
            if (_ptr)
                _deleter(_ptr);
        }

        /** \brief move assignment */
        nocopy_ptr & operator = (nocopy_ptr && other)
        {
            using std::swap;
            swap(*this,other);
            return *this;
        }

        /** dereferences pointer to the managed object */
        Type& operator* () const
        {
            return *_ptr;
        }

        /** dereferences pointer to the managed object */
        Type* operator-> () const
        {
            return _ptr;
        }

        /** \brief the pointer casts to true if it is not a nullptr */
        explicit operator bool() const
        {
            return (_ptr != 0);
        }

        // TODO
        /*
          void reset(Imp*)
          void reset(Imp*,Del)
          void reset(Imp*,Crea,Del)
         */

        /** \brief Releases the ownership of the managed object if any.

            get() returns nullptr after the call.
         */
        Type* release()
        {
            Type *p = nullptr;
            using std::swap;
            swap(p,_ptr);
            return p;
        }

        /** \brief Returns the stored pointer. */
        Type* get() const
        {
            return _ptr;
        }

        Deleter & get_deleter()
        {
            return _deleter;
        }

        const Deleter & get_deleter() const
        {
            return _deleter;
        }

        Creator & get_creator()
        {
            return _creator;
        }

        const Creator & get_creator() const
        {
            return _creator;
        }

        friend nocopy_ptr clone (const nocopy_ptr & other)
        {
            if (other._ptr)
                return other._creator(other._ptr);
            return nocopy_ptr();
        }

        friend void swap(nocopy_ptr & a, nocopy_ptr & b)
        {
            using std::swap;
            swap(a._ptr,b._ptr);
            swap(a._creator,b._creator);
            swap(a._deleter,b._deleter);
        }

    private:
        Type* _ptr;
        Creator _creator;
        Deleter _deleter;
    };

    // template<typename T1, typename T2, typename... Args>
    // nocopy_ptr<T> make_nocopy_ptr()
    // {
    // }

} // end namespace Dune
