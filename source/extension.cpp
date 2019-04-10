#include <lars/parser/extension.h>
#include <lars/parser/generator.h>
#include <stdexcept>

std::shared_ptr<lars::Extension> lars::get_parser_extension(){
  using namespace lars;

  using ParserGenerator = lars::ParserGenerator<Any>;
  using Expression = ParserGenerator::Expression;

  auto expressionExtension = std::make_shared<Extension>();
  expressionExtension->set_class<ParserGenerator::Expression>();
  expressionExtension->add_function("evaluate", [](Expression &e){ return e.evaluate(); });
  expressionExtension->add_function("size", [](Expression &e)->unsigned{ return e.size(); });
  expressionExtension->add_function("get", [](Expression &e, unsigned i){ 
    if (i < e.size()) {
      return e[i];
    } else {
      throw std::runtime_error("invalid expression index");
    }
  });

  auto parserGeneratorExtension = std::make_shared<Extension>();
  parserGeneratorExtension->set_class<ParserGenerator>();
  parserGeneratorExtension->add_function("create", [](){ return ParserGenerator(); });
  parserGeneratorExtension->add_function("run", [](ParserGenerator &g, const std::string &str){ return g.run(str); });
  parserGeneratorExtension->add_function("setRule",[](ParserGenerator &g, const std::string &name, const std::string &grammar){
    return g.setRule(name, grammar);
  });
  parserGeneratorExtension->add_function("setRuleWithCallback",[](ParserGenerator &g, const std::string &name, const std::string &grammar, AnyFunction callback){
    return g.setRule(name, grammar, callback);
  });
  parserGeneratorExtension->add_function("setStartRule", [](ParserGenerator &g, const std::string &name){
    g.setStart(g.getRule(name));
  });
  parserGeneratorExtension->add_function("setSeparatorRule", [](ParserGenerator &g, const std::string &name){
    g.setSeparator(g.getRule(name));
  });
  
  auto extension = std::make_shared<Extension>();
  extension->add_extension("ParserGenerator", parserGeneratorExtension);
  extension->add_extension("Expression", expressionExtension);

  return extension;
}
