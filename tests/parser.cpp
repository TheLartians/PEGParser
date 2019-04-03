#include <catch2/catch.hpp>

#include <lars/peg.h>
#include <lars/parser_generator.h>
#include <string>

#include <lars/log.h>
#include <lars/to_string.h>

using namespace lars;

TEST_CASE("Number Program") {
  auto program = peg::createIntegerProgram();
  REQUIRE(program.run("42") == 42);
  REQUIRE(program.run("-3") == -3);
  REQUIRE_THROWS(program.run("42r"));
  REQUIRE_THROWS(program.run("not a number"));
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

TEST_CASE("Hex Program") {
  auto parser = peg::createHexProgram();
  REQUIRE(parser.run("42") == 0x42);
  REQUIRE(parser.run("FA34ABC") == 0xFA34ABC);
}

TEST_CASE("Character Program") {
  auto program = peg::createCharacterProgram();
  REQUIRE(program.run("a") == 'a');
  REQUIRE(program.run("5") == '5');
  REQUIRE(program.run("\\\\") == '\\');
  REQUIRE(program.run("\\n") == '\n');
  REQUIRE(program.run("\\t") == '\t');
  REQUIRE(program.run("\\0") == '\0');
}

TEST_CASE("String Program") {
  auto testString = [](std::string open, std::string close){
    auto program = peg::createStringProgram(open, close);
    REQUIRE(program.run(open + "Hello World!" + close) == "Hello World!");
    REQUIRE(program.run(open + "Hello\\nEscaped \\" + close + "!" + close) == "Hello\nEscaped " + close + "!");
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
  calculator.setSeparatorRule("Whitespace", "[\t ]");
  calculator.parser.grammar = calculator.setRule("Expression", "Sum");
  calculator.setRule("Sum", "Product ('+' Product)*", [](auto e){ float res = 0; for(auto t:e){ res += t.evaluate(); } return res; });
  calculator.setRule("Product", "Number ('*' Number)*", [](auto e){ float res = 1; for(auto t:e){ res *= t.evaluate(); } return res; });
  calculator.setRule("Number", numberProgram);
  REQUIRE(calculator.run("1+2") == 3);
  REQUIRE(calculator.run("2 * 3") == 6);
  REQUIRE(calculator.run("  1 + 2*3 +4 * 5  ") == 27);
  REQUIRE_THROWS(calculator.run("1+2*"));
}

TEST_CASE("Left recursion") {
  LARS_LOG("---------------------------------------------------------------------------------------------\n\n\n\n");
  ParserGenerator<float> calculator;
  calculator.setSeparatorRule("Whitespace", "[\t ]");
  calculator.parser.grammar = calculator.setRule("FullExpression", "Expression <EOF>");
  calculator.setRule("Expression", "Sum | Number");
  calculator.setRule("Sum", "Sum ('+' Number)+", [](auto e){ return e[0].evaluate() + e[1].evaluate(); });
  calculator.setRule("Number", peg::createFloatProgram());
  REQUIRE(calculator.run("2+3") == 5);
}
