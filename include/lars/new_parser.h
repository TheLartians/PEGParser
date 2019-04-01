#pragma once

#include <lars/grammar.h>

namespace lars {

  class Parser{
  private:
    std::shared_ptr<peg::Grammar> grammar;
    
  public:
    
    Parser(const std::shared_ptr<peg::Grammar> &grammar);
    void parse(const std::string &str);
  };
  
}
