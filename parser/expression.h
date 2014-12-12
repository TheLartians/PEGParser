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
  
  class grammar_base{
    public:
    using rule_id = unsigned;
    virtual const std::string & get_rule_name(rule_id)const= 0;
  };
  
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
    const grammar_base * grammar;
    uintptr_t rule_id,state;
    
    parsed_expresssion(){}
    parsed_expresssion(const parsed_expresssion & other):begin(other.begin),end(other.end),grammar(other.grammar),rule_id(other.rule_id),state(other.state){
    }
    
    parsed_expresssion & operator=(const parsed_expresssion & other){
      begin = other.begin;
      end = other.end;
      grammar = other.grammar;
      rule_id = other.rule_id;
      state = other.state;
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
    
    class error;
    
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
    
    uintptr_t size()const{ return vertex.size(); }
    uintptr_t rule_id()const{ return raw_expression().rule_id; }
    const std::string &rule_name()const{ return raw_expression().grammar->get_rule_name(rule_id()); }
    
    uintptr_t state()const{ return raw_expression().state; }
    const lars::grammar<Visitor> * grammar()const{ return (lars::grammar<Visitor> *)raw_expression().grammar; }
    expression operator[](uintptr_t i)const{ return expression(vertex[i],expr_data,current_visitor); }
    
    const std::string & full_string()const{ return expr_data->parsed_string; }
    std::string string()const{uintptr_t b=raw_expression().begin.location,e=raw_expression().end.location; return full_string().substr(b,e-b); }
    char character(uintptr_t pos = 0)const{ return full_string()[begin_position().location+pos]; }
    
    std::string intermediate(uintptr_t i=1)const{
      std::string str;
      int l,j;
      if(i==0)l=-1;
      else l=(i-1);
      if(i==size())j=vertex.size();
      else j=(i);
      int b,e;
      for (; l<j; ++l) {
        if(l==-1)b=begin_position().location;
        else b=vertex[l].content().end.location;
        if(l==vertex.size()-1)e=end_position().location;
        else e=vertex[l+1].content().begin.location;
        if(b<e)str+=full_string().substr(b,e-b);
      }
      return str;
    }
    
    parser_position begin_position()const{ return raw_expression().begin; }
    parser_position end_position()const{ return raw_expression().end; }
    uintptr_t length()const{ return end_position().location-begin_position().location; }

    Visitor & visitor()const{ return *current_visitor; }

    operator bool(){ return get_global_data()->parse_successful; }
    
    void abort(){ expr_data->abort = true; }
    bool is_aborted(){ return expr_data->abort; }

    void accept(Visitor *I){ current_visitor = I; grammar()->evaluate(*this); }
    void accept(){ grammar()->evaluate(*this); }
    
    template<typename... Args> std::unique_ptr<Visitor> evaluate(Args & ... args){
      current_visitor = new Visitor(args...);
      grammar()->evaluate(*this);
      return std::unique_ptr<Visitor>(current_visitor);
    }
    
    const Visitor & get_value(){
      accept();
      return visitor();
    }
    
    Visitor & value()const{
      return visitor();
    }
    
    void throw_error(const std::string &message, int code = error::unspecified){ throw error(*this,message,code); }
    void throw_error(int code){ throw error(*this,"",code); }
    
    iterator begin()const{ return iterator(*this,0); }
    iterator end()const{ return iterator(*this,size()); }
    
    ~expression(){

    }
    
  };
  
  template <class V> class expression<V>::error:public expression<V>{
    int error_code;
    std::string message;
    grammar_base::rule_id top_rule_id;
  public:
    enum codes:int{ unspecified = 0, parsing_error = 1, min_unreserved_code=2 };
    error(expression<V> error_expression,const std::string &mes,int c):expression<V>(error_expression),error_code(c),message(mes){}
    const std::string & error_message(){ return message; }
    int code(){ return error_code; }
  };

  
  
  template <class Visitor> char expression<Visitor>::vertex_placeholder_char = '\0';
  
    

  
}
