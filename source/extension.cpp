#include <lars/parser/extension.h>
#include <lars/parser/generator.h>
#include <stdexcept>
#include <lars/log.h>

::glue::Element lars::glue::parser(){
  using namespace lars;
  using namespace ::glue;

  using ParserGenerator = lars::ParserGenerator<Any,const Any &>;
  using Expression = ParserGenerator::Expression;

  Element expression;
  setClass<Expression>(expression);

  expression["evaluate"] = [](Expression &e,const Any &d){
    return e.evaluate(d);
  };
  
  expression["size"] = [](Expression &e)->unsigned{
    return e.size();
  };
  
  expression["get"] = [](Expression &e, unsigned i){
    if (i < e.size()) {
      return e[i];
    } else {
      throw std::runtime_error("invalid expression index");
    }
  };

  expression["string"] = [](Expression &e){
    return e.string();
  };
  
  expression["position"] = [](Expression &e)->unsigned{
    return e.position();
  };
  
  expression["length"] = [](Expression &e)->unsigned{
    return e.length();
  };
  

  Element program;
  setClass<ParserGenerator>(program);
  
  program["create"] = [](){ return ParserGenerator(); };
  
  program["run"] = [](ParserGenerator &g, const std::string &str, const Any& arg){
    return g.run(str, arg);
  };
  
  program["setRule"] = [](ParserGenerator &g, const std::string &name, const std::string &grammar){
    return g.setRule(name, grammar);
  };
  
  program["setRuleWithCallback"] = [](ParserGenerator &g, const std::string &name, const std::string &grammar, AnyFunction callback){
    return g.setRule(name, grammar, [callback](auto e, const Any &v)->Any{
      return callback(e, v);
    });
  };
  
  program["setStartRule"] = [](ParserGenerator &g, const std::string &name){
    g.setStart(g.getRule(name));
  };
  
  program["setSeparatorRule"] = [](ParserGenerator &g, const std::string &name){
    g.setSeparator(g.getRule(name));
  };
  
  Element parser;
  parser["Program"] = program;
  parser["Expression"] = expression;
  
  return parser;
}
