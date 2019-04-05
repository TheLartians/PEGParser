/**
 *  A command-line calculator that uses a sequental grammar instead of left-recursion.
 */

#include <iostream>
#include <unordered_map>
#include <cmath>
#include <numeric>

#include <lars/iterators.h>
#include <lars/parser_generator.h>

int main() {
  using namespace std;
  using VariableMap = unordered_map<string, float>;
  
  lars::ParserGenerator<float, VariableMap &> calculator;
  
  auto &g = calculator;
  g.setSeparator(g["Whitespace"] << "[\t ]");
  
  g["Expression"] << "Set | Sum";
  
  g["Set"] << "Name '=' Sum" >> [](auto e, auto &v){ return v[e[0].string()] = e[1].evaluate(v); };
  
  shared_ptr<lars::peg::Rule> sumOp = g["SumOp"] << "'+'", subOp = g["SubOp"] << "'-'";
  g["Summand"] << "(SumOp | SubOp) Product" >> [=](auto e, auto &v){
    return e[0].rule() == subOp ? -e[1].evaluate(v) : e[1].evaluate(v);
  };
  g["Sum"] << "Product Summand*" >> [=](auto e, auto &v){
    return std::accumulate(e.begin(), e.end(), 0, [&](auto a, auto b){ return a+b.evaluate(v); });
  };
  
  shared_ptr<lars::peg::Rule> mulOp = g["MulOp"] << "'*'", divOp = g["DivOp"] << "'/'";
  g["Term"] << "(MulOp | DivOp) Power" >> [=](auto e, auto &v){ 
    return e[0].rule() == divOp ? 1/e[1].evaluate(v) : e[1].evaluate(v);
  };
  g["Product"] << "Power Term*" >> [](auto e, auto &v){
    return std::accumulate(e.begin(), e.end(), 1, [&](auto a, auto b){ return a*b.evaluate(v); });
  };
  
  g["Power"] << "Atomic ('^' Power) | Atomic" >> [](auto e, auto &v){
    return e.size() == 2 ? pow(e[0].evaluate(v), e[1].evaluate(v)) : e[0].evaluate(v);
  };
  
  g["Atomic"    ] << "Number | Brackets | Variable";
  g["Brackets"  ] << "'(' Sum ')'";
  g["Variable"  ] << "Name" >> [](auto e, auto &v){ return v[e[0].string()]; };
  g["Name"      ] << "[a-zA-Z] [a-zA-Z0-9]*";
  
  // We can re-use previously defined programs as rules
  g.setProgramRule("Number", lars::peg::createFloatProgram());
  
  g.setStart(g["Expression"]);
  
  cout << "Enter an expression to be evaluated.\n";
  
  unordered_map<string, float> variables;
  
  while (true) {
    string str;
    cout << "> ";
    getline(cin,str);
    if(str == "q" || str == "quit"){ break; }
    try {
      auto result = calculator.run(str, variables);
      cout << str << " = " << result << endl;
    } catch (lars::SyntaxError error) {
      auto syntax = error.syntax;
      cout << "  ";
      cout << string(syntax->begin, ' ');
      cout << string(syntax->length(), '~');
      cout << "^\n";
      cout << "  " << "Syntax error while parsing " << syntax->rule->name << endl;
    }
  }
  
  return 0;
}

