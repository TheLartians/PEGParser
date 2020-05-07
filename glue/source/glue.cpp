#include <glue/class.h>
#include <peg_parser/generator.h>
#include <peg_parser/glue.h>

#include <stdexcept>

glue::MapValue peg_parser::glue() {
  using Any = glue::Any;
  using AnyFunction = glue::AnyFunction;
  using Program = peg_parser::ParserGenerator<Any, const Any &>;
  using Expression = Program::Expression;

  auto parser = glue::createAnyMap();

  parser["Expression"]
      = glue::createClass<Expression>()
            .addMethod("evaluate", [](Expression &e, const Any &d) { return e.evaluate(d); })
            .addMethod("size", [](Expression &e) { return e.size(); })
            .addMethod("string", [](Expression &e) { return e.string(); })
            .addMethod("position", [](Expression &e) { return e.position(); })
            .addMethod("length", [](Expression &e) { return e.length(); })
            .addMethod("get", [](Expression &e, unsigned i) {
              if (i < e.size()) {
                return e[i];
              } else {
                throw std::runtime_error("invalid expression index");
              }
            });

  parser["Program"]
      = glue::createClass<Program>()
            .addConstructor<>()
            .addMethod("run", [](Program &g, const std::string &str,
                                 const Any &arg) { return g.run(str, arg); })
            .addMethod("setRule", [](Program &g, const std::string &name,
                                     const std::string &grammar) { g.setRule(name, grammar); })
            .addMethod("setRuleWithCallback",
                       [](Program &g, const std::string &name, const std::string &grammar,
                          AnyFunction callback) {
                         g.setRule(name, grammar, [callback](auto e, const Any &v) -> Any {
                           return callback(e, v);
                         });
                       })
            .addMethod("setStartRule",
                       [](Program &g, const std::string &name) { g.setStart(g.getRule(name)); })
            .addMethod("setSeparatorRule", [](Program &g, const std::string &name) {
              g.setSeparator(g.getRule(name));
            });

  return parser;
}
