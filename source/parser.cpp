
#include <lars/new_parser.h>
#include <lars/to_string.h>
#include <lars/hashers.h>
#include <tuple>
#include <stack>

#define LARS_PARSER_TRACE

#ifdef LARS_PARSER_TRACE
#include <lars/log.h>
#define LARS_PARSER_ADVANCE
#define PARSER_TRACE(X) LARS_LOG("parser: " << X)
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
  
  std::string makePrintable(const char &c) {
    if (c == '\0') return "\\0";
    return std::string(1,c);
  }
  
  class State {
  public:
    std::string_view string;

  private:
    size_t position;
    using CacheKey = std::tuple<size_t,peg::Rule*>;
    std::unordered_map<CacheKey, std::shared_ptr<SyntaxTree>, lars::TupleHasher<CacheKey>> cache;
    
  public:
    
    struct Saved {
      size_t position;
    };
    
    State(const std::string_view &s, size_t c = 0):string(s), position(c){ }
    
    peg::Letter current(){
      return string[position];
    }
    
    void advance(size_t amount = 1){
      position += amount;
      if (position > string.size()) { position = string.size(); }
      PARSER_ADVANCE("advancing " << amount << " to " << position << ": '" << makePrintable(current()) << "'");
    }
    
    void setPosition(size_t p){
      if (p == position) { return; }
      position = p;
      PARSER_ADVANCE("resetting to " << position << ": '" << makePrintable(current()) << "'");
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
    
    void addToCache(const std::shared_ptr<peg::Rule> &rule, const std::shared_ptr<SyntaxTree> &tree) {
      cache[std::make_pair(position, rule.get())] = tree;
    }
    
    void addInnerSyntaxTree(const std::shared_ptr<SyntaxTree> &tree){
      if (stack.size() > 0) {
        stack.back()->inner.push_back(tree);
      }
    }
    
    std::vector<std::shared_ptr<SyntaxTree>> stack;

  };
  
  bool parse(const std::shared_ptr<peg::GrammarNode> &node, State &state);
  
  std::shared_ptr<SyntaxTree> parseRule(const std::shared_ptr<peg::Rule> &rule, State &state) {
    auto cached = state.getCached(rule);
    
    if (cached) {
      if (cached->valid) {
        state.addInnerSyntaxTree(cached);
        state.advance();
        state.setPosition(cached->end);
      }
      return cached;
    }
    
    auto syntaxTree = std::make_shared<SyntaxTree>(rule, state.string, state.getPosition());
    state.addToCache(rule, syntaxTree);
    
    auto saved = state.save();
    state.stack.push_back(syntaxTree);

    if (parse(rule->node, state)) {
      syntaxTree->valid = true;
      syntaxTree->end = state.getPosition();
      state.addInnerSyntaxTree(syntaxTree);
    } else {
      state.load(saved);
    }
    
    state.stack.pop_back();

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
            return false;
          }
          state.advance();
        }
        return true;
      }
        
      case lars::peg::GrammarNode::Symbol::ANY: {
        state.advance();
        return true;
      }
        
      case Symbol::RANGE:{
        auto &v = std::get<std::array<peg::Letter, 2>>(node->data);
        if (c >= v[0] && c<= v[1]) {
          state.advance();
          return true;
        }
        else return false;
      }
        
      case Symbol::SEQUENCE:{
        for (auto n: std::get<std::vector<peg::GrammarNode::Shared>>(node->data)) {
          if(!parse(n, state)) return false;
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
        if (!parse(data, state)) { return false; }
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
        
      case lars::peg::GrammarNode::Symbol::GO_TO_RULE: {
        const auto &data = std::get<std::weak_ptr<peg::Rule>>(node->data);
        if (auto rule = data.lock()) {
          return parseRule(rule, state)->valid;
        } else {
          throw Parser::GrammarError(Parser::GrammarError::INVALID_RULE, node);
        }
      }

      case lars::peg::GrammarNode::Symbol::GO_TO_GRAMMAR: {
        
        break;
      }
        
      case lars::peg::GrammarNode::Symbol::END_OF_FILE: {
        return state.isAtEnd();
      }
    }
    
    throw Parser::GrammarError(Parser::GrammarError::UNKNOWN_SYMBOL, node);
  }
  
}

SyntaxTree::SyntaxTree(const std::shared_ptr<peg::Rule> &r, std::string_view s, unsigned p): rule(r), fullString(s), begin(p), end(p), valid(false){
  
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

Parser::Parser(const std::shared_ptr<peg::Grammar> &g):grammar(g){
  
}

std::shared_ptr<SyntaxTree> Parser::parse(const std::string_view &str) {
  State state(str);
  return parseRule(grammar->getStartRule(), state);
}
