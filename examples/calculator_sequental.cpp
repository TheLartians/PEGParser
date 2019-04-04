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
  g["Set"       ] << "Name '=' Sum" >> [](auto e, auto &v){ return v[e[0].string()] = e[1].evaluate(v); };
  
  g["SumOp"     ] << "'+' | '-'";
  g["Sum"       ] << "Product (SumOp Product)*" >> [](auto e, auto &v){
    auto result = e[0].evaluate(v);
    for(auto i:lars::range<size_t>(1, e.size(), 2)){
      if (e[i].view()[0] == '-') { result -= e[i+1].evaluate(v); }
      else { result += e[i+1].evaluate(v); }
    };		
    return result;
  };
  
  g["ProdOp"    ] << "'*' | '/'";
  g["Product"   ] << "Atomic (ProdOp Atomic)*" >> [](auto e, auto &v){
    auto result = e[0].evaluate(v);
    for(auto i:lars::range<size_t>(1, e.size(), 2)){
      if (e[i].view()[0] == '/') { result /= e[i+1].evaluate(v); }
      else { result *= e[i+1].evaluate(v); }
    };
    return result;
  };
  
  g["Atomic"    ] << "Number | Brackets | Variable";
  g["Brackets"  ] << "'(' Sum ')'";
  g["Variable"  ] << "Name" >> [](auto e, auto &v){ return v[e[0].string()]; };
  g["Name"      ] << "[a-zA-Z] [a-zA-Z0-9]*";
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

