#include <catch2/catch.hpp>
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

  auto program = createProgram();
  REQUIRE_NOTHROW(setRule(program, "Whitespace", "[\t ]"));
  REQUIRE_NOTHROW(setSeparator(program, "Whitespace"));
  REQUIRE_NOTHROW(setRule(program, "Sum", "Add | Subtract | Product"));
  REQUIRE_NOTHROW(setRule(program, "Product", "Multiply | Divide | Atomic"));
  REQUIRE_NOTHROW(setRule(program, "Atomic", "Number | '(' Sum ')'"));
  REQUIRE_NOTHROW(setRuleWithCallback("Add", "Sum '+' Product", [=](lars::Any e){ return evaluate(get(e,0)).get_numeric<float>(); }));

/*
  g.setSeparator(g["Whitespace"] << "[\t ]");
  g["Sum"     ] << "Add | Subtract | Product";
  g["Product" ] << "Multiply | Divide | Atomic";
  g["Atomic"  ] << "Number | '(' Sum ')'";
  g["Add"     ] << "Sum '+' Product"    >> [](auto e){ return e[0].evaluate() + e[1].evaluate(); };
  g["Subtract"] << "Sum '-' Product"    >> [](auto e){ return e[0].evaluate() - e[1].evaluate(); };
  g["Multiply"] << "Product '*' Atomic" >> [](auto e){ return e[0].evaluate() * e[1].evaluate(); };
  g["Divide"  ] << "Product '/' Atomic" >> [](auto e){ return e[0].evaluate() / e[1].evaluate(); };
  g["Number"  ] << "'-'? [0-9]+ ('.' [0-9]+)?" >> [](auto e){ return stof(e.string()); };
  g.setStart(g["Sum"]);

  REQUIRE(g.run("2+3") == Approx(5));
  REQUIRE(g.run("2*3") == Approx(6));
  REQUIRE(g.run("1+2+3") == Approx(6));
  REQUIRE(g.run("1+2*3") == Approx(7));
  REQUIRE(g.run("1+2-3") == Approx(0));
  REQUIRE(g.run("2*2/4*3") == Approx(3));
  REQUIRE(g.run("1 - 2*3/2 + 4") == Approx(2));
  REQUIRE(g.run("1 + 2 * (3+4)/ 2 - 3") == Approx(5));
*/


}
