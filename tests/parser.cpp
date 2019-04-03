#include <catch2/catch.hpp>

#include <lars/peg.h>
#include <lars/parser_generator.h>
#include <string>

#include <lars/log.h>
#include <lars/to_string.h>

using namespace lars;

TEST_CASE("Number parser") {
  auto parser = peg::createIntegerProgram();
  REQUIRE(parser.run("42") == 42);
  REQUIRE(parser.run("-3") == -3);
  REQUIRE(parser.run("-6rest") == -6);
  REQUIRE_THROWS(parser.run("not a number"));
}

TEST_CASE("Hex parser") {
  auto parser = peg::createHexProgram();
  REQUIRE(parser.run("42") == 0x42);
  REQUIRE(parser.run("FA34ABC") == 0xFA34ABC);
}

TEST_CASE("Character parser") {
  auto parser = peg::createCharacterProgram();
  REQUIRE(parser.run("a") == 'a');
  REQUIRE(parser.run("5") == '5');
  REQUIRE(parser.run("\\\\") == '\\');
  REQUIRE(parser.run("\\n") == '\n');
  REQUIRE(parser.run("\\t") == '\t');
  REQUIRE(parser.run("\\0") == '\0');
}

TEST_CASE("String parser") {
  auto testString = [](std::string open, std::string close){
    auto parser = peg::createStringProgram(open, close);
    REQUIRE(parser.run(open + "Hello World!" + close) == "Hello World!");
    REQUIRE(parser.run(open + "Hello\\nEscaped \\" + close + "!" + close) == "Hello\nEscaped " + close + "!");
  };
  
  testString("'","'");
  testString("``","''");
  testString("begin "," end");
}

TEST_CASE("PEG Parser") {
  auto parser = peg::createGrammarProgram([](std::string_view name){
    return peg::GrammarNode::Rule(peg::makeRule(name, peg::GrammarNode::Empty()));
  });
  REQUIRE(lars::stream_to_string(*parser.run("rule")) == "rule");
  REQUIRE(lars::stream_to_string(*parser.run("rule_2")) == "rule_2");
  REQUIRE(lars::stream_to_string(*parser.run("!rule")) == "!rule");
  REQUIRE(lars::stream_to_string(*parser.run("&rule")) == "&rule");
  REQUIRE(lars::stream_to_string(*parser.run("rule+")) == "rule+");
  REQUIRE(lars::stream_to_string(*parser.run("rule*")) == "rule*");
  REQUIRE(lars::stream_to_string(*parser.run("rule?")) == "rule?");
  REQUIRE(lars::stream_to_string(*parser.run("'word'")) == "'word'");
  REQUIRE(lars::stream_to_string(*parser.run("[a-z]")) == "[a-z]");
  REQUIRE(lars::stream_to_string(*parser.run("[abc]")) == "('a' | 'b' | 'c')");
  REQUIRE(lars::stream_to_string(*parser.run("[abc-de]")) == "('a' | 'b' | [c-d] | 'e')");
  REQUIRE(lars::stream_to_string(*parser.run("[abc\\-d]")) == "('a' | 'b' | 'c' | '-' | 'd')");
  REQUIRE(lars::stream_to_string(*parser.run("<EOF>")) == "<EOF>");
  REQUIRE(lars::stream_to_string(*parser.run("<>")) == "<>");
  REQUIRE(lars::stream_to_string(*parser.run(".")) == ".");
  REQUIRE(lars::stream_to_string(*parser.run("a   b  c")) == "(a b c)");
  REQUIRE(lars::stream_to_string(*parser.run("a   |  b |\tc")) == "(a | b | c)");
  REQUIRE(lars::stream_to_string(*parser.run("'hello' | world '!'")) == "('hello' | (world '!'))");
  REQUIRE(lars::stream_to_string(*parser.run("('a'+ (.? | b | <>)* [0-9] &<EOF>)")) == "('a'+ (.? | b | <>)* [0-9] &<EOF>)");
  REQUIRE_THROWS(parser.run("a | b | "));
  REQUIRE_THROWS(parser.run("a b @"));
  REQUIRE_THROWS(parser.run("42"));
}

TEST_CASE("Float Program") {
  
  auto testFloatProgram = [](auto p){
    REQUIRE(p.run("42") == Approx(42));
    REQUIRE(p.run("3.1412") == Approx(3.1412));
    REQUIRE(p.run("2E10") == Approx(2E10));
    REQUIRE(p.run("1.4e-3") == Approx(1.4e-3));
  };
  
  testFloatProgram(peg::createFloatProgram());
  testFloatProgram(peg::createDoubleProgram());
}

TEST_CASE("Parser Generator") {
  ParserGenerator<void> invalidProgram;
  REQUIRE_THROWS(invalidProgram.run(""));
  invalidProgram.setRule("A", "'a'");
  invalidProgram.parser.grammar = invalidProgram.setRule("B", "A A");
  REQUIRE(invalidProgram.parser.parse("aa")->valid);
  REQUIRE_THROWS(invalidProgram.run("aa"));

  ParserGenerator<int> numberProgram;
  numberProgram.parser.grammar = numberProgram.setRule("Number", "'-'? [0-9] [0-9]*", [](auto e){ return std::stoi(std::string(e.string())); });
  REQUIRE(numberProgram.run("3") == 3);
  REQUIRE(numberProgram.run("-42") == -42);

  ParserGenerator<float> calculator;
  calculator.parser.grammar = calculator.setRule("FullExpression", "Expression <EOF>");
  calculator.setSeparatorRule("Whitespace", "[\t ]");
  calculator.setRule("Expression", "Sum");
  calculator.setRule("Sum", "Product ('+' Product)*", [](auto e){ float res = 0; for(auto t:e){ res += t.evaluate(); } return res; });
  calculator.setRule("Product", "Number ('*' Number)*", [](auto e){ float res = 1; for(auto t:e){ res *= t.evaluate(); } return res; });
  calculator.setRule("Number", numberProgram, [](auto e){ return e.evaluate(); });
  REQUIRE(calculator.run("1+2") == 3);
  REQUIRE(calculator.run("2 * 3") == 6);
  REQUIRE(calculator.run("  1 + 2*3 +4 * 5  ") == 27);
  REQUIRE_THROWS(calculator.run("1+2*"));
}

TEST_CASE("Left recursion") {
  ParserGenerator<float> calculator;
  calculator.setSeparatorRule("Whitespace", "[\t ]");
  calculator.parser.grammar = calculator.setRule("FullExpression", "Expression <EOF>");
  calculator.setRule("Expression", "Sum | Number");
  calculator.setRule("Sum", "Number ('+' Number)+", [](auto e){ float res = 0; for(auto t:e){ res += t.evaluate(); } return res; });
  calculator.setRule("Number", peg::createIntegerProgram(), [](auto e){ return e.evaluate(); });
  REQUIRE(calculator.run("2+3") == 5);
}
