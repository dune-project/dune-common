// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <array>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <sstream>

#include <dune/common/exceptions.hh>
#include <dune/common/parametertree.hh>
#include <dune/common/parametertreeparser.hh>

// This assert macro does not depend on the value of NDEBUG
#define check_assert(expr)                                          \
  do                                                                \
  {                                                                 \
    if(!(expr))                                                     \
    {                                                               \
      std::cerr << __FILE__ << ":" << __LINE__ << ": check_assert(" \
                << #expr << ") failed" << std::endl;                \
      std::abort();                                                 \
    }                                                               \
  } while(false)

// Check that the given expression throws the given exception
#define check_throw(expr, except)                               \
  do {                                                          \
    try {                                                       \
      expr;                                                     \
      std::cerr << __FILE__ << ":" << __LINE__ << ": " << #expr \
                << " should throw " << #except << std::endl;    \
      std::abort();                                             \
    }                                                           \
    catch(const except&) {}                                            \
    catch(...) {                                                \
      std::cerr << __FILE__ << ":" << __LINE__ << ": " << #expr \
                << " should throw " << #except << std::endl;    \
      std::abort();                                             \
    }                                                           \
  } while(false)

template<class P>
void testparam(const P & p)
{
  // try accessing key
  check_assert(p.template get<int>("x1") == 1);
  check_assert(p.template get<double>("x1") == 1.0);
  check_assert(p.template get<std::string>("x2") == "hallo");
  check_assert(p.template get<bool>("x3") == false);
  // try reading array like structures
  std::vector<unsigned int>
  array1 = p.template get< std::vector<unsigned int> >("array");
  std::array<unsigned int, 8>
  array2 = p.template get< std::array<unsigned int, 8> >("array");
  Dune::FieldVector<unsigned int, 8>
  array3 = p.template get< Dune::FieldVector<unsigned int, 8> >("array");
  check_assert(array1.size() == 8);
  for (unsigned int i=0; i<8; i++)
  {
    check_assert(array1[i] == i+1);
    check_assert(array2[i] == i+1);
    check_assert(array3[i] == i+1);
  }
  // try accessing subtree
  p.sub("Foo");
  p.sub("Foo").template get<std::string>("peng");
  // check hasSub and hasKey
  check_assert(p.hasSub("Foo"));
  check_assert(!p.hasSub("x1"));
  check_assert(p.hasKey("x1"));
  check_assert(!p.hasKey("Foo"));
  // try accessing inexistent key
  try {
    p.template get<int>("bar");
    DUNE_THROW(Dune::Exception, "failed to detect missing key");
  }
  catch (Dune::RangeError & r) {}
  // try accessing inexistent subtree in throwing mode
  try {
    p.sub("bar",true);
    DUNE_THROW(Dune::Exception, "failed to detect missing subtree");
  }
  catch (Dune::RangeError & r) {}
  // try accessing inexistent nested subtree in throwing mode
  try {
    p.sub("Foo.Zoo",true);
    DUNE_THROW(Dune::Exception, "failed to detect missing nested subtree");
  }
  catch (Dune::RangeError & r) {}
  // try accessing inexistent subtree in non-throwing mode
  p.sub("bar");
  // try accessing inexistent subtree that shadows a value key
  try {
    p.sub("x1.bar");
    DUNE_THROW(Dune::Exception, "succeeded to access non-existent subtree that shadows a value key");
  }
  catch (Dune::RangeError & r) {}
  // try accessing key as subtree
  try {
    p.sub("x1");
    DUNE_THROW(Dune::Exception, "succeeded to access key as subtree");
  }
  catch (Dune::RangeError & r) {}
  // try accessing subtree as key
  try {
    p.template get<double>("Foo");
    DUNE_THROW(Dune::Exception, "succeeded to access subtree as key");
  }
  catch (Dune::RangeError & r) {}
}

template<class P>
void testmodify(P parameterSet)
{
  parameterSet["testDouble"] = "3.14";
  parameterSet["testInt"]    = "42";
  parameterSet["testString"] = "Hallo Welt!";
  parameterSet["testVector"] = "2 3 5 7 11";
  parameterSet.sub("Foo")["bar"] = "2";

  double testDouble      = parameterSet.template get<double>("testDouble");
  int testInt            = parameterSet.template get<int>("testInt");
  ++testDouble;
  ++testInt;
  std::string testString = parameterSet.template get<std::string>("testString");
  typedef Dune::FieldVector<unsigned, 5> FVector;
  FVector testFVector    = parameterSet.template get<FVector>("testVector");
  typedef std::vector<unsigned> SVector;
  SVector testSVector    = parameterSet.template get<SVector>("testVector");
  if(testSVector.size() != 5)
    DUNE_THROW(Dune::Exception, "Testing std::vector<unsigned>: expected "
               "size()==5, got size()==" << testSVector.size());
  for(unsigned i = 0; i < 5; ++i)
    if(testFVector[i] != testSVector[i])
      DUNE_THROW(Dune::Exception,
                 "testFVector[" << i << "]==" << testFVector[i] << " but "
                 "testSVector[" << i << "]==" << testSVector[i]);
  if (parameterSet.template get<std::string>("Foo.bar") != "2")
    DUNE_THROW(Dune::Exception, "Failed to write subtree entry");
  if (parameterSet.sub("Foo").template get<std::string>("bar") != "2")
    DUNE_THROW(Dune::Exception, "Failed to write subtree entry");
}

void testOptionsParserResults(std::vector<std::string> args,
  const std::vector<std::string> & keywords,
  unsigned int required,
  bool allow_more,
  bool overwrite,
  std::string foo, std::string bar,
  const std::string referr = "")
{
  Dune::ParameterTree pt;
  try {
    char * argv[10];
    for (std::size_t i = 0; i < args.size(); ++i)
      argv[i] = &args[i][0];
    Dune::ParameterTreeParser::readNamedOptions(args.size(), argv, pt, keywords, required, allow_more, overwrite);
    check_assert(referr == "");
  }
  catch (const Dune::ParameterTreeParserError & e)
  {
    std::string err = e.what();
    std::size_t offset = err.find("]: ");
    err = err.substr(offset + 3, err.find('\n') - offset - 3);
    check_assert(referr == err);
  }
  if (foo != "" && foo != pt.get<std::string>("foo"))
    DUNE_THROW(Dune::Exception, "Options parser failed... foo = "
      << pt.get<std::string>("foo") << " != " << foo);
  if (bar != "" && bar != pt.get<std::string>("bar"))
    DUNE_THROW(Dune::Exception, "Options parser failed... bar = "
      << pt.get<std::string>("bar") << " != " << bar);
}

void testOptionsParser()
{
  std::vector<std::string> keywords = { "foo", "bar" };
  // check normal behaviour
  {
    std::vector<std::string> args = { "progname", "--bar=ligapokal", "peng", "--bar=ligapokal", "--argh=other"};
    testOptionsParserResults(args,keywords,keywords.size(),true,true,"peng","ligapokal",
      "" /* no error */ );
  }
  // bail out on overwrite
  {
    std::vector<std::string> args = { "progname", "--bar=ligapokal", "peng", "--bar=ligapokal", "--argh=other"};
    testOptionsParserResults(args,keywords,keywords.size(),true,false,"peng","ligapokal",
      "parameter bar already specified");
  }
  // bail out on unknown options
  {
    std::vector<std::string> args = { "progname", "--bar=ligapokal", "peng", "--bar=ligapokal", "--argh=other"};
    testOptionsParserResults(args,keywords,keywords.size(),false,true,"peng","ligapokal",
      "unknown parameter argh");
  }
  // bail out on missing parameter
  {
    std::vector<std::string> args = { "progname", "--bar=ligapokal"};
    testOptionsParserResults(args,keywords,keywords.size(),true,true,"","ligapokal",
      "missing parameter(s) ...  foo");
  }
  // check optional parameter
  {
    std::vector<std::string> args = { "progname", "--foo=peng"};
    testOptionsParserResults(args,keywords,1,true,true,"peng","",
      "" /* no error */);
  }
  // check optional parameter, but bail out on missing parameter
  {
    std::vector<std::string> args = { "progname", "--bar=ligapokal"};
    testOptionsParserResults(args,keywords,1,true,true,"","ligapokal",
      "missing parameter(s) ...  foo");
  }
  // bail out on too many parameters
  {
    std::vector<std::string> args = { "progname", "peng", "ligapokal", "hurz"};
    testOptionsParserResults(args,keywords,keywords.size(),true,true,"peng","ligapokal",
      "superfluous unnamed parameter");
  }
  // bail out on missing value
  {
    std::vector<std::string> args = { "progname", "--foo=peng", "--bar=ligapokal", "--hurz"};
    testOptionsParserResults(args,keywords,keywords.size(),true,true,"peng","ligapokal",
      "value missing for parameter --hurz");
  }
}

void testFS1527()
{
  { // Check that junk at the end is not accepted (int)
    Dune::ParameterTree ptree;
    check_throw(ptree["setting"] = "0.5"; ptree.get("setting", 0),
                Dune::RangeError);
  }
  { // Check that junk at the end is not accepted (double)
    Dune::ParameterTree ptree;
    check_throw(ptree["setting"] = "0.5 junk"; ptree.get("setting", 0.0),
                Dune::RangeError);
  }
}

// check that negative values can be given on the command line
void testFS1523()
{
  static char arg0[] = "progname";
  static char arg1[] = "-setting";
  static char arg2[] = "-1";
  static char *argv[] = { arg0, arg1, arg2, NULL };
  int argc = sizeof argv / sizeof (char *) - 1;

  Dune::ParameterTree ptree;
  Dune::ParameterTreeParser::readOptions(argc, argv, ptree);

  check_assert(ptree.get<int>("setting") == -1);
}

void check_recursiveTreeCompare(const Dune::ParameterTree & p1,
  const Dune::ParameterTree & p2)
{
  check_assert(p1.getValueKeys() == p2.getValueKeys());
  check_assert(p1.getSubKeys() == p2.getSubKeys());
  typedef Dune::ParameterTree::KeyVector::const_iterator Iterator;
  for (Iterator it = p1.getValueKeys().begin();
       it != p1.getValueKeys().end(); ++it)
    check_assert(p1[*it] == p2[*it]);
  for (Iterator it = p1.getSubKeys().begin();
       it != p1.getSubKeys().end(); ++it)
    check_recursiveTreeCompare(p1.sub(*it), p2.sub(*it));
}

// test report method and read back in
void testReport()
{
  std::stringstream s;
  s << "foo.i = 1 \n foo.bar.peng = hurz";
  Dune::ParameterTree ptree;
  Dune::ParameterTreeParser::readINITree(s, ptree);

  std::stringstream s2;
  ptree.report(s2);
  Dune::ParameterTree ptree2;
  Dune::ParameterTreeParser::readINITree(s2, ptree2);
  check_recursiveTreeCompare(ptree, ptree2);
}

int main()
{
  try {
    // read config
    std::stringstream s;
    s << "x1 = 1 # comment\n"
      << "x2 = hallo\n"
      << "x3 = no\n"
      << "array = 1   2 3 4 5\t6 7 8\n"
      << "\n"
      << "[Foo] # another comment\n"
      << "peng = ligapokal\n";

    auto c = Dune::ParameterTreeParser::readINITree(s);

    // test modifying and reading
    testmodify<Dune::ParameterTree>(c);
    try {
      c.get<int>("testInt");
      DUNE_THROW(Dune::Exception, "unexpected shallow copy of ParameterTree");
    }
    catch (Dune::RangeError & r) {}

    // test for complex
    c.get<std::complex<double>>("x1");

    // more const tests
    testparam<Dune::ParameterTree>(c);

    // check the command line parser
    testOptionsParser();

    // check report
    testReport();

    // check for specific bugs
    testFS1527();
    testFS1523();
  }
  catch (Dune::Exception & e)
  {
    std::cout << e << std::endl;
    return 1;
  }
  return 0;
}
