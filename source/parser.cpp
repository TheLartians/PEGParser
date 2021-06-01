
#include <easy_iterator.h>
#include <peg_parser/parser.h>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <stack>
#include <tuple>
#include <utility>
#include <algorithm>

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
    StringViews svs;
    bool singleString;
  private:
    size_t position;
    using CacheKey = std::tuple<size_t, grammar::Rule *>;
    using Cache = std::unordered_map<CacheKey, std::shared_ptr<SyntaxTree>, TupleHasher<CacheKey>>;
    Cache cache;
    std::shared_ptr<SyntaxTree> errorTree;

  public:
    size_t maxPosition;

    explicit State(const std::string_view &s, size_t c = 0) : string(s), position(c), maxPosition(c), singleString(true) {}
    explicit State(StringViews s, size_t c = 0) : svs(std::move(s)), position(c), maxPosition(c), singleString(false) {}

    // bug here
    grammar::Letter current() const {
      if (singleString) {
//         DEBUG
//        std::cout << "(WARNING!!!!!)";
        return position < string.size() ? string[position] : '\0';
      } else {
        auto x = std::upper_bound(svs.presums.begin(), svs.presums.end(), position) - svs.presums.begin();
//        std::cout << "x is " << x << std::endl;
        if (x >= svs.strings.size()) return '\0';
        else return svs.strings[x][position - (x > 0 ? svs.presums[x - 1] : 0)];
      }
    }

    void advance(size_t amount = 1) {
      position += amount;
      position = std::min(position, (singleString ? string.size() : svs.size()));
      maxPosition = std::max(maxPosition, position);
      PARSER_ADVANCE("advancing " << amount << " to " << position << ": '" << current() << "'");
    }

    void setPosition(size_t p) {
      if (p == position) {
        return;
      }
      position = p;
      PARSER_ADVANCE("resetting to " << position << ": '" << current() << "'");
    }

    size_t getPosition() const { return position; }

    struct Saved {
      size_t position;
      size_t innerCount;
    };

    Saved save() { return Saved{position, !stack.empty() ? stack.back()->inner.size() : 0}; }

    void load(const Saved &s) {
      if (!stack.empty()) {
        stack.back()->end = getPosition();
        stack.back()->inner.resize(s.innerCount);
      }
      setPosition(s.position);
    }

    bool isAtEnd() const { return position == (singleString ? string.size() : svs.size()); }

    std::shared_ptr<SyntaxTree> getCached(const std::shared_ptr<grammar::Rule> &rule) {
      auto it = cache.find(std::make_pair(position, rule.get()));
      if (it != cache.end()) return it->second; // get cached SyntaxTree
      return std::shared_ptr<SyntaxTree>(); // if not cache
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
      if (!stack.empty() && !tree->rule->hidden) {
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

  bool parse(const std::shared_ptr<grammar::Node> &node, State &state);

  std::shared_ptr<SyntaxTree> parseRule(const std::shared_ptr<grammar::Rule> &rule, State &state,
                                        bool useCache = true) {
//    if (state.singleString) std::cout << "bug here" << std::endl;
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

    std::shared_ptr<SyntaxTree> syntaxTree;
    if (state.singleString) syntaxTree = std::make_shared<SyntaxTree>(rule, state.string, state.getPosition());
    else syntaxTree = std::make_shared<SyntaxTree>(rule, state.svs, state.getPosition());

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
          if (state.singleString) {
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
          else {
            State recursionState(state.svs, syntaxTree->begin);
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

  // most fundamental parse with respect to Node and its operations
  bool parse(const std::shared_ptr<grammar::Node> &node, State &state) {
    using Node = peg_parser::grammar::Node;
    using Symbol = Node::Symbol;

    PARSER_TRACE("parsing " << *node);

    auto c = state.current();
    switch (node->symbol) {
      case peg_parser::grammar::Node::Symbol::WORD: {
        auto saved = state.save();
        // must exactly match
        for (auto c : pget<std::string>(node->data)) {
          if (state.current() != c) {
            state.load(saved); // go back
            PARSER_TRACE("failed");
            return false;
          }
          state.advance();
        }
        return true;
      }

      case peg_parser::grammar::Node::Symbol::ANY: {
        if (state.isAtEnd()) {
          PARSER_TRACE("failed");
          return false;
        } else {
          state.advance();
          return true;
        }
      }

      case Symbol::RANGE: {
        auto &v = pget<std::array<grammar::Letter, 2>>(node->data);
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
        for (auto n : pget<std::vector<grammar::Node::Shared>>(node->data)) {
          if (!parse(n, state)) {
            state.load(saved); // reversed
            return false;
          }
        }
        return true;
      }

      case Symbol::CHOICE: {
        for (auto n : pget<std::vector<grammar::Node::Shared>>(node->data)) {
          // any one of the candidates successfully parsed is enough
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

      case peg_parser::grammar::Node::Symbol::ONE_OR_MORE: {
        const auto &data = pget<Node::Shared>(node->data);
        // no less than once
        if (!parse(data, state)) {
          return false;
        }
        // second phase: the same as ZERO_OR_MORE
        while (parse(data, state)) {
        }
        return true;
      }

      case peg_parser::grammar::Node::Symbol::OPTIONAL: {
        const auto &data = pget<Node::Shared>(node->data);
        // both are ok
        parse(data, state);
        return true;
      }

      case peg_parser::grammar::Node::Symbol::ALSO: {
        const auto &data = pget<Node::Shared>(node->data);
        auto saved = state.save();
        auto result = parse(data, state);
        // you must go back, without consuming anything
        state.load(saved);
        return result;
      }

      case peg_parser::grammar::Node::Symbol::NOT: {
        const auto &data = pget<Node::Shared>(node->data);
        auto saved = state.save();
        auto result = parse(data, state);
        state.load(saved);
        return !result;
      }

      case peg_parser::grammar::Node::Symbol::ERROR: {
        return false;
      }

      case peg_parser::grammar::Node::Symbol::EMPTY: {
        return true;
      }

      case peg_parser::grammar::Node::Symbol::RULE: {
        const auto &rule = pget<std::shared_ptr<grammar::Rule>>(node->data);
        return parseRule(rule, state)->valid;
      }

      case peg_parser::grammar::Node::Symbol::WEAK_RULE: {
        const auto &data = pget<std::weak_ptr<grammar::Rule>>(node->data);
        if (auto rule = data.lock()) {
          return parseRule(rule, state)->valid;
        } else {
          throw Parser::GrammarError(Parser::GrammarError::INVALID_RULE, node);
        }
      }

      case peg_parser::grammar::Node::Symbol::END_OF_FILE: {
        auto res = state.isAtEnd();
        if (!res) {
          PARSER_TRACE("failed");
        }
        return res;
      }

      case peg_parser::grammar::Node::Symbol::FILTER: {
        const auto &callback = pget<grammar::Node::FilterCallback>(node->data);
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

SyntaxTree::SyntaxTree(const std::shared_ptr<grammar::Rule> &r, std::string_view s, size_t p)
    : rule(r), fullString(s), begin(p), end(p), valid(false), active(true), singleString(true) {}
SyntaxTree::SyntaxTree(const std::shared_ptr<grammar::Rule> &r, StringViews  svs, size_t p)
    : rule(r), svs(std::move(svs)), begin(p), end(p), valid(false), active(true), singleString(false) {}

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

Parser::Parser(const std::shared_ptr<grammar::Rule> &g) : grammar(g) {}

Parser::Result Parser::parseAndGetError(const std::string_view &str,
                                        std::shared_ptr<grammar::Rule> grammar) {
  // use str to generate initial state
  State state(str);
  PARSER_TRACE("Begin parsing of: '" << str << "'");

  // state is passed by reference
  auto result = parseRule(grammar, state);
  auto error = state.getErrorTree();
  if (!error) {
    error = result;
  }
  return Parser::Result{result, error};
}

Parser::Result Parser::parseAndGetError(const StringViews &svs,
                                        std::shared_ptr<grammar::Rule> grammar) {
  State state(svs);
  auto result = parseRule(grammar, state);
  auto error = state.getErrorTree();
  if (!error) { error = result; }
  return Parser::Result{result, error};
}

std::shared_ptr<SyntaxTree> Parser::parse(const std::string_view &str,
                                          std::shared_ptr<grammar::Rule> grammar) {
  return parseAndGetError(str, grammar).syntax;
}

std::shared_ptr<SyntaxTree> Parser::parse(const std::string_view &str) const {
  return parse(str, grammar);
}

Parser::Result Parser::parseAndGetError(const std::string_view &str) const {
  return parseAndGetError(str, grammar);
}
Parser::Result Parser::parseAndGetError(const StringViews &svs) const {
  return parseAndGetError(svs, grammar);
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

StringViews::StringViews() = default;
StringViews::StringViews(std::vector<std::string_view> strings) : strings(std::move(strings)) {
  std::size_t now = 0;
  for (auto x : strings) {
    presums.push_back(now);
    now += x.size();
  }
}
std::size_t StringViews::size() const {
  std::size_t ret = 0;
  for (auto string : strings) {
    ret += string.size();
  }
  return ret;
}
