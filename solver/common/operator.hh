// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef OPERATOR_HH
#define OPERATOR_HH

#include <iostream>
// needed for bzero
#include <string.h>

namespace Dune {

  typedef double FLOAT;

  //! Where to store the data
  template <class GRID,class T=FLOAT> class Vector;
  //! Generic gridOperator
  template <class GRID, class OP> class gridOperator;
  //! Generic Class for mathematic operations
  template <class A, class B, class OP> class OperatorMath;
  //! Specific Classes for + operations
  template <class A, class B> class OperatorPlus;
  //! Specific Classes for - operations
  template <class A, class B> class OperatorMinus;
  //! Specific Classes for * operations
  template <class A, class B> class OperatorTimes;
  //! Specific Classes for / operations
  template <class A, class B> class OperatorDiv;

  //! Dummy class
  class EMPTY {};

  //! We need our own Buffer to store ,,temporary objects''
  class OperatorStack {
    enum { maxStackSize = 256 };
    void ** pointer;
    int _count;
  public:
    OperatorStack() : _count(0) {
      pointer = new void*[maxStackSize];
    }
    ~OperatorStack() {
      clear();
      delete pointer;
    }
    template <template <class,class> class OP, class A, class B>
    OP<A,B> &
    newOperator(A & a, B & b) {
      pointer[_count] = new OP<A,B>(a,b);
      OP<A,B> * P = static_cast<OP<A,B>*>(pointer[_count++]);
      return *P;
      //reinterpret_cast<OP<A,B>&>(*pointer[_count++]);
    }
    int count() { return _count; };
    inline void clear();
    void swap(OperatorStack & s) {
      void ** buffer = pointer;
      int c = _count;
      pointer = s.pointer;
      _count = s._count;
      s.pointer = buffer;
      s._count = c;
    }
  };
  OperatorStack opStack;

  template <class X>
  class Operator {
  private:
    X& engine() { return static_cast<X&>(*this); };
  public:
    // dummy implementations because pre-/postProcess often do nothing
    void preProcess() {};
    void postProcess() {};
    // That's the way to concatinate multiple objects
    template <class OP>
    OperatorPlus<X, OP> &
    operator + (OP & o) {
      return opStack.newOperator<OperatorPlus>(engine(), o);
    }
    template<class OP>
    OperatorMinus<X, OP> &
    operator - (OP & o) {
      return opStack.newOperator<OperatorMinus>(engine(), o);
    }
    template<class OP>
    OperatorTimes<X, OP> &
    operator * (OP & o) {
      return opStack.newOperator<OperatorTimes>(engine(), o);
    }
    template<class OP>
    OperatorDiv<X, OP> &
    operator / (OP & o) {
      return opStack.newOperator<OperatorDiv>(engine(), o);
    }
  };

  template <class A, class B, class OP>
  class OperatorMath : public Operator< OperatorMath<A,B,OP> > {
  protected:
    A & a;
    B & b;
  public:
    OP& engine() { return static_cast<OP&>(*this); };
    OperatorMath(A & _a, B & _b) : a(_a), b(_b) {};
    void preProcess() { a.preProcess(); b.preProcess(); };
    void postProcess() { a.postProcess(); b.postProcess(); };
    FLOAT applyLocal(int i) { return static_cast<OP&>(*this).applyLocal(i); }
  };

    #ifdef BLUB
  template<class OP>
  class Expression : public Operator< Expression<OP> > {
    #else
  class Expression : public Operator< Expression > {
    #endif
    OperatorStack localStack;
  public:
    #ifndef BLUB
    template<class OP>
    #endif
    Expression(OP & O) {
      static OP & o = O;
      localStack.swap(opStack);
    };
    //    FLOAT applyLocal(int i) { return o.applyLocal(i); }
  };

  template <class GRID, class OP>
  class gridOperator : public Operator< gridOperator<GRID,OP> > {
  protected:
    const GRID & g;
  public:
    OP& engine() { return static_cast<OP&>(*this); };
    gridOperator(const GRID & G) : g(G) {};
    void preProcess() { return static_cast<OP&>(*this).preProcess(); }
    void postProcess() { return static_cast<OP&>(*this).postProcess(); }
    FLOAT applyLocal(int i) { return static_cast<OP&>(*this).applyLocal(i); }
  };

  template <class GRID, class T>
  class Vector : public gridOperator< GRID, Vector<GRID> > {
    int _size;
    T *_data;
  public:
    Vector(const GRID &G) : gridOperator< GRID, Vector<GRID> >(G) {
      _size = g.max(g.smoothest());
      _data = new T[_size];
    }
    Vector(const GRID &G, FLOAT d) : gridOperator< GRID, Vector<GRID> >(G) {
      _size = g.max(g.smoothest());
      _data = new T[_size];
      for (int i=0; i<_size; i++) _data[i]=d;
    }
    Vector(const GRID &G, int d) : gridOperator< GRID, Vector<GRID> >(G) {
      _size = g.max(g.smoothest());
      _data = new T[_size];
      for (int i=0; i<_size; i++) _data[i]=d;
    }
    ~Vector() { delete[] _data; }
#if 0
    template <class OP>
    Vector(OP o) {
      o.preProcess();
      for (int i=0; i<_size; i++) {
        _data[i] = o.applyLocal(i);
      }
      o.postProcess();
    }
#endif
    Vector& operator = (Vector & o) {
      for (int i=0; i<_size; i++) { _data[i] = o.applyLocal(i); }
      opStack.clear();
      return *this;
    }
    template<class OP>
    Vector& operator = (OP & o) {
      o.preProcess();
      for (int i=0; i<_size; i++) { _data[i] = o.applyLocal(i); }
      o.postProcess();
      opStack.clear();
      return *this;
    }
    Vector& operator = (double d) {
      for (int i=0; i<_size; i++) { _data[i] = d; }
      return *this;
    }
    T & operator [] (int i) { return _data[i]; }
    const T & operator [] (int i) const { return _data[i]; }
    T* data() { return _data; };
    // dummy implementations
    void preProcess() {};
    void postProcess() {};
    // return value at pos i
    FLOAT applyLocal(int i) {
      return _data[i];
    };
    // print your Vector
    friend
    inline std::ostream &
    operator<< (std::ostream &o, const Vector<GRID> & v) {
      o << "[";
      for (int n=0; n<v._size-1; n++)
        o << v[n] << ",";
      o << v[v._size-1] << "]";
      return o;
    }
  };


  template <class A, class B>
  class OperatorPlus : public OperatorMath<A,B, OperatorPlus<A,B> > {
  public:
    OperatorPlus(A & _a, B & _b) :
      OperatorMath<A,B, OperatorPlus<A,B> >(_a, _b) {};
    FLOAT applyLocal(int i) {
      return a.applyLocal(i) + b.applyLocal(i);
    }
  };

  template <class A, class B>
  class OperatorMinus : public OperatorMath<A,B, OperatorMinus<A,B> > {
  public:
    OperatorMinus(A & _a, B & _b) :
      OperatorMath<A,B, OperatorMinus<A,B> >(_a, _b) {};
    FLOAT applyLocal(int i) {
      return a.applyLocal(i) - b.applyLocal(i);
    }
  };

  template <class A, class B>
  class OperatorTimes : public OperatorMath<A,B, OperatorTimes<A,B> > {
  public:
    OperatorTimes (A & _a, B & _b) :
      OperatorMath<A,B, OperatorTimes<A,B> >(_a, _b) {};
    FLOAT applyLocal(int i) {
      return a.applyLocal(i) * b.applyLocal(i);
    }
  };

  template <class A, class B>
  class OperatorDiv : public OperatorMath<A,B, OperatorDiv<A,B> > {
  public:
    OperatorDiv (A & _a, B & _b) :
      OperatorMath<A,B, OperatorDiv<A,B> >(_a, _b) {};
    FLOAT applyLocal(int i) {
      return a.applyLocal(i) / b.applyLocal(i);
    }
  };

  class Const : public Operator<Const> {
    FLOAT c;
  public:
    Const(FLOAT C) : c(C) {};
    FLOAT applyLocal(int) { return c; };
  };

  template <class GRID, class OP>
  class Shift : public gridOperator< GRID, Shift<GRID,OP> > {
    OP & o;
  public:
    Shift(const GRID & G, OP & O) :
      gridOperator< GRID, Shift<GRID,OP> >(G), o(O) {};
    FLOAT applyLocal(int i) {
      if (i > 0) return o.applyLocal(i-1);
      return o.applyLocal(g.max(g.smoothest())-1);
    };
  };
  template <class GRID, class OP>
  Shift<GRID,OP> shift(const GRID &g, OP & o) {
    Shift<GRID,OP> s(g,o); return s;
  };

  inline void OperatorStack::clear() {
    while(_count>0) {
      OperatorMath<EMPTY,EMPTY,EMPTY> * P =
        static_cast<OperatorMath<EMPTY,EMPTY,EMPTY> *>(pointer[_count-1]);
      delete(P);
      _count--;
    }
  };
}

#endif // OPERATOR_HH
