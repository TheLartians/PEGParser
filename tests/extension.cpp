#include <catch2/catch.hpp>
#include <unordered_map>

#include <lars/parser/extension.h>

TEST_CASE("Extension"){
  using namespace lars;
  auto extension = lars::glue::parser();

  auto programExtension = extension["Program"];
  auto expressionExtension = extension["Expression"];

  auto createProgram = programExtension["create"];
  auto setRule = programExtension["setRule"];
  auto setSeparator = programExtension["setSeparatorRule"];
  auto setStart = programExtension["setStartRule"];
  auto setRuleWithCallback = programExtension["setRuleWithCallback"];
  auto run = programExtension["run"];
  auto evaluate = expressionExtension["evaluate"];
  auto get = expressionExtension["get"];
  auto string = expressionExtension["string"];
  auto size = expressionExtension["size"];
  auto position = expressionExtension["position"];
  auto length = expressionExtension["length"];

  using VariableMap = std::unordered_map<std::string, float> ;
  
  auto program = createProgram();
  REQUIRE_NOTHROW(setRule(program, "Whitespace", "[\t ]"));
  REQUIRE_NOTHROW(setSeparator(program, "Whitespace"));
  REQUIRE_NOTHROW(setStart(program, "Sum"));
  REQUIRE_NOTHROW(setRule(program, "Sum", "Add | Subtract | Product"));
  REQUIRE_NOTHROW(setRule(program, "Product", "Multiply | Divide | Atomic"));
  REQUIRE_NOTHROW(setRule(program, "Atomic", "Number | '(' Sum ')'"));
  
  REQUIRE_NOTHROW(setRuleWithCallback(program, "Add", "Sum '+' Product", lars::AnyFunction([=](const lars::Any &e,VariableMap &d){
    return evaluate(get(e,0),d).get<float>() + evaluate(get(e,1),d).get<float>();
  })));
  
  REQUIRE_NOTHROW(setRuleWithCallback(program, "Subtract", "Sum '-' Product", lars::AnyFunction([=](const lars::Any &e,VariableMap &d){
    return evaluate(get(e,0),d).get<float>() - evaluate(get(e,1),d).get<float>();
  })));
  
  REQUIRE_NOTHROW(setRuleWithCallback(program, "Multiply", "Product '*' Atomic", lars::AnyFunction([=](const lars::Any &e,VariableMap &d){
    return evaluate(get(e,0),d).get<float>() * evaluate(get(e,1),d).get<float>();
  })));
  
  REQUIRE_NOTHROW(setRuleWithCallback(program, "Divide", "Product '/' Atomic", lars::AnyFunction([=](const lars::Any &e,VariableMap &d){
    return evaluate(get(e,0),d).get<float>() / evaluate(get(e,1),d).get<float>();
  })));
  
  REQUIRE_NOTHROW(setRuleWithCallback(program, "Number", "'-'? [0-9]+ ('.' [0-9]+)?", lars::AnyFunction([=](const lars::Any &e,VariableMap &){
    REQUIRE(size(e).get<int>() == 0);
    REQUIRE(position(e).get<int>() >= 0);
    REQUIRE(length(e).get<int>() == string(e).get<std::string>().size());
    return float(stof(string(e).get<std::string>()));
  })));
  
  REQUIRE_NOTHROW(setRuleWithCallback(program, "Variable", "[a-zA-Z]+", lars::AnyFunction([=](const lars::Any &e,VariableMap &d){
    auto &vars = d;
    return vars[string(e).get<std::string>()];
  })));
  
  VariableMap variables;
  REQUIRE(run(program,"42",variables).get<float>() == Approx(42));
  REQUIRE(run(program,"2+3",variables).get<float>() == Approx(5));
  REQUIRE(run(program,"2*3",variables).get<float>() == Approx(6));
  REQUIRE(run(program,"1+2+3",variables).get<float>() == Approx(6));
  REQUIRE(run(program,"1+2*3",variables).get<float>() == Approx(7));
  REQUIRE(run(program,"1+2-3",variables).get<float>() == Approx(0));
  REQUIRE(run(program,"2*2/4*3",variables).get<float>() == Approx(3));
  REQUIRE(run(program,"1 - 2*3/2 + 4",variables).get<float>() == Approx(2));
  REQUIRE(run(program,"1 + 2 * (3+4)/ 2 - 3",variables).get<float>() == Approx(5));

}
