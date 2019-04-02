#include <lars/grammar.h>
#include <lars/iterators.h>

using namespace lars::peg;

std::shared_ptr<Rule> Grammar::addRule(const std::string &name, const std::shared_ptr<GrammarNode> &node){
  auto rule = std::shared_ptr<Rule>(new Rule{name, node});
  addRule(rule);
  return rule;
}

std::shared_ptr<Rule> Grammar::addRule(const std::string &name, const std::shared_ptr<Rule> &rule){
  rules[name] = rule;
  return rule;
}

std::shared_ptr<Rule> Grammar::addRule(const std::shared_ptr<Rule> &rule){
  addRule(rule->name, rule);
  return rule;
}

std::shared_ptr<Rule> Grammar::createRule(const std::string &name){
  auto rule = std::shared_ptr<Rule>(new Rule{name, GrammarNode::Empty()});
  addRule(rule);
  return rule;
}

std::shared_ptr<Grammar> lars::peg::createParsingExpressionGrammar(){
  using GN = GrammarNode;
  
  auto grammar = std::make_shared<Grammar>("PEG Grammar");
  
  auto alphabetic = GrammarNode::Choice({GN::Range('a', 'z'), GN::Range('A', 'Z')});
  auto alphanumeric = GrammarNode::Choice({GN::Range('a', 'z'), GN::Range('A', 'Z'), GN::Range('0', '9')});
  auto identifier = GN::Rule(grammar->addRule("Identifier", GN::Sequence({alphabetic, GN::ZeroOrMore(alphanumeric)})));
  
  auto singleQuote = GN::Word("'");
  auto backslash = GN::Word("\\");
  auto escaped = GN::Rule(grammar->addRule("Escaped",GN::Sequence({backslash,GN::Any()})));
  auto character = GN::Rule(grammar->addRule("Character", GN::Any()));
  
  auto word = GN::Rule(grammar->addRule("Word", GrammarNode::Sequence({
    singleQuote,
    GN::ZeroOrMore(GN::Sequence({GN::Choice({escaped,GN::Sequence({GN::ButNot(singleQuote),character})})})),
    singleQuote
  })));
  
  auto rule = grammar->addRule("Rule", GN::Sequence({word, GN::EndOfFile()}) );
  
  grammar->setStartRule(rule);
  return grammar;
}

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
      stream << "0" << *std::get<GrammarNode::Shared>(node.data);
      break;
    }
      
    case GrammarNode::Symbol::GO_TO_RULE: {
      if (auto rule = std::get<std::weak_ptr<Rule>>(node.data).lock()) {
        stream << rule->name;
      } else {
        stream << "{this rule has been deleted}";
      }
      break;
    }
      
    case GrammarNode::Symbol::GO_TO_GRAMMAR: {
      stream << "Grammar:";
      if (auto grammar = std::get<std::weak_ptr<Grammar>>(node.data).lock()) {
        stream << grammar->name;
      } else {
        stream << "{this grammar has been deleted}";
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
