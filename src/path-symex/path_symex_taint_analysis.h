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
// 0 always represents top (UT)
typedef unsigned short taintt;

/**
 * 	Interface for taint analysis, which differ in their considered domains.
 */
class path_symex_taint_analysist{

public:

	virtual ~path_symex_taint_analysist(){};

	// Returns the maximal/top element of the lattice.
	virtual taintt get_max_elem() = 0;

	// Returns the minimal/lowest element of the lattice.
	virtual taintt get_min_elem() = 0;

	// Given two taint types, the meet of the two is returned.
	virtual taintt meet(irep_idt id, taintt taint1, taintt taint2) throw() = 0;

	// Returns the name of the taint analysis engine
	virtual std::string get_taint_analysis_name() = 0;

	// Returns the taint type corresponding to the string
	virtual taintt parse_taint(std::string taint_name) throw() = 0;

	// Returns the name of a given taint type
	virtual std::string get_taint_name(taintt taint) throw() = 0;
};

class path_symex_no_taint_analysist: public path_symex_taint_analysist
{
public:
  inline taintt get_max_elem() { return 0; }
  inline taintt meet(irep_idt id, taintt taint1, taintt taint2) { return 0; }
  inline std::string get_taint_analysis_name() { return "None"; }
  inline taintt parse_taint(std::string taint_name) { return 0; }
  inline std::string get_taint_name(taintt taint) { return ""; }
};

class path_symex_simple_taint_analysist: public path_symex_taint_analysist
{
public:

	// For simple taint analysis, we solely consider two taint types.
	static const taintt UNTAINTED = 0;
	static const taintt TAINTED = 1;

	~path_symex_simple_taint_analysist() {}

	inline taintt get_max_elem(){
		return UNTAINTED;
	}

	taintt meet(irep_idt id, taintt taint1, taintt taint2) {

		std::cout << "*** MEET HAS BEEN CALLED\n";
		std::cout << "Parameters: 1: " << get_taint_name(taint1) << "  -- 2: " << get_taint_name(taint2) << "\n";

		// Perform range checks on passed taint types.
		if (taint1 > get_max_elem() || taint1 < get_min_elem()){
			throw "First taint type  passed to meet function is invalid.";
		}

		if (taint2 > get_max_elem() || taint2 < get_min_elem()){
			throw "Second taint type passed to meet function is invalid.";
		}

		// If either one of the taint states is tainted, then the result is tainted.
		return (taint1 == TAINTED || taint2 == TAINTED)
				? TAINTED : UNTAINTED;
	}

	inline std::string get_taint_analysis_name() { return "Simple taint domain."; }

	inline taintt parse_taint(std::string taint_name) {
		/* Parse from strings -> taint types */
		if(taint_name == "untainted")
			return UNTAINTED;
		if(taint_name == "tainted")
			return TAINTED;
		throw "Taint type not recognised";
	}

	inline std::string get_taint_name(taintt taint) {
		/* Parse from taint -> strings types */
		switch(taint) {
		case UNTAINTED:
			return "untainted";
		case TAINTED:
			return "tainted";
		}
		throw "Taint type not recognised";
	}
};

#endif
