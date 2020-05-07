/**
 *  A command-line calculator that uses a sequental grammar instead of
 * left-recursion.
 */

#include <peg_parser/generator.h>

#include <cmath>
#include <iostream>
#include <numeric>
#include <unordered_map>

int main() {
  using namespace std;
  using VariableMap = unordered_map<string, float>;

  peg_parser::ParserGenerator<float, VariableMap &> calculator;

  auto &g = calculator;
  g.setSeparator(g["Whitespace"] << "[\t ]");

  g["Expression"] << "Assign | Sum";

  g["Assign"] << "Name '=' Sum" >>
      [](auto e, auto &v) { return v[e[0].string()] = e[1].evaluate(v); };

  g["Sum"] << "Product Summand*" >> [=](auto e, auto &v) {
    return std::accumulate(e.begin(), e.end(), 0,
                           [&](auto a, auto b) { return a + b.evaluate(v); });
  };
  g["PositiveSummand"] << "'+' Product" >> [=](auto e, auto &v) { return e[0].evaluate(v); };
  g["NegativeSummand"] << "'-' Product" >> [=](auto e, auto &v) { return -e[0].evaluate(v); };
  g["Summand"] << "PositiveSummand | NegativeSummand";

  g["Product"] << "Power Term*" >> [](auto e, auto &v) {
    return std::accumulate(e.begin(), e.end(), 1,
                           [&](auto a, auto b) { return a * b.evaluate(v); });
  };
  g["NormalTerm"] << "'*' Power" >> [=](auto e, auto &v) { return e[0].evaluate(v); };
  g["InverseTerm"] << "'/' Power" >> [=](auto e, auto &v) { return 1 / e[0].evaluate(v); };
  g["Term"] << "NormalTerm | InverseTerm";

  g["Power"] << "Atomic ('^' Power) | Atomic" >> [](auto e, auto &v) {
    return e.size() == 2 ? pow(e[0].evaluate(v), e[1].evaluate(v)) : e[0].evaluate(v);
  };

  g["Atomic"] << "Number | Brackets | Variable";

  g["Brackets"] << "'(' Sum ')'";
  g["Variable"] << "Name" >> [](auto e, auto &v) { return v[e[0].string()]; };
  g["Name"] << "[a-zA-Z] [a-zA-Z0-9]*";

  // We can also use other programs as rules
  g.setProgramRule("Number", peg_parser::presets::createFloatProgram());

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
    try {
      auto result = calculator.run(str, variables);
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
