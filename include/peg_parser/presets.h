#pragma once

#include "interpreter.h"

namespace peg_parser {

  namespace presets {
    Program<int> createIntegerProgram();
    Program<float> createFloatProgram();
    Program<double> createDoubleProgram();
    Program<int> createHexProgram();
    std::function<char(char)> defaultEscapeCodeCallback();
    Program<char> createCharacterProgram(const std::function<char(char)> escapeCodeCallback
                                         = defaultEscapeCodeCallback());
    Program<std::string> createStringProgram(const std::string &open, const std::string &close);

    using RuleGetter = const std::function<grammar::Node::Shared(const std::string_view &)> &;
    using GrammarProgram = Program<grammar::Node::Shared, RuleGetter &>;
    GrammarProgram createPEGProgram();
  }  // namespace presets

}  // namespace peg_parser
