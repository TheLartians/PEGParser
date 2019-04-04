#include <lars/peg.h>
#include <string>
#include <lars/log.h>

using namespace lars;
using namespace lars::peg;
using GN = GrammarNode;

Program<int> peg::createIntegerProgram(){
  Program<int> program;
  auto pattern = GN::Sequence({GN::Optional(GN::Word("-")),GN::OneOrMore(GN::Range('0', '9'))});
  program.parser.grammar = program.interpreter.makeRule("Number", pattern, [](auto e){ return std::stoi(e.string()); });
  return program;
}

namespace {

  GrammarNode::Shared createFloatGrammar() {
    return GN::Sequence({
      GN::Optional(GN::Word("-")),
      GN::OneOrMore(GN::Range('0', '9')),
      GN::Optional(GN::Sequence({
        GN::Word("."),
        GN::OneOrMore(GN::Range('0', '9'))
      })),
      GN::Optional(GN::Sequence({
        GN::Choice({GN::Word("e"),GN::Word("E")}),
        GN::Optional(GN::Word("-")),
        GN::OneOrMore(GN::Range('0', '9'))
      }))
    });
  }

}

Program<float> peg::createFloatProgram(){
  Program<float> program;
  program.parser.grammar = program.interpreter.makeRule("Float", createFloatGrammar(), [](auto e){
    return std::stof(e.string());
  });
  return program;
}

Program<double> peg::createDoubleProgram(){
  Program<double> program;
  program.parser.grammar = program.interpreter.makeRule("Float", createFloatGrammar(), [](auto e){
    return std::stod(e.string());
  });
  return program;
}

Program<int> peg::createHexProgram(){
  Program<int> program;
  auto pattern = GN::Sequence({GN::OneOrMore(GN::Choice({GN::Range('0', '9'),GN::Range('a', 'f'),GN::Range('A', 'F')}))});
  program.parser.grammar = program.interpreter.makeRule("Hex", pattern, [](auto e){ return std::stoi(e.string(),0,16); });
  return program;
}

std::function<char(char)> peg::defaultEscapeCodeCallback(){
  std::unordered_map<char, char> codes{
    {'n','\n'},
    {'t','\t'},
    {'0','\0'}
  };
  return [codes](char c){
    auto it = codes.find(c);
    if (it != codes.end()){
      return it->second;
    } else {
      return c;
    }
  };
}

Program<char> peg::createCharacterProgram(const std::function<char(char)> escapeCodeCallback){
  Program<char> program;
  
  auto backslash = GN::Word("\\");

  auto escaped = GN::Rule(program.interpreter.makeRule("Escaped", GN::Sequence({backslash,GN::Any()}), [escapeCodeCallback](auto e){
    return escapeCodeCallback(e.view()[1]);
  }));
  
  auto numberParser = createHexProgram();
  auto escapedCode = GN::Rule(program.interpreter.makeRule("escapedCode", GN::Sequence({backslash,GN::Rule(numberParser.parser.grammar)}), [interpreter=numberParser.interpreter](auto e){
    return char(0 + e[0].evaluateBy(interpreter));
  }));

  auto character = GN::Rule(program.interpreter.makeRule("SingleCharacter", GN::Any(), [](auto e){
    return e.view()[0];
  }));
  
  program.parser.grammar = program.interpreter.makeRule("Character", GN::Choice({escapedCode, escaped, character}), [](auto e){ return e[0].evaluate(); });
  
  return program;
}

Program<std::string> peg::createStringProgram(const std::string &open, const std::string &close){
  Program<std::string> program;
  
  auto characterProgram = createCharacterProgram();
  
  auto pattern = GN::Sequence({
    GN::Word(open),
    GN::ZeroOrMore(GN::Sequence({GN::Not(GN::Word(close)),GN::Rule(characterProgram.parser.grammar)})),
    GN::Word(close)
  });
  
  program.parser.grammar = program.interpreter.makeRule("String", pattern, [interpreter=characterProgram.interpreter](auto e){
    std::string res;
    for (auto c: e){
      res += c.evaluateBy(interpreter);
    }
    return res;
  });
  
  return program;
}

Program<peg::GrammarNode::Shared> peg::createGrammarProgram(const std::function<GrammarNode::Shared(const std::string_view &)> &getRuleCallback){
  Program<GN::Shared> program;
  
  auto whitespaceRule = makeRule("Whitespace", GN::ZeroOrMore(GN::Choice({GN::Word(" "),GN::Word("\t")})));
  whitespaceRule->hidden = true;
  auto whitespace = GN::Rule(whitespaceRule);
  auto withWhitespace = [whitespace](GN::Shared node){ return GN::Sequence({whitespace, node, whitespace}); };

  auto stringProgram = createStringProgram("'", "'");
  
  auto expressionRule = program.interpreter.makeRule("Expression", GN::Empty(), [](auto e){ return e[0].evaluate(); });
  auto expression = GN::WeakRule(expressionRule);
  
  auto atomicRule = program.interpreter.makeRule("Atomic", GN::Empty(), [](auto e){ return e[0].evaluate(); });
  auto atomic = GN::WeakRule(atomicRule);

  auto endOfFile = GN::Rule(program.interpreter.makeRule("EndOfFile", GN::Word("<EOF>"), [](auto){
    return GN::EndOfFile();
  }));

  auto empty = GN::Rule(program.interpreter.makeRule("Empty", GN::Word("<>"), [](auto){
    return GN::Empty();
  }));
  
  auto any = GN::Rule(program.interpreter.makeRule("Any", GN::Word("."), [](auto){
    return GN::Any();
  }));
  
  auto selectCharacterProgram = createCharacterProgram();
  auto selectCharacter = GN::Sequence({GN::Not(GN::Choice({GN::Word("-"),GN::Word("]")})), GN::Rule(selectCharacterProgram.parser.grammar)});
  auto range = GN::Rule(program.interpreter.makeRule("Range", GN::Sequence({selectCharacter,GN::Word("-"),selectCharacter}), [interpreter=selectCharacterProgram.interpreter](auto e){
    return GN::Range(e[0].evaluateBy(interpreter), e[1].evaluateBy(interpreter));
  }));
  auto singeCharacter = GN::Rule(program.interpreter.makeRule("Character", selectCharacter, [interpreter=selectCharacterProgram.interpreter](auto e){
    return GN::Word(std::string(1,e[0].evaluateBy(interpreter)));
  }));
  auto selectSequence = GN::Sequence({GN::Word("["),GN::ZeroOrMore(GN::Choice({range, singeCharacter})),GN::Word("]")});
  auto select = GN::Rule(program.interpreter.makeRule("Select", selectSequence, [](auto e){
    if (e.size() == 1){ return e[0].evaluate(); }
    std::vector<GN::Shared> args;
    for (auto c:e) { args.push_back(c.evaluate()); }
    return GN::Choice(args);
  }));

  auto word = GN::Rule(program.interpreter.makeRule("Word", stringProgram.parser.grammar, [interpreter=stringProgram.interpreter](auto e){
    return GN::Word(e[0].evaluateBy(interpreter));
  }));

  auto ruleName = GN::Sequence({GN::Not(GN::Range('0', '9')),GN::OneOrMore(GN::Choice({GN::Range('a', 'z'),GN::Range('A', 'Z'),GN::Range('0', '9'),GN::Word("_")}))});
  auto rule = GN::Rule(program.interpreter.makeRule("Rule", ruleName, [getRuleCallback](auto e){
    return getRuleCallback(e.view());
  }));

  auto brackets = GN::Sequence({GN::Word("("), expression, GN::Word(")")});
  
  auto andPredicate = GN::Rule(program.interpreter.makeRule("AndPredicate", GN::Sequence({GN::Word("&"), atomic}), [](auto e){
    return GN::Also(e[0].evaluate());
  }));

  auto notPredicate = GN::Rule(program.interpreter.makeRule("NotPredicate", GN::Sequence({GN::Word("!"), atomic}), [](auto e){
    return GN::Not(e[0].evaluate());
  }));

  atomicRule->node = withWhitespace(GN::Choice({andPredicate, notPredicate, word, brackets, endOfFile, any, empty, select, rule}));

  auto predicate = GN::Rule(makeRule("Predicate", GN::Choice({GN::Word("+"),GN::Word("*"),GN::Word("?")})));

  auto unary = withWhitespace(GN::Rule(program.interpreter.makeRule("Unary", GN::Sequence({GN::Rule(atomicRule), GN::Optional(predicate)}), [](auto e){
    auto inner = e[0].evaluate();
    if (e.size() == 1) { return inner; }
    auto op = e[1].view()[0];
    if (op == '*') { return GN::ZeroOrMore(inner); }
    if (op == '+') { return GN::OneOrMore(inner); }
    if (op == '?') { return GN::Optional(inner); }
    throw std::runtime_error("unexpected unary operator");
  })));
  
  auto sequence = GN::Rule(program.interpreter.makeRule("Sequence", GN::Sequence({unary, GN::ZeroOrMore(unary)}), [](auto e){
    if (e.size() == 1) { return e[0].evaluate(); }
    std::vector<GN::Shared> args;
    for (auto c:e) { args.push_back(c.evaluate()); }
    return GN::Sequence(args);
  }));

  auto choice = GN::Rule(program.interpreter.makeRule("Choice", GN::Sequence({sequence, GN::ZeroOrMore(GN::Sequence({GN::Word("|"), sequence}))}), [](auto e){
    if (e.size() == 1) { return e[0].evaluate(); }
    std::vector<GN::Shared> args;
    for (auto c:e) { args.push_back(c.evaluate()); }
    return GN::Choice(args);
  }));
  
  expressionRule->node = withWhitespace(choice);
  
  auto fullExpression = program.interpreter.makeRule("FullExpression", GN::Sequence({GN::Rule(expressionRule), GN::EndOfFile()}), [](auto e){ return e[0].evaluate(); });
  program.parser.grammar = fullExpression;

  return program;
}
