/**
 *  This example demonstrate how we can use peg_parser::parser to define a
 * command-line calculator and use a visitor pattern to evaluate the result.
 */

#include <peg_parser/generator.h>

#include <cmath>
#include <iostream>
#include <unordered_map>

int main() {
  using namespace std;

  struct Visitor;

  using Expression = peg_parser::Interpreter<void, Visitor &>::Expression;

  struct Visitor {
    float result;
    unordered_map<string, float> variables;

    float getValue(Expression e) {
      e.evaluate(*this);
      return result;
    }

    void visitAddition(Expression l, Expression r) { result = getValue(l) + getValue(r); }
    void visitSubtraction(Expression l, Expression r) { result = getValue(l) - getValue(r); }
    void visitMultiplication(Expression l, Expression r) { result = getValue(l) * getValue(r); }
    void visitDivision(Expression l, Expression r) { result = getValue(l) / getValue(r); }
    void visitPower(Expression l, Expression r) { result = pow(getValue(l), getValue(r)); }
    void visitVariable(Expression name) { result = variables[name.string()]; }
    void visitAssignment(Expression name, Expression value) {
      result = (variables[name.string()] = getValue(value));
    }
    void visitNumber(Expression value) { result = stod(value.string()); }
  };

  peg_parser::ParserGenerator<void, Visitor &> calculator;

  auto &g = calculator;
  g.setSeparator(g["Whitespace"] << "[\t ]");

  g["Expression"] << "Assign | Sum";
  g["Assign"] << "Name '=' Sum" >> [](auto e, auto &v) { v.visitAssignment(e[0], e[1]); };
  g["Sum"] << "Add | Subtract | Product";
  g["Product"] << "Multiply | Divide | Exponent";
  g["Exponent"] << "Power | Atomic";
  g["Atomic"] << "Number | Brackets | Variable";
  g["Brackets"] << "'(' Sum ')'";
  g["Add"] << "Sum '+' Product" >> [](auto e, auto &v) { v.visitAddition(e[0], e[1]); };
  g["Subtract"] << "Sum '-' Product" >> [](auto e, auto &v) { v.visitSubtraction(e[0], e[1]); };
  g["Multiply"] << "Product '*' Exponent" >>
      [](auto e, auto &v) { v.visitMultiplication(e[0], e[1]); };
  g["Divide"] << "Product '/' Exponent" >> [](auto e, auto &v) { v.visitDivision(e[0], e[1]); };
  g["Power"] << "Atomic ('^' Exponent)" >> [](auto e, auto &v) { v.visitPower(e[0], e[1]); };
  g["Variable"] << "Name" >> [](auto e, auto &v) { v.visitVariable(e); };
  g["Name"] << "[a-zA-Z] [a-zA-Z0-9]*";
  g["Number"] << "'-'? [0-9]+ ('.' [0-9]+)?" >> [](auto e, auto &v) { v.visitNumber(e); };

  g.setStart(g["Expression"]);

  cout << "Enter an expression to be evaluated.\n";

  while (true) {
    string str;
    cout << "> ";
    getline(cin, str);
    if (str == "q" || str == "quit") {
      break;
    }
    try {
      Visitor visitor;
      calculator.run(str, visitor);
      cout << str << " = " << visitor.result << endl;
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
