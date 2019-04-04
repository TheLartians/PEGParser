

#include <iostream>
#include <unordered_map>
#include <cmath>

#include <lars/parser_generator.h>

int main() {
  using namespace std;
  using VariableMap = unordered_map<std::string, float>;
  
  lars::ParserGenerator<float, VariableMap &> calculator;
  
  auto &g = calculator;
  g.setSeparator(g["Whitespace"] << "[\t ]");

  g["Expression"] << "(Set | Sum) <EOF>";
  g["Set"       ] << "Name '=' Sum" >> [](auto e, auto &v){ return v[e[0].string()] = e[1].evaluate(v); };
  g["Sum"       ] << "Add | Subtract | Product";
  g["Add"       ] << "Sum '+' Product" >> [](auto e, auto &v){ return e[0].evaluate(v) + e[1].evaluate(v); };
  g["Subtract"  ] << "Sum '-' Product" >> [](auto e, auto &v){ return e[0].evaluate(v) - e[1].evaluate(v); };
  g["Product"   ] << "Multiply | Divide | Exponent";
  g["Multiply"  ] << "Product '*' Exponent" >> [](auto e, auto &v){ return e[0].evaluate(v) * e[1].evaluate(v); };
  g["Divide"    ] << "Product '/' Exponent" >> [](auto e, auto &v){ return e[0].evaluate(v) / e[1].evaluate(v); };
  g["Exponent"  ] << "Power | Atomic";
  g["Power"     ] << "Atomic '^' Exponent" >> [](auto e, auto &v){ return pow(e[0].evaluate(v),e[1].evaluate(v)); };
  g["Atomic"    ] << "Number | Brackets | Variable";
  g["Brackets"  ] << "'(' Sum ')'";
  g["Number"    ] << "'-'? [0-9]+ ('.' [0-9]+)?" >> [](auto e, auto &){ return stod(e.string()); };
  g["Variable"  ] << "Name" >> [](auto e, auto &v){ return v[e[0].string()]; };
  g["Name"      ] << "[a-zA-Z] [a-zA-Z0-9]*";
  
  g.setStart(g["Expression"]);

  std::cout << "Enter an expression to be evaluated.\n";

  std::unordered_map<std::string, float> variables;

  while (true) {
    std::string str;
    std::cout << "> ";
    std::getline(std::cin,str);
    if(str == "q" || str == "quit"){ break; }
    try {
      auto result = calculator.run(str, variables);
      std::cout << str << " = " << result << std::endl;
    } catch (lars::SyntaxError error) {
      auto errorTree = error.syntaxTree;
      std::cout << "  ";
      std::cout << std::string(errorTree->begin, ' ');
      std::cout << std::string(errorTree->length(), '~');
      std::cout << "^\n";
      std::cout << "  " << "Syntax error while parsing " << errorTree->rule->name << std::endl;
    }
  }

  return 0;
}

