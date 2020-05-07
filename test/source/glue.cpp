#include <glue/context.h>
#include <glue/keys.h>
#include <peg_parser/glue.h>

#include <catch2/catch.hpp>
#include <unordered_map>

TEST_CASE("Extension") {
  using namespace peg_parser;

  auto parserGlue = peg_parser::glue();
  glue::Context context;
  context.addRootMap(parserGlue);

  auto programGlue = parserGlue["Program"];
  auto expressionGlue = parserGlue["Expression"];

  auto createProgram = programGlue[glue::keys::constructorKey];
  auto setRule = programGlue["setRule"];
  auto setSeparator = programGlue["setSeparatorRule"];
  auto setStart = programGlue["setStartRule"];
  auto setRuleWithCallback = programGlue["setRuleWithCallback"];
  auto run = programGlue["run"];
  auto evaluate = expressionGlue["evaluate"];
  auto get = expressionGlue["get"];
  auto string = expressionGlue["string"];
  auto size = expressionGlue["size"];
  auto position = expressionGlue["position"];
  auto length = expressionGlue["length"];

  using VariableMap = std::unordered_map<std::string, float>;

  auto program = createProgram();
  REQUIRE_NOTHROW(setRule(program, "Whitespace", "[\t ]"));
  REQUIRE_NOTHROW(setSeparator(program, "Whitespace"));
  REQUIRE_NOTHROW(setStart(program, "Sum"));
  REQUIRE_NOTHROW(setRule(program, "Sum", "Add | Subtract | Product"));
  REQUIRE_NOTHROW(setRule(program, "Product", "Multiply | Divide | Atomic"));
  REQUIRE_NOTHROW(setRule(program, "Atomic", "Number | '(' Sum ')'"));

  REQUIRE_NOTHROW(setRuleWithCallback(
      program, "Add", "Sum '+' Product", glue::AnyFunction([=](const glue::Any &e, VariableMap &d) {
        return evaluate(get(e, 0), d)->get<float>() + evaluate(get(e, 1), d)->get<float>();
      })));

  REQUIRE_NOTHROW(setRuleWithCallback(program, "Subtract", "Sum '-' Product",
                                      glue::AnyFunction([=](const glue::Any &e, VariableMap &d) {
                                        return evaluate(get(e, 0), d)->get<float>()
                                               - evaluate(get(e, 1), d)->get<float>();
                                      })));

  REQUIRE_NOTHROW(setRuleWithCallback(program, "Multiply", "Product '*' Atomic",
                                      glue::AnyFunction([=](const glue::Any &e, VariableMap &d) {
                                        return evaluate(get(e, 0), d)->get<float>()
                                               * evaluate(get(e, 1), d)->get<float>();
                                      })));

  REQUIRE_NOTHROW(setRuleWithCallback(program, "Divide", "Product '/' Atomic",
                                      glue::AnyFunction([=](const glue::Any &e, VariableMap &d) {
                                        return evaluate(get(e, 0), d)->get<float>()
                                               / evaluate(get(e, 1), d)->get<float>();
                                      })));

  REQUIRE_NOTHROW(setRuleWithCallback(program, "Number", "'-'? [0-9]+ ('.' [0-9]+)?",
                                      glue::AnyFunction([=](const glue::Any &e, VariableMap &) {
                                        REQUIRE(size(e)->get<int>() == 0);
                                        REQUIRE(position(e)->get<int>() >= 0);
                                        REQUIRE(length(e)->get<size_t>()
                                                == string(e)->get<std::string>().size());
                                        return float(stof(string(e)->get<std::string>()));
                                      })));

  REQUIRE_NOTHROW(setRuleWithCallback(program, "Variable", "[a-zA-Z]+",
                                      glue::AnyFunction([=](const glue::Any &e, VariableMap &d) {
                                        auto &vars = d;
                                        return vars[string(e)->get<std::string>()];
                                      })));

  VariableMap variables;
  REQUIRE(run(program, "42", variables)->get<float>() == Approx(42));
  REQUIRE(run(program, "2+3", variables)->get<float>() == Approx(5));
  REQUIRE(run(program, "2*3", variables)->get<float>() == Approx(6));
  REQUIRE(run(program, "1+2+3", variables)->get<float>() == Approx(6));
  REQUIRE(run(program, "1+2*3", variables)->get<float>() == Approx(7));
  REQUIRE(run(program, "1+2-3", variables)->get<float>() == Approx(0));
  REQUIRE(run(program, "2*2/4*3", variables)->get<float>() == Approx(3));
  REQUIRE(run(program, "1 - 2*3/2 + 4", variables)->get<float>() == Approx(2));
  REQUIRE(run(program, "1 + 2 * (3+4)/ 2 - 3", variables)->get<float>() == Approx(5));
}
