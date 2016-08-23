
// The MIT License (MIT)
//
// Copyright (c) 2016 Lars Melchior
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once

#include <iterator>
#include <vector>
#include <assert.h>
#include <cmath>
#include <tuple>
#include <utility>
#include <algorithm>

namespace lars {
  
#pragma mark Range
#pragma mark -
  
#if defined(__GNUC__)
#  define UNUSED __attribute__ ((unused))
#elif defined(_MSC_VER)
#  define UNUSED __pragma(warning(suppress:4100))
#else
#  define UNUSED
#endif
  
  template<class T> class range_wrapper {
  
    const T r_start,r_end,r_increment;

  public:
    
    using value_type = T;
    
    class const_iterator : public std::iterator<std::bidirectional_iterator_tag, T>{
  
    private:
      T current;
      const T increment;
      
    public:
      const_iterator(const T &current,const T &increment) : current(current),increment(increment){ }
      
      T operator*()const{ return current; }
      
      const_iterator &operator++() { current+=increment; return *this; }
      const_iterator operator++(int) { const_iterator range_copy(*this); current+=increment; return range_copy; }
      const_iterator &operator--() { current-=increment; return *this; }
      const_iterator operator--(int) { const_iterator range_copy(*this); current-=increment; return range_copy; }
      
      const_iterator & operator+=(const int &v){ current+=v*increment; return *this; }
      const_iterator operator+(const int &v){ const_iterator range_copy(*this); range_copy+=v*increment; return range_copy; }
      const_iterator & operator-=(const int &v){ current-=v*increment; return *this; }
      const_iterator operator-(const int &v){ const_iterator range_copy(*this); range_copy-=v*increment; return range_copy; }
      
      bool operator!=(const const_iterator &rhs) { return (increment>=0 && (current < rhs.current)) || (increment<0 && (current > rhs.current)); }
      
    };
    
    
    range_wrapper(const T &r_start, const T &r_end,const T &r_increment) : r_start(r_start), r_end(r_end), r_increment(r_increment) {

    }

    using const_reverse_iterator = const_iterator;

    const_iterator begin() const { return const_iterator(r_start,r_increment); }
    const_iterator end()   const { return const_iterator(r_end  ,r_increment); }

    const_reverse_iterator rbegin() const { return const_reverse_iterator(r_end-1    ,-r_increment); }
    const_reverse_iterator rend()   const { return const_reverse_iterator(r_start-1  ,-r_increment); }
    
    range_wrapper<T> operator+(const T &d){ return range_wrapper<T>(r_start + d,r_end + d,r_increment); }
    range_wrapper<T> operator*(const T &m){ return range_wrapper<T>(r_start * m,r_end * m,r_increment * m); }
    
  };

  template<class T> range_wrapper<T> range(const T &start, const T &end, const T & increment) { return range_wrapper<T>(start, end, increment); }
  template<class T> range_wrapper<T> range(const T &start, const T &end) { return range_wrapper<T>(start, end, 1); }
  template<class T> range_wrapper<T> range(const T &end) { return range_wrapper<T>(0, end, 1); }
  
  template <class C> range_wrapper<size_t> indices(const C &c){ return range<size_t>(c.size()); }
  
#pragma mark Reverse
#pragma mark -
  
  template<class T> struct reverse_wrapper{
    T & obj;
    typedef typename T::reverse_iterator iterator;
    typedef typename T::iterator reverse_iterator;
    reverse_wrapper(T &o):obj(o){}
    iterator begin(){ return obj.rbegin(); }
    iterator end(){ return obj.rend(); }
    reverse_iterator rbegin(){ return obj.begin(); }
    reverse_iterator rend(){ return obj.end(); }
  };
  
  template<class T> struct const_reverse_wrapper{
    const T & obj;
    typedef typename T::const_reverse_iterator const_iterator;
    typedef typename T::const_iterator const_reverse_iterator;
    const_reverse_wrapper(const T &o):obj(o){}
    const_iterator begin()const{ return obj.rbegin(); }
    const_iterator end()const{ return obj.rend(); }
    const_reverse_iterator rbegin()const{ return obj.begin(); }
    const_reverse_iterator rend()const{ return obj.end(); }
  };
  
  template<class T> reverse_wrapper<T> reversed(T & obj){
    return reverse_wrapper<T>(obj);
  }
  
  template<class T> const_reverse_wrapper<T> reversed(const T & obj){
    return const_reverse_wrapper<T>(obj);
  }

  template<class T> struct const_reverse_wrapper<range_wrapper<T>>{
    const range_wrapper<T> obj;
    typedef typename range_wrapper<T>::const_reverse_iterator const_iterator;
    typedef typename range_wrapper<T>::const_iterator const_reverse_iterator;
    const_reverse_wrapper(const range_wrapper<T> &o):obj(o){}
    const_iterator begin()const{ return obj.rbegin(); }
    const_iterator end()const{ return obj.rend(); }
    const_reverse_iterator rbegin()const{ return obj.begin(); }
    const_reverse_iterator rend()const{ return obj.end(); }
  };


#pragma mark Split
#pragma mark -

  template<class string_type> class string_ref{
  private:
  
  using value_type = typename string_type::value_type;
  
  value_type const * begin_;
  int             size_;
  
  public:
  int size() const { return size_; }
  value_type const* begin() const { return begin_; }
  value_type const* end() const { return begin_ + size_ ; }
    const value_type &operator[](unsigned i){ return *(begin_ + i); }
  
  string_ref( value_type const* const begin, int const size ) : begin_( begin ) , size_( size ){}
    
    operator string_type()const{
      string_type str;
      for(auto c:*this)str+=c;
      return str;
    }
    
  };
  
  template<class string_type> std::vector<string_ref<string_type>> split( const string_type & str, typename string_type::value_type delimiter = ' ' ){
    
    using value_type = typename string_type::value_type;

  std::vector<string_ref<string_type>>   result;
  
  enum State { inSpace, inToken };
  
  State state = inSpace;
  value_type const*     pTokenBegin = 0;    // Init to satisfy compiler.
  for( auto it = str.begin(); it != str.end(); ++it )
    {
    State const newState = (*it == delimiter? inSpace : inToken);
    if( newState != state )
      {
      switch( newState )
        {
          case inSpace:
          result.push_back( string_ref<string_type>( pTokenBegin, &*it - pTokenBegin ) );
          break;
          case inToken:
          pTokenBegin = &*it;
        }
      }
    state = newState;
  }
  if( state == inToken ) result.push_back( string_ref<string_type>( pTokenBegin, &str.back() - pTokenBegin + 1 ) );
  return result;
  }

#pragma mark subarrays
  
  class subarray_indices{
    public:
    
    class iterator:public std::iterator<std::forward_iterator_tag,std::vector<unsigned>>{
    protected:
      std::vector<unsigned> positions;
      unsigned N;
      unsigned size;
      
    public:
      
      iterator() = default;
      virtual ~iterator(){}
      
      iterator(unsigned UN, unsigned _size, bool end = false):N(std::min(UN,size)),size(_size){
        init(N,_size,end);
      }
      
      void init(unsigned UN, unsigned _size, bool end = false){
        N = UN;
        size = _size;
        positions.resize(N);
        if(!end) for(auto i:range(N)) positions[i] = i;
        else for(auto i:range(N)){
          positions[i] = size - N + i;
          positions[0]++;
        }
      }
      
      bool increment_index(unsigned idx){
        if(positions[idx] == size - N + idx ){
          if(idx == 0){ ++positions[0]; return false; }
          if(!increment_index(idx - 1)) return false;
          positions[idx] = positions[idx-1] + 1;
        }
        else ++positions[idx];
        return true;
      }
      
      virtual bool step(){
        return increment_index(N-1);
      }
      
      iterator & operator++(){
        step();
        return *this;
      }
      
      const std::vector<unsigned> & operator*()const{
        return positions;
      }
      
      bool operator!=(const iterator &other)const{
        return positions != other.positions;
      }
      
    };
    
  private:
      iterator bit,eit;
  public:
    
    subarray_indices(unsigned N,unsigned size):bit(iterator(N,size)),eit(iterator(N,size,true)){ }

    template <class T> subarray_indices(unsigned N,T & arr):bit(iterator(N,arr.size())),eit(iterator(N,arr.size(),true)){ }

    iterator begin()const{ return bit; }
    iterator end()const{ return eit; }
  };
  
  class permutated_subarray_indices{
  public:
    
    class iterator:public subarray_indices::iterator{
      public:
      
      iterator() = default;
      iterator(unsigned N, unsigned size, bool end = false):subarray_indices::iterator(N,size,end){ }
      
      bool step()override{
        if(std::next_permutation(positions.begin(),positions.end())) return true;
        return increment_index(N-1);
      }
      

    };
    
  private:
    iterator bit,eit;
  public:
    
    permutated_subarray_indices(unsigned N,unsigned size):bit(iterator(N,size)),eit(iterator(N,size,true)){ }
    
    template <class T> permutated_subarray_indices(unsigned N,T & arr):bit(iterator(N,arr.size())),eit(iterator(N,arr.size(),true)){ }
    
    iterator begin()const{ return bit; }
    iterator end()const{ return eit; }
    
  };
  
#pragma mark enumerate
  
  template <typename iterator> class enumerate_iterator{
    iterator it;
    unsigned idx = 0;
    
  public:
    
    struct value_type{
      unsigned index;
      typename std::iterator_traits<iterator>::value_type value;
    };
    
    enumerate_iterator(const iterator _it):it(_it){}
    
    value_type operator*()const{ return value_type{idx, *it}; }
    value_type operator*(){ return value_type{idx, *it}; }
    
    enumerate_iterator & operator++(){ ++it; ++idx; return *this; }
    bool operator!=(const enumerate_iterator<iterator> &other)const{ return it!=other.it; }
  };
  
  template <typename iterator> class enumerate_iterator_wrapper{
    enumerate_iterator<iterator> bit,eit;
  public:
    enumerate_iterator_wrapper(const iterator & begin,const iterator & end):bit(begin),eit(end){}
    enumerate_iterator<iterator> begin()const{ return bit; }
    enumerate_iterator<iterator> end()const{ return eit; }
  };
  
  template <typename T> enumerate_iterator_wrapper<typename T::iterator> enumerate(T & arr){
    return enumerate_iterator_wrapper<typename T::iterator>(arr.begin(),arr.end());
  }
  
  template <typename T> enumerate_iterator_wrapper<typename T::const_iterator> enumerate(const T & arr){
    return enumerate_iterator_wrapper<typename T::const_iterator>(arr.begin(),arr.end());
  }
  
#pragma mark slice
  
  template <typename iterator> class slice_wrapper{
    iterator bit,eit;
  public:
    slice_wrapper(const iterator & begin,const iterator & end):bit(begin),eit(end){}
    iterator begin()const{ return bit; }
    iterator end()const{ return eit; }
  };
  
  template <typename T> slice_wrapper<T> slice(const T & b,const T & e){
    return slice_wrapper<T>(b,e);
  }


  
}

