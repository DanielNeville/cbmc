/*
 * path_symex_taint_data.h
 *
 *  Created on: 22 Sep 2016
 *  Author: Daniel Neville
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

	// The location where taint is forced
	unsigned int loc;

	// The taint state the force.
    taintt taint_state;

    // A flag to denote that taint must be forced at a particular array index.
    bool array_index_flag;

    // The array index where taint is forced.
    unsigned array_index;

    // A flag to denote that taint must be forced at for a particular member in a struct.
    bool struct_member_flag;

    // The member that must be associated with a taint state.
    std::string struct_member;


    inline void output(path_symex_taint_analysist &taint_analysis, std::ostream &out) const {

      out << "Location: " << loc << " set to " <<
    		  taint_analysis.get_taint_state_name(taint_state);
    }

    inline taint_rulet(path_symex_taint_analysist &taint_analysis)
    {

    	loc = 0;
    	taint_state = taint_analysis.get_top_position();
    	array_index_flag = false;
    	array_index = 0;
    	struct_member_flag = false;
    	struct_member = "";
    }
  };


  typedef std::vector<taint_rulet> datat;
  datat data;


  /*******************************************************************\

  Function: taint_datat::add

    Inputs: The location where a given taint state is introduced.

   Outputs: Nothing

   Purpose: Registers a taint rule, normally parsed from the JSON file.

  \*******************************************************************/

  inline void add(
      unsigned loc,
      taintt taint_state,
      bool array_index_flag,
      unsigned array_index,
      bool struct_member_flag,
      std::string struct_member) {

    taint_rulet rule;
    rule.loc = loc;
    rule.taint_state = taint_state;
    rule.array_index = array_index;
    rule.array_index_flag = array_index_flag;
    rule.struct_member = struct_member;
    rule.struct_member_flag = struct_member_flag;
    data.push_back(rule);
  }


  inline bool check_rules(
	path_symex_taint_analysist &taint_analysis,
	locst &locs,
    std::ostream & warning) {

    bool error = false;

    for(auto rule : data) {
      // Check whether the rule is outside program.
      if(rule.loc >= locs.size()) {

    	//Not a meaningful rule.
        warning << "Following rule outside program scope:" << "\n";
        rule.output(taint_analysis, warning);
        warning << "\n";
        error = true;
      }

      // Retrieve instruction
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
