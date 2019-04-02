#include <lars/grammar.h>
#include <lars/iterators.h>
#include <lars/interpreter.h>

using namespace lars::peg;

std::ostream & lars::peg::operator<<(std::ostream &stream, const GrammarNode &node){
  using Symbol = lars::peg::GrammarNode::Symbol;
  
  switch (node.symbol) {
    case GrammarNode::Symbol::WORD: {
      stream << "'" << std::get<std::string>(node.data) << "'";
      break;
    }
      
    case GrammarNode::Symbol::ANY: {
      stream << ".";
      break;
    }
    case Symbol::RANGE:{
      auto &v = std::get<std::array<lars::peg::Letter, 2>>(node.data);
      stream << "[" << v[0] << "-" << v[1] << "]";
      break;
    }
      
    case Symbol::SEQUENCE:{
      const auto &data = std::get<std::vector<GrammarNode::Shared>>(node.data);
      stream << "(";
      for (auto [i,n]: lars::enumerate(data)) {
        stream << *n << (i + 1 == data.size() ? "" : " ");
      }
      stream << ")";
      break;
    }
      
    case Symbol::CHOICE:{
      stream << "(";
      const auto &data = std::get<std::vector<GrammarNode::Shared>>(node.data);
      for (auto [i,n]: lars::enumerate(data)) {
        stream << *n << (i + 1 == data.size() ? "" : " | ");
      }
      stream << ")";
      break;
    }
      
    case Symbol::ZERO_OR_MORE:{
      const auto &data = std::get<GrammarNode::Shared>(node.data);
      stream << *data << "*";
      break;
    }
      
    case Symbol::ONE_OR_MORE:{
      const auto &data = std::get<GrammarNode::Shared>(node.data);
      stream << *data << "+";
      break;
    }
      
    case GrammarNode::Symbol::OPTIONAL: {
      stream << *std::get<GrammarNode::Shared>(node.data) << "?";
      break;
    }
      
    case GrammarNode::Symbol::AND_ALSO: {
      stream << "&" << *std::get<GrammarNode::Shared>(node.data);
      break;
    }
      
    case GrammarNode::Symbol::BUT_NOT: {
      stream << "!" << *std::get<GrammarNode::Shared>(node.data);
      break;
    }
      
    case GrammarNode::Symbol::EMPTY: {
      stream << "0";
      break;
    }
      
    case GrammarNode::Symbol::RULE: {
      auto rule = std::get<std::shared_ptr<Rule>>(node.data);
      stream << rule->name;
      break;
    }
            
    case GrammarNode::Symbol::WEAK_RULE: {
      if (auto rule = std::get<std::weak_ptr<Rule>>(node.data).lock()) {
        stream << rule->name;
      } else {
        stream << ":DeletedRule";
      }
      break;
    }
    case GrammarNode::Symbol::END_OF_FILE: {
      stream << ":EOF";
      break;
    }
  }
  
  return stream;
}
