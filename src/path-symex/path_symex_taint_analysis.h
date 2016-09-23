/*******************************************************************\

Module: We define taint lattices to represent taint types (the domain). In order to implement analysis using a new taint domain,
one needs to implement the path_symex_taint_analysist interface. This file contains defines the interface class, as well
as its implementors.

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

// We represent positions in a taint lattice as unsigned short value.
typedef unsigned short taintt;

/**
 * 	Interface for taint analysis, which differ in their considered domains.
 */
class path_symex_taint_analysist{

public:

	// Returns the maximal element of the lattice.
	virtual taintt get_top_position() = 0;

	// Given two taint states, the meet of the two is returned.
	virtual taintt meet(irep_idt id, taintt taint_state1, taintt taint_state2) = 0;

	// Returns the name of the taint analysis
	virtual std::string get_taint_analysis_name(taintt taint_state) = 0;

	// Returns the taint state of
	virtual taintt parse_taint_state(std::string input) = 0;

	// Returns the name of a given taint state
	virtual std::string get_taint_state_name(taintt input) = 0;


};

class path_symex_simple_taint_analysist: public path_symex_taint_analysist
{
public:

	// For simple taint analysis, we solely consider two taint types.
	static const taintt UNTAINTED = 0;
	static const taintt TAINTED = 1;

	inline taintt get_top_position(){
		return UNTAINTED;
	}


   taintt meet(irep_idt id, taintt taint_one, taintt taint_two) {

    std::cout << "*** MEET HAS BEEN CALLED\n";
    std::cout << "Parameters: 1: " << get_taint_state_name(taint_one) << "  -- 2: " << get_taint_state_name(taint_two) << "\n";

    // If either one of the taint states is tainted, then the result is tainted.
    return (taint_one == TAINTED || taint_two == TAINTED)
        ? TAINTED : UNTAINTED;
  }

  inline taintt parse_taint_state(std::string input) {
    /* Parse from strings -> taint types */
    if(input == "untainted")
      return UNTAINTED;
    if(input == "tainted")
      return TAINTED;
    throw "Taint type not recognised";
  }

  inline std::string get_taint_state_name(taintt input) {
    /* Parse from taint -> strings types */
    switch(input) {
    case UNTAINTED:
      return "untainted";
    case TAINTED:
      return "tainted";
    }
    throw "Taint type not recognised";
  }

  inline std::string get_taint_analysis_name() { return "Simple taint domain."; }
};

#endif
