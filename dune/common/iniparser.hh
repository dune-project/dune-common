#ifndef DUNE_COMMON_INIPARSER_HH
#define DUNE_COMMON_INIPARSER_HH

#include <fstream>
#include <string>

#include <dune/common/exceptions.hh>

class ParsingException : public Dune::Exception {
public:
  ParsingException(std::string line, std::string reason = "") {
    message_ = "Unable to parse line: " + line;
    if (reason != "")
      message_ += " (" + reason + ")";
  }

  const char *what() const noexcept override { return message_.c_str(); }

private:
  std::string message_;
};

std::string ltrim(std::string const &s, std::string const &characterClass) {
  std::size_t const firstAdmissible = s.find_first_not_of(characterClass);

  if (firstAdmissible != std::string::npos)
    return s.substr(firstAdmissible);
  return std::string();
}

std::string rtrim(std::string const &s, std::string const &characterClass) {
  std::size_t const lastAdmissible = s.find_last_not_of(characterClass);

  if (lastAdmissible != std::string::npos)
    return s.substr(0, lastAdmissible + 1);
  return std::string();
}

// Parse the ini-format information from instream. For each key-value pair,
// call store(prefix, key, value).
template <class Action> void parse(std::istream &instream, Action &&store) {
  std::string const ws = " \t";
  // Characters that could make up an identifier.
  std::string const identifierWhitelist = "abcdefghijklmnopqrstuvwxyz"
                                          "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                          "0123456789._+-" +
                                          ws;
  std::string const simpleStringBlacklist = "'\"\\#";

  std::string prefix;
  std::string line;
  // Start is inclusive, End is exclusive
  while (std::getline(instream, line)) {
    auto contentStart = line.find_first_not_of(ws);
    if (contentStart == std::string::npos)
      continue; // Skip lines with nothing but whitespace

    switch (line[contentStart]) {
    case '#': // Handle comments
      break;
    case '[': { // Handle prefixes
      size_t const prefixStart =
          line.find_first_not_of(ws, contentStart + 1); // skip '['.
      if (prefixStart == std::string::npos)
        throw ParsingException(line,
                               "declaration of scope not terminated by ']'");

      // Start from prefixStart to allow empty prefix
      size_t const prefixEnd =
          line.find_first_not_of(identifierWhitelist, prefixStart);
      if (prefixEnd == std::string::npos)
        throw ParsingException(line,
                               "declaration of scope not terminated by ']'");
      if (line[prefixEnd] != ']')
        throw ParsingException(line, "invalid character in prefix");

      // After the content, only comments are allowed
      size_t const trailingStart = line.find_first_not_of(ws, prefixEnd + 1);
      if (trailingStart != std::string::npos && line[trailingStart] != '#')
        throw ParsingException(line, "unexpected content after prefix");

      // Only set the prefix if the entire line could be parsed.
      prefix = line.substr(prefixStart, prefixEnd - prefixStart);
      prefix = ltrim(rtrim(prefix, ws), ws);
      break;
    }
    default: { // Handle anything else (i.e., assignments)
      std::string key, value;

      size_t const keyStart = contentStart;
      // Intentionally re-read the first character to validate it
      size_t const keyEnd =
          line.find_first_not_of(identifierWhitelist, keyStart);
      if (keyEnd == std::string::npos)
        throw ParsingException(line, "'=' missing from assignment");
      if (line[keyEnd] != '=')
        throw ParsingException(line, "invalid character in key");
      size_t const equalSignPosition = keyEnd;

      key = line.substr(contentStart, keyEnd - keyStart);
      key = ltrim(rtrim(key, ws), ws);
      if (key == "")
        throw ParsingException(line, "key cannot be empty");

      size_t const valueStart =
          line.find_first_not_of(ws, equalSignPosition + 1);
      size_t valueEnd;

      if (valueStart != std::string::npos &&
          (line[valueStart] == '\'' || line[valueStart] == '"')) {
        // Handle quoted strings
        char const quote = line[valueStart];
        // We read the string in backslash-free chunks since only
        // backslashes require special handling
        size_t chunkStart = valueStart;
        size_t chunkEnd;
        while (true) {
          chunkEnd =
              line.find_first_of(std::string("\\") + quote, chunkStart + 1);
          if (chunkEnd == std::string::npos) {
            std::string multiline_comment_line;
            if (!std::getline(instream, multiline_comment_line))
              throw ParsingException(
                  line, "reached last line looking for closing quote");
            line += "\n";
            line += multiline_comment_line;
            continue;
          }
          value += line.substr(chunkStart + 1, chunkEnd - (chunkStart + 1));

          chunkEnd += 1; // make end exclusive
          if (line[chunkEnd - 1] == quote)
            break;

          if (chunkEnd >= line.size())
            throw ParsingException(line, "line ended with a single backslash");

          char const escapedCharacter = line[chunkEnd];
          if (escapedCharacter == '\\' || escapedCharacter == quote)
            value += escapedCharacter;
          else if (escapedCharacter == 'n')
            value += '\n';
          else
            throw ParsingException(line, "unexpected escape in quoted string");
          chunkStart = chunkEnd;
        }
        valueEnd = chunkEnd;
      } else { // Handle unquoted or empty strings

        // Intentionally re-read the first character to validate it
        valueEnd = line.find_first_of(simpleStringBlacklist, valueStart);
        if (valueStart != valueEnd)
          value = line.substr(valueStart, valueEnd - valueStart);
        value = ltrim(rtrim(value, ws), ws);
      }

      // After the content, only comments are allowed
      size_t const trailingStart = line.find_first_not_of(ws, valueEnd);
      if (trailingStart != std::string::npos && line[trailingStart] != '#')
        throw ParsingException(line, "unexpected content after assignment");

      store(prefix, key, value);
    }
    }
  }
}
#endif
