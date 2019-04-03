
/*
#include <iostream>
#include <unordered_map>
#include <cmath>

#include <lars/parser.h>

using namespace std;
using namespace lars;

class math_visitor{
  
  double value;
  unordered_map<string,double> variables;
  
public:
  
  double get_value(){
    return value;
  }
  
  double get_value(Expression<math_visitor> e){
    e.accept(this);
    return get_value();
  }
  
  void visit_number(Expression<math_visitor> e){
    value = stod(e.string());
  }
  
  void visit_set_variable(Expression<math_visitor> e){
    variables[e[0].string()] = get_value(e[1]);
  }
  
  void visit_variable(Expression<math_visitor> e){
    value = variables[e[0].string()];
  }
  
  void visit_left_binary_operator_list (Expression<math_visitor> e){
    double lhs = get_value(e[0]);
    
    for(auto i:range((e.size()-1)/2)*2+1){
      double rhs = get_value(e[i+1]);
           if(e[i].character()=='+'){ lhs = lhs + rhs; }
      else if(e[i].character()=='-'){ lhs = lhs - rhs; }
      else if(e[i].character()=='*'){ lhs = lhs * rhs; }
      else if(e[i].character()=='/'){ lhs = lhs / rhs; }
      else throw "undefined operator";
    }
    
    value = lhs;
  }
  
  void visit_exponent(Expression<math_visitor> e){
    if(e.size() == 1) e[0].accept();
    else value = pow(get_value(e[0]), get_value(e[1]));
  }
  
};

 */

int main(int argc, char ** argv){
  /*
  ParsingExpressionGrammarBuilder<math_visitor> g;
  using expression = Expression<math_visitor>;
  
  g["Expression"] << "(Set | Sum) &'\\0'"                      << [](expression e){ e[0].accept(); };
  g["Set"       ] << "Name '=' Sum"                            << [](expression e){ e[0].visitor().visit_set_variable(e); };
  g["Sum"       ] << "Product  (AddSub Product)*"              << [](expression e){ e.visitor().visit_left_binary_operator_list(e); };
  g["AddSub"    ] << "[+-]"                                    ;
  g["Product"   ] << "Exponent (MulDiv Exponent)*"             << [](expression e){ e.visitor().visit_left_binary_operator_list(e); };
  g["MulDiv"    ] << "[/*]"                                    ;
  g["Exponent"  ] << "Atomic ('^' Exponent) | Atomic"          << [](expression e){ e.visitor().visit_exponent(e); };
  g["Atomic"    ] << "Number | Brackets | Variable"            << [](expression e){ e[0].accept(); };
  g["Brackets"  ] << "'(' Sum ')'"                             << [](expression e){ e[0].accept(); };
  g["Number"    ] << "'-'? [0-9]+ ('.' [0-9]+)?"               << [](expression e){ e.visitor().visit_number(e); };
  g["Variable"  ] << "Name"                                    << [](expression e){ e.visitor().visit_variable(e); };
  g["Name"      ] << "[a-zA-Z] [a-zA-Z]*"                      ;
  
  g.set_starting_rule("Expression");
  
  g["Whitespace"] << "[ \t]";
  
  g.set_separator_rule("Whitespace");
  
  auto p = g.get_parser();
  
  math_visitor visitor;

  while (true) {
    string str;
    cout << "> ";
    getline(cin,str);
    if(str == "q" || str == "quit")break;
    try { 
      p.parse(str).accept(&visitor); 
      cout << str << " = " << visitor.get_value() << endl;
    }
    catch (Parser<math_visitor>::error e){
      cout << "  ";
      for(auto i UNUSED :range(e.begin_position().character-1))cout << " ";
      for(auto i UNUSED :range(e.length()))cout << "~";
      cout << "^\n";
      cout << e.error_message() << " while parsing " << e.rule_name() << endl;
    }
  }
  
  return 0;*/
}

