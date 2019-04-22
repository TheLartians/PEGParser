#include <catch2/catch.hpp>
#include <unordered_map>

#include <lars/parser/extension.h>

TEST_CASE("Extension"){
  using namespace lars;
  auto extension = lars::extensions::parser();

  auto programExtension = extension->get_extension("Program");
  auto expressionExtension = extension->get_extension("Expression");

  auto createProgram = programExtension->get_function("create");
  auto setRule = programExtension->get_function("setRule");
  auto setSeparator = programExtension->get_function("setSeparatorRule");
  auto setStart = programExtension->get_function("setStartRule");
  auto setRuleWithCallback = programExtension->get_function("setRuleWithCallback");
  auto run = programExtension->get_function("run");
  auto evaluate = expressionExtension->get_function("evaluate");
  auto get = expressionExtension->get_function("get");
  auto string = expressionExtension->get_function("string");
  auto size = expressionExtension->get_function("size");
  auto position = expressionExtension->get_function("position");
  auto length = expressionExtension->get_function("length");

  auto program = createProgram();
  REQUIRE_NOTHROW(setRule(program, "Whitespace", "[\t ]"));
  REQUIRE_NOTHROW(setSeparator(program, "Whitespace"));
  REQUIRE_NOTHROW(setStart(program, "Sum"));
  REQUIRE_NOTHROW(setRule(program, "Sum", "Add | Subtract | Product"));
  REQUIRE_NOTHROW(setRule(program, "Product", "Multiply | Divide | Atomic"));
  REQUIRE_NOTHROW(setRule(program, "Atomic", "Number | '(' Sum ')'"));
  
  REQUIRE_NOTHROW(setRuleWithCallback(program, "Add", "Sum '+' Product", lars::AnyFunction([=](lars::Any e,lars::Any &d){
    return evaluate(get(e,0),d).get<float>() + evaluate(get(e,1),d).get<float>();
  })));
  
  REQUIRE_NOTHROW(setRuleWithCallback(program, "Subtract", "Sum '-' Product", lars::AnyFunction([=](lars::Any e,lars::Any &d){
    return evaluate(get(e,0),d).get<float>() - evaluate(get(e,1),d).get<float>();
  })));
  
  REQUIRE_NOTHROW(setRuleWithCallback(program, "Multiply", "Product '*' Atomic", lars::AnyFunction([=](lars::Any e,lars::Any &d){
    return evaluate(get(e,0),d).get<float>() * evaluate(get(e,1),d).get<float>();
  })));
  
  REQUIRE_NOTHROW(setRuleWithCallback(program, "Divide", "Product '/' Atomic", lars::AnyFunction([=](lars::Any e,lars::Any &d){
    return evaluate(get(e,0),d).get<float>() / evaluate(get(e,1),d).get<float>();
  })));
  
  REQUIRE_NOTHROW(setRuleWithCallback(program, "Number", "'-'? [0-9]+ ('.' [0-9]+)?", lars::AnyFunction([=](lars::Any e,lars::Any &){
    REQUIRE(size(e).get_numeric() == 0);
    REQUIRE(position(e).get_numeric() >= 0);
    REQUIRE(length(e).get_numeric() == string(e).get<std::string>().size());
    return float(stof(string(e).get<std::string>()));
  })));
  
  REQUIRE_NOTHROW(setRuleWithCallback(program, "Variable", "[a-zA-Z]+", lars::AnyFunction([=](lars::Any e,lars::Any &d){
    auto &vars = d.get<std::unordered_map<std::string, float>>();
    return vars[string(e).get<std::string>()];
  })));
  
  std::unordered_map<std::string, float> variables;
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
