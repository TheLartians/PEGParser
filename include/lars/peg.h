#pragma once

#include "interpreter.h"

namespace lars {
  
  namespace peg{
    Program<int> createIntegerProgram();
    Program<float> createFloatProgram();
    Program<double> createDoubleProgram();
    Program<int> createHexProgram();
    std::function<char(char)> defaultEscapeCodeCallback();
    Program<char> createCharacterProgram(const std::function<char(char)> escapeCodeCallback = defaultEscapeCodeCallback());
    Program<std::string> createStringProgram(const std::string &open, const std::string &close);
    Program<peg::GrammarNode::Shared> createGrammarProgram(const std::function<GrammarNode::Shared(const std::string_view &)> &getRule);
  }
  
}
