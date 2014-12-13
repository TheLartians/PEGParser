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
#include <sstream>

#include <vector>
#include <unordered_map>
#include <tuple>

namespace lars {
  
  template <class I> class parser{
    
    struct state;
    I * default_visitor = nullptr;
    
  public:
    
    using error = typename expression<I>::error;
    
    std::shared_ptr<parsing_expression_grammar<I>> grammar;
    
    parser(const std::shared_ptr<parsing_expression_grammar<I>> &g):grammar(g){}
    
    expression<I> parse(std::string str){
      expression<I> e(default_visitor);
      e.get_global_data()->parsed_string = str;
      e.get_global_data()->abort = false;
      
      state s(e.get_global_data(),&*grammar,default_visitor);

      bool success = parse(grammar->get_start_vertex(),s);
      if(!success || !(s.current_position.location==str.size())) s.throw_error("Syntax error");
      
      return expression<I>(s.get_start_vertex(), e.get_global_data());
    }
    
    void set_visitor(I * v){
      default_visitor = v;
    }
    
  private:
    
    struct state{
      
      parser_position current_position,maximum_position;
      
      typename parse_tree::vertex_descriptor maximum_error_vertex;
      
      unsigned offset = 0;
      
      typedef std::tuple<unsigned,const grammar_base *, typename parsing_expression_grammar<I>::rule_id> matrix_key;
      
      std::vector<typename parse_tree::vertex_descriptor*> parse_stack;
      std::unordered_map<matrix_key, typename parse_tree::vertex_descriptor> parse_matrix;
      
      const parsing_expression_grammar<I> * current_grammar;
      std::shared_ptr<expression_data> data;
      
      bool parsing_ignored = false,parsing_separator = false;
      
      I * default_visitor = nullptr;

      state(std::shared_ptr<expression_data> d,const parsing_expression_grammar<I> *g,I *v):current_grammar(g),data(d),default_visitor(v){
        current_position.character = 0;
        current_position.location = 0;
        current_position.line = 0;
        maximum_position = current_position;
        maximum_error_vertex = parse_tree::invalid_vertex_descriptor();
      }
      
      typename parse_tree::vertex get_start_vertex(){
        for(auto i UNUSED :range(data->parsed_string.size())){
          auto key = matrix_key(i,current_grammar,current_grammar->start_id);
          if(parse_matrix.find(key) != parse_matrix.end()) return data->tree.get_vertex(parse_matrix[key]);
        }
        throw "No starting rule parsed!";
      }
      
      typename parse_tree::vertex parse_stack_top(){
        assert(parse_stack.size()>0);
        return data->tree.get_vertex(*parse_stack.back());
      }
      
      void throw_error(const std::string &str){
        data->tree.get_content(maximum_error_vertex).end = maximum_position;
        expression<I> e(data->tree.get_vertex(maximum_error_vertex),data,default_visitor);
        e.throw_error(str,error::parsing_error);
      }
      
      char current_token(){
        if(current_position.location>=data->parsed_string.size()) return '\0';
        return data->parsed_string[current_position.location];
      }
      
      void next_token(){
        ++current_position.location;
        if(current_token()!='\n')current_position.character++;
        else { current_position.character=1; current_position.line++; }
        if(current_position.location>maximum_position.location){ maximum_position=current_position; }
      }
      
      void set_position(parser_position pos){
        if(parse_stack.size()>0 && current_position.location>=pos.location-offset){
          if(current_position.location == maximum_position.location) maximum_error_vertex = *parse_stack.back();
          typename parse_tree::vertex n=data->tree.get_vertex( *parse_stack.back() );
          int i; for (i=0; i<n.size(); ++i) { if (n.target(i).content().end.location>pos.location) { break; }  }
          n.resize(i);
        }//*/
        current_position=pos;
      }
      
      typename parse_tree::vertex create_vertex(const parsing_expression_grammar<I> *g, typename parsing_expression_grammar<I>::rule_id r){
        typename parse_tree::vertex v = data->tree.create_vertex();
        v.content().begin = current_position;
        v.content().end.location = -1;
        v.content().rule_id = r;
        v.content().state = 0;
        v.content().grammar = g;
        return v;
      }
      
      void add_vertex(typename parse_tree::vertex_descriptor n){
        if(parse_stack.size()>0 && !parsing_ignored && !parsing_separator) parse_stack_top().add_edge(n);
      }
      
      int inside_left_recursion = 0;
      
      void enter_left_recursion(typename parsing_expression_grammar<I>::rule_id r,parse_tree::vertex_descriptor &v){
        inside_left_recursion++;
        
        assert(parse_matrix.find(matrix_key(current_position.location,current_grammar,r)) != parse_matrix.end());
      
        std::unordered_set<matrix_key> keys_to_keep;
        keys_to_keep.insert(matrix_key(current_position.location,current_grammar,r));
        for(auto i : parse_stack)if(data->tree.get_content(*i).begin.location == current_position.location){
          keys_to_keep.insert(matrix_key(current_position.location,current_grammar,data->tree.get_content(*i).rule_id));
        }

        for(auto i:current_grammar->rule_names){
          auto key = matrix_key(current_position.location,current_grammar,i.second);
          if(keys_to_keep.find(key)!=keys_to_keep.end())continue;
          if(parse_matrix.find(key) != parse_matrix.end()){
            parse_matrix.erase(key);
          }
        }
        
        auto ver =create_vertex(current_grammar,r);
        v = ver.id;
        parse_stack.push_back(&v);
      }
      
      bool exit_left_recursion(bool s){
        inside_left_recursion--;
        
        auto n = parse_stack_top();
        parse_stack.pop_back();
        
        if(s){
          auto key = matrix_key(n.content().begin.location,current_grammar,n.content().rule_id);
          assert(parse_matrix.find(key) != parse_matrix.end());
          if(data->tree.get_vertex(parse_matrix[key]).content().end.location >= current_position.location){
            assert(data->tree.get_vertex(parse_matrix[key]).content().end.location != -1);
            return false;
          }
          
          n.content().end=current_position;
          n.content().state = 1;
          parse_matrix[key] = n.id;
        }
        
        return s;
      }
      
      void print_debug_parse_stack(){
        for(auto it = parse_stack.rbegin(),end = parse_stack.rend();it!=end;++it){
          auto v = data->tree.get_vertex(**it);
        }
      }
      
      int enter_rule(typename parsing_expression_grammar<I>::rule_id r){ // 1: remembered, 0: rejected, -1: unknown
        auto key = matrix_key(current_position.location,current_grammar,r);
        
        if(parse_matrix.find(key)!=parse_matrix.end()) {
          
          auto n=parse_matrix[key];
          
          if(n != parse_tree::invalid_vertex().id){
            if(data->tree.get_content(n).end.location == -1) {
              //throw "left recursion";
              data->tree.get_content(n).state = 1; // left-recursive
              return 0;
            }
            
            add_vertex(n);
            set_position(data->tree.get_content(n).end);
            return 1;
          }
          
          if(inside_left_recursion == 0){
            return 0;
          }
        }
        
        parse_matrix[key] = create_vertex(current_grammar,r).id;
        parse_stack.push_back(&parse_matrix[key]);
        
        return -1;
      }
      
      bool filter_rule(){
        auto  & f = (*current_grammar)[parse_stack_top().content().rule_id].filter;
        if(f)return f(expression<I>(parse_stack_top(),data,default_visitor));
        return true;
      }
      
      bool exit_rule(bool s){
        typename parse_tree::vertex n = parse_stack_top();
        
        if(s==false){
          *parse_stack.back()=data->tree.invalid_vertex().id;
          parse_stack.pop_back();
        }
        else{
          n.content().end=current_position;
          parse_stack.pop_back();
          if(parse_stack.size()>0 && n.content().state != 1){
            add_vertex(n.id);
          }
        }
        
        if(s && n.content().state == 1){
          return true;
        }
        
        return false;
      }
      
      bool aborted(){ return data->abort; }
      
    };
    
    
    static void parse_ignored(state &s){
      const parsing_expression_grammar<I> &grammar = *s.current_grammar;
      if(grammar.ignored_id == -1)return;
      s.parsing_ignored=true;
      if(s.enter_rule(grammar.ignored_id)==-1){
        while (parse(grammar.get_rule_vertex(grammar.ignored_id),s));
        s.exit_rule(true);
      }
      s.parsing_ignored=false;
    }
    
    static void parse_separated(state &s){
      const parsing_expression_grammar<I> &grammar = *s.current_grammar;
      if(grammar.separator_id == -1)return;
      s.parsing_separator=true;
      if(s.enter_rule(grammar.separator_id) == -1){
        while (parse(grammar.get_rule_vertex(grammar.separator_id),s));
        s.exit_rule(true);
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
          
          auto prev_position = s.current_position;
          bool status = parse(grammar.get_rule_vertex(*n[0]),s);
          if(status)status = s.filter_rule();
          if(status == false) s.current_position = prev_position;
          
          parse_tree::vertex_descriptor prev_vertex = s.parse_stack_top().id;
          
          bool left_recursion = s.exit_rule(status);
          
          if(status==true && left_recursion){
            
            while(true){
              auto last_position = s.current_position;
              s.current_position = prev_position;
              parse_tree::vertex_descriptor v;
              
              s.enter_left_recursion(*n[0],v);
              bool pstatus = parse(grammar.get_rule_vertex(*n[0]),s);
              
              bool status = s.exit_left_recursion(pstatus);
              
              if(!status){
                s.current_position = last_position;
                s.add_vertex(prev_vertex);
                break;
              }
              
              prev_vertex = v;
            }
            
          }
          
          if(status==true)if(!s.parsing_separator && !s.parsing_ignored)parse_separated(s);
          
          return status;
        }break;
          
        case parsing_expression_grammar_symbol::gotogrammar:{
          const parsing_expression_grammar<I> *p=s.current_grammar;
          s.current_grammar=&*grammar.embedded_grammars[*n[0]];
          bool stat=parse( n[1], s );
          s.current_grammar = p;
          return stat;
        }break;
          
        default:
          throw "Illegal grammar instruction";
          break;
      }
      
      return false;
      
    }
    
  };
  
  template <class Visitor> parser<Visitor> create_packrat_parser(parsing_expression_grammar<Visitor> g){
    return parser<Visitor>(std::make_shared<parsing_expression_grammar<Visitor>>(g));
  }
  
  template <class Visitor> parser<Visitor> create_packrat_parser(std::shared_ptr<parsing_expression_grammar<Visitor>> g){
    return parser<Visitor>(g);
  }
  
  template <class Visitor> class parsing_expression_grammar_builder:public parsing_expression_grammar<Visitor>::builder{
    std::shared_ptr<parsing_expression_grammar<Visitor>> grammar;
    parser<parsing_expression_grammar_grammar_visitor<Visitor>> peg_parser;
    
  public:
    
    struct production_rule_builder:public parsing_expression_grammar<Visitor>::production_rule_builder{
      parsing_expression_grammar_builder <Visitor> * pegb_parent;
      
      using rule_builder = typename parsing_expression_grammar<Visitor>::production_rule_builder;
      using rule_evaluator = typename parsing_expression_grammar<Visitor>::rule_evaluator;
      
      production_rule_builder(parsing_expression_grammar_builder <Visitor> *p,const std::string &rule):parsing_expression_grammar<Visitor>::production_rule_builder(p->grammar->rule(rule)),pegb_parent(p){}
      
      rule_builder & operator<<(const std::string &str){
        return rule_builder::operator<<(pegb_parent->peg_parser.parse(str).evaluate(*this->parent)->get_vertex());
      }
      
      rule_builder & operator<<(const rule_evaluator & c){ return rule_builder::operator<<(c); }
      
    };
    
    parsing_expression_grammar_builder(const std::string &name=""):parsing_expression_grammar<Visitor>::builder(new parsing_expression_grammar<Visitor>(name)),grammar(parsing_expression_grammar<Visitor>::builder::parent),peg_parser(create_packrat_parser(parsing_expression_grammar_grammar<Visitor>())){
      
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
    
    parser<Visitor> get_parser(){
      return create_packrat_parser(grammar);
    }
    
  };
  
}
