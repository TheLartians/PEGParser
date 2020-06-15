#pragma once

#include <stdexcept>

#include "grammar.h"

namespace peg_parser {

  struct SyntaxTree {
    std::shared_ptr<grammar::Rule> rule;
    std::string_view fullString;
    std::vector<std::shared_ptr<SyntaxTree>> inner;
    size_t begin, end;

    bool valid = false;
    bool active = true;
    bool recursive = false;

    SyntaxTree(const std::shared_ptr<grammar::Rule> &r, std::string_view s, size_t p);

    size_t length() const { return end - begin; }
    std::string_view view() const { return fullString.substr(begin, length()); }
    std::string string() const { return std::string(view()); }
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
    static std::shared_ptr<SyntaxTree> parse(const std::string_view &str,
                                             std::shared_ptr<grammar::Rule> grammar);

    std::shared_ptr<SyntaxTree> parse(const std::string_view &str) const;
    Result parseAndGetError(const std::string_view &str) const;
  };

  std::ostream &operator<<(std::ostream &stream, const SyntaxTree &tree);

}  // namespace peg_parser
