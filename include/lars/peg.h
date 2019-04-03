#pragma once

#include "interpreter.h"

namespace lars {
  
  namespace peg{
    Program<int> createIntegerParser();
    Program<int> createHexParser();
    std::function<char(char)> defaultEscapeCodeCallback();
    Program<char> createCharacterParser(const std::function<char(char)> escapeCodeCallback = defaultEscapeCodeCallback());
    Program<std::string> createStringParser(const std::string &open, const std::string &close);
    Program<peg::GrammarNode::Shared> createGrammarParser(const std::function<GrammarNode::Shared(const std::string_view &)> &getRule);
  }
  
}
