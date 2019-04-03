
#include <lars/parser.h>
#include <lars/to_string.h>
#include <lars/iterators.h>
#include <lars/hashers.h>
#include <tuple>
#include <stack>

// Macros for debugging parsers
#define LARS_PARSER_TRACE
// #define LARS_PARSER_ADVANCE

#ifdef LARS_PARSER_TRACE
#include <lars/log.h>
#define LARS_PARSER_ADVANCE
#define PARSER_TRACE(X) LARS_LOG("parser[" << state.getPosition() << "," << state.current() << "]: " << X)
#else
#define PARSER_TRACE(X)
#endif

#ifdef LARS_PARSER_ADVANCE
#include <lars/log.h>
#define PARSER_ADVANCE(X) LARS_LOG("parser: " << X)
#else
#define PARSER_ADVANCE(X)
#endif

using namespace lars;

namespace {
  
  class State {
  public:
    std::string_view string;

  private:
    size_t position;
    using CacheKey = std::tuple<size_t,peg::Rule*>;
    std::unordered_map<CacheKey, std::shared_ptr<SyntaxTree>, lars::TupleHasher<CacheKey>> cache;
    
  public:
    size_t maxPosition;
    
    struct Saved {
      size_t position;
    };
    
    State(const std::string_view &s, size_t c = 0):string(s), position(c), maxPosition(c){ }
    
    peg::Letter current(){
      return string[position];
    }
    
    void advance(size_t amount = 1){
      position += amount;
      if (position > string.size()) { position = string.size(); }
      if (position > maxPosition) { maxPosition = position; }
      PARSER_ADVANCE("advancing " << amount << " to " << position << ": '" << current() << "'");
    }
    
    void setPosition(size_t p){
      if (p == position) { return; }
      position = p;
      PARSER_ADVANCE("resetting to " << position << ": '" << current() << "'");
    }

    unsigned getPosition(){
      return position;
    }
    
    Saved save(){
      return Saved{position};
    }
    
    void load(const Saved &s){
      setPosition(s.position);
    }
    
    bool isAtEnd(){
      return position == string.size();
    }
    
    std::shared_ptr<SyntaxTree> getCached(const std::shared_ptr<peg::Rule> &rule) {
      auto it = cache.find(std::make_pair(position, rule.get()));
      if (it != cache.end()) return it->second;
      return std::shared_ptr<SyntaxTree>();
    }
    
    void addToCache(const std::shared_ptr<SyntaxTree> &tree) {
      cache[std::make_pair(tree->begin, tree->rule.get())] = tree;
    }
    
    void removeFromCache(const std::shared_ptr<SyntaxTree> &tree){
      auto it = cache.find(std::make_pair(tree->begin, tree->rule.get()));
      if (it != cache.end()){
        cache.erase(it);
      }
    }
    
    void addInnerSyntaxTree(const std::shared_ptr<SyntaxTree> &tree){
      if (stack.size() > 0 && !tree->rule->hidden) {
        stack.back()->inner.push_back(tree);
      }
    }
    
    std::vector<std::shared_ptr<SyntaxTree>> stack;

  };
  
  bool parse(const std::shared_ptr<peg::GrammarNode> &node, State &state);
  
  std::shared_ptr<SyntaxTree> parseRule(const std::shared_ptr<peg::Rule> &rule, State &state, bool useCache = true) {
    PARSER_TRACE("enter rule " << rule->name);

    if (useCache) {
      auto cached = state.getCached(rule);

      if (cached) {
        PARSER_TRACE("cached");
        if (cached->valid) {
          state.addInnerSyntaxTree(cached);
          state.advance();
          state.setPosition(cached->end);
        } else {
          if (cached->active && !cached->recursive) {
            PARSER_TRACE("found left recursion");
            cached->recursive = true;
          }
          PARSER_TRACE("failed");
        }
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
    syntaxTree->active = false;
    state.stack.pop_back();

    if (syntaxTree->valid) {
      syntaxTree->end = state.getPosition();
      
      if (useCache && syntaxTree->recursive) {
        while (true) {
          PARSER_TRACE("enter left recursion: " << rule->name);
          State tmpState(state.string, syntaxTree->begin);
          tmpState.addToCache(syntaxTree);
          auto tmp = parseRule(rule, tmpState, false);
          if (tmp->valid && tmp->end > syntaxTree->end) {
            syntaxTree = tmp;
            if (useCache) {
              state.addToCache(syntaxTree);
            }
            state.setPosition(tmp->end);
            PARSER_TRACE("got to " << tmp->end);
          } else {
            PARSER_TRACE("exit left recursion");
            break;
          }
        }
      } else {
        state.addInnerSyntaxTree(syntaxTree);
      }

    } else {
      syntaxTree->end = state.maxPosition;
      state.load(saved);
    }
    
    PARSER_TRACE("exit rule " << rule->name);
    return syntaxTree;
  }
  
  bool parse(const std::shared_ptr<peg::GrammarNode> &node, State &state){
    using Node = lars::peg::GrammarNode;
    using Symbol = Node::Symbol;
    
    PARSER_TRACE("parsing " << *node);
    
    auto c = state.current();
    switch (node->symbol) {
        
      case lars::peg::GrammarNode::Symbol::WORD: {
        auto saved = state.save();
        for (auto c: std::get<std::string>(node->data)) {
          if (state.current() != c) {
            state.load(saved);
            PARSER_TRACE("failed");
            return false;
          }
          state.advance();
        }
        return true;
      }
        
      case lars::peg::GrammarNode::Symbol::ANY: {
        if (state.isAtEnd()) {
          PARSER_TRACE("failed");
          return false;
        } else {
          state.advance();
          return true;
        }
      }
        
      case Symbol::RANGE:{
        auto &v = std::get<std::array<peg::Letter, 2>>(node->data);
        if (c >= v[0] && c<= v[1]) {
          state.advance();
          return true;
        } else {
          PARSER_TRACE("failed");
          return false;
        }
      }
        
      case Symbol::SEQUENCE:{
        auto saved = state.save();
        for (auto n: std::get<std::vector<peg::GrammarNode::Shared>>(node->data)) {
          if(!parse(n, state)){
            state.load(saved);
            return false;
          }
        }
        return true;
      }
        
      case Symbol::CHOICE:{
        for (auto n: std::get<std::vector<peg::GrammarNode::Shared>>(node->data)) {
          if(parse(n, state)) {
            return true;
          }
        }
        return false;
      }
        
      case Symbol::ZERO_OR_MORE:{
        while (parse(std::get<Node::Shared>(node->data), state)) { }
        return true;
      }
        
      case lars::peg::GrammarNode::Symbol::ONE_OR_MORE: {
        const auto &data = std::get<Node::Shared>(node->data);
        if (!parse(data, state)) {
          return false;
        }
        while (parse(data, state)) { }
        return true;
      }
        
      case lars::peg::GrammarNode::Symbol::OPTIONAL: {
        const auto &data = std::get<Node::Shared>(node->data);
        parse(data, state);
        return true;
      }
        
      case lars::peg::GrammarNode::Symbol::AND_ALSO: {
        const auto &data = std::get<Node::Shared>(node->data);
        auto saved = state.save();
        auto result = parse(data, state);
        state.load(saved);
        return result;
      }
        
      case lars::peg::GrammarNode::Symbol::BUT_NOT: {
        const auto &data = std::get<Node::Shared>(node->data);
        auto saved = state.save();
        auto result = parse(data, state);
        state.load(saved);
        return !result;
      }
        
      case lars::peg::GrammarNode::Symbol::EMPTY: {
        return true;
      }
        
      case lars::peg::GrammarNode::Symbol::RULE: {
        const auto &rule = std::get<std::shared_ptr<peg::Rule>>(node->data);
        return parseRule(rule, state)->valid;
      }

      case lars::peg::GrammarNode::Symbol::WEAK_RULE: {
        const auto &data = std::get<std::weak_ptr<peg::Rule>>(node->data);
        if (auto rule = data.lock()) {
          return parseRule(rule, state)->valid;
        } else {
          throw Parser::GrammarError(Parser::GrammarError::INVALID_RULE, node);
        }
      }
        
      case lars::peg::GrammarNode::Symbol::END_OF_FILE: {
        auto res = state.isAtEnd();
        if(!res){ PARSER_TRACE("failed"); }
        return res;
      }
    }
    
    throw Parser::GrammarError(Parser::GrammarError::UNKNOWN_SYMBOL, node);
  }
  
}

SyntaxTree::SyntaxTree(const std::shared_ptr<peg::Rule> &r, std::string_view s, unsigned p): rule(r), fullString(s), begin(p), end(p), valid(false), active(true){
  
}

const char * lars::Parser::GrammarError::what()const noexcept{
  if (buffer.size() == 0) {
    std::string typeName;
    switch (type) {
      case UNKNOWN_SYMBOL:
        typeName = "UNKNOWN_SYMBOL";
        break;
      case INVALID_RULE:
        typeName = "INVALID_RULE";
        break;
      case INVALID_GRAMMAR:
        typeName = "INVALID_GRAMMAR";
        break;
    }
    buffer = "internal error in grammar node (" + typeName + "): " + lars::stream_to_string(*node);
  }
  return buffer.c_str();
}

Parser::Parser(const std::shared_ptr<peg::Rule> &g):grammar(g){
  
}

std::shared_ptr<SyntaxTree> Parser::parse(const std::string_view &str, std::shared_ptr<peg::Rule> grammar) {
  State state(str);
  PARSER_TRACE("Begin parsing of: '" << str << "'");
  return parseRule(grammar, state);
}

std::shared_ptr<SyntaxTree> Parser::parse(const std::string_view &str) const {
  return parse(str, grammar);
}

std::ostream & lars::operator<<(std::ostream &stream, const SyntaxTree &tree) {
  stream << tree.rule->name << '(';
  if (tree.inner.size() == 0) {
    stream << '\'' << tree.string() << '\'';
  } else {
    for (auto [i,arg]: lars::enumerate(tree.inner)) {
      stream << (*arg) << (i + 1 == tree.inner.size() ? "" : ",");
    }
  }
  stream << ')';
  return stream;
}
