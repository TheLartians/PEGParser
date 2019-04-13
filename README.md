[![Build Status](https://travis-ci.com/TheLartians/Parser.svg?branch=master)](https://travis-ci.com/TheLartians/Parser)

lars::parser
============
A linear-time C++17 PEG parser generator supporting memoization, left-recursion and context-dependent grammars.

Example
-------
The following defines a simple calculator program. It is able to parse and evaluate the basic operations `+`, `-`, `*`, `/` while obeying operator and bracket precedence and ignoring whitespace characters between tokens.

```c++
lars::ParserGenerator<float> g;

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
float result = g.run("1 + 2 * (3+4)/2 - 3"); // = 5
```

Quickstart
----------
lars::parser requires at least cmake 3.14 and the ability to compile C++17 code. The following shows how to compile and run the calculator example.

```bash
git clone https://github.com/TheLartians/Parser.git
cmake -HParser -BParser/build -DBUILD_LARS_PARSER_EXAMPLES=true
cmake --build Parser/build
./Parser/build/examples/calculator
```

You should familiarize yourself with the syntax of [parsing expression grammars](http://en.wikipedia.org/wiki/Parsing_expression_grammar). The included [examples](https://github.com/TheLartians/Parser/tree/master/examples) should help you to get started.

Installation and usage
----------------------
With [CPM](https://github.com/TheLartians/CPM), lars::parser can be added to your project by adding the following to your projects' `CMakeLists.txt`.

```cmake
CPMAddPackage(
  NAME LarsParser
  VERSION 1.7
  GIT_REPOSITORY https://github.com/TheLartians/Parser.git
)

target_link_libraries(myProject LarsParser)
```

Alternatively, you can use [FetchContent](https://cmake.org/cmake/help/v3.11/module/FetchContent.html) with similar arguments, or download include it via `add_subdirectory`. Installing lars::parser will allow it to be found via `find_package`.

Performance
-----------
lars::parser uses memoization, resulting in linear time complexity (as a function of input string length) for grammars without left-recursion. Left-recursive grammars have squared time complexity in worst case. Memoization can also be disabled on a per-rule basis, reducing the memory footprint.

License
-------
lars::parser is published under the BSD 3-Clause license. See the [LICENSE](https://github.com/TheLartians/Parser/blob/master/LICENSE) for more info.
