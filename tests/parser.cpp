#include <catch2/catch.hpp>

#include <lars/peg.h>
#include <lars/log.h>
#include <lars/to_string.h>

using namespace lars;

TEST_CASE("Number parser") {
  auto parser = peg::createIntegerParser();
  REQUIRE(parser.run("42") == 42);
  REQUIRE(parser.run("-3") == -3);
  REQUIRE(parser.run("-6rest") == -6);
  REQUIRE_THROWS(parser.run("not a number"));
}

TEST_CASE("Hex parser") {
  auto parser = peg::createHexParser();
  REQUIRE(parser.run("42") == 0x42);
  REQUIRE(parser.run("FA34ABC") == 0xFA34ABC);
}

TEST_CASE("Character parser") {
  auto parser = peg::createCharacterParser();
  REQUIRE(parser.run("a") == 'a');
  REQUIRE(parser.run("5") == '5');
  REQUIRE(parser.run("\\\\") == '\\');
  REQUIRE(parser.run("\\n") == '\n');
  REQUIRE(parser.run("\\t") == '\t');
  REQUIRE(parser.run("\\0") == '\0');
}

TEST_CASE("String parser") {
  auto testString = [](std::string open, std::string close){
    auto parser = peg::createStringParser(open, close);
    REQUIRE(parser.run(open + "Hello World!" + close) == "Hello World!");
    REQUIRE(parser.run(open + "Hello Escaped \\" + close + "!" + close) == "Hello Escaped " + close + "!");
  };
  
  testString("'","'");
  testString("``","''");
}

TEST_CASE("PEG Parser") {
  auto parser = peg::createGrammarParser();
  REQUIRE(lars::stream_to_string(*parser.run("rule")) == "rule");
  REQUIRE(lars::stream_to_string(*parser.run("!rule")) == "!rule");
  REQUIRE(lars::stream_to_string(*parser.run("&rule")) == "&rule");
  REQUIRE(lars::stream_to_string(*parser.run("rule+")) == "rule+");
  REQUIRE(lars::stream_to_string(*parser.run("rule*")) == "rule*");
  REQUIRE(lars::stream_to_string(*parser.run("rule?")) == "rule?");
  REQUIRE(lars::stream_to_string(*parser.run("'word'")) == "'word'");
  REQUIRE(lars::stream_to_string(*parser.run("'hello' | world '!'")) == "('hello' | (world '!'))");
  LARS_LOG(" ----------------------------------------------------------------------------- ");
  REQUIRE(lars::stream_to_string(*parser.run("'a' ('+' 'b')*")) == "('a' ('+' 'b')*)");

}

/*
TEST_CASE("Old") {
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
}
*/
