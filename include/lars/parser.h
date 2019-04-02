#pragma once

#include <lars/grammar.h>
#include <stdexcept>

namespace lars {

  struct SyntaxTree {
    std::shared_ptr<peg::Rule> rule;
    std::string_view fullString;
    
    std::vector<std::shared_ptr<SyntaxTree>> inner;
    unsigned begin, end;
    bool valid;
    
    SyntaxTree(const std::shared_ptr<peg::Rule> &r, std::string_view s, unsigned p);
    
    std::string_view string(){ return fullString.substr(begin, end); }
  };
  
  class Parser{
  private:
    std::shared_ptr<peg::Grammar> grammar;
    
  public:
    
    struct GrammarError: std::exception{
      enum Type { UNKNOWN_SYMBOL, INVALID_RULE, INVALID_GRAMMAR } type;
      peg::GrammarNode::Shared node;
      mutable std::string buffer;
      GrammarError(Type t, peg::GrammarNode::Shared n):type(t),node(n){}
      const char * what()const noexcept override;
    };
    
    Parser(const std::shared_ptr<Rule> &grammar);
    std::shared_ptr<SyntaxTree> parse(const std::string_view &str);
  };
  
}
