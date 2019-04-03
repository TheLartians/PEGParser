///////////////////////////////////////////////////////////
//
//  lars::parser filter example
//  
//  In this example a grammar ambigouity will be solved 
//  using filters. Note the different meanings of the
//  expression "x * y", if "x" is a) a variable or b) a 
//  type.
//
//  The filter is defined in line 48.
//
//  Example input: 
//  > x * y 
//  > type x
//  > x * y
//  > x * z
//  > y * z
//

#include <iostream>
#include <unordered_map>
#include <cmath>
#include <lars/parser.h>

using namespace std;
using namespace lars;

int main(int argc, char ** argv){
  /*
  ParsingExpressionGrammarBuilder<int> g;
  using expression = Expression<int>;
  
  unordered_set<string> types;
  unordered_set<string> variables;
  
  auto propagate = [](expression e){ for(auto c:e)c.accept(); };
  auto typedef_hander = [&](expression e){ types.insert(e[0].string()); cout << "Create type " << e[0].string() << endl; };
  auto vardef_hander  = [&](expression e){ variables.insert(e[1].string()); cout << "Create variable " << e[1].string() << " of type " << e[0].string() << endl; };
  auto multiply_hander = [&](expression e){ propagate(e); if(e.size()==2) cout << "Multiply: (" << e[0].string() << ") * (" << e[1].string() << ")" << endl; };
  auto variable_handler = [&](expression e){ auto v = e[0].string(); if(variables.find(v) == variables.end()) e.throw_error("Undefined variable " + v); };
  
  g["Expression"]     << "(Typedef | Vardef | Multiplication) &'\\0'" << propagate;
  g["Typedef"]        << "'type' Name"       << typedef_hander;
  g["Vardef"]         << "Type Name"         << vardef_hander;
  g["Multiplication"] << "Multiplication '*' Atomic | Atomic" << multiply_hander;
  g["Type"]           << "AtomicType '*'?"   << propagate;
  g["AtomicType"]     << "Name"              >> [&](expression e)->bool{ return types.find(e[0].string()) != types.end(); };
  g["Variable"]       << "Name"              << variable_handler;
  g["Atomic"]         << "Variable"          << propagate;
  g["Name"]           << "[a-zA-Z] [a-zA-Z0-9]*";
  
  g.set_starting_rule("Expression");
  
  g["Whitespace"] << "[ \t]";
  g.set_separator_rule("Whitespace");
  
  auto p = g.get_parser();
  
  while (true) {
    string str;
    cout << "> ";
    getline(cin,str);
    if(str == "q" || str == "quit")break;
    try {
      auto e = p.parse(str);
      e.evaluate();
    }
    catch (Parser<int>::error e){
      cout << "  ";
      for(auto i UNUSED :range(e.begin_position().character-1))cout << " ";
      for(auto i UNUSED :range(e.length()))cout << "~";
      cout << "^\n";
      cout << "Error: " << e.error_message();
      if(e.code() == Parser<int>::error::syntax_error) cout << " while parsing " << e.rule_name();
      cout << endl;
    }
  }*/
  
  return 0;
}





