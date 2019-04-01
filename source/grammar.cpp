#include <lars/grammar.h>

using namespace lars::peg;

void lars::peg::Grammar::addRule(const std::string &name, const std::shared_ptr<GrammarNode> &node){
  rules[name] = std::shared_ptr<GrammarRule>(new GrammarRule{node});
}

std::shared_ptr<Grammar> lars::peg::parsingExpressionGrammar(){
  auto grammar = std::make_shared<Grammar>();
  auto alphabetic = GrammarNode::Choice({GrammarNode::Range('a', 'z'), GrammarNode::Range('A', 'Z')});
  auto alphanumeric = GrammarNode::Choice({GrammarNode::Range('a', 'z'), GrammarNode::Range('A', 'Z'), GrammarNode::Range('0', '9')});
  grammar->addRule("Identifier", GrammarNode::Sequence({alphabetic, GrammarNode::ZeroOrMore(alphanumeric)}));
  grammar->setStartRule(grammar->getRule("Identifier"));
  return grammar;
}
