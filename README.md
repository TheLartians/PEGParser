[![Actions Status](https://github.com/TheLartians/PEGParser/workflows/MacOS/badge.svg)](https://github.com/TheLartians/PEGParser/actions)
[![Actions Status](https://github.com/TheLartians/PEGParser/workflows/Windows/badge.svg)](https://github.com/TheLartians/PEGParser/actions)
[![Actions Status](https://github.com/TheLartians/PEGParser/workflows/Ubuntu/badge.svg)](https://github.com/TheLartians/PEGParser/actions)
[![Actions Status](https://github.com/TheLartians/PEGParser/workflows/Style/badge.svg)](https://github.com/TheLartians/PEGParser/actions)
[![codecov](https://codecov.io/gh/TheLartians/PEGParser/branch/master/graph/badge.svg)](https://codecov.io/gh/TheLartians/PEGParser)

# PEGParser

A linear-time C++17 PEG parser generator supporting memoization, left-recursion and context-dependent grammars.

## Example

The following defines a simple calculator program. It is able to parse and evaluate the basic operations `+`, `-`, `*`, `/` while obeying operator and bracket precedence and ignoring whitespace characters between tokens.

```c++
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
```

## Quickstart

PEGParser requires at least cmake 3.14 and the ability to compile C++17 code. The following shows how to compile and run the calculator example.

```bash
cmake -Hexample -Bbuild/example
cmake --build build/example -j8
./build/examples/calculator
```

You should familiarize yourself with the syntax of [parsing expression grammars](http://en.wikipedia.org/wiki/Parsing_expression_grammar). The included [examples](https://github.com/TheLartians/Parser/tree/master/examples) should help you to get started.

## Installation and usage

PEGParser can be easily added to your project through [CPM.cmake](https://github.com/TheLartians/CPM.cmake).

```cmake
CPMAddPackage(
  NAME PEGParser
  VERSION 2.0
  GIT_REPOSITORY https://github.com/TheLartians/PEGParser.git
)

target_link_libraries(myProject PEGParser)
```

## Performance

PEGParser uses memoization, resulting in linear time complexity (as a function of input string length) for grammars without left-recursion. Left-recursive grammars have squared time complexity in worst case. Memoization can also be disabled on a per-rule basis, reducing the memory footprint and allowing context-dependent rules.
