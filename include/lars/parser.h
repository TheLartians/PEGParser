#pragma once

#include "grammar.h"

#include <stdexcept>

namespace lars {

  struct SyntaxTree {
    std::shared_ptr<peg::Rule> rule;
    std::string_view fullString;
    
    std::vector<std::shared_ptr<SyntaxTree>> inner;
    unsigned begin, end;
    bool valid;
    
    SyntaxTree(const std::shared_ptr<peg::Rule> &r, std::string_view s, unsigned p);
    
    size_t length() const { return end - begin; }
    std::string_view string()const{ return fullString.substr(begin,length()); }
  };
  
  struct Parser{

    struct GrammarError: std::exception{
      enum Type { UNKNOWN_SYMBOL, INVALID_RULE, INVALID_GRAMMAR } type;
      peg::GrammarNode::Shared node;
      mutable std::string buffer;
      GrammarError(Type t, peg::GrammarNode::Shared n):type(t),node(n){}
      const char * what()const noexcept override;
    };
    
    std::shared_ptr<peg::Rule> grammar;
    
    Parser(const std::shared_ptr<peg::Rule> &grammar = std::make_shared<peg::Rule>("undefined", peg::GrammarNode::Empty()));
    
    static std::shared_ptr<SyntaxTree> parse(const std::string_view &str, std::shared_ptr<peg::Rule> grammar);
    std::shared_ptr<SyntaxTree> parse(const std::string_view &str)const;
  };
  
  std::ostream & operator<<(std::ostream &stream, const SyntaxTree &tree);
  
}
