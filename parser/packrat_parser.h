///////////////////////////////////////////////////////////
//
//    Created in 2014 by Lars Melchior
//
//    This file is distributed under the GNU
//    General Public License (GPL).
//    See LICENSE.TXT for details.
//

#pragma once

#include "parsing_expression_grammar.h"
#include "hashers.h"

#include <stack>
#include <unordered_map>
#include <tuple>

namespace lars {
  
  template <class I> class packrat_parser{
    
    struct state;
    
    public:
    
    std::shared_ptr<parsing_expression_grammar<I>> grammar;

    packrat_parser(const std::shared_ptr<parsing_expression_grammar<I>> &g):grammar(g){}
    
    expression<I> parse(std::string str){
      expression<I> e;
      e.get_global_data()->parsed_string = str;
      e.get_global_data()->abort = false;
      
      state s(e.get_global_data(),&*grammar);
      e = expression<I>(s.parse_stack_top(), e.get_global_data());
      
      parse_ignored_and_separated(s);
      bool success = parse(grammar->get_rule_vertex(grammar->start_id),s);
      parse_ignored_and_separated(s);

      if(!success || !(s.current_position.location==str.size())) throw "syntax error";
      
      return expression<I>(s.parse_stack_top(), e.get_global_data());
    }
    
    private:

    struct state{
      
      parser_position current_position,maximum_position,maximum_error_position;
      typename parse_tree::vertex_descriptor maximum_error_vertex;
      
      unsigned offset = 0;
      
      typedef std::tuple<uintptr_t,const parsing_expression_grammar<I>*, typename parsing_expression_grammar<I>::rule_id> matrix_key;
      
      std::stack<typename parse_tree::vertex_descriptor*> parse_stack;
      std::unordered_map<matrix_key, typename parse_tree::vertex_descriptor> parse_matrix;

      const parsing_expression_grammar<I> * current_grammar , *max_grammar;
      std::shared_ptr<expression_data> data;
      
      typename parsing_expression_grammar<I>::rule_id max_rule;
      
      bool parsing_ignored = false,parsing_separator = false;
      
      state(std::shared_ptr<expression_data> d,const parsing_expression_grammar<I> *g):current_grammar(g),data(d)
      {
        current_position.character = 0;
        current_position.location = 0;
        current_position.line = 0;
        enter_rule(g->start_id);
      }
      
      typename parse_tree::vertex parse_stack_top(){
        return data->tree.get_vertex(*parse_stack.top());
      }

      char current_token(){
        if(current_position.location>=data->parsed_string.size()) return '\0';
        return data->parsed_string[current_position.location];
      }
      
      void next_token(){
        //std::cout << "Parsed " << current_token() << " in " << current_grammar->rule_name(parse_stack_top().content().rule_id) << std::endl;
        ++current_position.location;
        if(current_token()!='\n')current_position.character++;
        else { current_position.character=1; current_position.line++; }
        if(current_position.location>maximum_position.location){
          maximum_position=current_position;
          max_grammar = (parsing_expression_grammar<I>*)(parse_stack_top().content().grammar);
          max_rule=parse_stack_top().content().rule_id;
        }
      }
      
      
      void set_position(parser_position pos){
        if(current_position.location>=pos.location-offset){
          typename parse_tree::vertex n=data->tree.get_vertex( *parse_stack.top() );
          int i; for (i=0; i<n.size(); ++i) { if (n.target(i).content().end.location>pos.location) { break; }  }
          int j; for (j=0; j<n.content().non_ignored_edges->size(); ++j) { if((*n.content().non_ignored_edges)[j]>=i) break; }
          n.resize(i);
          n.content().non_ignored_edges->resize(j);
          
          if( parse_stack.size()>0 && current_position.location>=maximum_error_position.location){
            typename parse_tree::vertex n=data->tree.get_vertex( *parse_stack.top() );
            if(true || n.content().begin.location<n.content().end.location){
              maximum_error_position=current_position;
              maximum_error_vertex=n.id;
            }
          }
          
        }
        current_position=pos;
      }
      
      typename parse_tree::vertex create_vertex(const parsing_expression_grammar<I> *g, typename parsing_expression_grammar<I>::rule_id r){
        typename parse_tree::vertex v = data->tree.create_vertex();
        v.content().begin = current_position;
        v.content().rule_id = r;
        v.content().grammar = g;
        v.content().non_ignored_edges.reset(new std::vector<uintptr_t>());
        return v;
      }
      
      void add_vertex(typename parse_tree::vertex_descriptor n){
        if(!parsing_ignored && !parsing_separator){ parse_stack_top().content().non_ignored_edges->push_back(parse_stack_top().size()); }
        parse_stack_top().add_edge(n);
      }
      
      int enter_rule(typename parsing_expression_grammar<I>::rule_id r){ // 1: remembered, 0: reconsidered, -1: unknown
        auto key = matrix_key(current_position.location,current_grammar,r);

        if(parse_matrix.find(key)!=parse_matrix.end()){
          typename parse_tree::vertex_descriptor n=parse_matrix.find(matrix_key(current_position.location,current_grammar,r))->second;
          if(n != parse_tree::invalid_vertex().id){
            add_vertex(n);
            set_position(data->tree.get_content(n).end);
            return 1;
          }
          return 0;
        }
        
        parse_matrix[key] = create_vertex(current_grammar,r).id;
        parse_stack.push(&parse_matrix[key]);
        
        return -1;
      }
      
      bool exitRule(bool s){
        typename parse_tree::vertex n = parse_stack_top();
        
        if(s==false){
          *parse_stack.top()=data->tree.invalid_vertex().id;
          parse_stack.pop();
        }
        else{
          n.content().end=current_position;
          parse_stack.pop();
          if(parse_stack.size()>0)add_vertex(n.id);
        }
        
        return s;
      }
      
      bool aborted(){ return data->abort; }

    };
    
    
    static void parse_ignored(state &s){
      const parsing_expression_grammar<I> &grammar = *s.current_grammar;
      if(grammar.ignored_id == -1)return;
      s.parsing_ignored=true;
      if(s.enter_rule(grammar.ignored_id)==-1){
        while (parse(grammar.get_rule_vertex(grammar.ignored_id),s));
        s.exitRule(true);
      }
      s.parsing_ignored=false;
    }
    
    static void parse_separated(state &s){
      const parsing_expression_grammar<I> &grammar = *s.current_grammar;
      if(grammar.separator_id == -1)return;
      s.parsing_separator=true;
      if(s.enter_rule(grammar.separator_id) == -1){
        while (parse(grammar.get_rule_vertex(grammar.separator_id),s));
        s.exitRule(true);
      }
      s.parsing_separator=false;
    }
    
    static void parse_ignored_and_separated(state &s){
      int pos=s.current_position.location,lastpos=-1;
      while (pos!=lastpos) {
        lastpos=pos;
        parse_separated(s);
        parse_ignored(s);
        pos=s.current_position.location;
      }
    }

    
    static bool parse(typename parsing_expression_grammar<I>::graph::const_vertex n,state &s){
      if(s.aborted())return false;
      
      const parsing_expression_grammar<I> &grammar=*s.current_grammar;
      
      //std::cout << "[" << s.current_position.location << "," << (char)s.current_token() << "] Parsing rule : " << n << std::endl;
      
      switch (*n) {
        case parsing_expression_grammar_symbol::word:
          if(n.size()==1){ if(s.current_token()==*n[0]){ s.next_token(); if(!s.parsing_ignored)parse_ignored(s); return true; } }
          else{
            parser_position pos=s.current_position;
            for (int i=0; i<n.size(); ++i) {
              if(s.current_token()!=*n[i]){
                s.set_position(pos);
                return false;
              }
              s.next_token();
              if(!s.parsing_ignored)parse_ignored(s);
            }
            return true;
          }
          break;
          
        case parsing_expression_grammar_symbol::any:
          if(!s.parsing_ignored)parse_ignored(s);
          if(s.current_token()!='\0'){
            s.next_token();
            if(!s.parsing_ignored)parse_ignored(s);
            return true;
          }
          break;
          
        case parsing_expression_grammar_symbol::empty:
          return true;
          break;
          
        case parsing_expression_grammar_symbol::range:
          if (s.current_token()>=*n[0] && s.current_token()<=*n[1]){ s.next_token(); if(!s.parsing_ignored)parse_ignored(s); return true; };
          break;
          
        case parsing_expression_grammar_symbol::sequence:{
          parser_position pos=s.current_position;
          for(int i=0;i<n.size();++i){
            if(!parse(n[i], s)){
              s.set_position(pos);
              return false;
            }
          }
          return true;
        }break;
          
        case parsing_expression_grammar_symbol::choice:
          for(int i=0;i<n.size();++i){ if(parse(n[i], s)) return true; }
          break;
          
        case parsing_expression_grammar_symbol::zerom:
          while(parse(n[0], s)){ }
          return true;
          break;
          
        case parsing_expression_grammar_symbol::onem:
          if(parse(n[0], s)){ while(parse(n[0], s)){ } return true; }
          break;
          
        case parsing_expression_grammar_symbol::opt:
          parse(n[0], s);
          return true;
          break;
          
        case parsing_expression_grammar_symbol::andp:{
          parser_position pos=s.current_position;
          bool p=parse(n[0], s);
          s.set_position(pos);
          return p;
        }break;
          
        case parsing_expression_grammar_symbol::notp:{
          parser_position pos=s.current_position;
          bool p=parse(n[0], s);
          s.set_position(pos);
          return !p;
        }break;
          
        case parsing_expression_grammar_symbol::gotorule:{
          if(!s.parsing_separator && !s.parsing_ignored)parse_separated(s);
          int e=s.enter_rule(*n[0]);
          
          if (e==0) { return false; }
          if (e==1) { if(!s.parsing_separator && !s.parsing_ignored)parse_separated(s); return true; }
          
          bool status = parse(grammar.get_rule_vertex(*n[0]),s);
          
          status=s.exitRule(status);
          if(status==true)if(!s.parsing_separator && !s.parsing_ignored)parse_separated(s);
          
          return status;
        }break;
          
        case parsing_expression_grammar_symbol::gotogrammar:{
          const parsing_expression_grammar<I> *p=s.current_grammar;
          
          s.current_grammar=&*grammar.embedded_grammars[*n[0]];
          
          if(!s.parsing_separator && !s.parsing_ignored)parse_separated(s);
          
          int e=s.enter_rule((*grammar.embedded_grammars[*n[0]]).start_id);
          
          if (e==0) {
            s.current_grammar=(const parsing_expression_grammar<I> *)p;
            return false;
          }
          if (e==1) {
            s.current_grammar=(const parsing_expression_grammar<I> *)p;
            if(!s.parsing_separator && !s.parsing_ignored)parse_separated(s);
            return true;
          }
          
          bool stat=parse(  s.current_grammar->get_rule_vertex(s.current_grammar->start_id), s );
          
          stat=s.exitRule(stat);
          if(stat==true)if(!s.parsing_separator && !s.parsing_ignored)parse_separated(s);
          
          s.current_grammar = p;
          return stat;
        }break;
          
        default:
          throw "Invalid symbol";
          break;
      }
      
      return false;
      
    }
    
  };
  
  template <class Visitor> packrat_parser<Visitor> create_packrat_parser(parsing_expression_grammar<Visitor> g){
    return packrat_parser<Visitor>(std::make_shared<parsing_expression_grammar<Visitor>>(g));
  }
  
  template <class Visitor> packrat_parser<Visitor> create_packrat_parser(std::shared_ptr<parsing_expression_grammar<Visitor>> g){
    return packrat_parser<Visitor>(g);
  }
  
  template <class Visitor> class parsing_expression_grammar_builder:public parsing_expression_grammar<Visitor>::builder{
    std::shared_ptr<parsing_expression_grammar<Visitor>> grammar;
    packrat_parser<parsing_expression_grammar_grammar_visitor<Visitor>> parser;
    
  public:
    
    struct production_rule_builder:public parsing_expression_grammar<Visitor>::production_rule_builder{
      parsing_expression_grammar_builder <Visitor> * pegb_parent;
      
      using rule_builder = typename parsing_expression_grammar<Visitor>::production_rule_builder;
      using rule_evaluator = typename parsing_expression_grammar<Visitor>::rule_evaluator;
      
      production_rule_builder(parsing_expression_grammar_builder <Visitor> *p,const std::string &rule):parsing_expression_grammar<Visitor>::production_rule_builder(p->grammar->rule(rule)),pegb_parent(p){}
      
      rule_builder & operator<<(const std::string &str){
        return rule_builder::operator<<(pegb_parent->parser.parse(str).evaluate(*this->parent)->get_vertex());
      }
      
      rule_builder & operator<<(const rule_evaluator & c){ return rule_builder::operator<<(c); }
      
    };
    
    parsing_expression_grammar_builder(const std::string &name=""):parsing_expression_grammar<Visitor>::builder(new parsing_expression_grammar<Visitor>(name)),grammar(parsing_expression_grammar<Visitor>::builder::parent),parser(create_packrat_parser(parsing_expression_grammar_grammar<Visitor>())){
      
    }
    
    void set_starting_rule(const std::string &name){ grammar->set_starting_rule(name); }
    void set_ignored_rule(const std::string &name){ grammar->set_ignored_rule(name); }
    void set_separator_rule(const std::string &name){ grammar->set_separator_rule(name); }
    
    production_rule_builder operator[](const std::string &str){
      return production_rule_builder(this,str);
    }
    
    std::shared_ptr<parsing_expression_grammar<Visitor>> get_grammar(){
      return grammar;
    }
    
    packrat_parser<Visitor> get_parser(){
      return create_packrat_parser(grammar);
    }
    
  };
    
}
