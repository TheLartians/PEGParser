

#include <iostream>
#include "lars/packrat_parser.h"

using namespace std;
using namespace lars;

class math_visitor{
  
  double value;
  
public:
  
  double get_value(){
    return value;
  }
  
  double get_value(expression<math_visitor> e){
    e.accept(this);
    return get_value();
  }
  
  void set_value(double v){
    value = v;
  }
  
  void visit_binary_operator_list (expression<math_visitor> e){
    double lhs = get_value(e[0]);
    
    for(auto i:range(e.size()-1)+1){
      double rhs = get_value(e[i]);
      if(e.intermediate(i)=="+"){ lhs = lhs + rhs; }
      if(e.intermediate(i)=="-"){ lhs = lhs - rhs; }
      if(e.intermediate(i)=="*"){ lhs = lhs * rhs; }
      if(e.intermediate(i)=="/"){ lhs = lhs / rhs; }
    }
    
    value = lhs;
  }

};

int main(int argc, char ** argv){
  parsing_expression_grammar_builder<math_visitor> g;
  using expression = expression<math_visitor>;
  
  g["Expression"] << "Sum"                       << [](expression e){ e[0].accept(); };
  g["Sum"       ] << "Product ([+-] Product)*"   << [](expression e){ e.visitor().visit_binary_operator_list(e); };
  g["Product"   ] << "Atomic  ([*/] Atomic )*"   << [](expression e){ e.visitor().visit_binary_operator_list(e); };
  g["Atomic"    ] << "Number | Brackets"         << [](expression e){ e[0].accept(); };
  g["Brackets"  ] << "'(' Sum ')'"               << [](expression e){ e[0].accept(); };
  g["Number"    ] << "'-'? [0-9]+ ('.' [0-9]+)?" << [](expression e){ e.visitor().set_value(stod(e.string())); };
  
  g.set_starting_rule("Expression");
  auto p = g.get_parser();
  
  while (true) {
    string str;
    cout << "> ";
    cin >> str;
    if(str == "q" || str == "quit")break;
    cout << " -> ";
    try { cout << p.parse(str).evaluate()->get_value(); }
    catch (const char * error){ std::cout << error; }
    cout << std::endl;
  }

  return 0;
}

