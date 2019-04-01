
#include <lars/new_parser.h>

using namespace lars;

namespace {
  
  struct State {
    const std::string &str;
    size_t character;
    peg::Letter current(){ return str[character]; }
    void advance(size_t amount = 1){ character+=amount; }
  };
  
  bool parse(const std::shared_ptr<peg::GrammarNode> &node, State &state){
    auto c = state.current();
    switch (node->symbol) {
        
      case lars::peg::GrammarNode::Symbol::RANGE:{
        auto &v = std::get<std::array<peg::Letter, 2>>(node->data);
        if (c >= v[0] && c<= v[1]) {
          state.advance();
          return true;
        }
        else return false;
      }
        
      case lars::peg::GrammarNode::Symbol::SEQUENCE:{
        for (auto n: std::get<std::vector<peg::GrammarNode::Shared>>(node->data)) {
          if(!parse(n, state)) return false;
          return true;
        }
      }
        
    }
  }
  
}

Parser::Parser(const std::shared_ptr<peg::Grammar> &g):grammar(g){
  
}

void Parser::parse(const std::string &str){
  
}

