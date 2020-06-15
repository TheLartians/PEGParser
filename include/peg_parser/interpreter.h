#pragma once

#include <iterator>
#include <type_traits>

#include "parser.h"

namespace peg_parser {

  struct InterpreterError : public std::exception {
    std::shared_ptr<SyntaxTree> tree;
    mutable std::string buffer;
    InterpreterError(const std::shared_ptr<SyntaxTree> &t) : tree(t) {}
    const char *what() const noexcept override;
  };

  template <class R, typename... Args> class Interpreter {
  public:
    class Expression;
    using Callback = std::function<R(const Expression &e, Args... args)>;

    class Expression {
    protected:
      struct iterator {
        using iterator_category = std::input_iterator_tag;
        using value_type = Expression;
        using pointer = Expression *;
        using reference = Expression &;

        const Expression &parent;
        size_t idx;
        iterator(const Expression &p, size_t i) : parent(p), idx(i) {}
        iterator &operator++() {
          idx++;
          return *this;
        }
        Expression operator*() const { return parent[idx]; }
        bool operator!=(const iterator &other) const {
          return other.idx != idx || &other.parent != &parent;
        }
      };

      const Interpreter<R, Args...> &interpreter;
      std::shared_ptr<SyntaxTree> syntaxTree;

    public:
      Expression(const Interpreter<R, Args...> &i, std::shared_ptr<SyntaxTree> s)
          : interpreter(i), syntaxTree(s) {}

      auto size() const { return syntaxTree->inner.size(); }
      auto view() const { return syntaxTree->view(); }
      auto string() const { return std::string(view()); }
      auto position() const { return syntaxTree->begin; }
      auto length() const { return syntaxTree->length(); }
      auto rule() const { return syntaxTree->rule; }
      auto syntax() const { return syntaxTree; }

      Expression operator[](size_t idx) const {
        return interpreter.interpret(syntaxTree->inner[idx]);
      }
      iterator begin() const { return iterator(*this, 0); }
      iterator end() const { return iterator(*this, size()); }

      template <class R2, typename... Args2>
      auto evaluateBy(const Interpreter<R2, Args2...> &interpreter, Args2... args) const {
        return interpreter.evaluate(syntaxTree, args...);
      }

      R evaluate(Args... args) const {
        auto it = interpreter.evaluators.find(syntaxTree->rule.get());
        if (it == interpreter.evaluators.end()) {
          if (interpreter.defaultEvaluator) {
            return interpreter.defaultEvaluator(*this, args...);
          }
          throw InterpreterError(syntaxTree);
        }
        return it->second(*this, args...);
      }
    };

  private:
    std::unordered_map<grammar::Rule *, Callback> evaluators;

    static R __defaultEvaluator(const Expression &e, Args... args) {
      size_t N = e.size();
      if (N > 0) {
        for (size_t i = 0; i < N - 1; ++i) {
          e[i].evaluate(std::forward<Args>(args)...);
        }
        return e[N - 1].evaluate(std::forward<Args>(args)...);
      }
      if (!std::is_same<R, void>::value) {
        throw InterpreterError(e.syntax());
      }
    };

  public:
    Callback defaultEvaluator = __defaultEvaluator;

    std::shared_ptr<grammar::Rule> makeRule(const std::string_view &name,
                                            const grammar::Node::Shared &node,
                                            const Callback &callback) {
      auto rule = std::make_shared<grammar::Rule>(name, node);
      setEvaluator(rule, callback);
      return rule;
    }

    std::shared_ptr<grammar::Rule> makeRule(const std::string &name,
                                            const std::shared_ptr<grammar::Rule> &rule,
                                            const Callback &callback) {
      return makeRule(name, grammar::Node::Rule(rule), callback);
    }

    void setEvaluator(const std::shared_ptr<grammar::Rule> &rule, const Callback &callback) {
      if (callback) {
        evaluators[rule.get()] = callback;
      } else {
        auto it = evaluators.find(rule.get());
        if (it != evaluators.end()) {
          evaluators.erase(it);
        }
      }
    }

    Expression interpret(const std::shared_ptr<SyntaxTree> &tree) const {
      return Expression{*this, tree};
    }

    R evaluate(const std::shared_ptr<SyntaxTree> &tree, Args... args) const {
      return interpret(tree).evaluate(args...);
    }
  };

  class SyntaxError : public std::exception {
  private:
    mutable std::string buffer;

  public:
    std::shared_ptr<SyntaxTree> syntax;

    SyntaxError(const std::shared_ptr<SyntaxTree> &t) : syntax(t) {}
    const char *what() const noexcept override;
  };

  template <class R, typename... Args> struct Program {
    using Expression = typename Interpreter<R, Args...>::Expression;

    Parser parser;
    Interpreter<R, Args...> interpreter;

    std::shared_ptr<SyntaxTree> parse(const std::string_view &str) const {
      return parser.parse(str);
    }

    Expression interpret(const std::shared_ptr<SyntaxTree> &tree) const {
      if (!tree->valid) {
        throw SyntaxError(tree);
      }
      return interpreter.interpret(tree);
    }

    R run(const std::string_view &str, Args &&... args) const {
      auto parsed = parser.parseAndGetError(str);
      if (!parsed.syntax->valid || parsed.syntax->end < str.size()) {
        throw SyntaxError(parsed.error);
      }
      return interpret(parsed.syntax).evaluate(std::forward<Args>(args)...);
    }
  };

}  // namespace peg_parser
