/*******************************************************************\

Module:

Author:

\*******************************************************************/

#ifndef CPROVER_PATH_SYMEX_TAINT_ANALYSIS_H
#define CPROVER_PATH_SYMEX_TAINT_ANALYSIS_H

#include <iostream>

// TODO: consider arrays, structs, member, index, dereference, if, array, vector, string constant,
// TODO: how to introduce / remove taint
// TODO: get
// TODO: JSON parser
// TODO: basic extensibility for different domains.
// TODO: function call considerations.
//
///* Base Interface */
//
//template<typename domainT>
//class path_symex_taint_analysist
//{
//public:
//  typedef domainT taintt;
//  /* domainT refers to the taint domain to be used */
//
//  path_symex_taint_analysist() {};
//
//  taintt meet(taintt taint_one, taintt taintt_two);
//  /* Meet of two taints should be handled by inheritance */
//  inline taintt meet(taintt taint_one) { return taint_one; }
//  /* Meet of a single taint is simply the taint itself. */
//
//  inline taintt parser(std::string input);
//  // Parses JSON strings -> items in taint domain.
//
//
//  // introduction
//  // remove
//  // get
//};

/* End Base Interface */


enum class simple_taint_domaint {
  UNTAINTED, TAINTED
};

class path_symex_simple_taint_analysist
{
public:
  typedef simple_taint_domaint taintt;

  inline taintt meet(taintt taint_one, taintt taint_two) {
    return (taint_one == taintt::TAINTED || taint_two == taintt::TAINTED)
        ? taintt::TAINTED : taintt::UNTAINTED;
  }

  inline taintt parser(std::string input) {
    switch(input) {
    case "untainted":
      return taintt::UNTAINTED;
    case "tainted":
      return taintt::TAINTED;
    }
    throw "Taint type not recognised";
  }

  inline std::string parser(taintt input) {
    switch(input) {
    case taintt::UNTAINTED:
      return "untainted";
    case taintt::TAINTED:
      return "tainted";
    }
    throw "Taint type not recognised";
  }

  inline std::string taint_engine() { return "Simple taint domain."; }
};

#endif
