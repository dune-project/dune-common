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

// Parse the ini-format information from instream. For each key-value pair,
// call store(prefix, key, value).
template <class Action> void parse(std::istream &instream, Action &&store) {
  std::string const ws = " \t";
  // Characters that could make up an identifier.
  std::string const identifierWhitelist = "abcdefghijklmnopqrstuvwxyz"
                                          "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                          "0123456789._";
  std::string const simpleStringBlacklist = "'\"\\#=";

  std::string prefix;
  std::string line;
  // Start is inclusive, End is exclusive
  while (std::getline(instream, line)) {
    auto contentStart = line.find_first_not_of(ws);
    if (contentStart == std::string::npos)
      continue; // Skip lines with nothing but whitespace

    switch (line[contentStart]) {
    case '#': // Handle comments
      continue;
    case '[': { // Handle prefixes
      size_t prefixStart = line.find_first_not_of(ws, contentStart + 1);
      if (prefixStart == std::string::npos)
        throw ParsingException(line);

      size_t prefixEnd;
      size_t contentEnd;
      if (line[prefixStart] == ']') {
        prefixEnd = prefixStart; // empty prefix
        contentEnd = prefixEnd + 1;
      } else {
        prefixEnd =
            line.find_first_not_of(identifierWhitelist, prefixStart + 1);
        if (prefixEnd == std::string::npos)
          throw ParsingException(line);

        contentEnd = line.find_first_not_of(ws, prefixEnd);
        if (contentEnd == std::string::npos || line[contentEnd] != ']')
          throw ParsingException(line);
        else
          contentEnd += 1; // make end exclusive
      }

      // After the content, only comments are allowed
      size_t trailingStart = line.find_first_not_of(ws, contentEnd);
      if (trailingStart != std::string::npos && line[trailingStart] != '#')
        throw ParsingException(line, "unexpected content after prefix");

      // Only set the prefix if the entire line could be parsed.
      prefix = line.substr(prefixStart, prefixEnd - prefixStart);
      break;
    }
    default: { // Handle anything else (i.e., assignments)
      std::string key, value;

      size_t keyEnd =
          line.find_first_not_of(identifierWhitelist, contentStart + 1);
      if (keyEnd == std::string::npos)
        throw ParsingException(line);
      key = line.substr(contentStart, keyEnd - contentStart);

      size_t equal_pos = line.find_first_not_of(ws, keyEnd);
      if (equal_pos == std::string::npos || line[equal_pos] != '=')
        throw ParsingException(line);

      size_t valueStart = line.find_first_not_of(ws, equal_pos + 1);
      if (valueStart == std::string::npos)
        throw ParsingException(line, "missing value in assignment");

      size_t valueEnd;
      switch (line[valueStart]) {
      case '#': // Allowing the empty string as a value here.
        value = "";
        // signal that we need no comment handling
        valueEnd = std::string::npos;
        break;
      case '\"':
      case '\'': {
        char const quote = line[valueStart];
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

          char const escaped_char = line[chunkEnd];
          if (escaped_char == '\\' || escaped_char == quote)
            value += escaped_char;
          else if (escaped_char == 'n')
            value += '\n';
          else
            throw ParsingException(line, "unexpected escape in quoted string");
          chunkStart = chunkEnd;
        }
        valueEnd = chunkEnd;
      } break;
      default: { // simple, unquoted string
        if (simpleStringBlacklist.find(line[valueStart]) != std::string::npos)
          throw ParsingException(line, "invalid character in simple string");
        valueEnd =
            line.find_first_of(ws + simpleStringBlacklist, valueStart + 1);
        value = line.substr(valueStart, valueEnd - valueStart);
      }
      }

      // After the content, only comments are allowed
      size_t trailingStart = line.find_first_not_of(ws, valueEnd);
      if (trailingStart != std::string::npos && line[trailingStart] != '#')
        throw ParsingException(line, "unexpected content after assignment");

      store(prefix, key, value);
    }
    }
  }
}
#endif
