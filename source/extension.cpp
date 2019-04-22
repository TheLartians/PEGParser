#include <lars/parser/extension.h>
#include <lars/parser/generator.h>
#include <stdexcept>
#include <lars/log.h>

std::shared_ptr<lars::Extension> lars::extensions::parser(){
  using namespace lars;

  using ParserGenerator = lars::ParserGenerator<Any,Any&>;
  using Expression = ParserGenerator::Expression;

  auto expressionExtension = std::make_shared<Extension>();
  expressionExtension->set_class<ParserGenerator::Expression>();
  
  expressionExtension->add_function("evaluate", [](Expression &e,Any &d){
    return e.evaluate(d);
  });
  
  expressionExtension->add_function("size", [](Expression &e)->unsigned{
    return e.size();
  });
  
  expressionExtension->add_function("get", [](Expression &e, unsigned i){ 
    if (i < e.size()) {
      return e[i];
    } else {
      throw std::runtime_error("invalid expression index");
    }
  });
  
  expressionExtension->add_function("string", [](Expression &e){
    return e.string();
  });
  
  expressionExtension->add_function("position", [](Expression &e)->unsigned{
    return e.position();
  });
  
  expressionExtension->add_function("length", [](Expression &e)->unsigned{
    return e.length();
  });

  auto parserGeneratorExtension = std::make_shared<Extension>();
  parserGeneratorExtension->set_class<ParserGenerator>();
  parserGeneratorExtension->add_function("create", [](){ return ParserGenerator(); });
  
  parserGeneratorExtension->add_function("run", [](ParserGenerator &g, const std::string &str, Any& arg){
    return g.run(str, arg);
  });
  
  parserGeneratorExtension->add_function("setRule",[](ParserGenerator &g, const std::string &name, const std::string &grammar){
    return g.setRule(name, grammar);
  });
  
  parserGeneratorExtension->add_function("setRuleWithCallback",[](ParserGenerator &g, const std::string &name, const std::string &grammar, AnyFunction callback){
    return g.setRule(name, grammar, [callback](auto e, Any v){
      return callback(e, v);
    });
  });
  
  parserGeneratorExtension->add_function("setStartRule", [](ParserGenerator &g, const std::string &name){
    g.setStart(g.getRule(name));
  });
  
  parserGeneratorExtension->add_function("setSeparatorRule", [](ParserGenerator &g, const std::string &name){
    g.setSeparator(g.getRule(name));
  });
  
  auto extension = std::make_shared<Extension>();
  extension->add_extension("Program", parserGeneratorExtension);
  extension->add_extension("Expression", expressionExtension);

  return extension;
}
