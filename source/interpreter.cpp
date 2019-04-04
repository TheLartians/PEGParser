#include <lars/interpreter.h>
#include <lars/to_string.h>
#include <lars/iterators.h>
#include <string>

using namespace lars;

const char * InterpreterError::what() const noexcept {
  if (buffer.size() == 0) {
    buffer = "no evaluator for rule '" + tree->rule->name + "'";
  }
  return buffer.c_str();
}

const char * SyntaxError::what() const noexcept {
  if (buffer.size() == 0) {
    buffer = "syntax error at character "
    + std::to_string(syntaxTree->end + 1)
    + " while parsing "
    + syntaxTree->rule->name;
  }
  return buffer.c_str();
}
