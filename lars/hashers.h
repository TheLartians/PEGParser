///////////////////////////////////////////////////////////
//
//    Created in 2014 by Lars Melchior
//
//    This file is distributed under the GNU
//    General Public License (GPL).
//    See LICENSE.TXT for details.
//

#pragma once

#include <tuple>

namespace std{
  
  template <typename T1, typename T2, typename T3> struct hash<std::tuple<T1, T2, T3> > {
    hash<T1> hash1;
    hash<T2> hash2;
    hash<T3> hash3;
    
  public:
    uintptr_t operator()(tuple<T1, T2, T3> x) const throw() {
      return hash1(get<0>(x)) ^ hash2(get<1>(x)) ^ hash3(get<2>(x)) ;
    }
  };
  
}
