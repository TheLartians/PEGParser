#pragma once

#include "presets.h"

namespace peg_parser {

  template <class R = void, typename... Args> class ParserGenerator : public Program<R, Args...> {
  private:
    presets::GrammarProgram grammarProgram;
    std::unordered_map<std::string, std::shared_ptr<presets::Rule>> rules;
    presets::GrammarNode::Shared separatorRule;

  public:
    ParserGenerator() { grammarProgram = presets::createGrammarProgram(); }

    std::shared_ptr<presets::Rule> getRule(const std::string &name) {
      auto it = rules.find(name);
      if (it != rules.end()) {
        return it->second;
      }
      auto rule = presets::makeRule(name, presets::GrammarNode::Error());
      rules[name] = rule;
      return rule;
    }

    presets::GrammarNode::Shared getRuleNode(const std::string &name) {
      auto rule = presets::GrammarNode::WeakRule(getRule(std::string(name)));
      if (separatorRule) {
        auto separator = presets::GrammarNode::ZeroOrMore(separatorRule);
        return presets::GrammarNode::Sequence({separator, rule, separator});
      } else {
        return rule;
      }
    }

    std::shared_ptr<presets::Rule> setRule(
        const std::string &name, const presets::GrammarNode::Shared &grammar,
        const typename Interpreter<R, Args...>::Callback &callback
        = typename Interpreter<R, Args...>::Callback()) {
      auto rule = getRule(name);
      rule->node = grammar;
      this->interpreter.setEvaluator(rule, callback);
      return rule;
    }

    presets::GrammarNode::Shared parseRule(const std::string_view &grammar) {
      presets::RuleGetter rg = [this](const auto &name) { return getRuleNode(std::string(name)); };
      return grammarProgram.run(grammar, rg);
    }

    std::shared_ptr<presets::Rule> setRule(
        const std::string &name, const std::string_view &grammar,
        const typename Interpreter<R, Args...>::Callback &callback
        = typename Interpreter<R, Args...>::Callback()) {
      return setRule(name, parseRule(grammar), callback);
    }

    template <class R2, typename... Args2> using OtherExpression =
        typename Interpreter<R2, Args2...>::Expression;
    template <class R2, typename... Args2> using ConversionCallback
        = std::function<R(OtherExpression<R2, Args2...>, Args...)>;

    template <class R2, typename... Args2>
    std::shared_ptr<presets::Rule> setProgramRule(const std::string &name,
                                                  Program<R2, Args2...> subprogram,
                                                  ConversionCallback<R2, Args2...> callback) {
      auto rule = getRule(name);
      rule->node = presets::GrammarNode::Rule(subprogram.parser.grammar);
      this->interpreter.setEvaluator(
          rule, [callback, interpreter = subprogram.interpreter](auto e, Args &&... args) {
            return callback(interpreter.interpret(e[0].syntax()), std::forward<Args>(args)...);
          });
      return rule;
    }

    template <class R2, typename... Args2>
    auto setProgramRule(const std::string &name, Program<R2, Args2...> subprogram) {
      static_assert(sizeof...(Args2) == 0);
      return setProgramRule(name, subprogram, [](auto e, auto &&...) { return e.evaluate(); });
    }

    std::shared_ptr<presets::Rule> setFilteredRule(
        const std::string &name, const std::string_view &grammar,
        const presets::GrammarNode::FilterCallback &filter,
        const typename Interpreter<R, Args...>::Callback &callback
        = typename Interpreter<R, Args...>::Callback()) {
      return setRule(name,
                     presets::GrammarNode::Sequence(
                         {parseRule(grammar), presets::GrammarNode::Filter(filter)}),
                     callback);
    }

    void setSeparator(const std::shared_ptr<presets::Rule> &rule) {
      rule->hidden = true;
      separatorRule = presets::GrammarNode::Rule(rule);
    }

    std::shared_ptr<presets::Rule> setSeparatorRule(const std::string &name,
                                                    const presets::GrammarNode::Shared &grammar) {
      auto rule = setRule(name, grammar);
      setSeparator(rule);
      return rule;
    }

    std::shared_ptr<presets::Rule> setSeparatorRule(const std::string &name,
                                                    const std::string_view &grammar) {
      return setSeparatorRule(name, parseRule(grammar));
    }

    void setStart(const std::shared_ptr<presets::Rule> &rule) { this->parser.grammar = rule; }

    void unsetSeparatorRule() { separatorRule.reset(); }

    /** Operator overloads */

    struct OperatorDelegate {
      ParserGenerator *parent;
      std::string ruleName;
      std::string grammar;
      typename Interpreter<R, Args...>::Callback callback;
      presets::GrammarNode::FilterCallback filter;

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

      OperatorDelegate &operator<<(const presets::GrammarNode::FilterCallback &ft) {
        this->filter = ft;
        return *this;
      }

      operator std::shared_ptr<presets::Rule>() { return parent->getRule(ruleName); }

      std::shared_ptr<presets::Rule> operator->() { return parent->getRule(ruleName); }

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
