[![Build Status](https://travis-ci.com/TheLartians/Parser.svg?branch=master)](https://travis-ci.com/TheLartians/Parser)

lars::parser
============

A linear-time c++ parsing expression grammar (PEG) parser generator supporting left-recursion and ambiguous grammars. Everything written in modern C++17.

Example
-------

Defining and evaluating 

```c++
ParserGenerator<float> g;
g.setSeparator(g["Whitespace"] << "[\t ]");
g["Sum"     ] << "Add | Subtract | Atomic";
g["Product" ] << "Multiply | Divide | Atomic";
g["Add"     ] << "Sum '+' Product"    >> [](auto e){ return e[0].evaluate() + e[1].evaluate(); };
g["Subtract"] << "Sum '-' Product"    >> [](auto e){ return e[0].evaluate() - e[1].evaluate(); };
g["Multiply"] << "Product '*' Atomic" >> [](auto e){ return e[0].evaluate() * e[1].evaluate(); };
g["Divide"  ] << "Product '/' Atomic" >> [](auto e){ return e[0].evaluate() / e[1].evaluate(); };
g["Atomic"  ] << "Number | '(' Sum ')'";
g["Number"  ] << "'-'? [0-9]+ ('.' [0-9]+)?" >> [](auto e){ return stod(e.string()); };
g.setStart(g["Sum"]);
```

Compiling
---------
lars::parser requires at least cmake 3.5 and c++17. To compile and run the calculator example: 

```bash
git clone https://github.com/TheLartians/Parser.git
cd Parser
mkdir build
cd build
cmake -DBUILD_EXAMPLES ..
make
./calculator
```

Quickstart
----------
You should familiar yourself with the syntax of [parsing expression grammars](http://en.wikipedia.org/wiki/Parsing_expression_grammar). The [examples](https://github.com/TheLartians/Parser/tree/master/examples) should help you get started quickly.

Time Complexity
---------------
lars::parser memorizes intermediate steps resulting in linear time complexity for grammars without left-recursion. Left-recursive grammars have squared time complexity (worst case).

License
-------
lars::parser is available under the BSD 3-Clause license. See the LICENSE file for more info.
