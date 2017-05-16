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

int all_tests() {
  int errors = 0;

  if (!parsesTo("[my.prefix]\n[]\nk0 = value", "k0", "value"))
    errors++;

  if (!parsesTo(" [  my.prefix]# one comment here\n  k1a= valuea",
                "my.prefix.k1a", "valuea"))
    errors++;

  if (!parsesTo("[my.prefix  ] # one comment here\nk1b =valueb",
                "my.prefix.k1b", "valueb"))
    errors++;

  if (!parsesTo("[my.prefix]#one comment here\nk1c=valuec#comment",
                "my.prefix.k1c", "valuec"))
    errors++;

  if (!parsesTo("[my.prefix]\nk2.a =\"string with hash (here: #) and "
                "a quote (here: \\\") in it\"",
                "my.prefix.k2.a",
                "string with hash (here: #) and a quote (here: \") in it"))
    errors++;

  if (!parsesTo(
          "[my.prefix]\nk2.b1 = 'string with a quote (here: \\') and "
          "a backslash (here: \\\\) in it\\n'",
          "my.prefix.k2.b1",
          "string with a quote (here: ') and a backslash (here: \\) in it\n"))
    errors++;

  if (!parsesTo("[my.prefix]\nk2.b2 = 'string without a quote in it' "
                "# with comment",
                "my.prefix.k2.b2", "string without a quote in it"))
    errors++;

  if (!parsesTo("[my.prefix]\nk2.c = 'multline\nstring with a hash "
                "(there: #)\n\nand newlines'",
                "my.prefix.k2.c",
                "multline\nstring with a hash (there: #)\n\nand newlines"))
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
