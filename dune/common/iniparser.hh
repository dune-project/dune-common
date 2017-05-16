#ifndef DUNE_COMMON_INIPARSER_HH
#define DUNE_COMMON_INIPARSER_HH

#include <exception>
#include <fstream>
#include <string>

class ParsingException : public std::exception {
public:
  ParsingException(std::string line, std::string reason = "") {
    message_ = "Unable to parse line: " + line;
    if (reason != "")
      message_ += " (" + reason + ")";
  }

  const char *what() const noexcept { return message_.c_str(); }

private:
  std::string message_;
};

// Parse the ini-format information from instream. For each key-value pair,
// call store(prefix, key, value).
template <class Action> void parse(std::istream &instream, Action &&store) {
  std::string const ws = " \t";
  // Characters that could make up an identifier.
  std::string const identifierCharacters = "abcdefghijklmnopqrstuvwxyz"
                                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                           "0123456789._";
  std::string const simpleStringInvalidCharacter = "'\"\\#=";

  std::string prefix;
  std::string line;
  // _start is inclusive, _end is exclusive
  while (std::getline(instream, line)) {
    auto content_start = line.find_first_not_of(ws);
    if (content_start == std::string::npos)
      continue; // Skip lines with nothing but whitespace

    switch (line[content_start]) {
    case '#': // Handle comments
      continue;
    case '[': { // Handle prefixes
      size_t prefix_start = line.find_first_not_of(ws, content_start + 1);
      if (prefix_start == std::string::npos)
        throw ParsingException(line);

      size_t prefix_end;
      size_t content_end;
      if (line[prefix_start] == ']') {
        prefix_end = prefix_start; // empty prefix
        content_end = prefix_end + 1;
      } else {
        prefix_end =
            line.find_first_not_of(identifierCharacters, prefix_start + 1);
        if (prefix_end == std::string::npos)
          throw ParsingException(line);

        content_end = line.find_first_not_of(ws, prefix_end);
        if (content_end == std::string::npos || line[content_end] != ']')
          throw ParsingException(line);
        else
          content_end += 1; // make end exclusive
      }

      // After the content, only comments are allowed
      size_t trailing_start = line.find_first_not_of(ws, content_end);
      if (trailing_start != std::string::npos && line[trailing_start] != '#')
        throw ParsingException(line, "unexpected content after prefix");

      // Only set the prefix if the entire line could be parsed.
      prefix = line.substr(prefix_start, prefix_end - prefix_start);
      break;
    }
    default: { // Handle anything else (i.e., assignments)
      std::string key, value;

      size_t key_end =
          line.find_first_not_of(identifierCharacters, content_start + 1);
      if (key_end == std::string::npos)
        throw ParsingException(line);
      key = line.substr(content_start, key_end - content_start);

      size_t equal_pos = line.find_first_not_of(ws, key_end);
      if (equal_pos == std::string::npos || line[equal_pos] != '=')
        throw ParsingException(line);

      size_t value_start = line.find_first_not_of(ws, equal_pos + 1);
      if (value_start == std::string::npos)
        throw ParsingException(line, "missing value in assignment");

      size_t value_end;
      switch (line[value_start]) {
      case '#': // Allowing the empty string as a value here.
        value = "";
        // signal that we need no comment handling
        value_end = std::string::npos;
        break;
      case '\"':
      case '\'': {
        char const quote = line[value_start];
        size_t chunk_begin = value_start;
        size_t chunk_end;
        while (true) {
          chunk_end =
              line.find_first_of(std::string("\\") + quote, chunk_begin + 1);
          if (chunk_end == std::string::npos) {
            std::string multiline_comment_line;
            if (!std::getline(instream, multiline_comment_line))
              throw ParsingException(
                  line, "reached last line looking for closing quote");
            line += "\n";
            line += multiline_comment_line;
            continue;
          }
          value += line.substr(chunk_begin + 1, chunk_end - (chunk_begin + 1));

          chunk_end += 1; // make end exclusive
          if (line[chunk_end - 1] == quote)
            break;

          if (chunk_end >= line.size())
            throw ParsingException(line, "line ended with a single backslash");

          char const escaped_char = line[chunk_end];
          if (escaped_char == '\\' || escaped_char == quote)
            value += escaped_char;
          else if (escaped_char == 'n')
            value += '\n';
          else
            throw ParsingException(line, "unexpected escape in quoted string");
          chunk_begin = chunk_end;
        }
        value_end = chunk_end;
      } break;
      default: { // simple, unquoted string
        if (simpleStringInvalidCharacter.find(line[value_start]) !=
            std::string::npos)
          throw ParsingException(line, "invalid character in simple string");
        value_end = line.find_first_of(ws + simpleStringInvalidCharacter,
                                       value_start + 1);
        value = line.substr(value_start, value_end - value_start);
      }
      }

      // After the content, only comments are allowed
      size_t trailing_start = line.find_first_not_of(ws, value_end);
      if (trailing_start != std::string::npos && line[trailing_start] != '#')
        throw ParsingException(line, "unexpected content after assignment");

      store(prefix, key, value);
    }
    }
  }
}
#endif
