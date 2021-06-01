#include <peg_parser/interpreter.h>
#include <cassert>
#include <string>
#include <utility>
#include <peg_parser/parser.h>


using namespace peg_parser;

const char *InterpreterError::what() const noexcept {
  if (buffer.size() == 0) {
    buffer = "no evaluator for rule '" + tree->rule->name + "'";
  }
  return buffer.c_str();
}

const char *SyntaxError::what() const noexcept {
  if (buffer.size() == 0) {
    buffer = "syntax error at character " + std::to_string(syntax->end + 1) + " while parsing "
             + syntax->rule->name;
  }
  return buffer.c_str();
}
