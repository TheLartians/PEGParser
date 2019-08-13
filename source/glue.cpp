#include <lars/parser/glue.h>
#include <lars/parser/generator.h>
#include <glue/class_element.h>

#include <stdexcept>
#include <lars/log.h>

::glue::Element lars::parser::glue(){
  using namespace lars;
  using namespace ::glue;

  using Program = lars::ParserGenerator<Any,const Any &>;
  using Expression = Program::Expression;

  Element parser;

  parser["Expression"] = glue::ClassElement<Expression>()
  .addFunction("evaluate", [](Expression &e,const Any &d){ return e.evaluate(d); })
  .addFunction("size", [](Expression &e){ return e.size(); })
  .addFunction("string", [](Expression &e){ return e.string(); })
  .addFunction("position", [](Expression &e){ return e.position(); })
  .addFunction("length", [](Expression &e){ return e.length(); })
  .addFunction("get", [](Expression &e, unsigned i){
    if (i < e.size()) {
      return e[i];
    } else {
      throw std::runtime_error("invalid expression index");
    }
  })
  ;
  
  parser["Program"] = glue::ClassElement<Program>()
  .addConstructor<>()
  .addFunction("run", [](Program &g, const std::string &str, const Any& arg){
    return g.run(str, arg);
  })
  .addFunction("setRule",[](Program &g, const std::string &name, const std::string &grammar){
    g.setRule(name, grammar);
  })
  .addFunction("setRuleWithCallback",[](Program &g, const std::string &name, const std::string &grammar, AnyFunction callback){
    g.setRule(name, grammar, [callback](auto e, const Any &v)->Any{
      return callback(e, v);
    });
  })
  .addFunction("setStartRule",[](Program &g, const std::string &name){
    g.setStart(g.getRule(name));
  })
  .addFunction("setSeparatorRule",[](Program &g, const std::string &name){
    g.setSeparator(g.getRule(name));
  })
  ;
    
  return parser;
}
