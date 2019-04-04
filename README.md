lars::parser
============

A linear-time c++ parsing expression grammar (PEG) parser generator supporting left-recursion and ambiguous grammars through a filter concept.

Documentation
-------------
As of now there is no documentation of lars::parser. However, the example files should be more or less self-explanatory. A great article about PEGs is available [here](http://en.wikipedia.org/wiki/Parsing_expression_grammar).


Compiling
---------
lars::parser requires c++11. To compile the examples: 

```
mkdir build
cmake ..
make
```

Time Complexity
---------------
lars::parser memorizes intermediate steps resulting in linear time complexity for grammars without left-recursion. Left-recursive grammars have squared time complexity (worst case).

License
-------
lars::parser is available under the BSD 3-Clause license. See the LICENSE file for more info.
For additional licencing options please contact the developer at thelartians@gmail.com .
