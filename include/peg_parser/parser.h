#pragma once

#include <stdexcept>
#include <utility>

#include "grammar.h"

class StringViews {
public:
  std::vector<std::string_view> strings;
  std::vector<std::size_t> presums;
  explicit StringViews();
  explicit StringViews(std::vector<std::string_view> strings);
  [[nodiscard]] std::size_t size() const;
  void append(std::string_view sv);
};

namespace peg_parser {

  struct SyntaxTree {
    std::shared_ptr<grammar::Rule> rule;
    std::string_view fullString;
    StringViews svs;
    bool singleString;
    std::vector<std::shared_ptr<SyntaxTree>> inner;
    size_t begin, end;

    bool valid = false;
    bool active = true;
    bool recursive = false;

    SyntaxTree(const std::shared_ptr<grammar::Rule> &r, std::string_view s, size_t p);
    SyntaxTree(const std::shared_ptr<grammar::Rule> &r, StringViews svs, size_t p);

    [[nodiscard]] size_t length() const { return end - begin; }
    // deprecated apis
    std::string_view view() const {
      if (!singleString) return std::string_view(); // cannot view when it is not single string
      return fullString.substr(begin, length());
    }
    std::string string() const {
      if (singleString) return std::string(view());
      else {
        std::size_t now = 0;
        std::string res;
        for (auto str : svs.strings) {
          if (now <= begin && begin < now + str.size()) {
            // exactly once
            if (now <= end && end <= now + str.size()) res += str.substr(begin - now, length());
            else res += str.substr(begin - now);
          } else if (now <= end && end <= now + str.size()) {
            res += str.substr(0, end - now);
          } else if (begin <= now && now + str.size() <= end) {
            res += str;
          }
          now += str.size();
        }
        return res;
      }
    }
  };

  struct Parser {
    struct Result {
      std::shared_ptr<SyntaxTree> syntax;
      std::shared_ptr<SyntaxTree> error;
    };

    struct GrammarError : std::exception {
      enum Type { UNKNOWN_SYMBOL, INVALID_RULE } type;
      grammar::Node::Shared node;
      mutable std::string buffer;
      GrammarError(Type t, grammar::Node::Shared n) : type(t), node(n) {}
      const char *what() const noexcept override;
    };

    std::shared_ptr<grammar::Rule> grammar;

    Parser(const std::shared_ptr<grammar::Rule> &grammar
           = std::make_shared<grammar::Rule>("undefined", grammar::Node::Error()));

    static Result parseAndGetError(const std::string_view &str,
                                   std::shared_ptr<grammar::Rule> grammar);
    static Result parseAndGetError(const StringViews &svs,
                                   std::shared_ptr<grammar::Rule> grammar);
    static std::shared_ptr<SyntaxTree> parse(const std::string_view &str,
                                             std::shared_ptr<grammar::Rule> grammar);

    std::shared_ptr<SyntaxTree> parse(const std::string_view &str) const;
    Result parseAndGetError(const std::string_view &str) const;
    Result parseAndGetError(const StringViews &svs) const;
  };

  std::ostream &operator<<(std::ostream &stream, const SyntaxTree &tree);

}  // namespace peg_parser
