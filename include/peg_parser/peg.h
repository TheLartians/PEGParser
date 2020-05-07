#pragma once

#include "interpreter.h"

namespace peg_parser {

  namespace peg {
    Program<int> createIntegerProgram();
    Program<float> createFloatProgram();
    Program<double> createDoubleProgram();
    Program<int> createHexProgram();
    std::function<char(char)> defaultEscapeCodeCallback();
    Program<char> createCharacterProgram(const std::function<char(char)> escapeCodeCallback
                                         = defaultEscapeCodeCallback());
    Program<std::string> createStringProgram(const std::string &open, const std::string &close);

    using RuleGetter = const std::function<GrammarNode::Shared(const std::string_view &)> &;
    using GrammarProgram = Program<peg::GrammarNode::Shared, RuleGetter &>;
    GrammarProgram createGrammarProgram();
  }  // namespace peg

}  // namespace peg_parser
