[![Build Status](https://travis-ci.com/TheLartians/Parser.svg?branch=master)](https://travis-ci.com/TheLartians/Parser)

lars::parser
============

A linear-time c++ parsing expression grammar (PEG) parser generator supporting left-recursion and ambiguous grammars. Written in C++17.

Example
-------

The following defines a simple calculator program. It is able to parse and evaluate the basic operations `+`, `-`, `*`, `/` while obeying operator and bracket precedence and ignoring whitespace characters between tokens.

```c++
ParserGenerator<float> g;

// Define grammar and evaluation rules
g.setSeparator(g["Whitespace"] << "[\t ]");
g["Sum"     ] << "Add | Subtract | Product";
g["Product" ] << "Multiply | Divide | Atomic";
g["Add"     ] << "Sum '+' Product"    >> [](auto e){ return e[0].evaluate() + e[1].evaluate(); };
g["Subtract"] << "Sum '-' Product"    >> [](auto e){ return e[0].evaluate() - e[1].evaluate(); };
g["Multiply"] << "Product '*' Atomic" >> [](auto e){ return e[0].evaluate() * e[1].evaluate(); };
g["Divide"  ] << "Product '/' Atomic" >> [](auto e){ return e[0].evaluate() / e[1].evaluate(); };
g["Atomic"  ] << "Number | '(' Sum ')'";
g["Number"  ] << "'-'? [0-9]+ ('.' [0-9]+)?" >> [](auto e){ return stof(e.string()); };
g.setStart(g["Sum"]);

// Execute a string
float result = g.run("1 + 2 * (3+4)/2 - 3"); // = 5
```

Compiling
---------
lars::parser requires at least cmake 3.5 and c++17. To compile and run the calculator example: 

```bash
git clone --recursive https://github.com/TheLartians/Parser.git
cd Parser
mkdir build
cd build
cmake -DBUILD_EXAMPLES=true ..
make
./calculator
```

Quickstart
----------
You should familiarize yourself with the syntax of [parsing expression grammars](http://en.wikipedia.org/wiki/Parsing_expression_grammar). The included [examples](https://github.com/TheLartians/Parser/tree/master/examples) should help you to get started quickly.

Time Complexity
---------------
lars::parser memorizes intermediate steps resulting in linear time complexity (as a function of input string length) for grammars without left-recursion. Left-recursive grammars have squared time complexity in worst case.

License
-------
lars::parser is available under the BSD 3-Clause license. See the [LICENSE](https://github.com/TheLartians/Parser/blob/master/LICENSE) for more info.
