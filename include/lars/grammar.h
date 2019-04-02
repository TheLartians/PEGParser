#pragma once

#include <string>
#include <vector>
#include <variant>
#include <ostream>
#include <unordered_map>

namespace lars {
  
  namespace peg{
    
    using Letter = std::string::value_type;
    class Grammar;
    struct GrammarNode;
    
    struct Rule {
      std::string name;
      std::shared_ptr<GrammarNode> node;
    };
    
    struct GrammarNode {
      
      enum class Symbol {
        WORD,
        ANY,
        RANGE,
        SEQUENCE,
        CHOICE,
        ZERO_OR_MORE,
        ONE_OR_MORE,
        OPTIONAL,
        AND_ALSO,
        BUT_NOT,
        EMPTY,
        GO_TO_RULE,
        END_OF_FILE,
        GO_TO_GRAMMAR
      };
      
      using Shared = std::shared_ptr<GrammarNode>;
      
      Symbol symbol;
      
      std::variant <
      std::vector<Shared>,
      Shared,
      std::weak_ptr<Grammar>,
      std::weak_ptr<Rule>,
      std::string,
      std::array<Letter, 2>
      > data;
      
    private:
      GrammarNode(Symbol s):symbol(s) {}
      template <class T> GrammarNode(Symbol s, const T &d):symbol(s), data(d) { }
      
    public:
      static Shared Word(const std::string &word){ return Shared(new GrammarNode(Symbol::WORD, word)); }
      static Shared Any(){ return Shared(new GrammarNode(Symbol::ANY)); }
      static Shared Range(Letter a, Letter b){ return Shared(new GrammarNode(Symbol::RANGE, std::array<Letter, 2>({{a,b}}))); }
      static Shared Sequence(const std::vector<Shared> &args){ return Shared(new GrammarNode(Symbol::SEQUENCE, args)); }
      static Shared Choice(const std::vector<Shared> &args){ return Shared(new GrammarNode(Symbol::CHOICE, args)); }
      static Shared ZeroOrMore(const Shared &arg){ return Shared(new GrammarNode(Symbol::ZERO_OR_MORE, arg)); }
      static Shared OneOrMore(const Shared &arg){ return Shared(new GrammarNode(Symbol::ONE_OR_MORE, arg)); }
      static Shared Optional(const Shared &arg){ return Shared(new GrammarNode(Symbol::OPTIONAL, arg)); }
      static Shared AndAlso(const Shared &arg){ return Shared(new GrammarNode(Symbol::AND_ALSO, arg)); }
      static Shared ButNot(const Shared &arg){ return Shared(new GrammarNode(Symbol::BUT_NOT, arg)); }
      static Shared Empty(){ return Shared(new GrammarNode(Symbol::EMPTY)); }
      static Shared Rule(const std::shared_ptr<Rule> &rule){ return Shared(new GrammarNode(Symbol::GO_TO_RULE, rule)); }
      static Shared Grammar(const std::shared_ptr<Grammar> &grammar){ return Shared(new GrammarNode(Symbol::GO_TO_GRAMMAR, grammar)); }
      static Shared EndOfFile(){ return Shared(new GrammarNode(Symbol::END_OF_FILE)); }
    };
    
    class Grammar {
    private:
      std::unordered_map<std::string, std::shared_ptr<Rule>> rules;
      std::shared_ptr<Rule> startRule;
    public:
      const std::string name;
      Grammar(const std::string &n = "Unnamed"):name(n){ }
      
      std::shared_ptr<Rule> addRule(const std::shared_ptr<Rule> &rule);
      std::shared_ptr<Rule> addRule(const std::string &internalName, const std::shared_ptr<Rule> &rule);
      std::shared_ptr<Rule> addRule(const std::string &name, const std::shared_ptr<GrammarNode> &node);
      std::shared_ptr<Rule> createRule(const std::string &name);

      std::shared_ptr<Rule> getRule(const std::string &name){ return rules[name]; }
      void setStartRule(const std::shared_ptr<Rule> &rule){ startRule = rule; }
      std::shared_ptr<Rule> getStartRule(){ return startRule; }
    };
  
    std::shared_ptr<Grammar> createParsingExpressionGrammar();
    
    std::ostream & operator<<(std::ostream &stream, const GrammarNode &node);
    
  }
}
