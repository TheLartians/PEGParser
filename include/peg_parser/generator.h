#pragma once

#include "presets.h"

namespace peg_parser {

  template <class R = void, typename... Args> class ParserGenerator : public Program<R, Args...> {
  private:
    presets::GrammarProgram grammarProgram;
    std::unordered_map<std::string, std::shared_ptr<grammar::Rule>> rules;
    grammar::Node::Shared separatorRule;

  public:
    ParserGenerator() { grammarProgram = presets::createPEGProgram(); }

    std::shared_ptr<grammar::Rule> getRule(const std::string &name) {
      auto it = rules.find(name);
      if (it != rules.end()) {
        return it->second;
      }
      auto rule = grammar::makeRule(name, grammar::Node::Error());
      rules[name] = rule;
      return rule;
    }

    grammar::Node::Shared getRuleNode(const std::string &name) {
      auto rule = grammar::Node::WeakRule(getRule(std::string(name)));
      if (separatorRule) {
        auto separator = grammar::Node::ZeroOrMore(separatorRule);
        return grammar::Node::Sequence({separator, rule, separator});
      } else {
        return rule;
      }
    }

    std::shared_ptr<grammar::Rule> setRule(
        const std::string &name, const grammar::Node::Shared &grammar,
        const typename Interpreter<R, Args...>::Callback &callback
        = typename Interpreter<R, Args...>::Callback()) {
      auto rule = getRule(name);
      rule->node = grammar;
      this->interpreter.setEvaluator(rule, callback);
      return rule;
    }

    grammar::Node::Shared parseRule(const std::string_view &grammar) {
      presets::RuleGetter rg = [this](const auto &name) { return getRuleNode(std::string(name)); };
      return grammarProgram.run(grammar, rg);
    }

    std::shared_ptr<grammar::Rule> setRule(
        const std::string &name, const std::string_view &grammar,
        const typename Interpreter<R, Args...>::Callback &callback
        = typename Interpreter<R, Args...>::Callback()) {
      return setRule(name, parseRule(grammar), callback);
    }

    template <class R2, typename... Args2, class C>
    std::shared_ptr<grammar::Rule> setProgramRule(const std::string &name,
                                                  Program<R2, Args2...> subprogram, C &&callback) {
      auto rule = getRule(name);
      rule->node = grammar::Node::Rule(subprogram.parser.grammar);
      this->interpreter.setEvaluator(
          rule, [callback = std::forward<C>(callback), interpreter = subprogram.interpreter](
                    auto e, Args &&... args) {
            return callback(interpreter.interpret(e[0].syntax()), std::forward<Args>(args)...);
          });
      return rule;
    }

    template <class R2, typename... Args2>
    auto setProgramRule(const std::string &name, Program<R2, Args2...> subprogram) {
      static_assert(sizeof...(Args2) == 0);
      static_assert(std::is_convertible<R2, R>::value);
      auto rule = getRule(name);
      rule->node = grammar::Node::Rule(subprogram.parser.grammar);
      this->interpreter.setEvaluator(rule,
                                     [interpreter = subprogram.interpreter](auto e, auto &&...) {
                                       return R(interpreter.interpret(e[0].syntax()).evaluate());
                                     });
    }

    std::shared_ptr<grammar::Rule> setFilteredRule(
        const std::string &name, const std::string_view &grammar,
        const grammar::Node::FilterCallback &filter,
        const typename Interpreter<R, Args...>::Callback &callback
        = typename Interpreter<R, Args...>::Callback()) {
      return setRule(name,
                     grammar::Node::Sequence(
                         {parseRule(grammar), grammar::Node::Filter(filter)}),
                     callback);
    }

    void setSeparator(const std::shared_ptr<grammar::Rule> &rule) {
      rule->hidden = true;
      separatorRule = grammar::Node::Rule(rule);
    }

    std::shared_ptr<grammar::Rule> setSeparatorRule(const std::string &name,
                                                    const grammar::Node::Shared &grammar) {
      auto rule = setRule(name, grammar);
      setSeparator(rule);
      return rule;
    }

    std::shared_ptr<grammar::Rule> setSeparatorRule(const std::string &name,
                                                    const std::string_view &grammar) {
      return setSeparatorRule(name, parseRule(grammar));
    }

    void setStart(const std::shared_ptr<grammar::Rule> &rule) { this->parser.grammar = rule; }

    void unsetSeparatorRule() { separatorRule.reset(); }

    /** Operator overloads */

    struct OperatorDelegate {
      ParserGenerator *parent;
      std::string ruleName;
      std::string grammar;
      typename Interpreter<R, Args...>::Callback callback;
      grammar::Node::FilterCallback filter;

      OperatorDelegate(ParserGenerator *p, const std::string &n) : parent(p), ruleName(n) {}
      OperatorDelegate(const OperatorDelegate &) = delete;

      OperatorDelegate &operator<<(const std::string_view &gr) {
        this->grammar = gr;
        return *this;
      }

      OperatorDelegate &operator>>(const typename Interpreter<R, Args...>::Callback &cp) {
        this->callback = cp;
        return *this;
      }

      OperatorDelegate &operator<<(const grammar::Node::FilterCallback &ft) {
        this->filter = ft;
        return *this;
      }

      operator std::shared_ptr<grammar::Rule>() { return parent->getRule(ruleName); }

      std::shared_ptr<grammar::Rule> operator->() { return parent->getRule(ruleName); }

      ~OperatorDelegate() {
        if (grammar.size() > 0) {
          if (filter) {
            parent->setFilteredRule(ruleName, grammar, filter, callback);
          } else {
            parent->setRule(ruleName, grammar, callback);
          }
        }
      }
    };

    OperatorDelegate operator[](const std::string &ruleName) {
      return OperatorDelegate(this, ruleName);
    }
  };

}  // namespace peg_parser
