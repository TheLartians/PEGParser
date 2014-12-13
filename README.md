lars::parser
============

A linear-time c++ parsing expression grammar (PEG) parser generator supporting left-recursion and ambiguous grammars through a filter concept.

Documentation
-------------
As of now there is no documentation of lars::parser. However, the example files [calculator.cpp](calculator.cpp), [calculator_visitor.cpp](calculator_visitor.cpp) and [type_filter.cpp](type_filter.cpp) should be more or less self-explanatory. A great article about PEGs is available [here](http://en.wikipedia.org/wiki/Parsing_expression_grammar).


Compiling
---------
lars::parser requires c++11. To compile and run the example: 

```
g++ -std=c++11 calculator.cpp -o calculator && ./calculator
```

Time Complexity
---------------
lars::parser memorizes intermediate steps resulting in linear time complexity for grammars without left-recursion. Left-recursive grammars have squared time complexity (worst case).

License
-------
lars::parser is available under the GNU GENERAL PUBLIC LICENSE license. See the LICENSE file for more info.
For additional licencing options please contact the developer at thelartians@gmail.com .
