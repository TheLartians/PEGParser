///////////////////////////////////////////////////////////
//
//    Created in 2014 by Lars Melchior
//
//    This file is distributed under the GNU
//    General Public License (GPL).
//    See LICENSE.TXT for details.
//

#pragma once

#include <iterator>
#include <vector>

namespace lars {
  
#if defined(__GNUC__)
#  define UNUSED __attribute__ ((unused))
#elif defined(_MSC_VER)
#  define UNUSED __pragma(warning(suppress:4100))
#else
#  define UNUSED
#endif
  

  
  template<class T> class range_iterator : public std::iterator<std::input_iterator_tag, T>{
  private:
    T item;
    const T dist;
    const T mult;
    
  public:
    range_iterator(const T &item,const T &d=0,const T &m=1) : item(item),dist(d),mult(m) {}
    
    T operator*()const{ return item * mult + dist; }
    
    range_iterator<T> &operator++() { ++item; return *this; }
    range_iterator<T> &operator++(int) { range_iterator<T> range_copy(*this); ++item; return range_copy; }
    bool operator==(const range_iterator<T> &rhs) { return item == rhs.item; }
    bool operator!=(const range_iterator<T> &rhs) { return !(item == rhs.item); }
  };
  
  template<class T> class range_wrapper {
  public:
    range_wrapper(const T &r_start, const T &r_end,const T &d=0,const T &m=1) : r_start(r_start), r_end(r_end), dist(d), mult(m) {}
    range_iterator<T> begin() { return range_iterator<T>(r_start,dist,mult); }
    range_iterator<T> end() { return range_iterator<T>(r_end,dist,mult); }
    
    range_wrapper<T> operator+(const T &d){ return range_wrapper<T>(r_start,r_end,dist + d,mult); }
    range_wrapper<T> operator*(const T &m){ return range_wrapper<T>(r_start,r_end,dist * m,mult * m); }
    
  private:
    const T r_start, r_end;
    const T dist,mult;
  };

  template<class T> range_wrapper<T> range(const T &start, const T &end) { return {start, end}; }
  template<class T> range_wrapper<T> range(const T &end) { return {T(), end}; }
  
  template<class T> struct reverse_wrapper{
    T & obj;
    typedef typename T::reverse_iterator iterator;
    reverse_wrapper(T &o):obj(o){}
    iterator begin(){ return obj.rbegin(); }
    iterator end(){ return obj.rend(); }
  };
  
  template<class T> struct const_reverse_wrapper{
    const T & obj;
    typedef typename T::const_reverse_iterator const_iterator;
    const_reverse_wrapper(const T &o):obj(o){}
    const_iterator begin()const{ return obj.rbegin(); }
    const_iterator end()const{ return obj.rend(); }
  };
  
  template<class T> reverse_wrapper<T> reverse(T & obj){
    return reverse_wrapper<T>(obj);
  }
  
  template<class T> const_reverse_wrapper<T> reverse(const T & obj){
    return const_reverse_wrapper<T>(obj);
  }
  
  

  //
  // ---- Split ----
  //
  
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
  


  
}

