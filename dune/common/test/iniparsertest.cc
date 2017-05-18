#include <exception>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include <dune/common/iniparser.hh>

bool parsesTo(std::string input, std::string key, std::string value) {
  try {
    std::stringstream sstream;
    sstream << input << std::endl;

    std::map<std::string, std::string> map;
    parse(sstream, [&](std::string const &p, std::string const &k,
                       std::string const &v) {
      std::string full_key = p == "" ? k : (p + "." + k);
      map[full_key] = v;
    });

    return map[key] == value;
  } catch (ParsingException) {
    return false;
  }
}

bool failsToParse(std::string input) {
  try {
    std::stringstream sstream;
    sstream << input << std::endl;

    std::map<std::string, std::string> map;
    parse(sstream, [&](std::string const &p, std::string const &k,
                       std::string const &v) {});
    return false;
  } catch (ParsingException) {
    return true;
  }
}

int all_tests() {
  int errors = 0;

  // resetting the prefix to the empty prefix
  if (!parsesTo("[my.prefix]\n[]\nk0 = value", "k0", "value"))
    errors++;

  // comment after prefix, whitespace surrounding prefix (a)
  if (!parsesTo(" [  my.prefix]# one comment here\n  k1a= valuea",
                "my.prefix.k1a", "valuea"))
    errors++;

  // comment after prefix, whitespace surrounding prefix (b)
  if (!parsesTo("[my.prefix  ] # one comment here\nk1b =valueb",
                "my.prefix.k1b", "valueb"))
    errors++;

  // comment after prefix, whitespace surrounding prefix (c)
  if (!parsesTo("[my.prefix]#one comment here\nk1c=valuec#comment",
                "my.prefix.k1c", "valuec"))
    errors++;

  // double-quoted string with hash and backslash
  if (!parsesTo("[my.prefix]\nk2.a =\"string with hash (here: #) and "
                "a quote (here: \\\") in it\"",
                "my.prefix.k2.a",
                "string with hash (here: #) and a quote (here: \") in it"))
    errors++;

  // single-quoted string with escaped quote and backslash
  if (!parsesTo(
          "[my.prefix]\nk2.b1 = 'string with a quote (here: \\') and "
          "a backslash (here: \\\\) in it\\n'",
          "my.prefix.k2.b1",
          "string with a quote (here: ') and a backslash (here: \\) in it\n"))
    errors++;

  // single-quoted string, followed by comment
  if (!parsesTo("[my.prefix]\nk2.b2 = 'string without a quote in it' "
                "# with comment",
                "my.prefix.k2.b2", "string without a quote in it"))
    errors++;

  // multiline single-quoted string, with hash
  if (!parsesTo("[my.prefix]\nk2.c = 'multline\nstring with a hash "
                "(there: #)\n\nand newlines'",
                "my.prefix.k2.c",
                "multline\nstring with a hash (there: #)\n\nand newlines"))
    errors++;

  // prefix/identifier with legal characters only
  if (!parsesTo("[a+b-c]\nd-e+f = value", "a+b-c.d-e+f", "value"))
    errors++;

  // simple-string with illegal character
  if (!failsToParse("[my.prefix]\nk3.a = abc\"def"))
    errors++;

  // simple-string with illegal whitespace
  if (!failsToParse("[my.prefix]\nk3.b = abc def"))
    errors++;

  // simple-string with legal characters only
  if (!parsesTo("[my.prefix]\nk3.c = abc=def", "my.prefix.k3.c", "abc=def"))
    errors++;

  return errors;
}

int main() {
  int errors = all_tests();

  if (errors > 0)
    std::cout << "WARNING: encountered " << errors << " error(s)!" << std::endl;
  else
    std::cout << "All is good. Encountered no errors." << std::endl;

  return errors > 0 ? 1 : 0;
}
