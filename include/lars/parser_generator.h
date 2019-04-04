#pragma once

#include "peg.h"

#include <lars/log.h>

namespace lars {
  
  template <class R = void, typename ... Args> class ParserGenerator: public Program<R, Args ...> {
  private:
    Program<peg::GrammarNode::Shared> grammarProgram;
    std::unordered_map<std::string, std::shared_ptr<peg::Rule>> rules;
    peg::GrammarNode::Shared separatorRule;
    
  public:
    
    ParserGenerator(){
      grammarProgram = peg::createGrammarProgram([this](const std::string_view &name){
        return getRuleNode(std::string(name));
      });
    }
    
    std::shared_ptr<peg::Rule> getRule(const std::string &name) {
      auto it = rules.find(name);
      if (it != rules.end()) {
        return it->second;
      }
      auto rule = peg::makeRule(name, peg::GrammarNode::Empty());
      rules[name] = rule;
      return rule;
    }
    
    peg::GrammarNode::Shared getRuleNode(const std::string &name) {
      auto rule = peg::GrammarNode::WeakRule(getRule(std::string(name)));
      if (separatorRule) {
        auto separator = peg::GrammarNode::ZeroOrMore(separatorRule);
        return peg::GrammarNode::Sequence({separator, rule, separator});
      } else {
        return rule;
      }
    }
    
    std::shared_ptr<peg::Rule> setRule(const std::string &name, const peg::GrammarNode::Shared &grammar, const typename Interpreter<R, Args ...>::Callback &callback = typename Interpreter<R, Args ...>::Callback()){
      auto rule = getRule(name);
      rule->node = grammar;
      this->interpreter.setEvaluator(rule, callback);
      return rule;
    }
    
    std::shared_ptr<peg::Rule> setRule(const std::string &name, const std::string_view &grammar, const typename Interpreter<R, Args ...>::Callback &callback = typename Interpreter<R, Args ...>::Callback()){
      return setRule(name, grammarProgram.run(grammar), callback);
    }

    template <class R2, typename ... Args2> std::shared_ptr<peg::Rule> setRule(const std::string &name, Program<R2, Args2 ...> subprogram, std::function<R(typename Interpreter<R2, Args2 ...>::Expression,Args...)> callback = [](auto e){ return e.evaluate(); }){
      auto rule = getRule(name);
      rule->node = peg::GrammarNode::Rule(subprogram.parser.grammar);
      this->interpreter.setEvaluator(rule, [callback, subprogram](auto e, Args ... args){
        return callback(subprogram.interpreter.interpret(e[0].syntaxTree), args...);
      });
      return rule;
    }
    
    std::shared_ptr<peg::Rule> setFilteredRule(const std::string &name, const std::string_view &grammar, const peg::GrammarNode::FilterCallback &filter, const typename Interpreter<R, Args ...>::Callback &callback = typename Interpreter<R, Args ...>::Callback()){
      return setRule(name, peg::GrammarNode::Sequence({grammarProgram.run(grammar), peg::GrammarNode::Filter(filter)}), callback);
    }
    
    void setSeparator(const std::shared_ptr<peg::Rule> &rule){
      rule->hidden = true;
      separatorRule = peg::GrammarNode::Rule(rule);
    }
    
    std::shared_ptr<peg::Rule> setSeparatorRule(const std::string &name, const peg::GrammarNode::Shared &grammar){
      auto rule = setRule(name, grammar);
      setSeparator(rule);
      return rule;
    }
    
    std::shared_ptr<peg::Rule> setSeparatorRule(const std::string &name, const std::string_view &grammar){
      return setSeparatorRule(name, grammarProgram.run(grammar));
    }

    void setStart(const std::shared_ptr<peg::Rule> &rule){
      this->parser.grammar = rule;
    }
    
    void unsetSeparatorRule(){
      separatorRule.reset();
    }
    
    /** Operator overloads */
    
    struct OperatorDelegate {
      ParserGenerator * parent;
      std::string ruleName;
      std::string grammar;
      typename Interpreter<R, Args ...>::Callback callback;
      peg::GrammarNode::FilterCallback filter;
      
      OperatorDelegate(ParserGenerator * p, const std::string &n):parent(p), ruleName(n){ }
      OperatorDelegate(const OperatorDelegate &) = delete;
      
      OperatorDelegate & operator<<(const std::string_view &grammar){
        this->grammar = grammar;
        return *this;
      }
      
      OperatorDelegate & operator>>(const typename Interpreter<R, Args ...>::Callback &callback){
        this->callback = callback;
        return *this;
      }
      
      OperatorDelegate & operator<<(const peg::GrammarNode::FilterCallback &filter){
        this->filter = filter;
        return *this;
      }

      operator std::shared_ptr<peg::Rule>() {
        return parent->getRule(ruleName);
      }
      
      ~OperatorDelegate(){
        if (grammar.size() > 0) {
          if (filter) {
            parent->setFilteredRule(ruleName, grammar, filter, callback);
          } else {
            parent->setRule(ruleName, grammar, callback);
          }
        }
      }
    };
    
    OperatorDelegate operator[](const std::string &ruleName){ return OperatorDelegate(this, ruleName); }
  };
  
}
