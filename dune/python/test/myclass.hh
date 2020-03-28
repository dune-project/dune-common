#ifndef MYCLASS_HH
#define MYCLASS_HH

#include <cmath>

struct MyClassA
{
  MyClassA(int a,int b) : a_(a), b_(b) {}
  int a_,b_;
};
template <class T>
struct MyClassB
{
  MyClassB(T &t, int p) : a_(std::pow(t.a_,p)), b_(std::pow(t.b_,p)) {}
  int a_,b_;
};
template <class T>
int run(T &t)
{
  return t.a_ * t.b_;
}

#endif
