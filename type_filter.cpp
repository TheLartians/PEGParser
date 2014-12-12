///////////////////////////////////////////////////////////
//
//  lars::parser censor example
//  
//  In this example a grammar ambigouity will be solved 
//  using censors. Note the different meanings of the
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

#include "parser/parser.h"

using namespace std;
using namespace lars;

int main(int argc, char ** argv){
  
  parsing_expression_grammar_builder<int> g;
  using expression = expression<int>;
  
  unordered_set<string> types;
  unordered_set<string> variables;
  
  auto propagate = [](expression e){ for(auto c:e)c.accept(); };
  auto typedef_hander = [&](expression e){ types.insert(e[0].string()); cout << "Create type " << e[0].string() << endl; };
  auto vardef_hander  = [&](expression e){ variables.insert(e[1].string()); cout << "Create variable " << e[1].string() << " of type " << e[0].string() << endl; };
  auto multiply_hander = [&](expression e){ propagate(e); if(e.size()==2) cout << "Multiply: " << e.string() << endl; };
  auto variable_handler = [&](expression e){ auto v = e[0].string(); if(variables.find(v) == variables.end()) e.throw_error("Undefined variable " + v); };
  
  g["Expression"]     << "Typedef | Vardef | Multiplication"  << propagate;
  g["Typedef"]        << "'type' Name"       << typedef_hander;
  g["Vardef"]         << "Type Name"         << vardef_hander;
  g["Multiplication"] << "Multiplication '*' Atomic | Atomic" << multiply_hander;
  g["Type"]           << "AtomicType '*'?"   << propagate;
  g["AtomicType"]     << "Name"              << [&](expression e)->bool{ return types.find(e[0].string()) != types.end(); };
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
    catch (parser<int>::error e){
      std::cout << "  ";
      for(auto i UNUSED :range(e.begin_position().character))std::cout << " ";
      for(auto i UNUSED :range(e.length()))std::cout << "~";
      std::cout << "^\n";
      std::cout << "Error: " << e.error_message();
      if(e.code() == parser<int>::error::parsing_error) cout << " while parsing " << e.rule_name();
      cout << endl;
    }
  }
  
  return 0;
}





