#include <peg_parser/generator.h>

#include <catch2/catch.hpp>
#include <numeric>
#include <sstream>
#include <string>
#include <tuple>

template <class T> std::string stream_to_string(const T &obj) {
  std::stringstream stream;
  stream << obj;
  return stream.str();
}

using namespace peg_parser;

TEST_CASE("Number Program") {
  auto program = presets::createIntegerProgram();
  REQUIRE(program.run("42") == 42);
  REQUIRE(program.run("-3") == -3);
  REQUIRE_THROWS(program.run("42r"));
  REQUIRE_THROWS(program.run("not a number"));
}

TEST_CASE("Float Program") {
  auto testFloatProgram = [](auto p) {
    REQUIRE(p.run("42") == Approx(42));
    REQUIRE(p.run("3.1412") == Approx(3.1412));
    REQUIRE(p.run("2E10") == Approx(2E10));
    REQUIRE(p.run("1.4e-3") == Approx(1.4e-3));
  };

  testFloatProgram(presets::createFloatProgram());
  testFloatProgram(presets::createDoubleProgram());
}

TEST_CASE("Hex Program") {
  auto parser = presets::createHexProgram();
  REQUIRE(parser.run("42") == 0x42);
  REQUIRE(parser.run("FA34ABC") == 0xFA34ABC);
}

TEST_CASE("Character Program") {
  auto program = presets::createCharacterProgram();
  REQUIRE(program.run("a") == 'a');
  REQUIRE(program.run("5") == '5');
  REQUIRE(program.run("\\\\") == '\\');
  REQUIRE(program.run("\\n") == '\n');
  REQUIRE(program.run("\\t") == '\t');
  REQUIRE(program.run("\\0") == '\0');
}

TEST_CASE("String Program") {
  auto [open, close]
      = GENERATE(as<std::tuple<std::string, std::string>>(), std::make_tuple("'", "'"),
                 std::make_tuple("``", "''"), std::make_tuple("begin ", " end"));
  auto program = presets::createStringProgram(open, close);
  REQUIRE(program.run(open + "Hello World!" + close) == "Hello World!");
  REQUIRE(program.run(open + "Hello\\nEscaped \\" + close + "!" + close)
          == "Hello\nEscaped " + close + "!");
}

TEST_CASE("PEG Parser") {
  auto rc = [](std::string_view name) {
    return presets::GrammarNode::Rule(presets::makeRule(name, presets::GrammarNode::Empty()));
  };

  auto parser = presets::createGrammarProgram();
  REQUIRE(stream_to_string(*parser.run("rule", rc)) == "rule");
  REQUIRE(stream_to_string(*parser.run("rule_2", rc)) == "rule_2");
  REQUIRE(stream_to_string(*parser.run("!rule", rc)) == "!rule");
  REQUIRE(stream_to_string(*parser.run("&rule", rc)) == "&rule");
  REQUIRE(stream_to_string(*parser.run("rule+", rc)) == "rule+");
  REQUIRE(stream_to_string(*parser.run("rule*", rc)) == "rule*");
  REQUIRE(stream_to_string(*parser.run("rule?", rc)) == "rule?");
  REQUIRE(stream_to_string(*parser.run("'word'", rc)) == "'word'");
  REQUIRE(stream_to_string(*parser.run("[a-z]", rc)) == "[a-z]");
  REQUIRE(stream_to_string(*parser.run("[abc]", rc)) == "('a' | 'b' | 'c')");
  REQUIRE(stream_to_string(*parser.run("[abc-de]", rc)) == "('a' | 'b' | [c-d] | 'e')");
  REQUIRE(stream_to_string(*parser.run("[abc\\-d]", rc)) == "('a' | 'b' | 'c' | '-' | 'd')");
  REQUIRE(stream_to_string(*parser.run("<EOF>", rc)) == "<EOF>");
  REQUIRE(parser.run("''", rc)->symbol == presets::GrammarNode::Symbol::EMPTY);
  REQUIRE(stream_to_string(*parser.run("''", rc)) == "''");
  REQUIRE(parser.run("[]", rc)->symbol == presets::GrammarNode::Symbol::ERROR);
  REQUIRE(stream_to_string(*parser.run("[]", rc)) == "[]");
  REQUIRE(stream_to_string(*parser.run(".", rc)) == ".");
  REQUIRE(stream_to_string(*parser.run("a   b  c", rc)) == "(a b c)");
  REQUIRE(stream_to_string(*parser.run("a   |  b |\tc", rc)) == "(a | b | c)");
  REQUIRE(stream_to_string(*parser.run("'hello' | world '!'", rc)) == "('hello' | (world '!'))");
  REQUIRE(stream_to_string(*parser.run("('a'+ (.? | b | '')* [0-9] &<EOF>)", rc))
          == "('a'+ (.? | b | '')* [0-9] &<EOF>)");
  REQUIRE_THROWS(parser.run("a | b | ", rc));
  REQUIRE_THROWS(parser.run("a b @", rc));
  REQUIRE_THROWS(parser.run("42", rc));
}

TEST_CASE("Program with return value") {
  ParserGenerator<int> program;
  REQUIRE_THROWS_AS(program.run("aa"), SyntaxError);
  REQUIRE_THROWS_WITH(program.run(""), "syntax error at character 1 while parsing undefined");
  program.setRule("A", "'a'");
  program.setStart(program.setRule("B", "A+"));
  REQUIRE(program.parser.parse("aa")->valid);
  REQUIRE_THROWS_AS(program.run("aa"), InterpreterError);
  REQUIRE_THROWS_WITH(program.run("aa"), "no evaluator for rule 'A'");
  auto count = 0;
  program.setRule("A", "'a'", [&](auto) { return ++count; });
  REQUIRE(program.run("aaa") == 3);
  REQUIRE(count == 3);
}

TEST_CASE("Program with argument") {
  ParserGenerator<void, int &> program;
  int count = 0;
  REQUIRE_THROWS(program.run("", count));
  REQUIRE_THROWS(program.run("aa", count));
  program.setRule("A", "'a'");
  program.setStart(program.setRule("B", "A+"));
  REQUIRE(program.parser.parse("aa")->valid);
  REQUIRE_NOTHROW(program.run("aa", count));
  program.setRule("A", "'a'", [&](auto, int &count) { ++count; });
  REQUIRE_NOTHROW(program.run("aaa", count));
  REQUIRE(count == 3);
}

TEST_CASE("Evaluation") {
  ParserGenerator<int> numberProgram;
  numberProgram.setStart(numberProgram.setRule("Number", "'-'? [0-9] [0-9]*",
                                               [](auto e) { return std::stoi(e.string()); }));
  REQUIRE(numberProgram.run("3") == 3);
  REQUIRE(numberProgram.run("-42") == -42);

  ParserGenerator<float> calculator;
  calculator.setSeparatorRule("Whitespace", "[\t ]");
  calculator.setStart(calculator.setRule("Expression", "Sum"));
  calculator.setRule("Sum", "Product ('+' Product)*", [](auto e) {
    float res = 0;
    for (auto t : e) {
      res += t.evaluate();
    }
    return res;
  });
  calculator.setRule("Product", "Number ('*' Number)*", [](auto e) {
    float res = 1;
    for (auto t : e) {
      res *= t.evaluate();
    }
    return res;
  });
  calculator.setProgramRule("Number", numberProgram);
  REQUIRE(calculator.run("42") == 42);
  REQUIRE(calculator.run("1+2") == 3);
  REQUIRE(calculator.run("2 * 3") == 6);
  REQUIRE(calculator.run("1 + 2*3") == 7);
  REQUIRE(calculator.run("  1 + 2*3*1 +4 * 5  ") == 27);
  REQUIRE_THROWS(calculator.run("1+2*"));
}
#include <iostream>
TEST_CASE("Left recursion") {
  ParserGenerator<float> calculator;
  calculator.setSeparatorRule("Whitespace", "[\t ]");
  calculator.setStart(calculator.setRule("Expression", "Sum | Atomic"));
  calculator.setRule("Sum", "Addition | Product");
  calculator.setRule("NegativeSummand", "'-' Product", [](auto e) { return -e[0].evaluate(); });
  calculator.setRule("Addition", "Sum ('+' Product | NegativeSummand)",
                     [](auto e) { return e[0].evaluate() + e[1].evaluate(); });
  calculator.setRule("Product", "Multiplication | Atomic");
  calculator.setRule("Multiplication", "Product '*' Atomic",
                     [](auto e) { return e[0].evaluate() * e[1].evaluate(); });
  calculator.setRule("Atomic", "Number | Negative | Brackets");
  calculator.setProgramRule("Number", presets::createFloatProgram());
  calculator.setRule("Negative", "'-' Atomic", [](auto e) { return -e[0].evaluate(); });
  calculator.setRule("Brackets", "'(' Expression ')'");

  REQUIRE(calculator.run("42") == 42);
  REQUIRE(calculator.run("1+2") == 3);
  REQUIRE(calculator.run("1+2-3-5") == -5);
  REQUIRE(calculator.run("2 * 3") == 6);
  REQUIRE(calculator.run("1 + 2*3") == 7);
  REQUIRE(calculator.run("  1 + 2*3*1 +4 * 5  ") == 27);

  REQUIRE(calculator.run("-42") == -42);
  REQUIRE(calculator.run("--42") == 42);
  REQUIRE(calculator.run("---42") == -42);
  REQUIRE(calculator.run("----------------------------------------------------42") == 42);

  REQUIRE_THROWS(calculator.run("1+2*"));
}

TEST_CASE("Filter") {
  ParserGenerator<> program;
  program.setStart(program.setFilteredRule("B", "A+", [](auto tree) {
    REQUIRE_THAT(tree->string(), Catch::Matchers::Matches("a+"));
    return tree->inner.size() % 3 == 0;
  }));
  program.setRule("A", "'a'");
  auto N = GENERATE(range<size_t>(1, 10));
  REQUIRE(program.parse(std::string(N, 'a'))->valid == (N % 3 == 0));
}

TEST_CASE("Broken Grammar") {
  SECTION("Wrong type") {
    auto node = presets::GrammarNode::Range('1', '9');
    node->data = std::string("nope");
    auto rule = makeRule("Invalid", node);
    REQUIRE_THROWS_WITH(Parser::parse("1", rule), "corrupted grammar node");
  }
  SECTION("Illegal type") {
    auto node = presets::GrammarNode::Any();
    node->symbol = presets::GrammarNode::Symbol(-1);
    auto rule = makeRule("Invalid", node);
    REQUIRE_THROWS_WITH(Parser::parse("", rule), Catch::Matchers::Contains("UNKNOWN_SYMBOL"));
  }
  SECTION("Deleted Rule") {
    auto deletedRule = makeRule("deletedRule", presets::GrammarNode::Any());
    auto rule = makeRule("Rule", presets::GrammarNode::WeakRule(deletedRule));
    REQUIRE_NOTHROW(Parser::parse("x", rule));
    deletedRule.reset();
    REQUIRE_THROWS_AS(Parser::parse("x", rule), Parser::GrammarError);
    REQUIRE_THROWS_WITH(Parser::parse("x", rule), Catch::Matchers::Contains("<DeletedRule>"));
  }
}

TEST_CASE("Syntax Tree") {
  ParserGenerator<> program;
  program.setStart(program.setRule("B", "A+"));
  program.setRule("A", ".");
  auto tree = program.parse("abc");
  REQUIRE(stream_to_string(*tree) == "B(A('a'), A('b'), A('c'))");
}

TEST_CASE("C++ Operators") {
  ParserGenerator<std::string> program;

  program["B"] << "A+" << [](auto tree) { return tree->inner.size() % 3 == 0; } >> [](auto e) {
    std::string res;
    for (auto arg : e) {
      res += arg.evaluate();
    }
    return res;
  };

  program["A"] << "." >> [](auto e) { return std::string(1, e.view()[0] + 1); };

  program.setStart(program["B"]);

  REQUIRE_THROWS(program.run("ab"));
  REQUIRE(program.run("abc") == "bcd");
}

TEST_CASE("Parsing") {
  ParserGenerator<int> program;
  program.setStart(program["A"]);
  program["A"] << "B (' ' A) | B" >> [](auto e) {
    return std::accumulate(e.begin(), e.end(), 0, [](auto a, auto b) { return a + b.evaluate(); });
  };
  program["B"] << "&'b' . ''" >> [](auto) { return 1; };
  REQUIRE_THROWS(program.run("a"));
  REQUIRE(program.run("b") == 1);
  REQUIRE(program.run("b b") == 2);
  REQUIRE(program.run("b b b") == 3);
}

TEST_CASE("Documentation Example") {
  ParserGenerator<float> g;
  g.setSeparator(g["Whitespace"] << "[\t ]");
  g["Sum"] << "Add | Subtract | Product";
  g["Product"] << "Multiply | Divide | Atomic";
  g["Atomic"] << "Number | '(' Sum ')'";
  g["Add"] << "Sum '+' Product" >> [](auto e) { return e[0].evaluate() + e[1].evaluate(); };
  g["Subtract"] << "Sum '-' Product" >> [](auto e) { return e[0].evaluate() - e[1].evaluate(); };
  g["Multiply"] << "Product '*' Atomic" >> [](auto e) { return e[0].evaluate() * e[1].evaluate(); };
  g["Divide"] << "Product '/' Atomic" >> [](auto e) { return e[0].evaluate() / e[1].evaluate(); };
  g["Number"] << "'-'? [0-9]+ ('.' [0-9]+)?" >> [](auto e) { return stof(e.string()); };
  g.setStart(g["Sum"]);

  REQUIRE(g.run("42") == Approx(42));
  REQUIRE(g.run("2+3") == Approx(5));
  REQUIRE(g.run("2*3") == Approx(6));
  REQUIRE(g.run("1+2+3") == Approx(6));
  REQUIRE(g.run("1+2*3") == Approx(7));
  REQUIRE(g.run("1+2-3") == Approx(0));
  REQUIRE(g.run("2*2/4*3") == Approx(3));
  REQUIRE(g.run("1 - 2*3/2 + 4") == Approx(2));
  REQUIRE(g.run("1 + 2 * (3+4)/ 2 - 3") == Approx(5));
}
