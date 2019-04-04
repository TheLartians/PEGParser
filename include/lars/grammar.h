#pragma once

#include <memory>
#include <string>
#include <vector>
#include <variant>
#include <ostream>
#include <unordered_map>

namespace lars {
  
  namespace peg{
    
    using Letter = std::string::value_type;
    struct GrammarNode;
    
    struct Rule {
      std::string name;
      std::shared_ptr<GrammarNode> node;
      bool hidden = false;
      Rule(const std::string_view &n, const std::shared_ptr<GrammarNode> &t):name(n), node(t){}
    };
    
    inline std::shared_ptr<Rule> makeRule(const std::string_view &name, const std::shared_ptr<GrammarNode> &node) {
      return std::make_shared<Rule>(name, node);
    }
    
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
        RULE,
        WEAK_RULE,
        END_OF_FILE
      };
      
      using Shared = std::shared_ptr<GrammarNode>;
      
      Symbol symbol;
      
      std::variant <
      std::vector<Shared>,
      Shared,
      std::weak_ptr<peg::Rule>,
      std::shared_ptr<peg::Rule>,
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
      static Shared Rule(const std::shared_ptr<peg::Rule> &rule){ return Shared(new GrammarNode(Symbol::RULE, rule)); }
      static Shared WeakRule(const std::weak_ptr<peg::Rule> &rule){ return Shared(new GrammarNode(Symbol::WEAK_RULE, rule)); }
      static Shared EndOfFile(){ return Shared(new GrammarNode(Symbol::END_OF_FILE)); }
    };
      
    std::ostream & operator<<(std::ostream &stream, const GrammarNode &node);
    
  }
}
