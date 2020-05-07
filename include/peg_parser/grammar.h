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

  namespace peg {

    using Letter = char;
    struct GrammarNode;

    struct Rule {
      std::string name;
      std::shared_ptr<GrammarNode> node;
      bool hidden = false;
      bool cacheable = true;
      Rule(const std::string_view &n, const std::shared_ptr<GrammarNode> &t) : name(n), node(t) {}
    };

    inline std::shared_ptr<Rule> makeRule(const std::string_view &name,
                                          const std::shared_ptr<GrammarNode> &node) {
      return std::make_shared<Rule>(name, node);
    }

    struct GrammarNode {
      using FilterCallback = std::function<bool(const std::shared_ptr<SyntaxTree> &)>;

      enum class Symbol {
        WORD,
        ANY,
        RANGE,
        SEQUENCE,
        CHOICE,
        ZERO_OR_MORE,
        ONE_OR_MORE,
        OPTIONAL,
        ALSO,
        NOT,
        EMPTY,
        ERROR,
        RULE,
        WEAK_RULE,
        END_OF_FILE,
        FILTER
      };

      using Shared = std::shared_ptr<GrammarNode>;

      Symbol symbol;

      std::variant<std::vector<Shared>, Shared, std::weak_ptr<peg::Rule>,
                   std::shared_ptr<peg::Rule>, std::string, std::array<Letter, 2>, FilterCallback>
          data;

    private:
      GrammarNode(Symbol s) : symbol(s) {}
      template <class T> GrammarNode(Symbol s, const T &d) : symbol(s), data(d) {}

    public:
      static Shared Word(const std::string &word) {
        return Shared(new GrammarNode(Symbol::WORD, word));
      }
      static Shared Any() { return Shared(new GrammarNode(Symbol::ANY)); }
      static Shared Range(Letter a, Letter b) {
        return Shared(new GrammarNode(Symbol::RANGE, std::array<Letter, 2>({{a, b}})));
      }
      static Shared Sequence(const std::vector<Shared> &args) {
        return Shared(new GrammarNode(Symbol::SEQUENCE, args));
      }
      static Shared Choice(const std::vector<Shared> &args) {
        return Shared(new GrammarNode(Symbol::CHOICE, args));
      }
      static Shared ZeroOrMore(const Shared &arg) {
        return Shared(new GrammarNode(Symbol::ZERO_OR_MORE, arg));
      }
      static Shared OneOrMore(const Shared &arg) {
        return Shared(new GrammarNode(Symbol::ONE_OR_MORE, arg));
      }
      static Shared Optional(const Shared &arg) {
        return Shared(new GrammarNode(Symbol::OPTIONAL, arg));
      }
      static Shared Also(const Shared &arg) { return Shared(new GrammarNode(Symbol::ALSO, arg)); }
      static Shared Not(const Shared &arg) { return Shared(new GrammarNode(Symbol::NOT, arg)); }
      static Shared Empty() { return Shared(new GrammarNode(Symbol::EMPTY)); }
      static Shared Error() { return Shared(new GrammarNode(Symbol::ERROR)); }
      static Shared Rule(const std::shared_ptr<peg::Rule> &rule) {
        return Shared(new GrammarNode(Symbol::RULE, rule));
      }
      static Shared WeakRule(const std::weak_ptr<peg::Rule> &rule) {
        return Shared(new GrammarNode(Symbol::WEAK_RULE, rule));
      }
      static Shared EndOfFile() { return Shared(new GrammarNode(Symbol::END_OF_FILE)); }
      static Shared Filter(const FilterCallback &callback) {
        return Shared(new GrammarNode(Symbol::FILTER, callback));
      }
    };

    std::ostream &operator<<(std::ostream &stream, const GrammarNode &node);

  }  // namespace peg
}  // namespace peg_parser
