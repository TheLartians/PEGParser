///////////////////////////////////////////////////////////
//
//    Created in 2014 by Lars Melchior
//
//    This file is distributed under the GNU
//    General Public License (GPL).
//    See LICENSE.TXT for details.
//

#pragma once

#include <functional>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

#include "graph.h"


namespace lars {
  
  template <class Visitor> class expression;
  
  class grammar_base{ };
  
  template <class Visitor> class grammar:public grammar_base{
    public:
    virtual void evaluate(expression<Visitor> e)const=0;
  };
  
  struct parser_position{
    unsigned character,line,location;
    parser_position(){ character=line=location=-1; }
  };
  
  struct parsed_expresssion{
    parser_position begin,end;
    std::unique_ptr<std::string> string;
    const grammar_base * grammar;
    std::unique_ptr<std::vector<size_t>> non_ignored_edges;
    uintptr_t rule_id,production_id;
    
    parsed_expresssion(){}
    parsed_expresssion(const parsed_expresssion & other):begin(other.begin),end(other.end),grammar(other.grammar),rule_id(other.rule_id),production_id(other.production_id){
      if(other.string)string.reset(new std::string(*other.string));
      if(other.non_ignored_edges)non_ignored_edges.reset(new std::vector<size_t>(*other.non_ignored_edges));
    }
    
    parsed_expresssion & operator=(const parsed_expresssion & other){
      begin = other.begin;
      end = other.end;
      grammar = other.grammar;
      rule_id = other.rule_id;
      production_id = other.production_id;
      if(other.string)string.reset(new std::string(*other.string));
      if(other.non_ignored_edges)non_ignored_edges.reset(new std::vector<size_t>(*other.non_ignored_edges));
      return *this;
    }
    
  };
  
  using parse_tree = graph<parsed_expresssion>;
  
  struct expression_data{
    std::string parsed_string;
    parse_tree  tree;
    bool abort;
  };
  
  template <class Visitor> class expression{
    
    typename parse_tree::vertex vertex;
    std::shared_ptr<expression_data> expr_data;
    Visitor * current_visitor;
    
    public:
    
    struct iterator{
      const expression &base;
      size_t position;
      expression operator*(){ return base[position]; }
      void operator++(){ ++position; }
      bool operator!=(const iterator &other)const{ return &other.base != &base || other.position != position; }
      iterator(const expression &b,size_t p):base(b),position(p){}
    };

    static char vertex_placeholder_char;
    
    expression(typename parse_tree::vertex v,std::shared_ptr<expression_data> d,Visitor * i=nullptr):vertex(v),expr_data(d),current_visitor(i){ }
    expression():vertex(parse_tree::invalid_vertex()),expr_data(std::make_shared<expression_data>()),current_visitor(nullptr){}
    
    expression deep_copy()const{
      std::shared_ptr<expression_data> data_copy = std::make_shared<expression_data>() ;
      data_copy->parsed_string = expr_data->parsed_string;
      return expression(data_copy->tree.copy_tree(vertex),data_copy);
    }
    
    std::shared_ptr<expression_data> get_global_data(){ return expr_data; }
    
    parsed_expresssion &raw_expression()const{ return vertex.content(); }
    parse_tree &get_tree()const{ return expr_data->tree; }
    typename parse_tree::vertex get_vertex()const{ return vertex; }
    expression get_expression(typename parse_tree::vertex v)const{ return expression(v, expr_data); }
    void set_vertex(typename parse_tree::vertex v){ vertex = v; }
    
    uintptr_t size()const{ if(raw_expression().non_ignored_edges)return raw_expression().non_ignored_edges->size(); return vertex.size(); }
    uintptr_t rule_id()const{ return raw_expression().rule_id; }
    uintptr_t production_id()const{ return raw_expression().production_id; }
    //const std::string &rule_name()const;
    const lars::grammar<Visitor> * grammar()const{ return (lars::grammar<Visitor> *)raw_expression().grammar; }
    uintptr_t get_edge_index(uintptr_t i)const{ if(raw_expression().non_ignored_edges)return (*raw_expression().non_ignored_edges)[i]; return i; }
    expression operator[](uintptr_t i)const{ return expression(vertex[get_edge_index(i)],expr_data,current_visitor); }
    const std::string & complete_string()const{ return expr_data->parsed_string; }
    std::string raw_string()const{uintptr_t b=raw_expression().begin.location,e=raw_expression().end.location; return complete_string().substr(b,e-b);}
    char character(uintptr_t pos = 0)const{ if(&*raw_expression().string)return string()[pos]; return complete_string()[begin_position().location+pos]; }
    parser_position begin_position()const{ return raw_expression().begin; }
    parser_position end_position()const{ return raw_expression().end; }
    uintptr_t length()const{ return end_position().location-begin_position().location; }

    Visitor & visitor()const{ return *current_visitor; }

    operator bool(){ return get_global_data()->parse_successful; }
    
    void abort(){ expr_data->abort = true; }
    bool is_aborted(){ return expr_data->abort; }

    void accept(Visitor *I){ current_visitor = I; grammar()->evaluate(*this); }
    void accept(){ assert(current_visitor); grammar()->evaluate(*this); }
    
    template<typename... Args> std::unique_ptr<Visitor> evaluate(Args & ... args){
      current_visitor = new Visitor(args...);
      grammar()->evaluate(*this);
      return std::unique_ptr<Visitor>(current_visitor);
    }
    
    const Visitor & get_value(){
      evaluate();
      return visitor();
    }
    
    Visitor & value()const{
      return visitor();
    }
    
    iterator begin()const{ return iterator(*this,0); }
    iterator end()const{ return iterator(*this,size()); }
    
    std::string string()const{
      if(&*raw_expression().string){
        if(size()==0) return *raw_expression().string;
        std::string str;
        uintptr_t j = 0;
        for (uintptr_t i = 0; i<raw_expression().string->size(); ++i){
          if ((*raw_expression().string)[i] == expression::vertex_placeholder_char)str+=(*this)[j++].string();
          else str += (*raw_expression().string)[i];
        }
        return str;
      }
      if(vertex.size()==0)return raw_string();
      std::string str;
      str.reserve(end_position().location-begin_position().location);
      for (int i=0; i<size(); ++i) {
        str+=intermediate(i);
        str+=(*this)[i].string();
      }
      str+=intermediate(size());
      return str;
    }
    
    std::string intermediate(uintptr_t i=1)const{
      std::string str;
      int l,j;
      if(i==0)l=-1;
      else l=get_edge_index(i-1);
      if(i==size())j=vertex.size();
      else j=get_edge_index(i);
      int b,e;
      for (; l<j; ++l) {
        if(l==-1)b=begin_position().location;
        else b=vertex[l].content().end.location;
        if(l==vertex.size()-1)e=end_position().location;
        else e=vertex[l+1].content().begin.location;
        if(b<e)str+=complete_string().substr(b,e-b);
      }
      return str;
    }
    
    ~expression(){

    }
    
  };
  
  
  template <class Visitor> char expression<Visitor>::vertex_placeholder_char = '\0';
  
    

  
}
