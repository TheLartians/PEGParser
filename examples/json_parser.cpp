/**
 * This example demonstrate how we can use lars::parser parse standard JSON.
 * https://en.wikipedia.org/wiki/JSON#Data_types_and_syntax
 */

#include <iostream>
#include <variant>
#include <map>
#include <algorithm>
#include <vector>
#include <memory>
#include <string>

#include <lars/parser/generator.h>

/** Class to store JSON objects */
struct JSON{
  enum Type { 
    NUMBER, STRING, BOOLEAN, ARRAY, OBJECT, EMPTY
  } type;
  std::variant<
    double,
    std::string,
    bool,
    std::vector<JSON>,
    std::map<std::string, JSON>
  > data;
  explicit JSON(double v):type(NUMBER),data(v){ }
  explicit JSON(std::string && v):type(STRING),data(v){ }
  explicit JSON(bool v):type(BOOLEAN),data(v){ }
  explicit JSON(std::vector<JSON> && v):type(ARRAY),data(v){ }
  explicit JSON(std::map<std::string, JSON> &&v):type(OBJECT),data(v){ }
  explicit JSON():type(EMPTY){}
};

/** Print JSON */
std::ostream &operator<<(std::ostream &stream, const JSON &json){
  switch (json.type) {
    case JSON::NUMBER:{
      stream << std::get<double>(json.data);
      break;
    }
    case JSON::STRING:{
      stream << '"' << std::get<std::string>(json.data) << '"';
      break;
    }
    case JSON::BOOLEAN:{
      stream << (std::get<bool>(json.data) ? "true" : "false");
      break;
    }
    case JSON::ARRAY:{
      stream << '[';
      for(auto v: std::get<std::vector<JSON>>(json.data)) stream << v << ',';
      stream << ']';
      break;
    }
    case JSON::OBJECT:{
      stream << '{';
      for(auto v: std::get<std::map<std::string, JSON>>(json.data)){
        stream << '"' << v.first << '"' << ':' << v.second << ',';
      }
      stream << '}';
      break;
    }
    case JSON::EMPTY:{
      stream << "null";
      break;
    } 
  }
  return stream;
}

/** Define the grammar */
lars::ParserGenerator<JSON> createJSONProgram(){
  lars::ParserGenerator<JSON> g;
  
  g.setSeparator(g["Separators"] << "[\t \n]");

  g["JSON"] << "Number | String | Boolean | Array | Object | Empty";

  // Number
  g.setProgramRule("Number", lars::peg::createDoubleProgram(), [](auto e){ return JSON(e.evaluate()); });

  // String
  g.setProgramRule("String", lars::peg::createStringProgram("\"","\""), [](auto e){ return JSON(e.evaluate()); });
  
  // Boolean
  g["Boolean"] << "True | False";  
  g["True"] << "'true'" >> [](auto){ return JSON(true); };
  g["False"] << "'false'" >> [](auto){ return JSON(false); };

  // Array
  g["Array"] << "'[' (JSON (',' JSON)*)? ']'" >> [](auto e){ 
    std::vector<JSON> data(e.size());
    std::transform(e.begin(),e.end(),data.begin(), [](auto v){ return v.evaluate(); });
    return JSON(std::move(data));
  };

  // Object
  g["Object"] << "'{' (Pair (',' Pair)*)? '}'" >> [](auto e){
    std::map<std::string, JSON> data;
    for(auto p:e){ data[std::get<std::string>(p[0].evaluate().data)] = p[1].evaluate(); }
    return JSON(std::move(data));
  };
  g["Pair"] << "String ':' JSON";

  // Empty
  g["Empty"] << "'null'" >> [](auto){ return JSON(); };

  g.setStart(g["JSON"]);
  
  return g;
}

/** Input */
int main() {
  using namespace std;

  auto json = createJSONProgram();

  cout << "Enter a valid JSON expression.\n";
  while (true) {
    string str;
    cout << "> ";
    getline(cin,str);
    if(str == "q" || str == "quit"){ break; }
    try {
      auto result = json.run(str);
      cout << "Parsed JSON: " << result << endl;
    } catch (lars::SyntaxError &error) {
      auto syntax = error.syntax;
      cout << "  ";
      cout << string(syntax->begin, ' ');
      cout << string(syntax->length(), '~');
      cout << "^\n";
      cout << "  " << "Syntax error while parsing " << syntax->rule->name << endl;
    }
  }

  return 0;
}

