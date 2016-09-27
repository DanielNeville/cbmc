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

	// Defines a taint rule.
	class taint_rulet
	{
	public:
	  // LHS enforced?
	  bool enforces_lhs;

		// The location where taint is forced.  (Redundant.)
		unsigned int loc;

		// The taint state the force.
		taintt taint_state;

		bool symbol_flag;

		irep_idt symbol_name;


		inline void output(taint_enginet &taint_engine, std::ostream &out) const {

			out << "Location: " << loc;
      if(symbol_flag)
        out << ", symbol_name: " << symbol_name;

			out << " set to " << taint_engine.get_taint_name(taint_state);
		}

		inline taint_rulet()
		{
			loc = 0;
			taint_state = 0; // 0 = maximal.
			symbol_flag = false;
			symbol_name = "";
			enforces_lhs = false;
		}
	};

	typedef std::map<unsigned, std::vector<taint_rulet> > datat;
	datat data;

	/*******************************************************************\

	  Function: taint_datat::taint_datat

	   Inputs: The considered taint analysis engine

	   Outputs: Nothing

	   Purpose: Constructor.

	  \*******************************************************************/
	inline taint_datat() {};


	/*******************************************************************\

    Function: taint_datat::add

    Inputs: The location where a given taint state is introduced.

    Outputs: Nothing

    Purpose: Registers a taint rule, normally parsed from the JSON file.

  \*******************************************************************/

	inline void add(
			unsigned loc,
			taintt taint_state,
			bool symbol_flag,
			irep_idt symbol_name) {

		taint_rulet rule;
		rule.loc = loc;
		rule.taint_state = taint_state;
		rule.symbol_name = symbol_name;
		rule.symbol_flag = symbol_flag;
		rule.enforces_lhs = !symbol_flag;
		data[loc].push_back(rule);
	}


	/*******************************************************************\

  Function: taint_datat::check_rules

    Inputs: Takes the locs of the program.

   Outputs: Returns true in case a rule is invalid

   Purpose: Checks specified taint introduction rules.

  \*******************************************************************/

	inline bool check_rules(
			locst &locs,
			std::ostream & warning,
			taint_enginet &taint_engine) {

	  for(auto rule_vector : data) {
	    for(auto rule: rule_vector.second) {
	      // Check whether the rule is outside program.
	      if(rule.loc >= locs.size()) {

	        //Not a meaningful rule.
	        warning << "Following rule outside program scope:" << "\n";
	        rule.output(taint_engine, warning);
	        warning << "\n";
	        return true;
	      }

	      // Retrieve instruction.
	      goto_programt::const_targett inst = locs.loc_vector[rule.loc].target;

	      // Check that the instruction is supported.
	      if (!inst->is_assign() && !inst->is_decl() && !inst->is_function_call()){

	        warning << "Following rule refers to an unsupported op (" << inst->type << ")\n";
	        rule.output(taint_engine, warning);
	        warning << "\n";
	        return true;

	      }else if (inst->is_function_call()){

	        // Need to check that the left hand side of the function call exists.
	        code_function_callt function_call = to_code_function_call(inst->code);

	        if(function_call.lhs().is_nil()) {
	          warning << "Following rule refers to function call with nil left-hand side:" << "\n";
	          rule.output(taint_engine, warning);
	          warning << "\n";
	          return true;
	        }
	      }
	    }
	  }

		// No errors found.
		return false;
	}


	/*******************************************************************\

  Function: taint_datat::check_rules

    Inputs: Takes Output stream.

   Outputs: Returns nothing.

   Purpose: Outputs to stream the specified rules.

  \*******************************************************************/

	inline void output(std::ostream &out, taint_enginet &taint_engine) const {
	  int i = 0;
	  for(auto rule_vector : data) {
	    for(auto rule: rule_vector.second) {
	      out << ++i << ": ";
	      rule.output(taint_engine, out);
	      out << "\n";
	      // TODO EOM?
	    }
	  }
	}
};

#endif /* PATH_SYMEX_PATH_SYMEX_TAINT_DATA_H_ */
