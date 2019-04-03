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

namespace {
  std::shared_ptr<SyntaxTree> findSyntaxError(const std::shared_ptr<SyntaxTree> &tree){
    for (auto i:lars::reversed(tree->inner)) {
      if(auto res = findSyntaxError(i)) return res;
    }
    if (tree->valid == false) {
      return tree;
    }
    return std::shared_ptr<SyntaxTree>();
  }
}

std::shared_ptr<SyntaxTree> SyntaxError::getErrorTree()const{
  auto innerError = findSyntaxError(outerTree);
  return innerError ? innerError : outerTree;
}

const char * SyntaxError::what() const noexcept {
  if (buffer.size() == 0) {
    auto errorTree = getErrorTree();
    buffer = "syntax error at character "
    + std::to_string(errorTree->end)
    + " while parsing "
    + errorTree->rule->name;
  }
  return buffer.c_str();
}
