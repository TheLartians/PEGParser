#pragma once

#include "peg.h"

#include <unordered_map>
#include <string>

namespace lars {
  
  template <class R, typename ... Args> class ParserGenerator: public Program<R, Args ...> {
  private:
    Program<peg::GrammarNode::Shared> grammarProgram;
    std::unordered_map<std::string, std::shared_ptr<peg::Rule>> rules;

  public:
    
    ParserGenerator(){
      grammarProgram = peg::createGrammarParser([this](const std::string_view &name){
        return peg::GrammarNode::WeakRule(getRule(std::string(name)));
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
    
    std::shared_ptr<peg::Rule> addRule(const std::string &name, const peg::GrammarNode::Shared &grammar, const typename Interpreter<R, Args ...>::Callback &evaluator){
      auto rule = getRule(name);
      rule->node = grammar;
      this->interpreter.setEvaluator(rule, evaluator);
      return rule;
    }
    
    std::shared_ptr<peg::Rule> addRule(const std::string &name, const std::string_view &grammar, const typename Interpreter<R, Args ...>::Callback &evaluator){
      return addRule(name, grammarProgram.run(grammar), evaluator);
    }
    
    template <class R2, typename ... Args2> std::shared_ptr<peg::Rule> addSubprogram(const std::string &name, Program<R2, Args2 ...> subprogram, std::function<R(typename Interpreter<R2, Args2 ...>::Expression,Args...)> callback){
      auto rule = getRule(name);
      rule->node = peg::GrammarNode::Rule(subprogram.parser.grammar);
      this->interpreter.setEvaluator(rule, [callback, subprogram](auto e, Args ... args){
        return callback(subprogram.interpreter.interpret(e[0].syntaxTree), args...);
      });
      return rule;
    }

  };
  
}
