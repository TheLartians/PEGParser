#include <catch2/catch.hpp>

#include <lars/new_parser.h>
#include <lars/to_string.h>

TEST_CASE("Range iteration 1") {
  
  
  auto grammar = lars::peg::createParsingExpressionGrammar();
  auto parser = lars::Parser(grammar);
  
  REQUIRE(!parser.parse("hello")->valid);

  auto tree = parser.parse("'hello'");
  REQUIRE(tree->valid);
  
  /*
  using T = std::vector<std::string>;
  using Expression = lars::Expression<T>;
  lars::ParsingExpressionGrammarBuilder<T> g;
  g["Start"] << "Word (Word | (Punctuation !'\\0'))*. Punctuation &'\\0'" << [](Expression e){ for(auto n: e) { n.accept(); } };
  g["Word"] << "[a-zA-Z]+" << [](Expression e){ e.visitor().push_back(e.string()); };
  g["Punctuation"] << "[.!?]+" << [](Expression UNUSED e){};
  g["Whitespace"] << "' '+";
  
  g.set_separator_rule("Whitespace");
  g.set_starting_rule("Start");

  auto parser = g.get_parser();
  
  T words = *parser.parse("Hello World!").evaluate();
  REQUIRE(words.size() == 2);
  REQUIRE(words[0] == "Hello");
  REQUIRE(words[1] == "World");
  
  REQUIRE_NOTHROW(parser.parse("Hello!"));
  REQUIRE_THROWS(parser.parse("Hello World"));
  REQUIRE_THROWS(parser.parse("!"));
   */
}
