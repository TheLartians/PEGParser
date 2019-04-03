#pragma once

#include "peg.h"

#include <unordered_map>
#include <string>

namespace lars {
  
  template <class R, typename ... Args> class ParserGenerator: public Program<R, Args ...> {
  private:
    Program<peg::GrammarNode::Shared> grammarProgram;
    std::unordered_map<std::string, std::shared_ptr<peg::Rule>> rules;
    peg::GrammarNode::Shared separatorRule;
    
  public:
    
    ParserGenerator(){
      grammarProgram = peg::createGrammarParser([this](const std::string_view &name){
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
    
    std::shared_ptr<peg::Rule> addRule(const std::string &name, const peg::GrammarNode::Shared &grammar, const typename Interpreter<R, Args ...>::Callback &callback = typename Interpreter<R, Args ...>::Callback()){
      auto rule = getRule(name);
      rule->node = grammar;
      if (callback) {
        this->interpreter.setEvaluator(rule, callback);
      }
      return rule;
    }
    
    std::shared_ptr<peg::Rule> addRule(const std::string &name, const std::string_view &grammar, const typename Interpreter<R, Args ...>::Callback &callback = typename Interpreter<R, Args ...>::Callback()){
      return addRule(name, grammarProgram.run(grammar), callback);
    }

    template <class R2, typename ... Args2> std::shared_ptr<peg::Rule> addSubprogram(const std::string &name, Program<R2, Args2 ...> subprogram, std::function<R(typename Interpreter<R2, Args2 ...>::Expression,Args...)> callback){
      auto rule = getRule(name);
      rule->node = peg::GrammarNode::Rule(subprogram.parser.grammar);
      this->interpreter.setEvaluator(rule, [callback, subprogram](auto e, Args ... args){
        return callback(subprogram.interpreter.interpret(e[0].syntaxTree), args...);
      });
      return rule;
    }
    
    std::shared_ptr<peg::Rule> setSeparatorRule(const std::string &name, const peg::GrammarNode::Shared &grammar){
      auto rule = addRule(name, grammar);
      rule->hidden = true;
      separatorRule = peg::GrammarNode::Rule(rule);
      return rule;
    }
    
    std::shared_ptr<peg::Rule> setSeparatorRule(const std::string &name, const std::string_view &grammar){
      return setSeparatorRule(name, grammarProgram.run(grammar));
    }

    void unsetSeparatorRule(){
      separatorRule.reset();
    }
    
  };
  
}
