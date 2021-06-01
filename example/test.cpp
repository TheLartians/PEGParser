//
// Created by garen on 6/1/21.
//
#include <peg_parser/generator.h>
#include <peg_parser/parser.h>

#include <cmath>
#include <iostream>
#include <unordered_map>

int main() {
  using namespace std;
  using VariableMap = unordered_map<string, float>;

  peg_parser::ParserGenerator<float, VariableMap &> calculator;

  auto &g = calculator;
  g.setSeparator(g["Whitespace"] << "[\t ]");

  g["Expression"] << "Set | Sum";
  g["Set"] << "Name '=' Sum" >> [](auto e, auto &v) { return v[e[0].string()] = e[1].evaluate(v); };
  g["Sum"] << "Add | Subtract | Product";
  g["Product"] << "Multiply | Divide | Exponent";
  g["Exponent"] << "Power | Atomic";
  g["Atomic"] << "Number | Brackets | Variable";
  g["Brackets"] << "'(' Sum ')'";
  g["Add"] << "Sum '+' Product" >>
           [](auto e, auto &v) { return e[0].evaluate(v) + e[1].evaluate(v); };
  g["Subtract"] << "Sum '-' Product" >>
                [](auto e, auto &v) { return e[0].evaluate(v) - e[1].evaluate(v); };
  g["Multiply"] << "Product '*' Exponent" >>
                [](auto e, auto &v) { return e[0].evaluate(v) * e[1].evaluate(v); };
  g["Divide"] << "Product '/' Exponent" >>
              [](auto e, auto &v) { return e[0].evaluate(v) / e[1].evaluate(v); };
  g["Power"] << "Atomic ('^' Exponent)" >>
             [](auto e, auto &v) { return pow(e[0].evaluate(v), e[1].evaluate(v)); };
  g["Variable"] << "Name" >> [](auto e, auto &v) { return v[e[0].string()]; };
  g["Name"] << "[a-zA-Z] [a-zA-Z0-9]*";
  g["Number"] << "'-'? [0-9]+ ('.' [0-9]+)?" >> [](auto e, auto &) { return stod(e.string()); };

  g.setStart(g["Expression"]);

  cout << "Enter an expression to be evaluated.\n";

  VariableMap variables;

  while (true) {
    string str;
    cout << "> ";
    getline(cin, str);
    if (str == "q" || str == "quit") {
      break;
    }
    std::string str1 = "1 ", str2 = "+ 2", str3 = " + 3";
    std::vector<std::string_view> temp;
    temp.push_back(str1);
    temp.push_back(str2);
    temp.push_back(str3);
    StringViews svs(temp);
    try {
      auto result = calculator.run(svs, variables);
      cout << str << " = " << result << endl;
    } catch (peg_parser::SyntaxError &error) {
      auto syntax = error.syntax;
      cout << "  ";
      cout << string(syntax->begin, ' ');
      cout << string(syntax->length(), '~');
      cout << "^\n";
      cout << "  "
           << "Syntax error while parsing " << syntax->rule->name << endl;
    }
  }

  return 0;
}
