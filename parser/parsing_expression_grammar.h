///////////////////////////////////////////////////////////
//
//    Created in 2014 by Lars Melchior
//
//    This file is distributed under the GNU
//    General Public License (GPL).
//    See LICENSE.TXT for details.
//

#pragma once

#include "expression.h"
#include "iterators.h"
#include "graph.h"

#include <unordered_map>
#include <string>
#include <assert.h>
#include <iostream>
#include <vector>
#include <type_traits>

namespace lars {
  
  
  class parsing_expression_grammar_symbol{
    char value;
  public:
    enum symbol_types:char{ word,any,range,sequence,choice,zerom,onem,opt,andp,notp,empty,gotorule,gotogrammar };
    parsing_expression_grammar_symbol(char v):value(v){}
    operator char()const{ return value; }
  };
  
    template <class visitor> class parsing_expression_grammar:public grammar<visitor>{
    public:
      
      using rule_evaluator = std::function<void(expression<visitor>)>;
      using rule_id = unsigned;
      
      using graph = lars::graph<parsing_expression_grammar_symbol> ;
      
      struct production_rule{
        typename graph::vertex_descriptor begin = graph::invalid_vertex_descriptor();
        rule_evaluator evaluator;
      };
      
    private:
      graph data;
      
      rule_id create_production_rule(const std::string &s){
        assert(rule_names.find(s) == rule_names.end());
        assert(production_rules.size() == rule_ids.size());
        rule_names[s]=rule_ids.size();
        rule_ids.push_back(s);
        
        production_rules.emplace_back();
        return rule_names[s];
      }
      
      void expose_symbol(const std::string &s){ if(rule_names.find(s)==rule_names.end()) create_production_rule(s); }
      
    public:
      
      struct production_rule_builder{
        parsing_expression_grammar *parent;
        rule_id rule;
        
        production_rule_builder(parsing_expression_grammar *p, rule_id r):parent(p),rule(r){}
        
        production_rule_builder & operator<<(const typename graph::vertex v){
          if(v.parent == &parent->data) parent->production_rules[rule].begin = v.id;
          else parent->production_rules[rule].begin = parent->data.copy_tree(v).id;
          return *this;
        }
        
        production_rule_builder & operator<<(const parsing_expression_grammar g){ return *this << parent->GoToGrammar(std::make_shared<parsing_expression_grammar>(g)); }
        production_rule_builder & operator<<(const std::shared_ptr<parsing_expression_grammar> g){ return *this << parent->GoToGrammar(g); }
        production_rule_builder & operator<<(const rule_evaluator & c){ parent->production_rules[rule].evaluator = c; return *this; }
        
      };
      
      std::unordered_map<std::string, rule_id> rule_names;
      std::vector<std::string> rule_ids;
      std::vector<production_rule> production_rules;
      
      std::vector<std::shared_ptr<parsing_expression_grammar>> embedded_grammars;
      
      std::string name;
      
      rule_id start_id = -1,ignored_id = -1, separator_id = -1;
      
      parsing_expression_grammar(const std::string n = ""):name(n){}
      parsing_expression_grammar(const parsing_expression_grammar & g) = delete;
      parsing_expression_grammar & operator=(const parsing_expression_grammar & g) = delete;
      
      void set_starting_rule(const std::string &str){
        start_id = rule_names[str];
      }
      
      void set_ignored_rule(const std::string &str){
        ignored_id = rule_names[str];
      }
      
      void set_separator_rule(const std::string &str){
        separator_id = rule_names[str];
      }
      
      production_rule_builder rule(const std::string &str){
        expose_symbol(str);
        return production_rule_builder(this,rule_names[str]);
      }
      
      rule_id operator[](const std::string &s){ expose_symbol(s); return rule_names.find(s)->second; }
      rule_id operator[](const std::string &s)const{ assert(rule_names.find(s)!=rule_names.end()); return rule_names.find(s)->second; }
      production_rule & operator[](rule_id i){ return production_rules[i]; }
      const production_rule & operator[](rule_id i)const{ return production_rules[i]; }

      const std::string &rule_name(rule_id i)const{ assert(rule_ids.size() > i); return rule_ids[i]; }
      
      void evaluate(expression<visitor> v)const{
        assert(production_rules[v.rule_id()].evaluator);
        production_rules[v.rule_id()].evaluator(v);
      }
      
      typename graph::const_vertex get_rule_vertex(rule_id r)const{
        assert(production_rules[r].begin != graph::invalid_vertex_descriptor());
        return data.get_vertex(production_rules[r].begin);
      }
      
      typename graph::vertex Word(const std::string &s){
        auto v = data.create_vertex(parsing_expression_grammar_symbol::word);
        for(char c:s)v.add_edge(data.create_vertex(c));
        return v;
      }
      
      typename graph::vertex Letter(char c){
        auto v = data.create_vertex(parsing_expression_grammar_symbol::word);
        v.add_edge(data.create_vertex(c));
        return v;
      }
      
      typename graph::vertex Any(){
        return data.create_vertex(parsing_expression_grammar_symbol::any);
      }
      
      typename graph::vertex Range(int b,int e){
        auto v = data.create_vertex(parsing_expression_grammar_symbol::range);
        v.add_edge(data.create_vertex(b));
        v.add_edge(data.create_vertex(e));
        return v;
      }
      
      typename graph::vertex Sequence(const std::vector<typename graph::vertex> &elements){
        auto v = data.create_vertex(parsing_expression_grammar_symbol::sequence);
        for(auto e:elements)v.add_edge(e);
        return v;
      }
      
      typename graph::vertex Choice(const std::vector<typename graph::vertex> &elements){
        auto v = data.create_vertex(parsing_expression_grammar_symbol::choice);
        for(auto e:elements)v.add_edge(e);
        return v;
      }
      
      typename graph::vertex ZeroOrMore(const typename graph::vertex &e){
        auto v = data.create_vertex(parsing_expression_grammar_symbol::zerom);
        v.add_edge(e);
        return v;
      }
      
      typename graph::vertex OneOrMore(const typename graph::vertex &e){
        auto v = data.create_vertex(parsing_expression_grammar_symbol::onem);
        v.add_edge(e);
        return v;
      }
      
      typename graph::vertex Optional(const typename graph::vertex &e){
        auto v = data.create_vertex(parsing_expression_grammar_symbol::opt);
        v.add_edge(e);
        return v;
      }
      
      typename graph::vertex Necessary(const typename graph::vertex &e){
        auto v = data.create_vertex(parsing_expression_grammar_symbol::andp);
        v.add_edge(e);
        return v;
      }
      
      typename graph::vertex And(const typename graph::vertex &e){
        auto v = data.create_vertex(parsing_expression_grammar_symbol::andp);
        v.add_edge(e);
        return v;
      }
      
      typename graph::vertex Not(const typename graph::vertex &e){
        auto v = data.create_vertex(parsing_expression_grammar_symbol::notp);
        v.add_edge(e);
        return v;
      }
      
      typename graph::vertex Empty(){
        return data.create_vertex(parsing_expression_grammar_symbol::empty);
      }
      
      typename graph::vertex GoToRule(const rule_id &e){
        auto v = data.create_vertex(parsing_expression_grammar_symbol::gotorule);
        v.add_edge(data.create_vertex(e));
        for(char c:rule_name(e))v.add_edge(data.create_vertex(c));
        return v;
      }
      
      typename graph::vertex GoToRule(const std::string &e){
        auto v = data.create_vertex(parsing_expression_grammar_symbol::gotorule);
        expose_symbol(e);
        assert(rule_name(rule_names[e]) == e);
        assert(rule_ids.size() >= rule_names[e]);
        v.add_edge(data.create_vertex(rule_names[e]));
        for(char c:e)v.add_edge(data.create_vertex(c));
        return v;
      }
      
      typename graph::vertex GoToGrammar(std::shared_ptr<parsing_expression_grammar<visitor>> g){
        auto v = data.create_vertex(parsing_expression_grammar_symbol::gotogrammar);
        v.add_edge(data.create_vertex(embedded_grammars.size()));
        embedded_grammars.emplace_back(g);
        for(char c:g->name)v.add_edge(data.create_vertex(c));
        return v;
      }
      
      class builder{
      protected:
        parsing_expression_grammar * parent;
        
      public:
        
        builder(parsing_expression_grammar *p):parent(p){}
        
        typename graph::vertex Word(const std::string &s){ return parent->Word(s); }
        typename graph::vertex Letter(char c){ return parent->Letter(c); }
        typename graph::vertex Any(){ return parent->Any(); }
        typename graph::vertex Range(int b,int e){ return parent->Range(b,e); }
        typename graph::vertex Sequence(const std::vector<typename graph::vertex> &elements){ return parent->Sequence(elements); }
        typename graph::vertex Choice(const std::vector<typename graph::vertex> &elements){ return parent->Choice(elements); }
        typename graph::vertex ZeroOrMore(const typename graph::vertex &e){ return parent->ZeroOrMore(e); }
        typename graph::vertex OneOrMore(const typename graph::vertex &e){ return parent->OneOrMore(e); }
        typename graph::vertex Optional(const typename graph::vertex &e){ return parent->Optional(e); }
        typename graph::vertex Necessary(const typename graph::vertex &e){ return parent->Necessary(e); }
        typename graph::vertex And(const typename graph::vertex &e){ return parent->And(e); }
        typename graph::vertex Not(const typename graph::vertex &e){ return parent->Not(e); }
        typename graph::vertex Empty(){ return parent->Empty(); }
        typename graph::vertex GoToRule(const rule_id &e){ return parent->GoToRule(e); }
        typename graph::vertex GoToRule(const std::string &e){ return parent->GoToRule(e); }
        typename graph::vertex GoToGrammar(std::shared_ptr<parsing_expression_grammar> g){ return parent->GoToGrammar(g); }
      };
      
      builder get_builder(){ return builder(this); }
      
  };
  
  inline std::ostream & operator<<(std::ostream &stream,const graph<parsing_expression_grammar_symbol>::const_vertex &s){
    
    switch (*s) {
        
      case parsing_expression_grammar_symbol::word:
        stream << "'";
        for (int i=0;i<s.size();++i) { if(*s[i]!='\0')stream << (char)(*s[i]); else stream << "\\0";  }
        stream << "'";
        break;
        
      case parsing_expression_grammar_symbol::any:
        stream << ".";
        break;
        
      case parsing_expression_grammar_symbol::range:
        stream << "[" << (char)(*s[0]) << "-" << (char)(*s[1]) << "]";
        break;
        
      case parsing_expression_grammar_symbol::sequence:
        for(int i=0;i<s.size();++i) {
          stream << s[i];
          if(i!=s.size()-1)stream << " ";
        }
        break;
        
      case parsing_expression_grammar_symbol::choice:
        stream << "(";
        for(int i=0;i<s.size();++i) {
          stream << s[i];
          if(i!=s.size()-1)stream << " | ";
        }
        stream << ")";
        break;
        
      case parsing_expression_grammar_symbol::zerom:
        stream << "(" << s[0] << ")*";
        break;
        
      case parsing_expression_grammar_symbol::onem:
        stream << "(" << s[0] << ")+";
        break;
        
      case parsing_expression_grammar_symbol::opt:
        stream << "(" << s[0] << ")?";
        break;
        
      case parsing_expression_grammar_symbol::andp:
        stream << "&" << s[0];
        break;
        
      case parsing_expression_grammar_symbol::notp:
        stream << "!" << s[0];
        break;
        
      case parsing_expression_grammar_symbol::empty:
        stream << "0";
        break;
        
      case parsing_expression_grammar_symbol::gotorule:
        for(int i=1;i<s.size();++i) stream << (char)*s[i];
        break;
        
      case parsing_expression_grammar_symbol::gotogrammar:
        stream << '{' ;
        for(int i=1;i<s.size();++i) stream << (char)*s[i];
        stream << '}' ;
        break;
        
      default:
        throw "Undefined Vertex";
        break;
        
    }
    
    return stream;
  }
  
  
  
  template <class I> std::ostream & operator<<(std::ostream &stream,const parsing_expression_grammar<I> &s){
    stream << "Grammar " << s.name << ":" << std::endl;
    for (int i=0; i<s.production_rules.size(); ++i) {
      stream << s.rule_name(i) << " <- " << std::flush;
      if(s.production_rules[i].begin != graph<parsing_expression_grammar_symbol>::invalid_vertex_descriptor()){
        typename  graph<parsing_expression_grammar_symbol>::const_vertex v = s.get_rule_vertex(i);
        stream << v;
      }
      else stream << "undefined.";
      stream << std::endl;
    }
    if(s.start_id == -1)stream << "No starting rule.";
    else stream << "start: " << s.rule_name(s.start_id) << std::endl;
    if(s.ignored_id != -1) stream << "Ignore: " << s.rule_name(s.ignored_id) << std::endl;
    if(s.separator_id != -1) stream << "Sepatate: " << s.rule_name(s.separator_id) << std::endl;
    for(auto eg:s.embedded_grammars)stream << std::endl << *eg;
    return stream;
  }
  
  class string_grammar_visitor{
    char ch;
    std::string str;
    
  public:
    void set_character(char c){ ch = c; }
    
    template <class V> void visit_string(expression<V> e){
      str.resize(e.size());
      for(auto i:range(e.size())){
        e[i].accept((V*)this);
        str[i] = ch;
      }
    }
    
    const std::string & get_string(){
      return str;
    }
    
    template <class V> const std::string & get_string(expression<V> e){
      e.accept((V*)this);
      return get_string();
    }
    
  };
  
  template <class I = string_grammar_visitor> std::shared_ptr<parsing_expression_grammar<I>> string_grammar(const std::string &begin, const std::string &end){
    
    std::shared_ptr<parsing_expression_grammar<I>> g = std::make_shared<parsing_expression_grammar<I>>("" + begin + end + "-string");
    using expression = expression<I>;
    
    auto b=g->get_builder();
    
    g->rule("Forbidden") << b.Choice({b.Word(begin),b.Word(end)});
    g->rule("Escaped")   << b.Sequence({b.Letter('\\'), b.Choice({b.OneOrMore(b.Range('0', '9')),b.Word(begin),b.Word(end),b.Any()})})
    << [begin,end](expression e){
      if(e.character(1)=='n') e.visitor().set_character('\n');
      else if(e.character(1)=='t') e.visitor().set_character('\t');
      else if(e.character(1)=='\\') e.visitor().set_character('\\');
      else if(e.character(1)=='r') e.visitor().set_character('\r');
      else if (e.character(1)==begin[0]) e.visitor().set_character(begin[0]);
      else if (e.character(1)==end[0]) e.visitor().set_character(end[0]);
      else if(strtol(e.string().substr(1,e.string().size()-1).c_str(),NULL,0)!=0) e.visitor().set_character('\0'+strtol(e.string().substr(1,e.string().size()-1).c_str(),NULL,0));
      else if(e.string()[1]=='0')e.visitor().set_character('\0');
      else throw "Unknown escape sequence";
    };
    g->rule("Letter") << b.Sequence({b.Not(b.GoToRule("Forbidden")),b.Any()}) <<  [](expression e){ e.visitor().set_character(e.character()); };
    g->rule("String") << b.Sequence({b.Word(begin),b.ZeroOrMore(b.Choice({b.GoToRule("Escaped"),b.GoToRule("Letter")})),b.Word(end)})
    << [](expression e){ e.visitor().visit_string(e); };
    
    g->set_starting_rule("String");
    
    return g;
  }
  
  template <class I> class parsing_expression_grammar_grammar_visitor:public parsing_expression_grammar<I>::builder,public string_grammar_visitor{
    using expression = lars::expression<parsing_expression_grammar_grammar_visitor<I>>;
    using vertex = graph<parsing_expression_grammar_symbol>::vertex;
    
    vertex current_vertex;
    
  public:
    
    parsing_expression_grammar<I> &grammar;
    
    parsing_expression_grammar_grammar_visitor(parsing_expression_grammar<I> &g):parsing_expression_grammar<I>::builder(&g),grammar(g){}
    
    void set_vertex(graph<parsing_expression_grammar_symbol>::vertex v){ current_vertex = v; }
    
    vertex get_vertex(){ return current_vertex; }
    
    vertex get_vertex(expression e){
      e.accept(this);
      return get_vertex();
    }
    
  };
  
  template <class I>  std::shared_ptr<parsing_expression_grammar<parsing_expression_grammar_grammar_visitor<I>>> parsing_expression_grammar_grammar(){
    
    using visitor = parsing_expression_grammar_grammar_visitor<I>;
    using grammar = parsing_expression_grammar<visitor>;
    using expression = expression<visitor>;
    using vertex = graph<parsing_expression_grammar_symbol>::vertex;
    
    std::shared_ptr<grammar> select = std::make_shared<grammar>("Select");
    auto b = select->get_builder();
    
    select->rule("Range") << b.Sequence({b.Not(b.Choice({b.Letter('\\'),b.Letter(']')})),b.Any(),b.Letter('-'),b.Not(b.Letter(']')),b.Any()})
    << [](expression e) {
      char a=e.character(0);
      char b=e.character(2);
      e.visitor().set_vertex(e.visitor().Range(a,b));
      if(a>=b) throw "Invalid range";
    };
    
    select->rule("Letter") << b.Choice({b.GoToRule("Escape") , b.Sequence({b.Not(b.Letter(']')),b.Any()})})
    << [](expression e) {
      if(e.size()==0) e.visitor().set_vertex(e.visitor().Letter(e.character()));
      else e.visitor().set_vertex(e.visitor().Letter(e[0].character(1)));
    };
    
    select->rule("Escape") << b.Sequence({b.Letter('\\'),b.Any()});
    
    select->rule("Select") << b.Sequence({b.Letter('['),b.OneOrMore(b.Choice({b.GoToRule("Range"),b.GoToRule("Letter")})),b.Letter(']')})
    << [](expression e) {
      if(e.size() == 1) e[0].accept();
      else {
        std::vector<vertex> elements(e.size());
        for(auto i:range(e.size())){ elements[i] = e.visitor().get_vertex(e[i]); }
        e.visitor().set_vertex(e.visitor().Choice(elements));
      }
    };
    
    select->set_starting_rule("Select");
    
    std::shared_ptr<grammar> peg = std::make_shared<grammar>("Parsing Expression Grammar Grammar");
    b = peg->get_builder();
    
    peg->rule("RuleName") << b.OneOrMore(b.Choice({b.Range('a', 'z'),b.Range('A', 'Z'),b.Range('1', '9'), b.Letter('_')}))
    << [](expression e) { e.visitor().set_vertex(e.visitor().GoToRule(e.string())); };
    
    peg->rule("Word") << b.Choice({b.GoToGrammar(string_grammar<visitor>("'","'")),b.GoToGrammar(string_grammar<visitor>("\"","\""))})
    << [](expression e) {  e.visitor().set_vertex(e.visitor().Word(e.visitor().get_string(e[0]))); };
    
    peg->rule("Select")   << select << [](expression e) { e[0].accept(); };
    peg->rule("Any")      << b.Letter('.') << [](expression e) { e.visitor().set_vertex(e.visitor().Any()); };
    peg->rule("Empty")    << b.Letter('0') << [](expression e) { e.visitor().set_vertex(e.visitor().Empty()); };
    
    peg->rule("Pred")     << b.Sequence({b.Choice({b.Letter('&'),b.Letter('!')}),b.GoToRule("Atomic")})
    << [](expression e) {
      if(e.character() == '&') e.visitor().set_vertex(e.visitor().And(e.visitor().get_vertex(e[0])));
      if(e.character() == '!') e.visitor().set_vertex(e.visitor().Not(e.visitor().get_vertex(e[0])));
    };
    
    peg->rule("Brackets") << b.Sequence({b.Letter('('),b.GoToRule("Choice"),b.Letter(')')}) << [](expression e) { e[0].accept(); };
    
    peg->rule("Atomic")   << b.Sequence({b.Choice({b.GoToRule("Empty"),b.GoToRule("Any"),b.GoToRule("Word"),b.GoToRule("Select"),b.GoToRule("Brackets"), b.GoToRule("Pred"),b.GoToRule("RuleName")}),b.Optional(b.Choice({b.Letter('+'),b.Choice({b.Letter('*'),b.Letter('?')})}))})
    << [](expression e) {
      auto v = e.visitor().get_vertex(e[0]);
      if     (e.intermediate(1) == "+") e.visitor().set_vertex(e.visitor().OneOrMore(v));
      else if(e.intermediate(1) == "*") e.visitor().set_vertex(e.visitor().ZeroOrMore(v));
      else if(e.intermediate(1) == "?") e.visitor().set_vertex(e.visitor().Optional(v));
      else e.visitor().set_vertex(v);
    };
    
    peg->rule("Sequence") << b.Sequence({b.GoToRule("Atomic"), b.ZeroOrMore(b.GoToRule("Atomic"))})
    << [](expression e) {
      if(e.size() == 1) e[0].accept();
      else {
        std::vector<vertex> elements(e.size());
        for(auto i:range(e.size())){ elements[i] = e.visitor().get_vertex(e[i]); }
        e.visitor().set_vertex(e.visitor().Sequence(elements));
      }
    };
    
    peg->rule("Choice")   << b.Sequence({b.GoToRule("Sequence"), b.ZeroOrMore(b.Sequence({b.Letter('|'), b.GoToRule("Sequence")}))})
    << [](expression e) {
      if(e.size() == 1) e[0].accept();
      else {
        std::vector<vertex> elements(e.size());
        for(auto i:range(e.size())){ elements[i] = e.visitor().get_vertex(e[i]); }
        e.visitor().set_vertex(e.visitor().Choice(elements));
      }
    };
    
    peg->rule("Rule")     << b.GoToRule("Choice") << [](expression e) { e[0].accept(); };
    
    peg->rule("Whitespace") << b.Choice({b.Letter(' '),b.Letter('\t')});
    
    peg->set_starting_rule("Rule");
    peg->set_separator_rule("Whitespace");
    
    return peg;
  }
    
}



