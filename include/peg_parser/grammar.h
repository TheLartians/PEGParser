#pragma once

#include <array>
#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace peg_parser {

  struct SyntaxTree;

  namespace grammar {

    using Letter = char;
    struct Node;

    struct Rule {
      std::string name;
      std::shared_ptr<Node> node;
      bool hidden = false;
      bool cacheable = true;
      Rule(const std::string_view &n, const std::shared_ptr<Node> &t) : name(n), node(t) {}
    };

    inline std::shared_ptr<Rule> makeRule(const std::string_view &name,
                                          const std::shared_ptr<Node> &node) {
      return std::make_shared<Rule>(name, node);
    }

    struct Node {
      using FilterCallback = std::function<bool(const std::shared_ptr<SyntaxTree> &)>;

      enum class Symbol {
        WORD, // 'word'
        ANY,  // .
        RANGE, // []
        SEQUENCE,
        CHOICE, // |
        ZERO_OR_MORE, // *
        ONE_OR_MORE, // +
        OPTIONAL, // ?
        ALSO, // &
        NOT, // !

        EMPTY,
        ERROR,
        RULE,
        WEAK_RULE,
        END_OF_FILE,
        FILTER
      };

      using Shared = std::shared_ptr<Node>;

      Symbol symbol;

      std::variant<std::vector<Shared>, Shared, std::weak_ptr<grammar::Rule>,
                   std::shared_ptr<grammar::Rule>, std::string, std::array<Letter, 2>,
                   FilterCallback>
          data;

    private:
      Node(Symbol s) : symbol(s) {}
      template <class T> Node(Symbol s, const T &d) : symbol(s), data(d) {}

    public:
      static Shared Word(const std::string &word) { return Shared(new Node(Symbol::WORD, word)); }
      static Shared Any() { return Shared(new Node(Symbol::ANY)); }
      static Shared Range(Letter a, Letter b) {
        return Shared(new Node(Symbol::RANGE, std::array<Letter, 2>({{a, b}})));
      }
      static Shared Sequence(const std::vector<Shared> &args) {
        return Shared(new Node(Symbol::SEQUENCE, args));
      }
      static Shared Choice(const std::vector<Shared> &args) {
        return Shared(new Node(Symbol::CHOICE, args));
      }
      static Shared ZeroOrMore(const Shared &arg) {
        return Shared(new Node(Symbol::ZERO_OR_MORE, arg));
      }
      static Shared OneOrMore(const Shared &arg) {
        return Shared(new Node(Symbol::ONE_OR_MORE, arg));
      }
      static Shared Optional(const Shared &arg) { return Shared(new Node(Symbol::OPTIONAL, arg)); }
      static Shared Also(const Shared &arg) { return Shared(new Node(Symbol::ALSO, arg)); }
      static Shared Not(const Shared &arg) { return Shared(new Node(Symbol::NOT, arg)); }
      static Shared Empty() { return Shared(new Node(Symbol::EMPTY)); }
      static Shared Error() { return Shared(new Node(Symbol::ERROR)); }
      static Shared Rule(const std::shared_ptr<grammar::Rule> &rule) {
        return Shared(new Node(Symbol::RULE, rule));
      }
      static Shared WeakRule(const std::weak_ptr<grammar::Rule> &rule) {
        return Shared(new Node(Symbol::WEAK_RULE, rule));
      }
      static Shared EndOfFile() { return Shared(new Node(Symbol::END_OF_FILE)); }
      static Shared Filter(const FilterCallback &callback) {
        return Shared(new Node(Symbol::FILTER, callback));
      }
    };

    std::ostream &operator<<(std::ostream &stream, const Node &node);

  }  // namespace grammar
}  // namespace peg_parser
