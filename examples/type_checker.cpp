/**
 *  This example shows how the parser behaviour changes with a grammar ambigouity in a c-like langauge.
 *  Note the different interpretation of `x * y` as either a pointer definition or a multiplication.
 *
 *  Example input:
 *  `x * y`  -> parsed as a multiplication
 *  `type x` -> parsed as a type definition
 *  `x * y`  -> as a variable definition (pointer to y)
 */

#include <iostream>
#include <unordered_set>

#include <lars/parser_generator.h>


int main() {
  using namespace std;


  lars::ParserGenerator<std::string> typeChecker;
  unordered_set<string> types;
  
  auto &g = typeChecker;
  g.setSeparator(g["Whitespace"] << "[\t ]");

  g.setStart(g["Expression"] << "Typedef | Vardef | Multiplication");
  
  g["Typedef"] << "'type' Name" >> [&](auto e){
    types.emplace(e[0].string());
    return "type definition";
  };
  
  // only accepts types that have are declared in types
  g["Typename"] << "Name" << [&](auto s) -> bool {
    auto name = s->inner[0]->string();
    return types.find(name) != types.end();
  };
  
  g["Type"] << "Typename '*'?";
  
  g["Vardef"] << "Type Name" >> [](auto) {
    return "variable definition";
  };

  g["Multiplication"] << "Variable '*' Variable" >> [](auto){
    return "multiplication";
  };
  
  g["Variable"] << "Name";
  g["Atomic"] << "Variable";
  g["Name"] << "[a-zA-Z] [a-zA-Z0-9]*";
  
  while (true) {
    string str;
    cout << "> ";
    getline(cin,str);
    if(str == "q" || str == "quit"){ break; }
    try {
      auto result = typeChecker.run(str);
      cout << str << " = " << result << endl;
    } catch (lars::SyntaxError error) {
      auto errorTree = error.syntaxTree;
      cout << "  ";
      cout << string(errorTree->begin, ' ');
      cout << string(errorTree->length(), '~');
      cout << "^\n";
      cout << "  " << "Syntax error while parsing " << errorTree->rule->name << endl;
    }
  }

  return 0;
}





