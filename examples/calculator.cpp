/**
 *  This example demonstrate how we can use lars::parser to define a simple command-line calculator.
 *  The parser supports the basic operators `+`, `-`, `*`, `/`, `^` as well as using and assigning
 *  variables via `=`.
 *
 *  Note, that The grammar is defined in a left-recursive way. While this is easiest to implement,
 *  it recomended to rewrite left-recursive grammars sequentially for optimal performance.
 */

#include <iostream>
#include <unordered_map>
#include <cmath>

#include <lars/parser/parser_generator.h>

int main() {
  using namespace std;
  using VariableMap = unordered_map<string, float>;
  
  lars::ParserGenerator<float, VariableMap &> calculator;
  
  auto &g = calculator;
  g.setSeparator(g["Whitespace"] << "[\t ]");

  g["Expression"] << "Set | Sum";
  g["Set"       ] << "Name '=' Sum" >> [](auto e, auto &v){ return v[e[0].string()] = e[1].evaluate(v); };
  g["Sum"       ] << "Add | Subtract | Product";
  g["Product"   ] << "Multiply | Divide | Exponent";
  g["Exponent"  ] << "Power | Atomic";
  g["Atomic"    ] << "Number | Brackets | Variable";
  g["Brackets"  ] << "'(' Sum ')'";
  g["Add"       ] << "Sum '+' Product"       >> [](auto e, auto &v){ return e[0].evaluate(v) + e[1].evaluate(v); };
  g["Subtract"  ] << "Sum '-' Product"       >> [](auto e, auto &v){ return e[0].evaluate(v) - e[1].evaluate(v); };
  g["Multiply"  ] << "Product '*' Exponent"  >> [](auto e, auto &v){ return e[0].evaluate(v) * e[1].evaluate(v); };
  g["Divide"    ] << "Product '/' Exponent"  >> [](auto e, auto &v){ return e[0].evaluate(v) / e[1].evaluate(v); };
  g["Power"     ] << "Atomic ('^' Exponent)" >> [](auto e, auto &v){ return pow(e[0].evaluate(v), e[1].evaluate(v)); };
  g["Variable"  ] << "Name" >> [](auto e, auto &v){ return v[e[0].string()]; };
  g["Name"      ] << "[a-zA-Z] [a-zA-Z0-9]*";
  g["Number"    ] << "'-'? [0-9]+ ('.' [0-9]+)?" >> [](auto e, auto &){ return stod(e.string()); };

  g.setStart(g["Expression"]);

  cout << "Enter an expression to be evaluated.\n";

  VariableMap variables;

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

