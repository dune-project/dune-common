#include <dune/common/copy_ptr.hh>
#include <iostream>

using namespace Dune;

struct A {
    virtual int foo() const = 0;
};

bool operator == (const A& a, const A& b)
{
    return (a.foo() == b.foo());
}

bool operator != (const A& a, const A& b)
{
    return (a.foo() != b.foo());
}

struct B : public A {
    B(int _i) : i(_i) { }
    B(const B & b) = default;
    int foo() const { return i; }
    int i;
};

struct DeepCopyError {};

struct NoCopy : public A {
    NoCopy(int _i) : i(_i) { }
    NoCopy(const NoCopy & b) { throw DeepCopyError(); }
    int foo() const { return i; }
    int i;
};

__attribute__ ((noinline))
bool touch(const nocopy_ptr<A> & ptr)
{
    if (ptr) return true;
    if (!ptr) return false;
}

using Ptr = nocopy_ptr<A>;

bool check_ptr_interface(Ptr & ptr)
{
    bool result = true;
    // copy_ptr must not be copy constructible
    result &= not std::is_copy_constructible<Ptr>::value;
    // get the pointer
    A* raw = ptr.get();
    result &= (raw != nullptr);
    // check explit cast to bool
    result &= bool(ptr);
    // get deleter and creator
    try {
        auto del = ptr.get_deleter();
        auto crea = ptr.get_creator();
    }
    catch (...)
    {
        result = false;
    }
    // same check with const interface
    try {
        const Ptr & cptr = ptr;
        auto del = cptr.get_deleter();
        auto crea = cptr.get_creator();
    }
    catch (...)
    {
        result = false;
    }
    // check operator* and operator->
    auto && a = *ptr;
    result &= (a.foo() == ptr->foo());

    return result;
}

bool check_move_interface(Ptr && ptr)
{
    bool result = true;
    // get the pointer
    A* raw = ptr.get();
    result &= (raw != nullptr);
    // temporary ptr
    Ptr tmp;
    result &= not bool(tmp);
    // try to move
    tmp = std::move(ptr);
    // now get must return null
    result &= (ptr.get() == nullptr);
    // and tmp must hold the object
    result &= (tmp.get() == raw);
    // and now use the move constructor
    Ptr tmp2 = std::move(tmp);
    // now tmp.get() must return null
    result &= (tmp.get() == nullptr);
    // and tmp2 must hold the object
    result &= (tmp2.get() == raw);

    return result;
}

bool check_clone_interface(const Ptr & ptr)
{
    bool result = true;
    // try to clone the ptr
    Ptr dup = clone(ptr);
    // the address must differ
    result &= (ptr.get() != dup.get());
    // the content must be equal
    result &= (*ptr == *dup);
    // check the ptr interface of the duplicated ptr
    result &= check_ptr_interface(dup);
    // check the move interface the duplicated ptr
    result &= check_move_interface(std::forward<Ptr>(dup));

    return result;
}

bool check_full_interface(Ptr && ptr)
{
    bool result = true;
    result &= check_ptr_interface(ptr);
    result &= check_clone_interface(ptr);
    result &= check_move_interface(std::forward<Ptr>(ptr));
    return result;
}

int main()
{
    bool result = true;
    // we create a derived class and check the full interface
    {
        std::cout << "== we create a derived class and check the full interface\n";
        nocopy_ptr<A> p(new B(2));
        result &= check_full_interface(std::forward<Ptr>(p));
    }
    // we create a derived class with the move constructor
    {
        std::cout << "== we create a derived class with the move constructor\n";
        nocopy_ptr<A> p = new B(2);
        result &= check_full_interface(std::forward<Ptr>(p));
    }
    // we try to create a class via make_nocopy_ptr
    {
    }
    // we try to create a class with a custom deleter
    {
        std::cout << "== we try to create a class with a custom deleter\n";
        int deletecount = 0;
        {
            auto del = [&deletecount](A* p) { deletecount++; delete p; };
            nocopy_ptr<A> p(new B(2), del);
            result &= check_full_interface(std::forward<Ptr>(p));
        }
        std::cout << "  deleter called "
                  << deletecount << " time(s)" << std::endl;
        result &= (deletecount == 1);
    }
    // we try to create a class with a custom deleter and creator
    {
        std::cout << "== we try to create a class with a custom deleter and creator\n";
        int deletecount = 0;
        int clonecount = 0;
        {
            auto del = [&deletecount](A* p) {
                deletecount++;
                delete p;
            };
            auto crea = [&clonecount,&del](A* p) {
                clonecount++;
                return Ptr(new B(static_cast<B&>(*p)),del);
            };
            nocopy_ptr<A> p(new B(2), crea, del);
            result &= check_full_interface(std::forward<Ptr>(p));
        }
        std::cout << "  creator called "
                  << clonecount << " time(s)" << std::endl;
        std::cout << "  deleter called "
                  << deletecount << " time(s)" << std::endl;
        result &= (deletecount == 2);
        result &= (clonecount == 1);
    }
    // we check that we don't do accidential deep copies by creating an object which throws an exception in case of a deep copy
    try {
        std::cout << "== we check that we don't do accidential deep copies by creating an object which throws an exception in case of a deep copy\n";
        nocopy_ptr<A> p(new NoCopy(2));
        result &= check_ptr_interface(p);
    }
    catch (DeepCopyError) {
        result = false;
    }
    // we check that we actually do a deep copy
    try {
        std::cout << "== we check that are actually do the deep copy\n";
        nocopy_ptr<A> p(new NoCopy(2));
        auto tmp = clone(p); // this should throw an exception!
        result = false;
        std::cout << "  failed\n";
    }
    catch (DeepCopyError) {
        std::cout << "  deep copy detected" << std::endl;
    }
    // check release
    {
        std::cout << "== we check that release actually transfers ownership\n";
        int deletecount = 0;
        A* raw = nullptr;
        {
            auto del = [&deletecount](A* p) {
                deletecount++;
                delete p;
            };
            nocopy_ptr<A> p(new B(2), del);
            // get the raw data
            raw = p.release();
            // now the ptr should be nullptr
            result &= (nullptr == p.get());
            result &= (nullptr == p.release());
        }
        if (raw) delete raw;
        std::cout << "  deleter called "
                  << deletecount << " time(s)" << std::endl;
        result &= (deletecount == 0);
    }
    // check deleter
    // check creater
    // check reset

    std::cout << "\n>> result: "
              << std::boolalpha
              << result << std::endl;
    return (result == true) ? 0 : 1;
}
