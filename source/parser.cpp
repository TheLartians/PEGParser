
#include <easy_iterator.h>
#include <peg_parser/parser.h>

#include <algorithm>
#include <sstream>
#include <stack>
#include <tuple>

// Macros for debugging parsers
// #define PEG_PARSER_TRACE

#ifdef PEG_PARSER_TRACE
#  define PEG_PARSER_DEBUG_LOG
#  define PARSER_TRACE(X) \
    LOG("parser[" << state.getPosition() << "," << state.current() << "]: " << __INDENT << X)
#  define PARSER_ADVANCE(X) \
    LOG("parser[" << getPosition() << "," << current() << "]: " << __INDENT << X)
#else
#  define PARSER_TRACE(X)
#  define PARSER_ADVANCE(X)
#endif

#ifdef PEG_PARSER_DEBUG_LOG
#  include <iostream>
#  define LOG(X) std::cout << X << std::endl;
namespace {
  std::string __INDENT = "";
}
#  define INCREASE_INDENT __INDENT = __INDENT + "  "
#  define DECREASE_INDENT __INDENT = __INDENT.substr(0, __INDENT.size() - 2)
#else
#  define INCREASE_INDENT
#  define DECREASE_INDENT
#endif

namespace {

  /**  alternative to `std::get` that works on iOS < 11 */
  template <class T, class V> const T &pget(const V &v) {
    if (auto r = std::get_if<T>(&v)) {
      return *r;
    } else {
      throw std::runtime_error("corrupted grammar node");
    }
  }

}  // namespace

using namespace peg_parser;

namespace {

  // Code from boost
  // Reciprocal of the golden ratio helps spread entropy
  //     and handles duplicates.
  // See Mike Seymour in magic-numbers-in-boosthash-combine:
  //     http://stackoverflow.com/questions/4948780

  template <class T> inline void hash_combine(std::size_t &seed, T const &v) {
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }

  // Recursive template code derived from Matthieu M.
  template <class Tuple, size_t Index = std::tuple_size<Tuple>::value - 1> struct HashValueImpl {
    static void apply(size_t &seed, Tuple const &tuple) {
      HashValueImpl<Tuple, Index - 1>::apply(seed, tuple);
      hash_combine(seed, std::get<Index>(tuple));
    }
  };

  template <class Tuple> struct HashValueImpl<Tuple, 0> {
    static void apply(size_t &seed, Tuple const &tuple) { hash_combine(seed, std::get<0>(tuple)); }
  };

  template <class Tuple> struct TupleHasher {
    size_t operator()(Tuple const &tt) const {
      size_t seed = 0;
      HashValueImpl<Tuple>::apply(seed, tt);
      return seed;
    }
  };

  template <class T> std::string streamToString(T &&v) {
    std::stringstream stream;
    stream << v;
    return stream.str();
  }

  class State {
  public:
    std::string_view string;

  private:
    size_t position;
    using CacheKey = std::tuple<size_t, peg::Rule *>;
    using Cache = std::unordered_map<CacheKey, std::shared_ptr<SyntaxTree>, TupleHasher<CacheKey>>;
    Cache cache;
    std::shared_ptr<SyntaxTree> errorTree;

  public:
    size_t maxPosition;

    State(const std::string_view &s, size_t c = 0) : string(s), position(c), maxPosition(c) {}

    peg::Letter current() { return string[position]; }

    void advance(size_t amount = 1) {
      position += amount;
      if (position > string.size()) {
        position = string.size();
      }
      if (position > maxPosition) {
        maxPosition = position;
      }
      PARSER_ADVANCE("advancing " << amount << " to " << position << ": '" << current() << "'");
    }

    void setPosition(size_t p) {
      if (p == position) {
        return;
      }
      position = p;
      PARSER_ADVANCE("resetting to " << position << ": '" << current() << "'");
    }

    size_t getPosition() { return position; }

    struct Saved {
      size_t position;
      size_t innerCount;
    };

    Saved save() { return Saved{position, stack.size() > 0 ? stack.back()->inner.size() : 0}; }

    void load(const Saved &s) {
      if (stack.size() > 0) {
        stack.back()->end = getPosition();
        stack.back()->inner.resize(s.innerCount);
      }
      setPosition(s.position);
    }

    bool isAtEnd() { return position == string.size(); }

    std::shared_ptr<SyntaxTree> getCached(const std::shared_ptr<peg::Rule> &rule) {
      auto it = cache.find(std::make_pair(position, rule.get()));
      if (it != cache.end()) return it->second;
      return std::shared_ptr<SyntaxTree>();
    }

    void addToCache(const std::shared_ptr<SyntaxTree> &tree) {
      cache[std::make_pair(tree->begin, tree->rule.get())] = tree;
    }

    const Cache &getCache() { return cache; }

    void removeFromCache(const std::shared_ptr<SyntaxTree> &tree) {
      auto it = cache.find(std::make_pair(tree->begin, tree->rule.get()));
      if (it != cache.end()) {
        cache.erase(it);
      }
    }

    void addInnerSyntaxTree(const std::shared_ptr<SyntaxTree> &tree) {
      if (stack.size() > 0 && !tree->rule->hidden) {
        stack.back()->inner.push_back(tree);
      }
    }

    std::vector<std::shared_ptr<SyntaxTree>> stack;

    std::shared_ptr<SyntaxTree> getErrorTree() { return errorTree; }

    void trackError(const std::shared_ptr<SyntaxTree> &tree) {
      if (!tree) {
        return;
      }
      if (tree->length() > 0 && !tree->rule->hidden) {
        if (errorTree) {
          if (tree->end >= errorTree->end) {
            errorTree = tree;
          }
        } else {
          errorTree = tree;
        }
      }
    }
  };

  bool parse(const std::shared_ptr<peg::GrammarNode> &node, State &state);

  std::shared_ptr<SyntaxTree> parseRule(const std::shared_ptr<peg::Rule> &rule, State &state,
                                        bool useCache = true) {
    PARSER_TRACE("enter rule " << rule->name);
    INCREASE_INDENT;

    if (useCache && rule->cacheable) {
      auto cached = state.getCached(rule);

      if (cached) {
        PARSER_TRACE("cached");
        if (cached->valid) {
          state.addInnerSyntaxTree(cached);
          state.advance();
          state.setPosition(cached->end);
        } else {
          PARSER_TRACE("failed");
          if (cached->active && !cached->recursive) {
            PARSER_TRACE("found left recursion");
            cached->recursive = true;
          }
        }
        DECREASE_INDENT;
        PARSER_TRACE("exit rule " << rule->name);
        return cached;
      }
    }

    auto syntaxTree = std::make_shared<SyntaxTree>(rule, state.string, state.getPosition());

    if (useCache) {
      state.addToCache(syntaxTree);
    }

    auto saved = state.save();
    state.stack.push_back(syntaxTree);
    syntaxTree->valid = parse(rule->node, state);
    syntaxTree->end = state.getPosition();
    syntaxTree->active = false;
    state.stack.pop_back();

    if (syntaxTree->valid) {
      if (useCache && syntaxTree->recursive) {
        PARSER_TRACE("enter left recursion: " << rule->name);
        while (true) {
          State recursionState(state.string, syntaxTree->begin);
          recursionState.trackError(state.getErrorTree());
          // Copy the cache except the currect position to the recursion state
          // TODO: keeping the current state and modifying the cache in place is
          // probably much more efficient.
          for (auto &cached : state.getCache()) {
            if (std::get<0>(cached.first) != syntaxTree->begin) {
              recursionState.addToCache(cached.second);
            }
          }
          recursionState.addToCache(syntaxTree);
          auto tmp = parseRule(rule, recursionState, false);
          state.trackError(recursionState.getErrorTree());
          if (tmp->valid && tmp->end > syntaxTree->end) {
            PARSER_TRACE("parsed left recursion");
            syntaxTree = tmp;
            if (useCache) {
              state.addToCache(syntaxTree);
            }
            state.setPosition(tmp->end);
          } else {
            break;
          }
        }
        PARSER_TRACE("exit left recursion");
      }

      state.addInnerSyntaxTree(syntaxTree);
    } else {
      state.trackError(syntaxTree);
      state.load(saved);
    }

    DECREASE_INDENT;
    PARSER_TRACE("exit rule " << rule->name);

    return syntaxTree;
  }

  bool parse(const std::shared_ptr<peg::GrammarNode> &node, State &state) {
    using Node = peg_parser::peg::GrammarNode;
    using Symbol = Node::Symbol;

    PARSER_TRACE("parsing " << *node);

    auto c = state.current();
    switch (node->symbol) {
      case peg_parser::peg::GrammarNode::Symbol::WORD: {
        auto saved = state.save();
        for (auto c : pget<std::string>(node->data)) {
          if (state.current() != c) {
            state.load(saved);
            PARSER_TRACE("failed");
            return false;
          }
          state.advance();
        }
        return true;
      }

      case peg_parser::peg::GrammarNode::Symbol::ANY: {
        if (state.isAtEnd()) {
          PARSER_TRACE("failed");
          return false;
        } else {
          state.advance();
          return true;
        }
      }

      case Symbol::RANGE: {
        auto &v = pget<std::array<peg::Letter, 2>>(node->data);
        if (c >= v[0] && c <= v[1]) {
          state.advance();
          return true;
        } else {
          PARSER_TRACE("failed");
          return false;
        }
      }

      case Symbol::SEQUENCE: {
        auto saved = state.save();
        for (auto n : pget<std::vector<peg::GrammarNode::Shared>>(node->data)) {
          if (!parse(n, state)) {
            state.load(saved);
            return false;
          }
        }
        return true;
      }

      case Symbol::CHOICE: {
        for (auto n : pget<std::vector<peg::GrammarNode::Shared>>(node->data)) {
          if (parse(n, state)) {
            return true;
          }
        }
        return false;
      }

      case Symbol::ZERO_OR_MORE: {
        auto data = pget<Node::Shared>(node->data);
        while (parse(data, state)) {
        }
        return true;
      }

      case peg_parser::peg::GrammarNode::Symbol::ONE_OR_MORE: {
        const auto &data = pget<Node::Shared>(node->data);
        if (!parse(data, state)) {
          return false;
        }
        while (parse(data, state)) {
        }
        return true;
      }

      case peg_parser::peg::GrammarNode::Symbol::OPTIONAL: {
        const auto &data = pget<Node::Shared>(node->data);
        parse(data, state);
        return true;
      }

      case peg_parser::peg::GrammarNode::Symbol::ALSO: {
        const auto &data = pget<Node::Shared>(node->data);
        auto saved = state.save();
        auto result = parse(data, state);
        state.load(saved);
        return result;
      }

      case peg_parser::peg::GrammarNode::Symbol::NOT: {
        const auto &data = pget<Node::Shared>(node->data);
        auto saved = state.save();
        auto result = parse(data, state);
        state.load(saved);
        return !result;
      }

      case peg_parser::peg::GrammarNode::Symbol::ERROR: {
        return false;
      }

      case peg_parser::peg::GrammarNode::Symbol::EMPTY: {
        return true;
      }

      case peg_parser::peg::GrammarNode::Symbol::RULE: {
        const auto &rule = pget<std::shared_ptr<peg::Rule>>(node->data);
        return parseRule(rule, state)->valid;
      }

      case peg_parser::peg::GrammarNode::Symbol::WEAK_RULE: {
        const auto &data = pget<std::weak_ptr<peg::Rule>>(node->data);
        if (auto rule = data.lock()) {
          return parseRule(rule, state)->valid;
        } else {
          throw Parser::GrammarError(Parser::GrammarError::INVALID_RULE, node);
        }
      }

      case peg_parser::peg::GrammarNode::Symbol::END_OF_FILE: {
        auto res = state.isAtEnd();
        if (!res) {
          PARSER_TRACE("failed");
        }
        return res;
      }

      case peg_parser::peg::GrammarNode::Symbol::FILTER: {
        const auto &callback = pget<peg::GrammarNode::FilterCallback>(node->data);
        bool res;
        if (state.stack.size() > 0) {
          auto tree = state.stack.back();
          tree->end = state.getPosition();
          res = callback(tree);
          state.setPosition(tree->end);
        } else {
          res = false;
        }
        if (!res) {
          PARSER_TRACE("failed");
        }
        return res;
      }
    }

    throw Parser::GrammarError(Parser::GrammarError::UNKNOWN_SYMBOL, node);
  }

}  // namespace

SyntaxTree::SyntaxTree(const std::shared_ptr<peg::Rule> &r, std::string_view s, size_t p)
    : rule(r), fullString(s), begin(p), end(p), valid(false), active(true) {}

const char *peg_parser::Parser::GrammarError::what() const noexcept {
  if (buffer.size() == 0) {
    std::string typeName;
    switch (type) {
      case UNKNOWN_SYMBOL:
        typeName = "UNKNOWN_SYMBOL";
        break;
      case INVALID_RULE:
        typeName = "INVALID_RULE";
        break;
    }
    buffer = "internal error in grammar node (" + typeName + "): " + streamToString(*node);
  }
  return buffer.c_str();
}

Parser::Parser(const std::shared_ptr<peg::Rule> &g) : grammar(g) {}

Parser::Result Parser::parseAndGetError(const std::string_view &str,
                                        std::shared_ptr<peg::Rule> grammar) {
  State state(str);
  PARSER_TRACE("Begin parsing of: '" << str << "'");
  auto result = parseRule(grammar, state);
  auto error = state.getErrorTree();
  if (!error) {
    error = result;
  }
  return Parser::Result{result, error};
}

std::shared_ptr<SyntaxTree> Parser::parse(const std::string_view &str,
                                          std::shared_ptr<peg::Rule> grammar) {
  return parseAndGetError(str, grammar).syntax;
}

std::shared_ptr<SyntaxTree> Parser::parse(const std::string_view &str) const {
  return parse(str, grammar);
}

Parser::Result Parser::parseAndGetError(const std::string_view &str) const {
  return parseAndGetError(str, grammar);
}

std::ostream &peg_parser::operator<<(std::ostream &stream, const SyntaxTree &tree) {
  stream << tree.rule->name << '(';
  if (tree.inner.size() == 0) {
    stream << '\'' << tree.view() << '\'';
  } else {
    for (auto &&[i, arg] : easy_iterator::enumerate(tree.inner)) {
      stream << (*arg) << (i + 1 == tree.inner.size() ? "" : ", ");
    }
  }
  stream << ')';
  return stream;
}
