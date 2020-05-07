#include <catch2/catch.hpp>

// clang-format off
#include <peg_parser/generator.h>
#include <iostream>

void example() {
  peg_parser::ParserGenerator<float> g;

  // Define grammar and evaluation rules
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

  // Execute a string
  auto input = "1 + 2 * (3+4)/2 - 3";
  float result = g.run(input); // -> 5
  std::cout << input << " = " << result << std::endl;
}
// clang-format on

TEST_CASE("Example") {
  auto orig_buf = std::cout.rdbuf();
  std::cout.rdbuf(orig_buf);
  CHECK_NOTHROW(example());
  std::cout.rdbuf(orig_buf);
}
