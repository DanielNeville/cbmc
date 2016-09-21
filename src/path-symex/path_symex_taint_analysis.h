/*******************************************************************\

Module:

Author:

\*******************************************************************/

#ifndef CPROVER_PATH_SYMEX_TAINT_ANALYSIS_H
#define CPROVER_PATH_SYMEX_TAINT_ANALYSIS_H

//class path_symex_taint_analysis_domaint;
//
//template<typename path_symex_taint_analysis_domaint>
//class path_symex_taint_analysist
//{
//public:
//  typedef path_symex_taint_analysis_domaint taintt;
//
//  virtual taintt meet(taintt taint_one, taintt taintt_two);
//  inline taintt meet(taintt taint_one) { return taint_one; }
//  // introduction
//  // remove
//  // get
//};

class path_symex_simple_taint_analysis_domaint
{
public:
  enum simple_taint_domaint {
    UNTAINTED = 1,
    TAINTED = 2,
  };

  typedef simple_taint_domaint taintt;

  inline simple_taint_domaint meet(simple_taint_domaint taint_one, simple_taint_domaint taint_two) {
    return taint_one > taint_two ? taint_one : taint_two;
  }
};



#endif
