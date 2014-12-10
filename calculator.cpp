

#include <iostream>
#include "parser/parser.h"

using namespace std;
using namespace lars;

int main(int argc, char ** argv){
  parsing_expression_grammar_builder<double> g;
  using expression = expression<double>;
  
  g["Expression"] << "Sum"                             << [](expression e){ e.value() = e[0].get_value();                    };
  g["Sum"       ] << "Add | Subtract | Product"        << [](expression e){ e.value() = e[0].get_value();                    };
  g["Add"       ] << "Sum '+' Product"                 << [](expression e){ e.value() = e[0].get_value() + e[1].get_value(); };
  g["Subtract"  ] << "Sum '-' Product"                 << [](expression e){ e.value() = e[0].get_value() - e[1].get_value(); };
  g["Product"   ] << "Multiply | Divide | Atomic"      << [](expression e){ e.value() = e[0].get_value();                    };
  g["Multiply"  ] << "Product '*' Atomic"              << [](expression e){ e.value() = e[0].get_value() * e[1].get_value(); };
  g["Divide"    ] << "Product '/' Atomic"              << [](expression e){ e.value() = e[0].get_value() / e[1].get_value(); };
  g["Atomic"    ] << "Number | Brackets"               << [](expression e){ e.value() = e[0].get_value();                    };
  g["Brackets"  ] << "'(' Sum ')'"                     << [](expression e){ e.value() = e[0].get_value();                    };
  g["Number"    ] << "'-'? [0-9]+ ('.' [0-9]+)?"       << [](expression e){ e.value() = stod(e.string());                    };
    
  g.set_starting_rule("Expression");

  g["Whitespace"] << "[ \t]";
  g.set_separator_rule("Whitespace");

  auto p = g.get_parser();
  
  while (true) {
    string str;
    cout << "> ";
    std::getline(std::cin,str);
    if(str == "q" || str == "quit")break;
    cout << " -> ";
    try { cout << p.parse(str).get_value(); }
    catch (const char * error){ std::cout << error; }
    cout << std::endl;
  }

  return 0;
}

