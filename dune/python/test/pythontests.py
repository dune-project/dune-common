from io import StringIO
classACode="""
struct MyClassA {
  MyClassA(int a,int b) : a_(a), b_(b) {}
  int a_,b_;
};
"""
classBCode="""
#include <cmath>
template <class T> struct MyClassB {
  MyClassB(T &t, int p) : a_(std::pow(t.a_,p)), b_(std::pow(t.b_,p)) {}
  int a_,b_;
};
"""
runCode="""
template <class T> int run(T &t) {
  return t.a_ * t.b_;
}
"""

def test_class_export():
    from dune.generator.importclass import load
    from dune.generator.algorithm   import run
    from dune.generator import path
    from dune.typeregistry import generateTypeName
    cls = load("MyClassA",StringIO(classACode),10,20)
    assert run("run",StringIO(runCode),cls) == 10*20
    clsName,includes = generateTypeName("MyClassB",cls)
    cls = load(clsName,StringIO(classBCode),cls,2)
    assert run("run",StringIO(runCode),cls) == 10**2*20**2
if __name__ == "__main__":
    try:
        from dune.common.module import get_dune_py_dir
        _ = get_dune_py_dir()
        test_class_export()
    except ImportError:
        pass
