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
                                          "0123456789._+-";
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
      continue;
    case '[': { // Handle prefixes
      size_t prefixStart =
          line.find_first_not_of(ws, contentStart + 1); // skip '['.
      if (prefixStart == std::string::npos)
        throw ParsingException(line,
                               "declaration of scope not terminated by ']'");

      size_t chunkStart = prefixStart;
      size_t chunkEnd;
      size_t prefixEnd;
      size_t contentEnd;
      while (true) {
        // Intentionally read the first character again, to avoid code
        // duplication
        chunkEnd = line.find_first_not_of(identifierWhitelist, chunkStart);
        if (chunkEnd == std::string::npos)
          throw ParsingException(line,
                                 "declaration of scope not terminated by ']'");
        if (line[chunkEnd] == ']') {
          contentEnd = chunkEnd + 1;
          break;
        }
        if (ws.find(line[chunkEnd]) == std::string::npos)
          throw ParsingException(line, "invalid character in prefix");

        // Whitespace may or may not be important. We only know once
        // we have read the first character after it.
        size_t potentialChunkEnd = line.find_first_not_of(ws, chunkEnd + 1);
        if (potentialChunkEnd == std::string::npos)
          throw ParsingException(line,
                                 "declaration of scope not terminated by ']'");
        if (line[potentialChunkEnd] == ']') {
          contentEnd = potentialChunkEnd + 1;
          break;
        }
        chunkEnd = potentialChunkEnd;
        chunkStart = chunkEnd;
      }
      prefixEnd = chunkEnd;

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

      size_t chunkStart = contentStart;
      size_t chunkEnd;
      size_t keyEnd;
      size_t equalSignPosition;
      while (true) {
        // Intentionally read the first character again, to avoid code
        // duplication
        chunkEnd = line.find_first_not_of(identifierWhitelist, chunkStart);
        if (chunkEnd == std::string::npos)
          throw ParsingException(line, "'=' missing from assignment");
        if (line[chunkEnd] == '=') {
          equalSignPosition = chunkEnd;
          break;
        }
        if (ws.find(line[chunkEnd]) == std::string::npos)
          throw ParsingException(line, "invalid character in key");

        // Whitespace may or may not be important. We only know once
        // we have read the first character after it.
        size_t potentialChunkEnd = line.find_first_not_of(ws, chunkEnd + 1);
        if (potentialChunkEnd == std::string::npos)
          throw ParsingException(line, "'=' missing from assignment");
        if (line[potentialChunkEnd] == '=') {
          equalSignPosition = potentialChunkEnd;
          break;
        }

        chunkEnd = potentialChunkEnd;
        chunkStart = chunkEnd;
      }
      keyEnd = chunkEnd;
      key = line.substr(contentStart, keyEnd - contentStart);

      size_t valueStart = line.find_first_not_of(ws, equalSignPosition + 1);
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
      } else {
        // Handle unquoted or empty strings

        // Intentionally read the first character again, to avoid code
        // duplication
        size_t chunkStart = valueStart;
        size_t chunkEnd;
        while (true) {
          // Read a chunk of whitespace-free characters not on the blacklist
          chunkEnd = line.find_first_of(ws + simpleStringBlacklist, chunkStart);
          if (chunkEnd == std::string::npos || line[chunkEnd] == '#')
            break;
          if (simpleStringBlacklist.find(line[chunkEnd]) != std::string::npos)
            throw ParsingException(line, "invalid character in simple string");

          // Whitespace might be important or trailing whitespace. We only know
          // once we have read the first character after it.
          size_t potentialChunkEnd = line.find_first_not_of(ws, chunkEnd + 1);
          if (potentialChunkEnd == std::string::npos ||
              line[potentialChunkEnd] == '#')
            break;
          if (simpleStringBlacklist.find(line[potentialChunkEnd]) !=
              std::string::npos)
            throw ParsingException(line, "invalid character in simple string");

          chunkEnd = potentialChunkEnd;
          chunkStart = chunkEnd;
        }
        valueEnd = chunkEnd;
        if (valueStart != valueEnd)
          value = line.substr(valueStart, valueEnd - valueStart);
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
