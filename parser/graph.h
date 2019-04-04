///////////////////////////////////////////////////////////
//
//    Created in 2014 by Lars Melchior
//
//    This file is distributed under the BSD 3-Clause 
//    license. See LICENSE.TXT for details.
//


#pragma once

#include <vector>
#include <limits>
#include <utility>
#include <assert.h>
#include <unordered_set>

namespace lars {
  
  struct no_content{};
  
  template <typename V, typename E = no_content> class graph{
  public:
    struct vertex;
    struct edge;
    struct const_vertex;
    struct const_edge;
    
    using vertex_descriptor = size_t;
    using edge_descriptor = std::pair<size_t,size_t>;
    std::vector<std::pair<std::vector<std::pair<vertex_descriptor,E> >, V> > AdjacencyList;
    std::unordered_set<vertex_descriptor> unused_vertices;
    
    vertex create_vertex();
    vertex create_vertex(const V &c);
    V & get_content(vertex_descriptor v){ return AdjacencyList[v].second; }
    E & get_content(edge_descriptor e){ return AdjacencyList[e.first].first[e.second].second; }
    const V & get_content(vertex_descriptor v)const{ return AdjacencyList[v].second; }
    const E & get_content(edge_descriptor e)const{ return AdjacencyList[e.first].first[e.second].second; }
    size_t capacity(){ return AdjacencyList.size(); }
    size_t size(){ return AdjacencyList.size() - unused_vertices.size(); }
    edge create_edge  (vertex_descriptor v1,vertex_descriptor v2,const E &c = E());
    edge create_edge  (vertex_descriptor v1,vertex_descriptor v2,size_t pos,const E &c = E());
    vertex get_vertex(vertex_descriptor v);
    edge get_edge  (edge_descriptor e);
    const_vertex get_vertex(vertex_descriptor v) const;
    const_edge get_edge  (edge_descriptor e) const;
    static vertex invalid_vertex();
    static vertex invalid_edge();
    static vertex_descriptor invalid_vertex_descriptor();
    static edge_descriptor invalid_edge_descriptor();
    void remove_unused_vertices(std::vector<vertex_descriptor> roots);
    void remove_unused_vertices(vertex_descriptor root){ std::vector<vertex_descriptor> roots; roots.push_back(root); remove_unused_vertices(roots); }
    
    vertex copy_tree(vertex);
    
    void erase_vertex(vertex_descriptor);
    void erase_tree(vertex_descriptor);
    
    struct edge{
      graph * parent;
      typename graph::edge_descriptor id;
      edge():parent(nullptr),id(graph::invalid_edge_descriptor()){}
      edge(graph * p,typename graph::edge_descriptor i):parent(p),id(i){ }
      vertex base()const{ return parent->get_vertex(id.first); }
      vertex target()const{ return parent->get_vertex(parent->AdjacencyList[id.first].first[id.second].first); }
      E & content()const{ return parent->get_content(id); }
      E & operator*()const{ return parent->get_content(id); }
    };
    
    struct vertex{
      graph * parent;
      typename graph::vertex_descriptor id;
      vertex():parent(nullptr),id(graph::invalid_vertex_descriptor()){}
      vertex(graph * p,typename graph::vertex_descriptor i):parent(p),id(i){ }
      vertex replace(const const_vertex &other);
      V & content()const{ return parent->get_content(id); }
      V & operator*()const{ return parent->get_content(id); }
      size_t size()const{ return parent->AdjacencyList[id].first.size(); }
      graph::edge add_edge(vertex v,const E &c = E())const{ assert(v.parent == parent); return parent->create_edge(id,v.id,c); }
      graph::edge add_edge(vertex_descriptor v,const E &c = E())const{ return parent->create_edge(id,v,c); }
      graph::edge add_edge(vertex v,size_t pos,const E &c = E())const{ assert(v.parent == parent); return parent->create_edge(id,v.id,pos,c); }
      graph::edge add_edge(vertex_descriptor v,size_t pos,const E &c = E())const{ return parent->create_edge(id,v,pos,c); }
      graph::edge edge(size_t i)const{ assert(size()>i); return parent->get_edge(typename graph::edge_descriptor(id,i)); }
      vertex target(size_t i)const{ assert(size()>i); return parent->get_vertex(parent->AdjacencyList[id].first[i].first); }
      vertex operator[](size_t i)const{ assert(size()>i); return parent->get_vertex(parent->AdjacencyList[id].first[i].first); }
      void resize(size_t s){ parent->AdjacencyList[id].first.resize(s); }
      void set_edge(size_t s,graph::vertex_descriptor i){ parent->AdjacencyList[id].first[s].first = i; }
      void set_edge(size_t s,graph::vertex i){ parent->AdjacencyList[id].first[i].first = i.id; }
      bool operator==(const const_vertex &other)const{ return parent == other.parent && id == other.id; }
      bool operator!=(const const_vertex &other)const{ return !(*this==other); }
      template <class F> void for_all(F f){ f(*this); for(size_t i=0;i<size();++i)target(i).for_all(f); }
    };
    
    struct const_edge{
      const graph * parent;
      typename graph::edge_descriptor id;
      const_edge():parent(nullptr),id(graph::invalid_edge_descriptor()){}
      const_edge(edge u):parent(u.parent),id(u.id){ }
      const_edge(const graph * p,typename graph::edge_descriptor i):parent(p),id(i){ }
      const_vertex base()const{ return parent->get_vertex(id.first); }
      const_vertex target()const{ return parent->get_vertex(parent->AdjacencyList[id.first].first[id.second].first); }
      const E & content()const{ return parent->get_content(id); }
      const E & operator*()const{ return parent->get_content(id); }
    };
    
    struct const_vertex{
      const graph * parent;
      typename graph::vertex_descriptor id;
      const_vertex():parent(nullptr),id(graph::invalid_vertex_descriptor()){}
      const_vertex(vertex u):parent(u.parent),id(u.id){ }
      const_vertex(const graph * p,typename graph::vertex_descriptor i):parent(p),id(i){ }
      const V & content()const{ return parent->get_content(id); }
      const V & operator*()const{ return parent->get_content(id); }
      size_t size()const{ return parent->AdjacencyList[id].first.size(); }
      graph::const_edge edge(size_t i)const{ assert(size()>i); return parent->get_edge(typename graph::edge_descriptor(id,i)); }
      const_vertex target(size_t i)const{ assert(size()>i); return parent->get_vertex(parent->AdjacencyList[id].first[i].first); }
      const_vertex operator[](size_t i)const{ assert(size()>i); return parent->get_vertex(parent->AdjacencyList[id].first[i].first); }
      bool operator==(const const_vertex &other)const{ return parent == other.parent && id == other.id; }
      bool operator!=(const const_vertex &other)const{ return !(*this==other); }
    };
    
  };
  
  template <typename V, typename E> typename graph<V,E>::vertex graph<V,E>::create_vertex(const V &c){
    if(unused_vertices.size() == 0){
      vertex_descriptor v = AdjacencyList.size();
      AdjacencyList.resize(AdjacencyList.size()+1,std::pair<std::vector<std::pair<vertex_descriptor,E> >, V> (std::vector<std::pair<vertex_descriptor,E> >(),V(c)));
      return graph<V,E>::vertex(this,v);
    }
    else {
      vertex_descriptor v = *unused_vertices.begin();
      unused_vertices.erase(unused_vertices.begin());
      AdjacencyList[v].first.resize(0);
      (&AdjacencyList[v].second)->~V();
      new (&AdjacencyList[v].second) V(c);
      return get_vertex(v);
    }
  }
  
  template <typename V, typename E> typename graph<V,E>::vertex graph<V,E>::create_vertex(){
    if(unused_vertices.size() == 0){
      vertex_descriptor v = AdjacencyList.size();
      AdjacencyList.resize(AdjacencyList.size()+1);
      return graph<V,E>::vertex(this,v);
    }
    else {
      vertex_descriptor v = *unused_vertices.begin();
      unused_vertices.erase(unused_vertices.begin());
      AdjacencyList[v].first.resize(0);
      new (&AdjacencyList[v].second) V();
      return get_vertex(v);
    }
  }
  
  template <typename V, typename E> typename graph<V,E>::edge graph<V,E>::create_edge(vertex_descriptor v1,vertex_descriptor v2,const E &c){
    AdjacencyList[v1].first.push_back(std::pair<vertex_descriptor,E>(v2,c));
    return graph<V,E>::edge(this,edge_descriptor(v1,AdjacencyList[v1].first.size()-1));
  }
  
  template <typename V, typename E> typename graph<V,E>::edge graph<V,E>::create_edge(vertex_descriptor v1,vertex_descriptor v2,size_t pos,const E &c){
    AdjacencyList[v1].first.insert(AdjacencyList[v1].first.begin()+pos, std::pair<vertex_descriptor,E>(v2,c));
    return graph<V,E>::edge(this,edge_descriptor(v1,AdjacencyList[v1].first.size()-1));
  }
  
  template <typename V, typename E> typename graph<V,E>::vertex graph<V,E>::get_vertex(vertex_descriptor v){
    return graph<V,E>::vertex(this,v);
  }
  
  template <typename V, typename E> typename graph<V,E>::edge graph<V,E>::get_edge  (edge_descriptor e){
    return graph<V,E>::edge(this,e);
  }

  template <typename V, typename E> typename graph<V,E>::vertex graph<V,E>::copy_tree(typename graph<V,E>::vertex e){
    vertex v = create_vertex(e.content());
    for (size_t i=0; i<e.size(); ++i) v.add_edge(copy_tree(e[i]));
    return v;
  }
  
  template <typename V, typename E> void graph<V,E>::erase_vertex(typename graph<V,E>::vertex_descriptor e){
    (&AdjacencyList[e].second)->~V();
    unused_vertices.insert(e);
  }

  template <typename V, typename E> void graph<V,E>::erase_tree(typename graph<V,E>::vertex_descriptor e){
    vertex v = get_vertex(e);
    for (size_t i=0; i<v.size(); ++i) erase_tree(copy_tree(v[i]));
    erase_vertex(e);
  }

  template <typename V, typename E> typename graph<V,E>::const_vertex graph<V,E>::get_vertex(vertex_descriptor v) const {
    return graph<V,E>::const_vertex(this,v);
  }
  
  template <typename V, typename E> typename graph<V,E>::const_edge graph<V,E>::get_edge  (edge_descriptor e) const {
    return graph<V,E>::const_edge(this,e);
  }
  
  template <typename V, typename E> typename graph<V,E>::vertex graph<V,E>::invalid_vertex(){
    return graph<V,E>::vertex(NULL,std::numeric_limits<vertex_descriptor>::max());
  }
  
  template <typename V, typename E> typename graph<V,E>::vertex graph<V,E>::invalid_edge(){
    return graph<V,E>::edge(NULL,std::numeric_limits<edge_descriptor>::max());
  }
  
  template <typename V, typename E> typename graph<V,E>::vertex_descriptor graph<V,E>::invalid_vertex_descriptor(){
    return std::numeric_limits<vertex_descriptor>::max();
  }
  
  template <typename V, typename E> typename graph<V,E>::edge_descriptor graph<V,E>::invalid_edge_descriptor(){
    return std::numeric_limits<edge_descriptor>::max();
  }
  
  template <typename V, typename E> typename graph<V,E>::vertex graph<V,E>::vertex::replace(const graph<V,E>::const_vertex &other){
    resize(other.size());
    for (int i=0; i<size(); ++i) set_edge(i, other.target(i).id);
    content().~V();
    new (&content()) V(other.content());
    return *this;
  }
  
  template <typename V, typename E> void graph<V,E>::remove_unused_vertices(std::vector<typename graph<V,E>::vertex_descriptor> roots){
    std::unordered_set<vertex_descriptor> connected_vertices;
    for(size_t i=0;i<roots.size();++i) get_vertex(roots[i]).for_all([&](vertex u){ connected_vertices.insert(u.id); });
    for(size_t i=0; i<size();++i) if(connected_vertices.find(i) == connected_vertices.end()) erase_vertex(i);
  }
  
}

