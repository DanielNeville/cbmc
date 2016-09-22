/*
 * path_symex_taint_data.h
 *
 *  Created on: 22 Sep 2016
 *      Author: dan
 */

#ifndef PATH_SYMEX_PATH_SYMEX_TAINT_DATA_H_
#define PATH_SYMEX_PATH_SYMEX_TAINT_DATA_H_

#include <path-symex/path_symex_taint_analysis.h>

class taint_datat
{
public:

  class taint_rulet
  {
  public:
    // TODO : Consider get / set / operator
    unsigned int loc;
    taintt taint;

    inline void output(std::ostream &out) const {
      path_symex_simple_taint_analysist taint_engine;

      out << "Location: " << loc << " set to " <<
          taint_engine.parser(taint);
    }

    inline taint_rulet(): loc(0), taint(taintt::UNTAINTED)
    {

    }
  };

  typedef std::vector<taint_rulet> datat;
  datat data;

  inline void add(unsigned loc, taintt taint) {
    taint_rulet rule;
    rule.loc = loc;
    rule.taint = taint;
    data.push_back(rule);
  }

  inline bool check_rules(
      locst &locs,
      std::ostream & warning) {

    bool error = false;

    for(auto rule : data) {
      // Rule outside program.  Not meaningful.
      if(rule.loc >= locs.size()) {
        warning << "Following rule outside program scope:" << "\n";
        rule.output(warning);
        warning << "\n";
        error = true;
      }

      loc_reft loc;
      loc.loc_number = rule.loc;
      goto_programt::const_targett inst = locs[loc].target;
      // Find instruction referred to by rule.

      if(inst->is_assign() || inst->is_decl() || inst->is_function_call())
      {
        if(inst->is_function_call()) {
          code_function_callt function_call = to_code_function_call(inst->code);
          if(function_call.lhs().is_nil()) {
            warning << "Following rule refers to function call with nil left-hand side:" << "\n";
            rule.output(warning);
            warning << "\n";
            error = true;
          }
        }
      } else
      {
        warning << "Following rule refers to an unsupported op (" << inst->type << ")\n";
        rule.output(warning);
        warning << "\n";
        error = true;
      }
    }

    return error;
  }

  inline void output(std::ostream &out) const {
    // TODO Templated later.

    int i = 0;
    for(auto taint_rule : data) {
      out << ++i << ": ";
      taint_rule.output(out);
      out << "\n";
      // TODO EOM.
    }
  }
};


#endif /* PATH_SYMEX_PATH_SYMEX_TAINT_DATA_H_ */
